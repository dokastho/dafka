#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>
#include <vector>
#include <memory.h>

#include "dafka.h"
#include "drpc.h"

typedef std::chrono::milliseconds MS;

std::mutex __l;

payload_t p{"sample"};

class GossipingHost
{
private:
    int me = 0;
    std::vector<drpc_host> pool;

public:
    StrongDafkaConnection *sdc;
    GossipingHost(drpc_host h, int id)
    {
        sdc = new StrongDafkaConnection(h, this, (void *)req_fn, (void *)rep_fn);
        me = id;
    }

    void subscribe_func(drpc_host &remote)
    {
        std::unique_lock<std::mutex> l(__l);

        Subscriber rs(remote);

        drpc_host my_host = sdc->get_host();
        if (my_host.hostname == remote.hostname && my_host.port == remote.port)
        {
            return;
        }

        payload_t payload;
        memcpy(payload.data, &my_host, sizeof(my_host));

        sdc->subscribe(remote, payload);

        if (std::find(pool.begin(), pool.end(), remote) != pool.end())
        {
            return;
        }
        
        pool.push_back(remote);
    }

    void spread_gossip()
    {
        std::cout << "sharing subscribers with my pool..." << std::endl;
        payload_t payload;
        std::vector<drpc_host> hosts = sdc->get_subscriber_hosts();
        drpc_host remote = hosts[rand() % (int)hosts.size()];
        memcpy(payload.data, &remote, sizeof(remote));
        sdc->notify_all(DafkaConnectionOp::REPLY, payload);
    }

    static void req_fn(GossipingHost *srv, uint8_t *data)
    {
        {
            std::unique_lock<std::mutex> l(__l);
            std::cout << srv->me << " subscribing in exchange..." << std::endl;
        }
        drpc_host dh;
        memcpy(&dh, data, sizeof(dh));
        srv->subscribe_func(dh);
    }

    static void rep_fn(GossipingHost *srv, uint8_t *data)
    {
        std::unique_lock<std::mutex> l(__l);
        std::cout << srv->me << " was notified. adding new hosts to my pool..." << data << std::endl;

        drpc_host host;
        memcpy(&host, data, sizeof(host));
        srv->pool.push_back(host);
    }

    drpc_host get_host()
    {
        return sdc->get_host();
    }

    ~GossipingHost()
    {
        delete sdc;
    }
};

int main()
{
    drpc_host dh{"localhost", 0};

    GossipingHost h1(dh, 1);
    drpc_host dh1 = h1.get_host();
    GossipingHost h2(dh, 2);
    drpc_host dh2 = h2.get_host();
    // GossipingHost h3(dh, 3);
    // drpc_host dh3 = h3.get_host();

    h1.subscribe_func(dh2);
    h2.subscribe_func(dh1);
    // h3.subscribe_func(dh1);


    size_t niter = 10;

    for (size_t i = 0; i < niter; i++)
    {
        h1.spread_gossip();
        h2.spread_gossip();
        // h3.spread_gossip();
        std::this_thread::sleep_for(MS(666));
    }

    return 0;
}