#include <thread>
#include <chrono>
#include <iostream>

#include "dafka.h"
#include "drpc.h"

typedef std::chrono::seconds S;

drpc_host srv_host{"localhost", 8555};

class Server
{
    private:
    StrongDafkaConnection *sdc;
    bool do_notify = false;
    public:
    Server()
    {
        sdc = new StrongDafkaConnection(srv_host);
    }

    void test_func()
    {
        std::cout << "notifying subscribers" << std::endl;
        sdc->notify_all();
    }

    ~Server()
    {
        delete sdc;
    }
};


int main()
{
    Server srv;

    drpc_host dh{"localhost", 0};
    // create some clients
    StrongDafkaConnection client1(dh);
    StrongDafkaConnection client2(dh);
    StrongDafkaConnection client3(dh);
    
    client1.subscribe(srv_host);
    client2.subscribe(srv_host);
    client3.subscribe(srv_host);

    srv.test_func();
    return 0;
}