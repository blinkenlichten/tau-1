#include <fty_proto.h>
#include <malamute.h>
#include "czmq.h"

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

enum METRICS_DATA_PRODUCER
{ METRICS, ALERT, ACTION, LOG, NONE_LAST};

static const char* g_data_producers_names[] = {"METRICS", "ALERT", "ACTION", "LOG", "NONE"};
static const char* name_producer(METRICS_DATA_PRODUCER item) { return g_data_producers_names[(int)item];}
//-----------------------------------------------------------------------------
struct metric_func_err_t
{
    void (*on_error)(size_t msgLen, const char* message, void* arg_data);
    void* arg_data;
#ifdef __cplusplus
    metric_func_err_t() : on_error(nullptr), arg_data(nullptr)
    { }
#endif
};
//-----------------------------------------------------------------------------
static const char* g_mlm_metrics_address = "metrics_events";
static const int g_metrics_timeout = 5000;
//-----------------------------------------------------------------------------
struct t1_metrics_ctx;

/** Function pointer that must point to a function
 * performing connection of a a source/producer of the metrics data to the Malamute data broker.
 *
 * @param ctx: passed by value to make sure there won't be shared state of any kind,
 * just the variables for this particular moment of connection.
 *
 @return connection client pointer, caller cares about invoking mlm_client_destroy((mlm_client_t*)ptr);
*/
typedef mlm_client_t* (*fn_mlm_connect_t)
    (t1_metrics_ctx ctx, const char* endpoint, metric_func_err_t p_error_fn);

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
typedef bool (*fn_send_metrics_t)
    (t1_metrics_ctx ctx, mlm_client_t *client,
     zmsg_t** p_data_zmsg, metric_func_err_t p_error_fn);

/** Receive metric data.
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
typedef zmsg_t* (*fn_receive_metrics_t)(t1_metrics_ctx ctx, mlm_client_t* client);
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct t1_metrics_ctx
{
    const char* mlm_address;
    int timeout;
    METRICS_DATA_PRODUCER data_type;
    fn_mlm_connect_t pfn_connect_emitter;
    fn_mlm_connect_t pfn_connect_consumer;
    fn_send_metrics_t pfn_send;
    fn_receive_metrics_t pfn_receive;
    metric_func_err_t fn_error/*reports errors in this context*/, fn_log/*info logs for the context*/;
    void* arbitrary;
#ifdef __cplusplus
    t1_metrics_ctx() { ::memset(this, 0x00, sizeof(t1_metrics_ctx)); }
#endif
};

t1_metrics_ctx t1_init_metrics(METRICS_DATA_PRODUCER type);

/// deallocate or simply 0-fill structure's pointer. @return 0-filled structure.
t1_metrics_ctx/*don't ignore it*/ t1_destroy_metrics(t1_metrics_ctx context);
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}//extern "C"
#endif //__cplusplus

#ifdef __cplusplus
#include <memory>

extern void mlm_client_destroy(mlm_client_t*);


namespace Tau1
{
struct MlmClientDeleter { void operator()(mlm_client_t* cl) { mlm_client_destroy(cl);} };
struct ZmsgDeleter { void operator()(zmsg_t* msg) { zmsg_destroy(&msg); } };

typedef std::unique_ptr<mlm_client_t,MlmClientDeleter> MlmClientUPtr;
typedef std::unique_ptr<zmsg_t, ZmsgDeleter> ZmsgUPtr;


class IMetricsRW
{
public:
    enum class MetricsConnType { PRODUCER, CONSUMER};

    static MlmClientUPtr connect(t1_metrics_ctx ctx, MetricsConnType type,
                                 const char* endpoint, metric_func_err_t p_error_fn);

    IMetricsRW(MlmClientUPtr&& connection, MetricsConnType type)
        : m_mlm_connection(std::move(connection)), m_metrics_connection_type(type)
    { }
    virtual ~IMetricsRW() = default;

    virtual void clear() { m_mlm_connection.reset(nullptr); }
    bool empty() const { return nullptr == m_mlm_connection; }

    virtual void tx(t1_metrics_ctx ctx, ZmsgUPtr&& msg, metric_func_err_t fn_err = metric_func_err_t(/*none*/));
    virtual ZmsgUPtr rx(t1_metrics_ctx ctx, metric_func_err_t fn_err = metric_func_err_t(/*none*/));

    MlmClientUPtr m_mlm_connection;
    MetricsConnType m_metrics_connection_type = MetricsConnType::CONSUMER;
};

}//namespace Tau1

#endif //__cplusplus

