#include <t1_anything_goes/scoped_fn.hpp>
#include <t1net/metrics_send.h>
#include <t1net/log.h>
#include <string>
#include <array>

const char* name_producer(METRICS_DATA_PRODUCER item)
{ return g_data_producers_names[(int)item]; }
//-----------------------------------------------------------------------------
namespace tau1 {

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
    scoped_invoke<decltype(fn_if_error)> err_invoker(std::move(fn_if_error));

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

static mlm_client_t* connect_emitter_or_receiver
(bool is_listener, t1_metrics_ctx ctx, const char* endpoint, metric_func_err_t p_error_fn)
{
    std::array<char, 256> error_msg;
    auto fn_quick_err_log = [&error_msg, p_error_fn, endpoint](const char* what)
    {
        log_write(e_severity::s_error, what);
        int nwritten = snprintf(error_msg.data(), error_msg.size(), "connect_emitter(): %s on endpoint %s", what, endpoint);
        p_error_fn.on_error((size_t)nwritten, error_msg.data(), p_error_fn.arg_data);
    };

    mlm_client_t* client_ptr = mlm_client_new();
    if (!client_ptr)
    {
        fn_quick_err_log("mlm_client_new() failed.");
        return nullptr;
    }
    int result = mlm_client_connect(client_ptr, endpoint, ctx.timeout, "METRICS");
    if (result < 0)
    {
        fn_quick_err_log("mlm_client_connect() failed.");
        mlm_client_destroy(&client_ptr);
    }
    if (is_listener)
        mlm_client_set_consumer(client_ptr, "METRICS", "*");
    else
        mlm_client_set_producer(client_ptr, "METRICS");
    return client_ptr;
}

mlm_client_t* connect_emitter(t1_metrics_ctx ctx, const char* endpoint, metric_func_err_t p_error_fn)
{
    return connect_emitter_or_receiver(false/*not listener*/, ctx, endpoint, p_error_fn);
}

mlm_client_t* connect_receiver(t1_metrics_ctx ctx, const char* endpoint, metric_func_err_t p_error_fn)
{
    return connect_emitter_or_receiver(true, ctx, endpoint, p_error_fn);
}

}//namespace tau1



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
t1_metrics_ctx t1_init_metrics(METRICS_DATA_PRODUCER type)
{
    t1_metrics_ctx _c = {};
    _c.mlm_address = g_mlm_metrics_address;
    _c.timeout = g_metrics_timeout;
    _c.data_type = type;
    _c.pfn_connect_emitter = &t1_connect_emitter;
    _c.pfn_connect_consumer = &t1_connect_receiver;
    _c.pfn_send = &tau1::send;
    _c.pfn_receive = &tau1::receive;
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
