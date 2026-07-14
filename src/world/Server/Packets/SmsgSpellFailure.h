/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSpellFailure : public ManagedPacket
    {
    public:
        WoWGuid casterGuid;
        uint8_t castNumber;
        uint32_t spellId;
        uint8_t result;

        SmsgSpellFailure() : SmsgSpellFailure(WoWGuid(), 0, 0, 0)
        {
        }

        SmsgSpellFailure(WoWGuid casterGuid, uint8_t castNumber, uint32_t spellId, uint8_t result) :
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
                packet << result;

                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[5]);
            }
            else // < Mop
            {
                packet << casterGuid;
                if (m_protocol.expansion > WoW::Expansion::_TBC)
                    packet << castNumber;

                packet << spellId << result;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
