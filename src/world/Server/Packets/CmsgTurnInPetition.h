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

        bool deserialise(WorldPacket& packet) override
        {
            if (packet.remaining() < expectedSize())
                return false;

            return internalDeserialise(packet);
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop())
                return 1; // packed guid: mask byte + only the non-zero guid bytes (1..9 bytes)
            return 0;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                WoWGuid128 item;
                packet >> item;
                itemGuid = WoWGuid::fromGuid128(item);
                return true;
            }

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
