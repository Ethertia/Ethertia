//
// Created by Dreamtowards on 2022/12/23.
//

#ifndef ETHERTIA_NETWORK_H
#define ETHERTIA_NETWORK_H

#include <stdexcept>

#ifndef ENET_IMPLEMENTATION
#define ENET_IMPLEMENTATION
#endif
#include <enet.h>


class Network
{
public:
    using Host = ENetHost;

    static void Init()
    {
        if (enet_initialize()) {
            throw std::runtime_error("failed to init enet.");
        }
    }

    static void Deinit(ENetHost* host)
    {
        if (host) {
            enet_host_destroy(host);
        }
        enet_deinitialize();
    }


    static ENetHost* newServer(int port)
    {
        ENetAddress addr = {};
        addr.host = ENET_HOST_ANY;
        addr.port = port;

        ENetHost* serv = enet_host_create(&addr, 10, 2, 0, 0);
        if (!serv)
            throw std::runtime_error("failed to create Enent server host.");

        return serv;
    }

    static ENetHost* newClient()
    {
        ENetHost* client = enet_host_create(nullptr /* create a client host */,
                                            1 /* only allow 1 outgoing connection */,
                                            2 /* allow up 2 channels to be used, 0 and 1 */,
                                            0 /* assume any amount of incoming bandwidth */,
                                            0 /* assume any amount of outgoing bandwidth */);
        if (!client)
            throw std::runtime_error("failed to create Enet client host.");

        return client;
    }

    static ENetPeer* connect(ENetHost* host, const char* hostname, int port)
    {
        ENetAddress addr = {};
        enet_address_set_host(&addr, hostname);
        addr.port = port;

        ENetPeer* peer = enet_host_connect(host, &addr, 2, 0);
        if (!peer)
            throw std::runtime_error("No available peers for initiating an ENet connection.");

        return peer;
    }

    static void disconnect(ENetPeer* peer) {

        enet_peer_disconnect(peer, 0);
    }


    /*
    Network::Poll(cli, true, [](auto& e){
        // Conn

        // e.peer->data = (void*)"ClientMark";  // set user ptr
    }, [](auto& e) {
        // Recv

        //
    }, [](auto& e) {
        // Drop

        // event.peer->data = NULL;
    });
     */
    static void Poll(ENetHost* host, const bool& running,
                   std::function<void(ENetEvent& e)> f_conn,
                   std::function<void(ENetEvent& e)> f_recv,
                   std::function<void(ENetEvent& e)> f_disc) {

        ENetEvent event;
        while (running) {
            int nqueue = enet_host_service(host, &event, 10);
            if (nqueue == 0)
                continue;

            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:               // only '.peer' is valid

                    f_conn(event);

                    // assert(event.peer->data);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:

                    f_recv(event);

                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:            // only .peer->data is valid
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:

                    f_disc(event);

                    // event.peer->data = NULL;
                    // assert(event.peer->data == nullptr);
                    break;
                case ENET_EVENT_TYPE_NONE:
                    throw std::runtime_error("None event");
            }
        }

    }

    static void SendPacket(ENetPeer* peer, std::string_view str) {
        SendPacket(peer, (void*)str.data(), str.length());
    }
    static void SendPacket(ENetPeer* peer, void* data, size_t len) {


        ENetPacket* pack = enet_packet_create(data, len, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, pack);
    }
    static void FlushPacket(ENetHost* host) {
        enet_host_flush(host);
    }
};

#endif //ETHERTIA_NETWORK_H
