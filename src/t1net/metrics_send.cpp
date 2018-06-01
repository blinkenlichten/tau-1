#include "t1_anything_goes/scoped_fn.hpp"
#include "t1net/metrics_send.h"
#include <string>
#include <array>

const char* name_producer(METRICS_DATA_PRODUCER item)
{ return g_data_producers_names[(int)item]; }
//-----------------------------------------------------------------------------
namespace Tau1 {

static bool send(t1_metrics_ctx ctx, mlm_client_t *producer,
                 zmsg_t** p_data_zmsg, metric_func_err_t p_error_fn)
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

    if (!p_data_zmsg)
    {
        error_msg = "zmsg_t* failed\n";
        return false;
    }
    // topic
    std::array<char, 128> temp_arr; temp_arr.fill(0x00);
    snprintf(temp_arr.data(), temp_arr.size(), "%s@emitter", name_producer(METRICS_DATA_PRODUCER::METRICS));

    if (-1 == mlm_client_send (producer, temp_arr.data(), p_data_zmsg))
    {
        error_msg = "mlm_client_send failed: subject = ";
        error_msg += temp_arr.data();
        zmsg_destroy(p_data_zmsg);
        return false;
    }
    return true;
}

static zmsg_t* receive(t1_metrics_ctx ctx, mlm_client_t* client)
{
    (void)ctx; return mlm_client_recv(client);
}

template<class FNProducerOrConsumerSetter>
Tau1::MlmClientUPtr __t1_metrics_connect(const char* endpoint, int timeout,
                                         const char* chan_address,
                                         FNProducerOrConsumerSetter&& setter)
{
    Tau1::MlmClientUPtr client(mlm_client_new());
    if ( 0 > mlm_client_connect(client.get(), endpoint, timeout, chan_address))
        return Tau1::MlmClientUPtr();
    FNProducerOrConsumerSetter _set = std::move(setter);
    if ( 0 > _set(client.get()) )
    {
        return Tau1::MlmClientUPtr();
    }
    return client;
}

static mlm_client_t* connect_emitter
(t1_metrics_ctx ctx, const char* endpoint, metric_func_err_t p_error_fn)
{
    auto setter = [](mlm_client_t* client) -> int
    { return mlm_client_set_producer(client, name_producer(METRICS_DATA_PRODUCER::METRICS));};

    Tau1::MlmClientUPtr producer = __t1_metrics_connect<decltype(setter)>
            (endpoint, ctx.timeout, "metrics", std::move(setter));
    if (nullptr == producer && p_error_fn.on_error)
    {
        std::array<char, 256> error_msg;
        int nwritten = snprintf(error_msg.data(), error_msg.size(), "t1_metrics_emitter_connect (endpoint = %s) failed.", endpoint);
        p_error_fn.on_error((size_t)nwritten, error_msg.data(), p_error_fn.arg_data);
    }
    return producer.release();
}

static mlm_client_t* connect_receiver(t1_metrics_ctx ctx, const char* endpoint, metric_func_err_t p_error_fn)
{
    auto setter = [](mlm_client_t* client) -> int
    { return mlm_client_set_consumer(client, name_producer(METRICS_DATA_PRODUCER::METRICS), "*");};

    Tau1::MlmClientUPtr receiver = __t1_metrics_connect<decltype(setter)>
            (endpoint, ctx.timeout, "metrics", std::move(setter));

    if (nullptr == receiver && p_error_fn.on_error)
    {
        std::array<char, 256> error_msg;
        int nwritten = snprintf(error_msg.data(), error_msg.size(), "t1_metrics_receiver_connect (endpoint = %s) failed.", endpoint);
        p_error_fn.on_error(nwritten, error_msg.data(), p_error_fn.arg_data);
    }
    return receiver.release();
}

MlmClientUPtr IMetricsRW::connect(t1_metrics_ctx ctx, IMetricsRW::MetricsConnType type,
                                  const char* endpoint, metric_func_err_t p_error_fn)
{
    return MlmClientUPtr( IMetricsRW::MetricsConnType::PRODUCER == type?
                              ctx.pfn_connect_emitter(ctx, endpoint, p_error_fn)
                            :
                              ctx.pfn_connect_consumer(ctx, endpoint, p_error_fn));
}

void IMetricsRW::tx(t1_metrics_ctx ctx, ZmsgUPtr& msg, metric_func_err_t fn_err)
{
    zmsg_t* released = msg.release();
    ctx.pfn_send(ctx, m_mlm_connection.get(), &released, fn_err);
}

ZmsgUPtr IMetricsRW::rx(t1_metrics_ctx ctx, metric_func_err_t fn_err)
{
    (void)fn_err;
    return ZmsgUPtr( ctx.pfn_receive(ctx, m_mlm_connection.get()) );
}

}//namespace Tau1



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
t1_metrics_ctx t1_init_metrics(METRICS_DATA_PRODUCER type)
{
    t1_metrics_ctx _c;
    _c.mlm_address = g_mlm_metrics_address;
    _c.timeout = g_metrics_timeout;
    _c.data_type = type;
    _c.pfn_connect_emitter = &Tau1::connect_emitter;
    _c.pfn_connect_consumer = &Tau1::connect_receiver;
    _c.pfn_send = &Tau1::send;
    _c.pfn_receive = &Tau1::receive;
    _c.fn_error = metric_func_err_t();
    _c.fn_log = metric_func_err_t();
    _c.arbitrary = nullptr;
    return _c;
}

t1_metrics_ctx t1_destroy_metrics(t1_metrics_ctx context)
{
    //does nothing but returning 0-filled structure, there's nothing to release.
    ::memset(&context, 0x00, sizeof(t1_metrics_ctx));
    return context;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
