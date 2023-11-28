#ifndef DAFKA_RPCS
#define DAFKA_RPCS

#include <cstddef>

#include "drpc.h"

#define OK 0
#define ERR 1

#define DATA_LEN 64
#define DAFKA_ENDPOINT "Subscribe"

// Enum type for Dafka connection identification
enum class DafkaConnectionType : char
{
    STRONG,
    WEAK
};

enum class DafkaConnectionOp : char
{
    SUBSCRIBE,
    UNSUBSCRIBE,
    REQUEST,
    REPLY
};

struct dafka_args
{
    int seed;
    drpc_host host;
    DafkaConnectionType type;
    DafkaConnectionOp op;
    payload data;
};

struct dafka_reply
{
    int status;
};

struct payload
{
    uint8_t data[DATA_LEN];
};

#endif
