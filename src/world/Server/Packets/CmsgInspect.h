/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgInspect : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgInspect() : CmsgInspect(0)
        {
        }

        CmsgInspect(uint64_t guid) :
            ManagedPacket(CMSG_INSPECT, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= Cata
            packet >> guid;
#else // Mop
            WoWGuid packetGuid;

            packetGuid[0] = packet.readBit();
            packetGuid[3] = packet.readBit();
            packetGuid[7] = packet.readBit();
            packetGuid[2] = packet.readBit();
            packetGuid[5] = packet.readBit();
            packetGuid[1] = packet.readBit();
            packetGuid[4] = packet.readBit();
            packetGuid[6] = packet.readBit();

            packet.ReadByteSeq(packetGuid[3]);
            packet.ReadByteSeq(packetGuid[5]);
            packet.ReadByteSeq(packetGuid[2]);
            packet.ReadByteSeq(packetGuid[4]);
            packet.ReadByteSeq(packetGuid[1]);
            packet.ReadByteSeq(packetGuid[6]);
            packet.ReadByteSeq(packetGuid[0]);
            packet.ReadByteSeq(packetGuid[7]);

            guid = packetGuid.getRawGuid();
#endif
            return true;
        }
    };
}
