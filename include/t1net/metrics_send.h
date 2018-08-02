
#include <malamute.h>
#include "czmq.h"

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

enum METRICS_DATA_PRODUCER
{ METRICS, ALERT, ACTION, LOG, NONE_LAST};

static const char* g_data_producers_names[] = {"METRICS", "ALERT", "ACTION", "LOG", "NONE"};
const char* name_producer(METRICS_DATA_PRODUCER item);
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
 * @return: zmsg_t* that caller CARES TO DESTRUCT by zmsg_destroy(zmsg_t**), can be nullptr.
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
    t1_metrics_ctx() { ::memset(this, 0x00, sizeof(*this)); }
#endif
};

t1_metrics_ctx t1_init_metrics(METRICS_DATA_PRODUCER type);
/// Destroy connections & free memory.
/// return 0-filled structure.
t1_metrics_ctx/*don't ignore it*/ t1_destroy_metrics(t1_metrics_ctx context);
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}//extern "C"
#endif //__cplusplus
