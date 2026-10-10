/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "ForeverSpellPacketUtils.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSpellFailure : public ManagedPacket
    {
    public:
        WoWGuid casterGuid;
        uint8_t castNumber;
        uint32_t spellId;
        int32_t result;
        WoWGuid castId = WoWGuid::createModernEmpty();
        uint32_t spellXSpellVisualId = 0;
        uint32_t scriptVisualId = 0;
        WoWGuid failedBy = WoWGuid::createModernEmpty();
        uint16_t mapId = 0;

        SmsgSpellFailure() : SmsgSpellFailure(WoWGuid(), 0, 0, 0)
        {
        }

        SmsgSpellFailure(WoWGuid casterGuid, uint8_t castNumber, uint32_t spellId, int32_t result) :
            ManagedPacket(SMSG_SPELL_FAILURE, 8 + 4),
            casterGuid(casterGuid),
            castNumber(castNumber),
            spellId(spellId),
            result(result)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                ForeverSpellPacket::writePackedGuid(packet, ForeverSpellPacket::toModernGuid(casterGuid, m_protocol.realmId, mapId));
                ForeverSpellPacket::writePackedGuid(packet, castId);
                packet << static_cast<int32_t>(spellId);
                packet << static_cast<int32_t>(spellXSpellVisualId);
                packet << static_cast<int32_t>(scriptVisualId);
                packet << static_cast<uint16_t>(result);
                ForeverSpellPacket::writePackedGuid(packet, ForeverSpellPacket::toModernGuid(failedBy, m_protocol.realmId, mapId));
                return true;
            }

            if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                WoWGuid guid = casterGuid.getRawGuid();
                packet.writeBit(guid[7]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[4]);

                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[1]);

                packet << castNumber;
                packet << spellId;
                packet << static_cast<uint8_t>(result);

                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[5]);
            }
            else // < Mop
            {
                packet << casterGuid;
                if (m_protocol.expansion > WoW::Expansion::_TBC)
                    packet << castNumber;

                packet << spellId << static_cast<uint8_t>(result);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
