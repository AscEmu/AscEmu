/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // a Battle.net rpc call sent through the world connection: method (id in the low word, service hash in the
    // high word), object id, token and the serialized request
    class CmsgBattlenetRequest : public ManagedPacket
    {
    public:
        uint64_t methodType = 0;
        uint64_t objectId = 0;
        uint32_t token = 0;
        uint32_t dataSize = 0;

        CmsgBattlenetRequest() : ManagedPacket(CMSG_BATTLENET_REQUEST, 24) {}

        uint32_t getMethodId() const { return static_cast<uint32_t>(methodType & 0xFFFFFFFF); }
        uint32_t getServiceHash() const { return static_cast<uint32_t>(methodType >> 32); }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isTheWarWithinOrLater())
                return false;

            packet >> methodType >> objectId >> token >> dataSize;
            if (dataSize > 0xFFFF || packet.rpos() + dataSize > packet.size())
                return false;

            return true;
        }
    };
}
