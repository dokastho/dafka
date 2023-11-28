#ifndef DAFKA_CONNECTIONS
#define DAFKA_CONNECTIONS

#include <map>
#include <string>
#include <algorithm>
#include <mutex>

#include "drpc.h"
#include "rpcs.h"

class Subscriber
{
private:
    drpc_host host;

public:
    Subscriber(drpc_host &);

    int notify(IDafkaConnection *, DafkaConnectionOp, uint8_t *);
};

// Interface class for a Dafka connection
class IDafkaConnection
{
protected:
    DafkaConnectionType type;
    std::vector<Subscriber> subscribers;
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

    friend class Subscriber;

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
    int notify_one(DafkaConnectionOp, payload &, int);

    // dafka host notifies all connections
    int notify_all(DafkaConnectionOp, payload &);
};

// Persistent Dafka connection class
class StrongDafkaConnection : public IDafkaConnection
{
public:
    StrongDafkaConnection(drpc_host &, void *, void *, void *);
};

// Not persistent Dafka connection class
class WeakDafkaConnection : public IDafkaConnection
{
public:
    WeakDafkaConnection(drpc_host &, void *, void *, void *);
};

#endif
