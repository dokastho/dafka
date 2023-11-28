#include <memory.h>
#include <netdb.h>

#include "rpcs.h"
#include "dafka.h"

IDafkaConnection::IDafkaConnection(drpc_host &dh)
{
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
    // reply
    dafka_reply *dr = (dafka_reply *)m.rep->args;

    if (idc->knows_seed(da->seed))
    {
        dr->status = OK;
        return;
    }
    {
        std::unique_lock<std::mutex>(__l);
        idc->seeds.push_back(da->seed);
        idc->subscribers.push_back(da->host);
    }
    dr->status = OK;
}

int IDafkaConnection::subscribe(drpc_host &remote)
{
    drpc_client c;
    dafka_reply r{ERR};
    dafka_args da;

    da.data_len = 0;
    da.data = malloc(da.data_len);

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