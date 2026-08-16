/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGuildXp : public ManagedPacket
    {
    public:
        uint64_t experience = 0;
        uint64_t weekActivity = 0;
        uint64_t totalActivity = 0;
        uint64_t xpForNextLevelRemaining = 0;

        SmsgGuildXp() : SmsgGuildXp(0, 0, 0, 0)
        {
        }

        SmsgGuildXp(uint64_t experience, uint64_t weekActivity, uint64_t totalActivity, uint64_t xpForNextLevelRemaining) :
            ManagedPacket(SMSG_GUILD_XP, 32),
            experience(experience), weekActivity(weekActivity), totalActivity(totalActivity), xpForNextLevelRemaining(xpForNextLevelRemaining)
        {
        }

    protected:
        size_t expectedSize() const override { return 32; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                return false;
            }
            else if (m_protocol.isCata())
            {
                packet << uint64_t(totalActivity);
                packet << uint64_t(xpForNextLevelRemaining);
                packet << uint64_t(0);      // unk
                packet << uint64_t(weekActivity);
                packet << uint64_t(experience);
                
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet << uint64_t(experience);
                packet << uint64_t(weekActivity);
                packet << uint64_t(totalActivity);
                packet << uint64_t(xpForNextLevelRemaining);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
