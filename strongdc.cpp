#include <string>
#include <memory.h>

#include "dafka.h"

StrongDafkaConnection::StrongDafkaConnection(drpc_host &my_host) : IDafkaConnection(my_host)
{
}

int StrongDafkaConnection::notify_one()
{
    return 0;
}

int StrongDafkaConnection::notify_all()
{
    return 0;
}
