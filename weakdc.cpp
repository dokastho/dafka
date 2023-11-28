#include <memory.h>

#include "dafka.h"

WeakDafkaConnection::WeakDafkaConnection(drpc_host &my_host) : IDafkaConnection(my_host)
{
}

int WeakDafkaConnection::notify_one()
{
    return 0;
}

int WeakDafkaConnection::notify_all()
{
    return 0;
}
