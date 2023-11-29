#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>

#include "dafka.h"
#include "drpc.h"

typedef std::chrono::seconds S;

std::mutex __l;

payload p{"sample data"};

class Host
{
private:
    int me = 0;

public:
    StrongDafkaConnection *sdc;
    Host(drpc_host h, int id)
    {
        sdc = new StrongDafkaConnection(h, this, (void *)req_fn, (void *)rep_fn);
        me = id;
    }

    void test_func()
    {
        std::unique_lock<std::mutex> l(__l);
        std::cout << "notifying subscribers" << std::endl;
        sdc->notify_all(DafkaConnectionOp::REPLY, p);
    }

    static void req_fn(Host *srv, uint8_t *data)
    {
        std::unique_lock<std::mutex> l(__l);
        std::cout << srv->me << " was requested. has data " << data << std::endl;
    }

    static void rep_fn(Host *srv, uint8_t *data)
    {
        std::unique_lock<std::mutex> l(__l);
        std::cout << srv->me << " was notified. has data " << data << std::endl;
    }

    ~Host()
    {
        delete sdc;
    }
};

int main()
{
    drpc_host srv_host{"localhost", 8555};
    drpc_host dh{"localhost", 0};
    Host srv(srv_host, 0);
    Host h1(dh, 1);
    // Host h2(dh, 1);
    // Host h3(dh, 1);

    h1.sdc->subscribe(srv_host);
    // h2.sdc->subscribe(srv_host);
    // h3.sdc->subscribe(srv_host);

    srv.test_func();
    return 0;
}