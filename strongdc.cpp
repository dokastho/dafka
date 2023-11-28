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