#ifndef DAFKA_RPCS
#define DAFKA_RPCS

#include <cstddef>

#include "drpc.h"

#define OK 0
#define ERR 1

#define DAFKA_ENDPOINT "Subscribe"

// Enum type for Dafka connection identification
enum class DafkaConnectionType : char
{
    STRONG,
    WEAK
};

struct dafka_args
{
    void *data;
    size_t data_len;
    int seed;
    drpc_host host;
    DafkaConnectionType type;
};

struct dafka_reply
{
    int status;
};

#endif
