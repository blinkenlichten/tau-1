#include <fty_proto.h>
#include <malamute.h>
#include "czmq.h"

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus
//-----------------------------------------------------------------------------
enum DATA_PRODUCERS
{ METRICS, ALERT, ACTION, LOG, NONE_LAST};
//-----------------------------------------------------------------------------
static const char* g_data_producers_names[] = {"METRICS", "ALERT", "ACTION", "LOG", "NONE"};
static const char* name_producer(DATA_PRODUCERS item) { return g_data_producers_names[(int)item];}
//-----------------------------------------------------------------------------
struct metric_func_err_t
{
    void (*on_error)(size_t msgLen, const char* message, void* arg_data);
    void* arg_data;
#if __cplusplus
    metric_func_err_t() : on_error(nullptr), arg_data(nullptr)
    { }
#endif
};
//-----------------------------------------------------------------------------
// simple metrics data structure.
struct metric_param_t
{
    const char* name;
    const char* value;
    const char* unit;
    uint32_t time_to_live;
#if __cplusplus
    metric_param_t()
        : name(nullptr), value(nullptr), unit(nullptr), time_to_live(0)
    { }
#endif //c++
};
//-----------------------------------------------------------------------------
static const char* g_mlm_metrics_address = "metrics_events";
static const int g_metrics_timeout = 5000;
//-----------------------------------------------------------------------------
/** Send simple metric data.
 Connect a source/producer of the metrics data.
 @return connection client pointer, caller cares about invoking mlm_client_destroy((mlm_client_t*)ptr);
*/
mlm_client_t* t1_metrics_emitter_connect(const char* endpoint, int timeout, metric_func_err_t p_error_fn);

/** Send simple metric data.
 *
 * @param producer: an existing malamute broker connection, caller cares about init/deinit routines:
 * @verbatim
    mlm_client_t *client = mlm_client_new ();
    t1_send_metric(client, ...);
    mlm_client_destroy(client);
   @endverbatim

   @param endpoint: IPC endpoint, usually "ipc://@/malamute".
   @param param: simple send data for metrics;
   @param p_error_fn: optional on_error() function pointer; here p_error_fn.arg_data is always NULL;
 */
bool t1_send_metrics
(mlm_client_t *producer,
 const char* endpoint,//"ipc://@/malamute"
 metric_param_t param,
 metric_func_err_t p_error_fn);
//-----------------------------------------------------------------------------
/** Send simple metric data.
 Connect to a source of metrics data.
 @return consumer connection client, caller cares about invoking mlm_client_destroy((mlm_client_t*)ptr);
*/
mlm_client_t* t1_metrics_receiver_connect(const char* endpoint, int timeout, metric_func_err_t p_error_fn);

/** Send simple metric data.
 * @return: (fty_proto_t*) decoded message pointer that caller MUST DESTROY
 * via fty_proto_destroy(pointer) after consuming it; the value MAY BE NULL.

 * @verbatim
    mlm_client_t *client = t1_metrics_receive_connect(const char* endpoint, int timeout);
    while(!zsys_interrupted)
    {
        fty_proto_t* msg = t1_receive_metrics(client);
        // parse (msg)
        fty_proto_destroy(&msg);
    }
    mlm_client_destroy(client);
   @endverbatim

   @param endpoint: IPC endpoint, usually "ipc://@/malamute".
 */
fty_proto_t* t1_receive_metrics(mlm_client_t* consumer);
//read the parameters: {name, unit, value}
metric_param_t t1_metrics_decode(fty_proto_t* msg);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}//extern "C"
#endif //__cplusplus

#ifdef __cplusplus
#include <memory>
#include <deque>
#include "t1_anything_goes/scoped_fn.hpp"

extern void mlm_client_destroy(mlm_client_t*);


namespace Tau1
{
struct MlmClientDeleter { void operator()(mlm_client_t* cl) { if (cl) mlm_client_destroy(cl);} };
struct ZmsgDeleter { void operator()(zmsg_t* msg) { auto temp = msg; if (temp) zmsg_destroy(&temp); } };
struct FtyProtoDeleter { void operator()(fty_proto_t* msg) { auto temp = msg; if(temp) fty_proto_destroy(&temp);} };

typedef TDisposableUniquePointer<mlm_client_t*,MlmClientDeleter> MlmClientUPtr;

typedef TDisposableUniquePointer<zmsg_t*, ZmsgDeleter> ZmsgUPtr;

class FtyProtoUptr : public TDisposableUniquePointer<fty_proto_t*, FtyProtoDeleter>
{
public:
    using __TBase::__TBase;
    //constructor that will do the decoding into FTY proto and destroy incoming message.
    FtyProtoUptr(ZmsgUPtr&& msg)
    {
        ZmsgUPtr local = std::move(msg);
        zmsg_t* raw = local.release();
        this->reset(fty_proto_decode(&raw)); //destroys (raw)'s allocation
    }
};

class IMetricsRW
{
public:
    enum class MetricsConnType { PRODUCER, CONSUMER};

    static MlmClientUPtr connect(MetricsConnType type, const char* endpoint, int timeout, metric_func_err_t p_error_fn);

    IMetricsRW(MlmClientUPtr&& connection, MetricsConnType type)
        : m_mlm_connection(std::move(connection)), m_metrics_connection_type(type)
    { }
    virtual ~IMetricsRW() = default;

    virtual void clear() { m_mlm_connection.reset(nullptr); }
    bool empty() const { return m_mlm_connection.empty(); }

    virtual void tx(metric_param_t param, metric_func_err_t fn_err = metric_func_err_t(/*none*/));
    virtual ZmsgUPtr rx(metric_func_err_t fn_err = metric_func_err_t(/*none*/));

    MlmClientUPtr m_mlm_connection;
    MetricsConnType m_metrics_connection_type = MetricsConnType::CONSUMER;
};

}//namespace Tau1

#endif //__cplusplus

