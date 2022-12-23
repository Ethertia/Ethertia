//
// Created by Dreamtowards on 2022/12/23.
//


#include <stdexcept>

#include <ethertia/util/Log.h>

#define ENET_IMPLEMENTATION
#include <enet.h>

int main() {



    if (enet_initialize()) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }


    ENetHost* client = enet_host_create(nullptr /* create a client host */,
                              1 /* only allow 1 outgoing connection */,
                              2 /* allow up 2 channels to be used, 0 and 1 */,
                              0 /* assume any amount of incoming bandwidth */,
                              0 /* assume any amount of outgoing bandwidth */);
    if (!client) {
        fprintf(stderr, "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }



    ENetAddress addr = {};
    enet_address_set_host(&addr, "127.0.0.1");
    addr.port = 8081;

    ENetPeer* peer = enet_host_connect(client, &addr, 2, 0);
    if (!peer) {
        fprintf(stderr, "No available peers for initiating an ENet connection.\n");
        exit(EXIT_FAILURE);
    }

    ENetEvent event = {};

    while (enet_host_service(client, &event, 5000) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                printf("A new client connected from %x:%u.\n",  event.peer->address.host, event.peer->address.port);


                event.peer->data = (void*)"Cli Mark 123";

                ENetPacket* pack = enet_packet_create("TestAbc123", 11, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(event.peer, event.channelID, pack);
                enet_host_flush(client);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                printf("A packet of length %lu containing %s was received from %s on channel %u.\n",
                       event.packet->dataLength,
                       event.packet->data,
                       event.peer->data,
                       event.channelID);
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy (event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%s disconnected.\n", event.peer->data);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
                break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                printf("%s disconnected due to timeout.\n", event.peer->data);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
                break;

            case ENET_EVENT_TYPE_NONE:
                break;


        }
    }

    enet_peer_disconnect(peer, 0);

    enet_host_destroy(client);
    enet_deinitialize();
}

//#include <sys/socket.h>
//#include <netinet/in.h>  // struct sockaddr_in
//#include <arpa/inet.h>   // ip addr lookup
//#include <unistd.h>      // close(conn)
//
//#include <ethertia/util/Log.h>
//
//int main()
//{
//    int port = 8081;
//
//    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
//    if (server_fd < 0) {
//        throw std::runtime_error("failed init socket.");
//    }
//
//    // // optional, force attach socket. reuse address.
//    // int opt = 1;
//    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
//    //     throw std::runtime_error("failed reuse socket address.");
//    // }
//
//    struct sockaddr_in addr{};
//    addr.sin_family = AF_INET;
//    addr.sin_addr.s_addr = INADDR_ANY;
//    addr.sin_port = htons(port);
//
//    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
//        throw std::runtime_error("failed bind socket.");
//    }
//
//    const int backlog = 3;
//    if (listen(server_fd, backlog) < 0) {
//        throw std::runtime_error("failed listen socket.");
//    }
//
//    std::error_code err;
//
//    while (true)
//    {
//        struct sockaddr_in cli_addr;
//        socklen_t addrlen = sizeof(addr);
//        int conn = accept(server_fd, (struct sockaddr*)&cli_addr, &addrlen);
//        if (conn < 0) {
//            throw std::runtime_error("failed accept socket.");
//        }
//
//        char cli_ip[INET_ADDRSTRLEN];
//        inet_ntop(AF_INET, &cli_addr.sin_addr, cli_ip, INET_ADDRSTRLEN);
//        Log::info("client ip: ", cli_ip);
//
//        char buf[1024];
//        int n = recv(conn, buf, sizeof(buf), 0);
//        Log::info("Recv ", buf);
//
//
//        char* data = "GET / HTTP/1.1\n"
//                     "Host: example.com\n"
//                     "Connection: close\n\nSth\n";
//        send(conn, data, strlen(data), 0);
//        Log::info("Msg Sent");
//
//        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//
//
//        close(conn);
//
//        break;
//    }
//
//
//    close(server_fd);
//    shutdown(server_fd, SHUT_RDWR);
//
//    return 0;
//}

//#include <sys/socket.h>
//#include <netinet/in.h>  // struct sockaddr_in
//#include <arpa/inet.h>   // ip addr lookup
//#include <unistd.h>      // close(conn)
//
//#include <ethertia/util/Log.h>
//
//void client()
//{
//    int port = 8081;
//
//    int sock = socket(AF_INET, SOCK_STREAM, 0);
//    if (sock < 0)
//        return;
//
//    struct sockaddr_in serv_addr{};
//    serv_addr.sin_family = AF_INET;
//    serv_addr.sin_port = htons(port);
//    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
//        return;
//
//    int client_fd = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
//    if (client_fd < 0)
//        return;
//
//    char* data = "Hello From Client";
//    send(sock, data, strlen(data), 0);
//
//    char buf[1024];
//    int n = recv(sock, buf, sizeof(buf), 0);
//    Log::info("Cli Recv: ", buf);
//
//
//    close(client_fd);
//}
//
//int main() {
//
//    client();
//
//}