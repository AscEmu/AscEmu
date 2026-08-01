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
    class SmsgSpellGo : public ManagedPacket
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

        std::vector<SpellUniqueTarget> hittedTargets {};
        std::vector<SpellTargetMod> missedTargets {};

        uint32_t powerType {0};

        Spell::ProjectileData projectile {0,0};

        uint8_t runeAvailableBefore {0};
        uint8_t currentRunes {0};

        float missilePitch {.0f};
        uint32_t missileTravelTime {0};

        SmsgSpellGo(WoWGuid casterGuid, WoWGuid casterUnitGuid, uint32_t spellId, uint32_t castFlags,
            uint8_t extraCastNumber, uint32_t timer, uint32_t castTime, SpellCastTargets targets) :
            ManagedPacket(SMSG_SPELL_GO, 0),
            casterGuid(casterGuid), casterUnitGuid(casterUnitGuid), spellId(spellId), castFlags(castFlags),
            extraCastNumber(extraCastNumber), timer(timer), castTime(castTime), targets(targets)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 8 + 4 + 4 + 1 + 4 + 4 + 4 + 4 + sizeof(SpellCastTargets) + (hittedTargets.size() * 8) + (missedTargets.size() * 8);
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
                    packet << uint16_t(castFlags);
                }

                if (m_protocol.expansion == WoW::Expansion::_Cata)
                    packet << uint32_t(timer);

                if (m_protocol.expansion > WoW::Expansion::_Classic)
                    packet << uint32_t(castTime);

                packet << uint8_t(hittedTargets.size());
                for (const auto& uniqueTarget : hittedTargets)
                    packet << uint64_t(uniqueTarget.first);

                packet << uint8_t(missedTargets.size());
                if (!missedTargets.empty())
                {
                    for (const auto& missedTarget : missedTargets)
                    {
                        packet << uint64_t(missedTarget.targetGuid);
                        packet << uint8_t(missedTarget.hitResult);

                        if (missedTarget.hitResult == SPELL_DID_HIT_REFLECT)
                            packet << uint8_t(missedTarget.extendedHitResult);
                    }
                }

                targets.write(packet);

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                    packet << uint32_t(powerType);

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                {
                    packet << uint32_t(projectile.displayInfo);
                    packet << uint32_t(projectile.inventoryType);
                }

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                {
                    if (castFlags & 0x200000) // SPELL_PACKET_FLAGS_RUNE_UPDATE
                    {
                        packet << uint8_t(runeAvailableBefore);
                        packet << uint8_t(currentRunes);
                        for (uint8_t i = 0; i < 6; ++i)
                        {
                            const uint8_t runeMask = 1U << i;
                            if ((runeMask & runeAvailableBefore) != (runeMask & currentRunes))
                                packet << uint8_t(0); // Value of the rune converted into byte
                        }
                    }

                    if (castFlags & 0x20000) // SPELL_PACKET_FLAGS_UPDATE_MISSILE
                    {
                        packet << float(missilePitch);
                        packet << uint32_t(missileTravelTime);
                    }

                    if (targets.hasDestination())
                        packet << uint8_t(0);
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
