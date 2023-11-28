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

int StrongDafkaConnection::notify(drpc_host & remote, DafkaConnectionOp op, payload & data)
{
    drpc_client c;
    dafka_reply r{ERR};
    dafka_args da;

    da.host = drpc_engine->get_host();
    da.seed = rand();
    da.type = type;
    da.op = op;
    memcpy(da.data.data, data.data, DATA_LEN);

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
}

int StrongDafkaConnection::stub(dafka_args *args)
{
    if (knows_seed(args->seed))
    {
        return 0;
    }
    
    switch (args->op)
    {
    case DafkaConnectionOp::REPLY:
        rep_endpoint(srv_ptr, args->data.data);
        break;

    case DafkaConnectionOp::REQUEST:
        req_endpoint(srv_ptr, args->data.data);
        break;
    
    default:
        return 1;
    }
    return 0;
}
