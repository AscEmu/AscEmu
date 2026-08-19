/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgLoot : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgLoot() : CmsgLoot(0)
        {
        }

        CmsgLoot(uint64_t guid) :
            ManagedPacket(CMSG_LOOT, 8),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                WoWGuid targetGuid;
                targetGuid[4] = packet.readBit();
                targetGuid[5] = packet.readBit();
                targetGuid[2] = packet.readBit();
                targetGuid[7] = packet.readBit();
                targetGuid[0] = packet.readBit();
                targetGuid[1] = packet.readBit();
                targetGuid[3] = packet.readBit();
                targetGuid[6] = packet.readBit();

                packet.readByteSeq(targetGuid[3]);
                packet.readByteSeq(targetGuid[5]);
                packet.readByteSeq(targetGuid[0]);
                packet.readByteSeq(targetGuid[6]);
                packet.readByteSeq(targetGuid[4]);
                packet.readByteSeq(targetGuid[1]);
                packet.readByteSeq(targetGuid[7]);
                packet.readByteSeq(targetGuid[2]);

                guid = targetGuid.getRawGuid();

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> guid;

                return true;
            }

            if (guid == 0)
                return false;

            return true;
        }
    };
}
