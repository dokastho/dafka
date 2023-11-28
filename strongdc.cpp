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
}

int StrongDafkaConnection::call_host(drpc_host & remote, DafkaConnectionOp op, uint8_t* data)
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

int StrongDafkaConnection::notify_one()
{
    int index = rand() % subscribers.size();
    
    
    return 0;
}

int StrongDafkaConnection::notify_all()
{
    return 0;
}
