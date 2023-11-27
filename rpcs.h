#ifndef DAFKA_RPCS
#define DAFKA_RPCS

#define OK 0;
#define ERR 1;

#define dafka_target_func_len 16

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
    int addr;
    short port;
    DafkaConnectionType type;
};

struct dafka_reply
{
    int status;
};

struct subscribe_t
{
    size_t len;
    char target[dafka_target_func_len];
    void* args;
};

#endif
