/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgTurnInPetition : public ManagedPacket
    {
    public:
        WoWGuid itemGuid;

        // arena fields
        uint32_t iconColor = 0;
        uint32_t icon = 0;
        uint32_t borderColor = 0;
        uint32_t border = 0;
        uint32_t background = 0;

        CmsgTurnInPetition() : CmsgTurnInPetition(0)
        {
        }

        CmsgTurnInPetition(uint64_t itemGuid) :
            ManagedPacket(CMSG_TURN_IN_PETITION, 8),
            itemGuid(itemGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                itemGuid.init(unpackedGuid);

                if (packet.size() >= 28)
                    packet >> iconColor >> icon >> borderColor >> border >> background;

                return true;
            }
            else if (m_protocol.isMop())
            {
                // Mop's client no longer sends arena team emblem data through this opcode
                itemGuid[1] = packet.readBit();
                itemGuid[2] = packet.readBit();
                itemGuid[3] = packet.readBit();
                itemGuid[0] = packet.readBit();
                itemGuid[5] = packet.readBit();
                itemGuid[7] = packet.readBit();
                itemGuid[4] = packet.readBit();
                itemGuid[6] = packet.readBit();

                packet.readByteSeq(itemGuid[2]);
                packet.readByteSeq(itemGuid[1]);
                packet.readByteSeq(itemGuid[4]);
                packet.readByteSeq(itemGuid[6]);
                packet.readByteSeq(itemGuid[0]);
                packet.readByteSeq(itemGuid[7]);
                packet.readByteSeq(itemGuid[5]);
                packet.readByteSeq(itemGuid[3]);
                return true;
            }

            return false;
        }
    };
}
