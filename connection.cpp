#include <memory.h>
#include <netdb.h>

#include "rpcs.h"
#include "connection.h"

IDafkaConnection::IDafkaConnection(drpc_host &dh, void *srv_ptr_arg)
{
    srv_ptr = srv_ptr_arg;
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

void IDafkaConnection::listen(IDafkaConnection *idc, drpc_msg &m)
{
    // args
    dafka_args *da = (dafka_args *)m.req->args;
    subscribe_t *s = (subscribe_t *)malloc(da->data_len);
    memcpy(s, da->data, da->data_len);
    // reply
    dafka_reply *dr = (dafka_reply *)m.rep->args;;
    // host
    drpc_host d{"", 0};

    // struct hostent *gethostbyaddr()

    StrongDafkaConnection *sdc_ptr;
    WeakDafkaConnection *wdc_ptr;

    switch (idc->type)
    {
    case DafkaConnectionType::STRONG:
        sdc_ptr = dynamic_cast<StrongDafkaConnection *>(idc);
        sdc_ptr->add_subscriber(d, s, dr, da->seed);
        break;

    case DafkaConnectionType::WEAK:
        wdc_ptr = dynamic_cast<WeakDafkaConnection *>(idc);
        wdc_ptr->add_subscriber(d, s, dr);
        break;
    }

    free(s);
}
