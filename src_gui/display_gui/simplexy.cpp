#include "simplexy.h"
#include <list>
#include <chrono>
#include <cmath>
#include <thread>
#include <mutex>
#include <atomic>

class XYSharedState
{
public:
    Tau1::MetricsContext ctx;
    const char* endpoint = "ipc://@/malamute";

    XYSharedState()
    {
        quit_flag.store(false);

        reader_thread = std::thread([this]()
        {
            Tau1::MlmClientUPtr ptr_ = Tau1::IMetricsRW::connect
              (ctx, Tau1::IMetricsRW::MetricsConnType::CONSUMER, endpoint);
            reader = std::move(Tau1::IMetricsRW(std::move(ptr_), Tau1::IMetricsRW::MetricsConnType::CONSUMER));

            for ( ; !quit_flag.load(); )
            {
                Tau1::ZmsgUPtr msg = reader.rx(ctx);

                for ( zframe_t* pframe = zmsg_first(msg.get()); pframe; pframe = zmsg_next(msg.get()))
                {
                    QVector<QPointF> arr;
                    // copy data of (Zframe_t) into QVector<QPointF>.
                    arr.fill(QPointF(.0f,.0f), (int)zframe_size(pframe));
                    ::memcpy(arr.data(), zframe_data(pframe), arr.size() * sizeof(QPointF));

                    //push new array into the shared object for rendering
                    std::unique_lock<std::mutex> lk(data_mutex);
                    data_bytes_cnt += arr.size();
                    if (data_bytes_cnt >= max_data_bytes)
                    {
                        for (size_t cleared_cnt = data.front().size();
                             cleared_cnt < max_data_bytes / 4;
                             cleared_cnt += data.front().size())
                        {/*pop while down to 1/4 of the max buffer size*/
                            data.pop_front();
                        }
                    }
                    data.emplace_back(std::move(arr));
                }
            }
        });
    }
    virtual ~XYSharedState()
    {
        reader_thread.join();
    }
    template<class FnObject>
    void enter_critical_section(FnObject&& fn_object)
    {
        std::unique_lock<std::mutex> lk(data_mutex);
        FnObject fn = std::move(fn_object);
        fn.operator()();
    }
    std::list<QVector<QPointF> >&& move_data()
    {
        return std::move(data);
    }

    std::atomic<bool> quit_flag;
    static constexpr size_t max_data_bytes = 10 * 1024 * 1024;
private:
    Tau1::IMetricsRW reader;
    std::thread reader_thread;
    std::mutex data_mutex;
    std::list<QVector<QPointF> > data;
    size_t data_bytes_cnt = 0;
} g_xycomm;

struct DummyWriter
{
    Tau1::IMetricsRW writer;
    DummyWriter()
    {
        std::thread writer_thread([&g_xycomm, this]()
        {
            Tau1::MlmClientUPtr ptr_ = Tau1::IMetricsRW::connect
              (g_xycomm.ctx, Tau1::IMetricsRW::MetricsConnType::CONSUMER, g_xycomm.endpoint);
            writer = std::move(Tau1::IMetricsRW(std::move(ptr_), Tau1::IMetricsRW::MetricsConnType::PRODUCER));
            QVector<QPointF> points_buf;
            points_buf.reserve(1024);
            size_t cnt = 0;
            for ( ; !g_xycomm.quit_flag.load(); )
            {
                points_buf.resize(1024);
                for ( size_t i = 0; i < points_buf.size(); ++i )
                {
                    points_buf[i].x = (float)i / 300.0f;
                    points_buf[i].y = ::sin(points_buf[i].x);
                }
                cnt += points_buf.size();
                Tau1::ZmsgUPtr uzmsg(zmsg_new());
                zmsg_addmem(uzmsg.get(), (void*)points_buf.data(), points_buf.size() * sizeof(QPointF));

                writer.tx(g_xycomm.ctx, std::move(uzmsg));
            }

        });
        writer_thread.detach();
    }
}g_wdummy;

SimpleXY::SimpleXY() : QLineSeries(parent)
{
    plot_vector.fill(QPointF(.0f, .0f), 1024);
}

SimpleXY::~SimpleXY()
{
    g_xycomm.quit_flag = true;
}

void SimpleXY::timerEvent(QTimerEvent *event)
{
    decltype(list) local_list;
    auto fn_move_data = [&local_list](){ local_list = std::move(g_xycomm.move_data());};
    g_xycomm.enter_critical_section<decltype(fn_move_data)>(std::move(fn_move_data));
    if ( local_list.empty() )
        return;
    for ( ; !local_list.empty(); )
        list.emplace_back(std::move(local_list.front()));

    size_t items_cnt = 0;
    std::swap(plot_vector, list.front());
    list.pop_front();
    this->replace(plot_vector);

    QLineSeries::timerEvent(event);
}

void SimpleXY::paintEvent(QEvent* event)
{
    QLineSeries::paintEvent(event);
}
