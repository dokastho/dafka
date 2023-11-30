#include <string>
#include <memory.h>

#include "dafka.h"

StrongDafkaConnection::StrongDafkaConnection(
        drpc_host &my_host,
        void* my_srv_ptr,
        void* req_endpoint_ptr,
        void* rep_endpoint_ptr
    ) : IDafkaConnection(
        my_host,
        my_srv_ptr,
        req_endpoint_ptr,
        rep_endpoint_ptr
    )
{
    type = DafkaConnectionType::STRONG;
}

int StrongDafkaConnection::notify(drpc_host & remote, DafkaConnectionOp op, payload_t & payload)
{
    drpc_client c;
    dafka_reply r{ERR};
    dafka_args da{
        rand(),
        drpc_engine->get_host(),
        type,
        op,
        payload
    };

    rpc_arg_wrapper req{(void *)&da, sizeof(da)};
    rpc_arg_wrapper rep{(void *)&r, sizeof(r)};

    int status;
    while (r.status != OK)
    {
        status = 1;
        r.status = ERR;
        status = c.Call(remote, DAFKA_ENDPOINT, &req, &rep);
        if (status == 1)
        {
            r.status = ERR;
        }
    }
    return 0;
}

int StrongDafkaConnection::stub(dafka_args *args)
{
    {
        std::unique_lock<std::mutex> l(__l);
        if (knows_seed(args->seed))
        {
            return 0;
        }
        seeds.push_back(args->seed);
    }
    
    switch (args->op)
    {
    case DafkaConnectionOp::REPLY:
        rep_endpoint(srv_ptr, args->payload.data);
        break;

    case DafkaConnectionOp::REQUEST:
        req_endpoint(srv_ptr, args->payload.data);
        break;
    
    default:
        return 1;
    }
    return 0;
}
