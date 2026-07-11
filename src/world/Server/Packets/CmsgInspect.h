/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> guid;
            }
            else // Mop
            {
                WoWGuid packetGuid;

                packetGuid[0] = packet.readBit();
                packetGuid[3] = packet.readBit();
                packetGuid[7] = packet.readBit();
                packetGuid[2] = packet.readBit();
                packetGuid[5] = packet.readBit();
                packetGuid[1] = packet.readBit();
                packetGuid[4] = packet.readBit();
                packetGuid[6] = packet.readBit();

                packet.readByteSeq(packetGuid[3]);
                packet.readByteSeq(packetGuid[5]);
                packet.readByteSeq(packetGuid[2]);
                packet.readByteSeq(packetGuid[4]);
                packet.readByteSeq(packetGuid[1]);
                packet.readByteSeq(packetGuid[6]);
                packet.readByteSeq(packetGuid[0]);
                packet.readByteSeq(packetGuid[7]);

                guid = packetGuid.getRawGuid();
            }
            return true;
        }
    };
}
