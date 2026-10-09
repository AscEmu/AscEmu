/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSpellFailedOther : public ManagedPacket
    {
    public:
        WoWGuid casterGuid;
        uint8_t castNumber;
        uint32_t spellId;
        uint8_t result;

        SmsgSpellFailedOther() : SmsgSpellFailedOther(WoWGuid(), 0, 0, 0)
        {
        }

        SmsgSpellFailedOther(WoWGuid casterGuid, uint8_t castNumber, uint32_t spellId, uint8_t result) :
            ManagedPacket(SMSG_SPELL_FAILED_OTHER, 8 + 4),
            casterGuid(casterGuid),
            castNumber(castNumber),
            spellId(spellId),
            result(result)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD())
            {
                // caster, cast count, spell, reason
                packet << casterGuid.toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << uint8_t(castNumber);
                packet << uint32_t(spellId);
                packet << uint8_t(result);
                return true;
            }

            if (m_protocol.isLegion() || m_protocol.isBfA())
            {
                // caster, cast guid, spell, visual, reason
                packet << casterGuid.toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << WoWGuid128();
                packet << uint32_t(spellId);
                packet << uint32_t(0);
                packet << uint8_t(result);
                return true;
            }

            packet << casterGuid;

            if (m_protocol.expansion > WoW::Expansion::_TBC)
                packet << castNumber;

            packet << spellId << result;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
