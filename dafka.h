#ifndef DAFKA_CONNECTIONS
#define DAFKA_CONNECTIONS

#include <map>
#include <string>
#include <algorithm>
#include <mutex>

#include "drpc.h"
#include "rpcs.h"

// Interface class for a Dafka connection
class IDafkaConnection
{
protected:
    DafkaConnectionType type;
    std::map<drpc_host, std::string> subscribers;
    drpc_server *drpc_engine;
    void *srv_ptr;
    static void (*req_endpoint)(void *, uint8_t *);
    static void (*rep_endpoint)(void *, uint8_t *);
    std::vector<int> seeds;
    std::mutex __l;

    bool knows_seed(int seed)
    {
        std::unique_lock<std::mutex>(__l);
        return std::find(seeds.begin(), seeds.end(), seed) != seeds.end();
    }

    void add_subscriber(dafka_args *);

    void remove_subscriber(dafka_args *);

public:
    IDafkaConnection(drpc_host &, void *, void *, void *);

    virtual ~IDafkaConnection();

    // dafka host receives subscription request
    static void listen(IDafkaConnection *, drpc_msg &);

    // dafka host subscribes to another's endpoint
    // note to self: setting function = 0 denotes it as pure virtual.
    // Can maintain as virtual if not = 0
    int subscribe(drpc_host &);

    // dafka host notifies one connection
    virtual int notify_one() = 0;

    // dafka host notifies all connections
    virtual int notify_all() = 0;
};

// Persistent Dafka connection class
class StrongDafkaConnection : public IDafkaConnection
{
private:
    int call_host(drpc_host &, DafkaConnectionOp, uint8_t *);

public:
    StrongDafkaConnection(drpc_host &, void *, void *, void *);

    int notify_one();

    int notify_all();
};

// Not persistent Dafka connection class
class WeakDafkaConnection : public IDafkaConnection
{
private:
    int call_host(drpc_host &, DafkaConnectionOp, uint8_t *);

public:
    WeakDafkaConnection(drpc_host &, void *, void *, void *);

    int notify_one();

    int notify_all();
};

#endif
