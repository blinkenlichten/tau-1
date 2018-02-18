#include <fty_proto.h>
#include <malamute.h>

#ifdef __cplusplus
#include <memory>

extern void mlm_client_destroy(mlm_client_t*);
struct MlmClientDeleter { void operator()(mlm_client_t* cl) { mlm_client_destroy(cl);} };

class MlmClientUPtr : public std::unique_ptr<mlm_client_t, MlmClientDeleter>
{ };

extern "C"
{
#endif //__cplusplus
enum DATA_PRODUCERS
{ METRICS, ALERT, ACTION, LOG, NONE_LAST};

static const char* g_data_producers_names[] = {"METRICS", "ALERT", "ACTION", "LOG", "NONE"};
static const char* name_producer(DATA_PRODUCERS item) { return g_data_producers_names[(int)item];}

struct metric_func_err_t
{
    bool success;
    void (*on_error)(size_t msgLen, const char* message, void* arg_data);
    void* arg_data;
#if __cplusplus
    metric_func_err_t() : success(false), on_error(nullptr), arg_data(nullptr)
    { }
#endif
};

struct metric_param_t
{
    DATA_PRODUCERS type;
    const char* name;
    const char* value;
    const char* unit;
    uint32_t time_to_live;
#if __cplusplus
    metric_param_t()
        : type(DATA_PRODUCERS::NONE_LAST), name(nullptr), value(nullptr),
          unit(nullptr), time_to_live(0)
    { }
#endif
};
static const char* g_mlm_metrics_address = "metric_events";
#ifdef __cplusplus
}
#endif //__cplusplus
