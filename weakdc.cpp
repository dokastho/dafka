#include <memory.h>

#include "connection.h"

void WeakDafkaConnection::add_subscriber(drpc_host &sub, subscribe_t *args, dafka_reply *reply)
{
    subscirbers.push_back(sub);
    std::string target = args->target;
    rpc_arg_wrapper *req = new rpc_arg_wrapper;
    rpc_arg_wrapper *rep = new rpc_arg_wrapper;
    drpc_msg m;
    m.req = req;
    m.rep = rep;

    req->args = malloc(args->len);
    req->len = args->len;
    memcpy(req->args, args->args, args->len);

    endpoint(srv_ptr, m);
    reply->status = OK;

    free(req->args);
    delete rep;
    delete req;
}

int WeakDafkaConnection::subscribe()
{
    return 0;
}

int WeakDafkaConnection::notify_one()
{
    return 0;
}

int WeakDafkaConnection::notify_all()
{
    return 0;
}
