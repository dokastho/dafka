#include <memory.h>
#include "dafka.h"

Subscriber::Subscriber(drpc_host &dh) : host(dh) {}

int Subscriber::notify(IDafkaConnection *idc, DafkaConnectionOp op, uint8_t *data)
{
    drpc_client c;
    dafka_reply r{ERR};
    dafka_args da;

    da.host = idc->drpc_engine->get_host();
    da.seed = rand();
    da.type = DafkaConnectionType::STRONG;
    da.op = op;
    memcpy(da.data.data, data, DATA_LEN);

    rpc_arg_wrapper req{(void *)&da, sizeof(da)};
    rpc_arg_wrapper rep{(void *)&r, sizeof(r)};

    int status;
    switch (idc->type)
    {
    case DafkaConnectionType::WEAK:
        return c.Call(host, DAFKA_ENDPOINT, &req, &rep);

    case DafkaConnectionType::STRONG:
        while (r.status != OK)
        {
            status = 0;
            r.status = ERR;
            status = c.Call(host, DAFKA_ENDPOINT, &req, &rep);
            if (status == 1)
            {
                r.status = ERR;
            }
        }

    default:
        return 1;
    }
}
