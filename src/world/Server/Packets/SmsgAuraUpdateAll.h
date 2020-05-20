/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgAuraUpdateAll : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid guid;

        struct AuraUpdate
        {
            uint8_t visualSlot;
            uint32_t spellId;
#if VERSION_STRING < Cata
            uint8_t flags;
#else
            uint16_t flags;
#endif
            uint8_t level;
            uint8_t stackCount;
            WoWGuid casterGuid;
            uint32_t duration;
            uint32_t timeLeft;
        };

        std::vector<AuraUpdate> aura_updates;

        SmsgAuraUpdateAll() : SmsgAuraUpdateAll(WoWGuid(), std::vector<AuraUpdate>())
        {
        }

        SmsgAuraUpdateAll(WoWGuid guid, std::vector<AuraUpdate> aura_updates) :
            ManagedPacket(SMSG_AURA_UPDATE_ALL, 200),
            guid(guid),
            aura_updates(move(aura_updates))
        {
        }

        void addAuraUpdate(AuraUpdate aura_update)
        {
            aura_updates.push_back(aura_update);
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid;

            for (const auto& auras : aura_updates)
            {
                packet << auras.visualSlot;
                packet << auras.spellId;

                packet << auras.flags;

                packet << auras.level;
                packet << auras.stackCount;

                if (!(auras.flags & 0x08))  // AFLAG_NOT_CASTER
                    packet << auras.casterGuid;

                if (auras.flags & 0x20)         // AFLAG_DURATION
                {
                    packet << auras.duration;
                    packet << auras.timeLeft;
                }
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
