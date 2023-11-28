#ifndef DAFKA_CONNECTIONS
#define DAFKA_CONNECTIONS

#include <vector>
#include <map>
#include <string>
#include <algorithm>

#include "drpc.h"
#include "rpcs.h"

// Interface class for a Dafka connection
class IDafkaConnection
{
protected:
    DafkaConnectionType type;
    std::vector<drpc_host> subscirbers;
    static void (*endpoint)(void *, drpc_msg &);
    void *srv_ptr;
    drpc_server *drpc_engine;

public:
    IDafkaConnection(drpc_host &, void *);
    ~IDafkaConnection();

    // dafka host receives subscription request
    static void listen(IDafkaConnection *, drpc_msg &);

    // dafka host subscribes to another's endpoint
    // note to self: setting function = 0 denotes it as pure virtual.
    // Can maintain as virtual if not = 0
    virtual int subscribe() = 0;

    // dafka host notifies one connection
    virtual int notify_one() = 0;

    // dafka host notifies all connections
    virtual int notify_all() = 0;
};

// Persistent Dafka connection class
class StrongDafkaConnection : IDafkaConnection
{
private:
    std::vector<int> seeds;

    bool knows_seed(int seed)
    {
        return std::find(seeds.begin(), seeds.end(), seed) != seeds.end();
    }

public:
    int subscribe();

    void add_subscriber(drpc_host &, subscribe_t *, dafka_reply *, int);

    int notify_one();

    int notify_all();
};

// Not persistent Dafka connection class
class WeakDafkaConnection : IDafkaConnection
{
public:
    int subscribe();

    void add_subscriber(drpc_host &, subscribe_t *, dafka_reply *);

    int notify_one();

    int notify_all();
};

#endif
