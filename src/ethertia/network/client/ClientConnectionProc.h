//
// Created by Dreamtowards on 2022/12/24.
//

#ifndef ETHERTIA_CLIENTCONNECTIONPROC_H
#define ETHERTIA_CLIENTCONNECTIONPROC_H

#include <ethertia/network/Network.h>

#include <ethertia/network/packet/PacketChat.h>



class ClientConnectionProc
{
public:

    static void initPackets() {

        _InitPacketIds();

        INIT_PACKET_PROC(PacketChat, ClientConnectionProc::handlePacket);


    }

    static void nil(CPacketLogin& p) {

    }

    static void handlePacket(const PacketChat& packet) {

        Log::info("", packet.message);

        GuiScreenChat::INST->appendMessage(packet.message);
    }
};

#endif //ETHERTIA_CLIENTCONNECTIONPROC_H
