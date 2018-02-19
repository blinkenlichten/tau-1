#include "t1_anything_goes/scoped_fn.hpp"
#include "t1net/metrics_send.h"
#include "fty_proto.h"
#include <string>

bool t1_send_metric
(mlm_client_t *producer,
 const char* endpoint,//"ipc://@/malamute"
 metric_param_t param,
 metric_func_err_t p_error_fn)
{

    // TTL
    uint32_t ttl = param.time_to_live;

    // topic
    std::string topic_buff = name_producer(param.type);
    topic_buff += '@';
    topic_buff += param.name;

    std::string error_msg;

    auto fn_on_return = [p_error_fn, &error_msg]()
    {
        if (p_error_fn.on_error)
            p_error_fn.on_error(error_msg.size(), error_msg.empty()? nullptr : error_msg.c_str(), p_error_fn.arg_data);
    };
    ScopedInvoke<decltype(fn_on_return)> err_reporter_invoke(std::move(fn_on_return)); (void)err_reporter_invoke;

    if ( -1 == mlm_client_connect(producer, endpoint, g_metrics_timeout, g_mlm_metrics_address))
    {
        error_msg = "mlm_client_connect (endpoint = ";
        error_msg += endpoint;
        error_msg += ", timeout = 5000, address = metric_generator) failed";
        return false;
    }

    if ( -1 == mlm_client_set_producer(producer, name_producer(param.type)))
    {
        error_msg = "mlm_client_set_producer(stream = 'METRICS') failed.";
        return false;
    }

    zmsg_t *msg = fty_proto_encode_metric (
            NULL,       // aux
            time (NULL),// time
            ttl,        // TTL
            name_producer(param.type),    // type
            param.name,    // name
            param.value,    // value
            param.unit     // unit
            );
    if (!msg)
    {
        error_msg = "zmsg_t* failed\n";
        return false;
    }

    if (-1 == mlm_client_send (producer, topic_buff.c_str(), &msg))
    {
        error_msg = "mlm_client_send failed: subject = ";
        error_msg += topic_buff;
        zmsg_destroy(&msg);
        return false;
    }
    return true;
}
