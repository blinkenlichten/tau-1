#include "simplexy.h"
#include <list>
#include <chrono>
#include <cmath>
#include <thread>
#include <mutex>
#include <atomic>
//-----------------------------------------------------------------------------
class XYSharedState : public QRunnable
{
public:
    tau1::MetricsContext ctx;
    const char* endpoint = "ipc://@/malamute";

    void run() override
    {
        quit_flag.store(false);

        reader_thread = std::thread([this]()
        {
            tau1::MlmClientUPtr ptr_;
            for ( ; !quit_flag.load() && nullptr == ptr_; )
            {
                ptr_ = tau1::IMetricsRW::connect(ctx, tau1::IMetricsRW::MetricsConnType::CONSUMER, endpoint);
            }

            reader = std::move(tau1::IMetricsRW(std::move(ptr_), tau1::IMetricsRW::MetricsConnType::CONSUMER));

            for ( tau1::ZmsgUPtr msg = reader.rx(ctx); !quit_flag.load(); msg = reader.rx(ctx))
            {
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
    tau1::IMetricsRW reader;
    std::thread reader_thread;
    std::mutex data_mutex;
    std::list<QVector<QPointF> > data;
    size_t data_bytes_cnt = 0;
};
//-----------------------------------------------------------------------------

struct DummyWriter : public QRunnable
{
    tau1::IMetricsRW writer;
    XYSharedState& m_ref;

    DummyWriter(XYSharedState& ref) : QRunnable(), m_ref(ref)
    { }
    void run() override
    {
        std::thread writer_thread([this]()
        {
            tau1::MlmClientUPtr ptr_;
            for ( ; !m_ref.quit_flag.load() && nullptr == ptr_; )
            {
                ptr_ = tau1::IMetricsRW::connect(m_ref.ctx, tau1::IMetricsRW::MetricsConnType::PRODUCER, m_ref.endpoint);
            }
            writer = std::move(tau1::IMetricsRW(std::move(ptr_), tau1::IMetricsRW::MetricsConnType::PRODUCER));
            QVector<QPointF> points_buf;
            points_buf.reserve(1024);
            size_t cnt = 0;
            for ( ; !m_ref.quit_flag.load(); )
            {
                points_buf.resize(1024);
                for ( int i = 0; i < (int)points_buf.size(); ++i )
                {
                    points_buf[i].rx() = (float)i / 300.0f;
                    points_buf[i].ry() = ::sin(points_buf[i].x());
                }
                cnt += points_buf.size();
                tau1::ZmsgUPtr uzmsg(zmsg_new());
                zmsg_addmem(uzmsg.get(), (void*)points_buf.data(), points_buf.size() * sizeof(QPointF));

                writer.tx(m_ref.ctx, uzmsg);
            }

        });
        writer_thread.detach();
    }
};
//-----------------------------------------------------------------------------
SimpleXY::SimpleXY(QObject* parent) : QLineSeries(parent)
{
    auto ptr = new XYSharedState;
    m_reader.reset(ptr);
    m_dummy_writer.reset(static_cast<QRunnable*>(new DummyWriter(*ptr)));
    m_reader->run();
    m_dummy_writer->run();
    plot_vector.fill(QPointF(.0f, .0f), 1024);
}

SimpleXY::~SimpleXY()
{
    dynamic_cast<XYSharedState*>(m_reader.get())->quit_flag = true;
}

void SimpleXY::timerEvent(QTimerEvent *event)
{
    decltype(list) local_list;
    auto p_reader = dynamic_cast<XYSharedState*>(m_reader.get());
    auto fn_move_data = [&local_list,p_reader](){ local_list = std::move(p_reader->move_data());};
    p_reader->enter_critical_section<decltype(fn_move_data)>(std::move(fn_move_data));
    if ( local_list.empty() )
        return;
    for ( ; !local_list.empty(); )
        list.emplace_back(std::move(local_list.front()));

//    size_t items_cnt = 0;
    std::swap(plot_vector, list.front());
    list.pop_front();
    this->replace(plot_vector);

    QLineSeries::timerEvent(event);
}

//-----------------------------------------------------------------------------
