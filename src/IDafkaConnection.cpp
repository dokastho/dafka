#include <memory.h>
#include <netdb.h>

#include "rpcs.h"
#include "dafka.h"

Subscriber::Subscriber(drpc_host &dh) : host(dh) {}

int Subscriber::notify(IDafkaConnection *idc, DafkaConnectionOp op, uint8_t *data)
{
    payload_t p;
    memcpy(p.data, data, DATA_LEN);

    StrongDafkaConnection *sdc_ptr;
    WeakDafkaConnection *wdc_ptr;
    switch (idc->type)
    {
    case DafkaConnectionType::STRONG:
        sdc_ptr = dynamic_cast<StrongDafkaConnection*>(idc);
        sdc_ptr->notify(host, op, p);
        break;

    case DafkaConnectionType::WEAK:
        wdc_ptr = dynamic_cast<WeakDafkaConnection*>(idc);
        wdc_ptr->notify(host, op, p);
        break;
    
    default:
        return 1;
    }
    return 0;
}

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
        std::unique_lock<std::mutex> l(__l);
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
        std::unique_lock<std::mutex> l(__l);
        seeds.push_back(da->seed);
        Subscriber s(da->host);
        subscribers.erase(std::find(subscribers.begin(), subscribers.end(), s));
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
        idc->stub(da);
        break;
    case DafkaConnectionOp::REPLY:
        idc->stub(da);
        break;
    
    default:
        return;
    }
    
    dr->status = OK;
}

int IDafkaConnection::subscribe(drpc_host &remote)
{
    StrongDafkaConnection *sdc_ptr = dynamic_cast<StrongDafkaConnection*>(this);
    sdc_ptr->type = DafkaConnectionType::STRONG;
    Subscriber rs(remote);
    rs.notify(sdc_ptr, DafkaConnectionOp::SUBSCRIBE, {});
    return 0;
}

int IDafkaConnection::notify_one(DafkaConnectionOp op, payload_t &p, int index)
{
    if (index > (int)subscribers.size())
    {
        throw std::runtime_error("Index invalid");
        exit(1);
    }
    
    Subscriber sub = subscribers[index];
    
    sub.notify(this, op, p.data);
    return 0;
}

int IDafkaConnection::notify_all(DafkaConnectionOp op, payload_t &p)
{
    for (Subscriber sub : subscribers)
    {
        sub.notify(this, op, p.data);
    }
    return 0;
}
