#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>
#include <vector>
#include <map>
#include <cassert>
#include <memory.h>

#include "dafka.h"
#include "drpc.h"

using std::ostream;

std::map<short, int> host_ports;

typedef std::chrono::milliseconds MS;

std::mutex lock;

payload_t p{"sample"};

class GossipingHost
{
private:
    int me = 0;

public:
    friend ostream &operator<<(ostream &out, const GossipingHost &gh);
    StrongDafkaConnection *sdc;
    GossipingHost(drpc_host h, int id)
    {
        sdc = new StrongDafkaConnection(h, this, (void *)req_fn, (void *)rep_fn);
        me = id;
    }

    void subscribe_func(drpc_host &remote)
    {

        Subscriber rs(remote);

        drpc_host my_host = sdc->get_host();
        if (my_host == remote)
        {
            return;
        }

        payload_t payload;
        memcpy(payload.data, &my_host, sizeof(my_host));

        sdc->subscribe(remote, payload);

        std::unique_lock<std::mutex> l(lock);
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
        std::cout << srv->me << " subscribing in exchange..." << std::endl;
        drpc_host dh;
        memcpy(&dh, data, sizeof(dh));
        srv->subscribe_func(dh);
    }

    static void rep_fn(GossipingHost *srv, uint8_t *data)
    {
        std::cout << srv->me << " was notified. adding new hosts to my pool..." << data << std::endl;

        drpc_host host;
        memcpy(&host, data, sizeof(host));
        srv->subscribe_func(host);
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

ostream &operator<<(ostream &out, const GossipingHost &gh)
{
    out << gh.me << ":";
    std::vector<drpc_host> hosts = gh.sdc->get_subscriber_hosts();
    for (size_t i = 0; i < hosts.size(); i++)
    {
        out << " " << host_ports[hosts[i].port];
    }
    out << std::endl;

    return out;
}

void print_info(std::vector<GossipingHost*> &hosts)
{
    std::cout << "Host info\n";
    for (size_t i = 0; i < hosts.size(); i++)
    {
        std::cout << *(hosts[i]);
    }
    std::cout << std::endl;
}

int main()
{
    drpc_host dh{"localhost", 0};

    int nhosts = 3;

    GossipingHost h1(dh, 1);
    drpc_host dh1 = h1.get_host();
    GossipingHost h2(dh, 2);
    drpc_host dh2 = h2.get_host();
    GossipingHost h3(dh, 3);
    drpc_host dh3 = h3.get_host();

    h1.subscribe_func(dh2);
    h2.subscribe_func(dh3);
    h3.subscribe_func(dh1);

    h1.spread_gossip();
    h2.spread_gossip();
    h3.spread_gossip();

    assert((int)h1.sdc->get_subscriber_hosts().size() == nhosts - 1);
    assert((int)h2.sdc->get_subscriber_hosts().size() == nhosts - 1);
    assert((int)h3.sdc->get_subscriber_hosts().size() == nhosts - 1);

    std::cout << "\npassed" << std::endl;

    return 0;
}