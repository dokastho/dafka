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
    drpc_server *drpc_engine;
    std::vector<int> seeds;

    bool knows_seed(int seed)
    {
        return std::find(seeds.begin(), seeds.end(), seed) != seeds.end();
    }

public:
    IDafkaConnection(drpc_host &);

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
public:
    StrongDafkaConnection(drpc_host &);

    int notify_one();

    int notify_all();
};

// Not persistent Dafka connection class
class WeakDafkaConnection : public IDafkaConnection
{
public:
    WeakDafkaConnection(drpc_host &);

    int notify_one();

    int notify_all();
};

#endif
