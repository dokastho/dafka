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
}

int WeakDafkaConnection::call_host(drpc_host & remote, DafkaConnectionOp op, uint8_t* data)
{
    drpc_client c;
    dafka_reply r{ERR};
    dafka_args da;

    da.host = drpc_engine->get_host();
    da.seed = rand();
    da.type = DafkaConnectionType::STRONG;
    da.op = op;
    memcpy(da.data, data, DATA_LEN);

    rpc_arg_wrapper req{(void *)&da, sizeof(da)};
    rpc_arg_wrapper rep{(void *)&r, sizeof(r)};

    return c.Call(remote, DAFKA_ENDPOINT, &req, &rep);
}

int WeakDafkaConnection::notify_one()
{
    return 0;
}

int WeakDafkaConnection::notify_all()
{
    return 0;
}
