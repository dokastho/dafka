#ifndef DAFKA_CONNECTIONS
#define DAFKA_CONNECTIONS

#include <vector>
#include <map>
#include <string>

#include "drpc.h"
#include "rpcs.h"

// Interface class for a Dafka connection
class IDafkaConnection
{
protected:
    DafkaConnectionType type;
    std::vector<drpc_host> subscirbers;
    std::map<std::string, void *> endpoints;
public:

// dafka host starts listening for inbound connections
virtual void listen();

// dafka host subscribes to another's endpoint
virtual int subscribe();

// dafka host notifies one connection
virtual int notify_one();

// dafka host notifies all connections
virtual int notify_all();
};

// Persistent Dafka connection class
class StrongDafkaConnection : IDafkaConnection
{
public:
    void listen();

    int subscribe();

    int notify_one();

    int notify_all();
};

// Not persistent Dafka connection class
class WeakDafkaConnection : IDafkaConnection
{
public:
    void listen();

    int subscribe();

    int notify_one();

    int notify_all();
};

#endif
