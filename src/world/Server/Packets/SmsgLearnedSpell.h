/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLearnedSpell : public ManagedPacket
    {
    public:
        uint32_t spellId;
        int32_t traitDefinitionId;
        bool suppressMessaging;
        
        SmsgLearnedSpell() : SmsgLearnedSpell(0)
        {
        }

        SmsgLearnedSpell(uint32_t spellId, int32_t traitDefinitionId = 0, bool suppressMessaging = false) :
            ManagedPacket(SMSG_LEARNED_SPELL, 6),
            spellId(spellId),
            traitDefinitionId(traitDefinitionId),
            suppressMessaging(suppressMessaging)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion == WoW::Expansion::_Forever)
                return traitDefinitionId != 0 ? 22 : 18;

            return 6;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion == WoW::Expansion::_Forever)
            {
                // Modern LEARNED_SPELLS with one LearnedSpellInfo entry.
                packet << uint32_t(1);   // ClientLearnedSpellData count
                packet << uint32_t(0);   // SpecializationID
                packet << int32_t(-1);   // MinActionBarSlot
                packet << int32_t(spellId);
                packet.writeBit(false);  // Favorite
                packet.writeBit(false);  // EquipableSpellInvSlot present
                packet.writeBit(false);  // Superceded present
                packet.writeBit(traitDefinitionId != 0);  // TraitDefinitionID present
                packet.flushBits();
                if (traitDefinitionId != 0)
                    packet << int32_t(traitDefinitionId);
                packet.writeBit(suppressMessaging);  // SuppressMessaging
                packet.writeBit(false);  // TraitGrantedByAura
                packet.flushBits();
            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                packet.writeBits(1, 22);
                packet.writeBit(0);
                packet << spellId;
            }
            else
            {
                packet << spellId;

                if (m_protocol.expansion < WoW::Expansion::_Cata)
                {
                    packet << uint16_t(0);  //unknown
                }
                else
                {
                    packet << uint32_t(0);  //unknown
                }
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
