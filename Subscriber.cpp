#include <memory.h>
#include "dafka.h"

Subscriber::Subscriber(drpc_host &dh) : host(dh) {}

int Subscriber::notify(IDafkaConnection *idc, DafkaConnectionOp op, uint8_t *data)
{
    payload p;
    memcpy(p.data, data, DATA_LEN);
    switch (idc->type)
    {
    case DafkaConnectionType::STRONG:
        StrongDafkaConnection *sdc_ptr = dynamic_cast<StrongDafkaConnection*>(idc);
        sdc_ptr->notify(host, op, p);
        break;

    case DafkaConnectionType::WEAK:
        WeakDafkaConnection *wdc_ptr = dynamic_cast<WeakDafkaConnection*>(idc);
        wdc_ptr->notify(host, op, p);
        break;
    
    default:
        return 1;
    }
    return 0;
}
