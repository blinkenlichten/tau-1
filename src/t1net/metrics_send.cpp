#include "t1_anything_goes/scoped_fn.hpp"
#include "fty_proto.h"

bool t1_encode_metric
(mlm_client_t *producer,
 metric_func_err_t* p_error_fn,
 const char* endpoint,//"ipc://@/malamute"
 metric_param_t param)
{

    // TTL
    uint32_t ttl = param.metric_time_to_live;

    // topic
    std::string topic_buff = name_producer(param.metric_type);
    topic_buff += '@';
    topic_buff += param.metric_name;

    std::string error_msg;
    ON_SCOPE_EXIT(_report, if (p_error_fn) p_error_fn(error_msg.size(), error_msg.c_str(), nullptr); )

    if ( -1 == mlm_client_connect (producer, param.endpoint, 5000, "metric_generator"))
    {
        error_msg = "mlm_client_connect (endpoint = ';
        error_msg += param.endpoint;
        error_msg += "', timeout = '5000', address = 'metric_generator') failed";
        return false;
    }

    if ( -1 == mlm_client_set_producer (producer, name_producer(param.metric_type)))
    {
        error_msg = "mlm_client_set_producer (stream = 'METRICS') failed.";
        return false;
    }

    zmsg_t *msg = fty_proto_encode_metric (
            NULL,       // aux
            time (NULL),// time
            ttl,        // TTL
            name_producer(param.metric_type),    // type
            argv[2],    // name
            argv[3],    // value
            argv[4]     // unit
            );

    if (!msg || -1 == mlm_client_send (producer, topic_buff.c_str(), &msg))
    {
        error_msg = "mlm_client_send failed: subject = ";
        error_msg += topic_buff;
        return false;
    }
    return true;
}
