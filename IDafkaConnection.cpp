#include <memory.h>
#include <netdb.h>

#include "rpcs.h"
#include "dafka.h"

IDafkaConnection::IDafkaConnection(drpc_host &dh, void* my_srv_ptr, void* req_endpoint_ptr, void* rep_endpoint_ptr)
{
    srv_ptr = my_srv_ptr;
    req_endpoint = (void (*)(void *, uint8_t *))req_endpoint_ptr;
    rep_endpoint = (void (*)(void *, uint8_t *))rep_endpoint_ptr;
    drpc_engine = new drpc_server(dh, this);
    drpc_engine->publish_endpoint(DAFKA_ENDPOINT, (void *)listen);
    drpc_engine->start();
}

IDafkaConnection::~IDafkaConnection()
{
    // notify?

    // stop drpc server
    delete drpc_engine;
}

void IDafkaConnection::add_subscriber(dafka_args *da)
{
    if (knows_seed(da->seed))
    {
        return;
    }
    {
        std::unique_lock<std::mutex>(__l);
        seeds.push_back(da->seed);
        Subscriber s(da->host);
        subscribers.push_back(s);
    }
}

void IDafkaConnection::remove_subscriber(dafka_args *da)
{
    if (knows_seed(da->seed))
    {
        return;
    }
    {
        std::unique_lock<std::mutex>(__l);
        seeds.push_back(da->seed);
        subscribers.erase(std::find(subscribers.begin(), subscribers.end(), da->host));
    }
}

void IDafkaConnection::listen(IDafkaConnection *idc, drpc_msg &m)
{
    // args
    dafka_args *da = (dafka_args *)m.req->args;
    // reply
    dafka_reply *dr = (dafka_reply *)m.rep->args;

    switch (da->op)
    {
    case DafkaConnectionOp::SUBSCRIBE:
        idc->add_subscriber(da);
        break;
    case DafkaConnectionOp::UNSUBSCRIBE:
        idc->remove_subscriber(da);
        break;
    case DafkaConnectionOp::REQUEST:
        idc->req_endpoint(idc->srv_ptr, da->data.data);
        break;
    case DafkaConnectionOp::REPLY:
        idc->rep_endpoint(idc->srv_ptr, da->data.data);
        break;
    
    default:
        return;
    }
    
    dr->status = OK;
}

int IDafkaConnection::subscribe(drpc_host &remote)
{
    drpc_client c;
    dafka_reply r{ERR};
    dafka_args da;

    da.host = drpc_engine->get_host();
    da.seed = rand();
    da.type = DafkaConnectionType::STRONG; // irrelevant; syntactic sugar for subscribe ops

    rpc_arg_wrapper req{(void *)&da, sizeof(da)};
    rpc_arg_wrapper rep{(void *)&r, sizeof(r)};

    int status;
    while (r.status != OK)
    {
        status = 0;
        r.status = ERR;
        status = c.Call(remote, DAFKA_ENDPOINT, &req, &rep);
        if (status == 1)
        {
            r.status = ERR;
        }
    }
    return 0;
}

int IDafkaConnection::notify_one(DafkaConnectionOp op, payload &p, int index)
{
    if (index > subscribers.size())
    {
        throw std::runtime_error("Index invalid");
        exit(1);
    }
    
    Subscriber sub = subscribers[index];
    
    sub.notify(this, op, p.data);
    return 0;
}

int IDafkaConnection::notify_all(DafkaConnectionOp op, payload &p)
{
    for (Subscriber sub : subscribers)
    {
        sub.notify(this, op, p.data);
    }
    return 0;
}
