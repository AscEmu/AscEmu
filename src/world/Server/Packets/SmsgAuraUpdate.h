/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgAuraUpdate : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid guid;
        bool remove;

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

        AuraUpdate aura_updates;

        SmsgAuraUpdate() : SmsgAuraUpdate(WoWGuid(), {0}, false)
        {
        }

        SmsgAuraUpdate(WoWGuid guid, AuraUpdate aura_updates, bool remove = false) :
            ManagedPacket(SMSG_AURA_UPDATE, 30),
            guid(guid),
            aura_updates(aura_updates),
            remove(remove)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid;
            packet << aura_updates.visualSlot;

            if (remove)
            {
                packet << uint32_t(0);
            }
            else
            {
                packet << aura_updates.spellId;

                packet << aura_updates.flags;

                packet << aura_updates.level;
                packet << aura_updates.stackCount;

                if (!(aura_updates.flags & 0x08))  // AFLAG_NOT_CASTER
                    packet << aura_updates.casterGuid;

                if (aura_updates.flags & 0x20)         // AFLAG_DURATION
                {
                    packet << aura_updates.duration;
                    packet << aura_updates.timeLeft;
                }
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
