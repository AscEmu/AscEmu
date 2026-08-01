/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Spell/SpellCastTargets.hpp"
#include "Spell/Spell.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSpellStart : public ManagedPacket
    {
    public:
        WoWGuid casterGuid;
        WoWGuid casterUnitGuid;

        uint32_t spellId;
        uint32_t castFlags;
        uint8_t extraCastNumber;

        uint32_t timer;
        uint32_t castTime;

        SpellCastTargets targets;

        uint32_t powerType {0};

        Spell::ProjectileData projectile {0,0};

        SmsgSpellStart(WoWGuid casterGuid, WoWGuid casterUnitGuid, uint32_t spellId, uint32_t castFlags,
            uint8_t extraCastNumber, uint32_t timer, uint32_t castTime, SpellCastTargets targets) :
            ManagedPacket(SMSG_SPELL_START, 0),
            casterGuid(casterGuid), casterUnitGuid(casterUnitGuid), spellId(spellId), castFlags(castFlags),
            extraCastNumber(extraCastNumber), timer(timer), castTime(castTime), targets(targets)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 8 + 4 + 4 + 1 + 4 + 4 + 4 + 4 + sizeof(SpellCastTargets);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << casterGuid;
                packet << casterUnitGuid;

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                {
                    packet << uint8_t(extraCastNumber);
                    packet << uint32_t(spellId);
                    packet << uint32_t(castFlags);
                }
                else //< WotLK
                {
                    packet << uint32_t(spellId);

                    if (m_protocol.expansion > WoW::Expansion::_Classic)
                        packet << uint8_t(extraCastNumber);

                    packet << uint16_t(castFlags);
                }

                packet << uint32_t(timer);

                if (m_protocol.expansion == WoW::Expansion::_Cata)
                    packet << uint32_t(castTime);

                targets.write(packet);

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                    packet << uint32_t(powerType);

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                {
                    packet << uint32_t(projectile.displayInfo);
                    packet << uint32_t(projectile.inventoryType);
                }
            }
            else
            {
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
