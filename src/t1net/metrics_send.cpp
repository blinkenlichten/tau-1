#include "t1_anything_goes/scoped_fn.hpp"
#include "t1net/metrics_send.h"
#include "fty_proto.h"
#include <string>

bool t1_send_metrics
(mlm_client_t *producer,
 metric_param_t param,
 metric_func_err_t p_error_fn)
{
    std::string error_msg;
    auto fn_if_error = [&error_msg, p_error_fn]()
    {
        if ( !error_msg.empty() && p_error_fn.on_error )
        {
            p_error_fn.on_error(error_msg.size(), error_msg.data(), nullptr);
        }
    };
    ScopedInvoke<decltype(fn_if_error)> err_invoker(std::move(fn_if_error));

    if (nullptr == producer)
    {
        error_msg = "not connected yet.";
        return false;
    }
    // TTL
    uint32_t ttl = param.time_to_live;

    // topic
    std::string topic_buff = name_producer(DATA_PRODUCERS::METRICS);
    topic_buff += '@';
    topic_buff += param.name;


    zmsg_t *msg = fty_proto_encode_metric (
            NULL,       // aux
            time (NULL),// time
            ttl,        // TTL
            name_producer(DATA_PRODUCERS::METRICS),    // type
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
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template<class FNProducerOrConsumerSetter>
Tau1::MlmClientUPtr __t1_metrics_connect(const char* endpoint, int timeout,
                                         const char* client_type,
                                         FNProducerOrConsumerSetter&& setter)
{
    Tau1::MlmClientUPtr client(mlm_client_new());
    if ( 0 > mlm_client_connect(client.get(), endpoint, timeout, client_type))
        return Tau1::MlmClientUPtr();
    FNProducerOrConsumerSetter _set = std::move(setter);
    if ( 0 > _set(client.get()) )
    {
        return Tau1::MlmClientUPtr();
    }
    return client;
}

mlm_client_t* t1_metrics_emitter_connect(const char* endpoint, int timeout, metric_func_err_t p_error_fn)
{
    auto setter = [](mlm_client_t* client) -> int
    { return mlm_client_set_producer(client, name_producer(DATA_PRODUCERS::METRICS));};

    Tau1::MlmClientUPtr producer = __t1_metrics_connect<decltype(setter)>
            (endpoint, timeout, "metrics_producer", std::move(setter));
    if (producer.empty() && p_error_fn.on_error)
    {
        std::string error_msg = "t1_metrics_emitter_connect (endpoint = ";
        error_msg += endpoint;
        error_msg += ", timeout = 5000, address = metrics_producer) failed";
        p_error_fn.on_error(error_msg.size(), error_msg.empty()? nullptr : error_msg.c_str(), p_error_fn.arg_data);
    }
    return producer.release();
}

mlm_client_t* t1_metrics_receiver_connect(const char* endpoint, int timeout, metric_func_err_t p_error_fn)
{
    auto setter = [](mlm_client_t* client) -> int
    { return mlm_client_set_consumer(client, name_producer(DATA_PRODUCERS::METRICS), ".*");};
    Tau1::MlmClientUPtr receiver = __t1_metrics_connect<decltype(setter)>
            (endpoint, timeout, "metrics_consumer", std::move(setter));
    if (receiver.empty() && p_error_fn.on_error)
    {
        std::string error_msg = "t1_metrics_receiver_connect (endpoint = ";
        error_msg += endpoint;
        error_msg += ", timeout = 5000, address = metrics_consumer) failed";
        p_error_fn.on_error(error_msg.size(), error_msg.empty()? nullptr : error_msg.c_str(), p_error_fn.arg_data);
    }
    return receiver.release();
}
//-----------------------------------------------------------------------------
fty_proto_t* t1_receive_metrics(mlm_client_t* consumer)
{
    zmsg_t* msg = mlm_client_recv(consumer);
    return fty_proto_decode(&msg);
}

metric_param_t t1_metrics_decode(fty_proto_t* msg)
{
    metric_param_t param;
    param.value = fty_proto_value(msg);
    param.unit = fty_proto_unit(msg);
    param.name = fty_proto_name(msg);
    return param;
}
//-----------------------------------------------------------------------------
namespace Tau1 {


MlmClientUPtr IMetricsRW::connect(IMetricsRW::MetricsConnType type, const char* endpoint, int timeout, metric_func_err_t p_error_fn)
{
    return MlmClientUPtr( IMetricsRW::MetricsConnType::PRODUCER == type?
                              t1_metrics_emitter_connect(endpoint, timeout, p_error_fn)
                            :
                              t1_metrics_receiver_connect(endpoint, timeout, p_error_fn));
}

void IMetricsRW::tx(metric_param_t param, metric_func_err_t fn_err)
{
    t1_send_metrics(m_mlm_connection.get(), param, fn_err);
}

ZmsgUPtr IMetricsRW::rx(metric_func_err_t fn_err)
{
    (void)fn_err;
    return ZmsgUPtr(mlm_client_recv(m_mlm_connection.get()));
}

}//namespace Tau1
