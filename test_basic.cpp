#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>

#include "dafka.h"
#include "drpc.h"

typedef std::chrono::seconds S;

std::mutex __l;

class Host
{
private:
    int me = 0;
public:
    StrongDafkaConnection *sdc;
    Host(drpc_host h, int id)
    {
        sdc = new StrongDafkaConnection(h, this, (void*)req_fn, (void*)rep_fn);
        me = id;
    }

    void test_func()
    {
        std::unique_lock<std::mutex>(__l);
        std::cout << "notifying subscribers" << std::endl;
        sdc->notify_all();
    }

    static void req_fn(Host* srv, uint8_t* data)
    {
        std::unique_lock<std::mutex>(__l);
        std::cout << srv->me << " was requested" << std::endl;
    }
    
    static void rep_fn(Host* srv, uint8_t* data)
    {
        std::unique_lock<std::mutex>(__l);
        std::cout << srv->me << " was notified" << std::endl;
    }

    ~Host()
    {
        delete sdc;
    }
};


int main()
{
    std::vector<Host> hosts;
    drpc_host srv_host{"localhost", 8555};
    hosts.emplace_back(Host(srv_host, 0));
    drpc_host dh{"localhost", 0};
    size_t nhosts = 5;
    // create some other hosts
    for (int i = 1; i < nhosts; i++)
    {
        hosts.emplace_back(Host(dh, i));
        hosts.back().sdc->subscribe(srv_host);
    }

    hosts.front().test_func();
    
    return 0;
}