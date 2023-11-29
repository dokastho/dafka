#include <memory.h>

#include "dafka.h"

WeakDafkaConnection::WeakDafkaConnection(
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
    type = DafkaConnectionType::WEAK;
}

int WeakDafkaConnection::notify(drpc_host & remote, DafkaConnectionOp op, payload_t & payload)
{
    drpc_client c;
    dafka_reply r{ERR};
    dafka_args da;

    da.host = drpc_engine->get_host();
    da.type = type;
    da.op = op;
    memcpy(da.payload.data, payload.data, DATA_LEN);

    rpc_arg_wrapper req{(void *)&da, sizeof(da)};
    rpc_arg_wrapper rep{(void *)&r, sizeof(r)};

    return c.Call(remote, DAFKA_ENDPOINT, &req, &rep);
}

int WeakDafkaConnection::stub(dafka_args *args)
{
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