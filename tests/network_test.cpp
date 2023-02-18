#include <thread>
#include "gloux/network.h"

int TEST_SOCKET(){
    using namespace gloux::network;
    std::thread server([](){
        Socket server;
        InetAddress address("127.0.0.1", 1145);
        server.bind(address);
        server.listen();
        Socket client = server.accept();
        printf("new connection: %s:%d\n", client.get_address().get_ip(), client.get_address().get_port());
        client.close();
        server.close();
        //client.free();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::thread client([](){
        Socket client;
        InetAddress address("127.0.0.1", 1145);
        client.connect(address);
        client.close();
    });
    server.join();
    client.join();
    return 0;
}

int main(){
    TEST_SOCKET();
}