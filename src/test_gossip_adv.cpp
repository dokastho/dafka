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

    int nhosts = 10;
    std::vector<GossipingHost *> hosts;
    std::vector<drpc_host> addrs;

    for (int i = 0; i < nhosts; i++)
    {
        hosts.push_back(new GossipingHost(dh, i));
        addrs.push_back(hosts.back()->get_host());
        host_ports[addrs.back().port] = i;
    }

    for (int i = 0; i < nhosts; i++)
    {
        hosts[i]->subscribe_func(addrs[(i + 1) % addrs.size()]);
    }

    print_info(hosts);

    size_t niter = 5;

    for (size_t i = 0; i < niter; i++)
    {
        for (int j = 0; j < nhosts; j++)
        {
            hosts[j]->spread_gossip();
        }

        std::this_thread::sleep_for(MS(666));
    }

    print_info(hosts);

    for (int i = 0; i < nhosts; i++)
    {
        assert((int)hosts[i]->sdc->get_subscriber_hosts().size() == nhosts - 1);
        delete hosts[i];
        hosts[i] = nullptr;
    }

    std::cout << "passed" << std::endl;

    return 0;
}