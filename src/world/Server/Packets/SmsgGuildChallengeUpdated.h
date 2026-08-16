/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Guild/GuildDefinitions.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGuildChallengeUpdated : public ManagedPacket
    {
    public:
        SmsgGuildChallengeUpdated() :
            ManagedPacket(SMSG_GUILD_CHALLENGE_UPDATED, 4 * 6 * 5)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 * 6 * 5; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                return false;
            }
            else if (m_protocol.isCata())
            {
                for (int i = 0; i < 4; ++i)
                    packet << uint32_t(guildChallengeXPReward[i]);

                for (int i = 0; i < 4; ++i)
                    packet << uint32_t(guildChallengeGoldReward[i]);

                for (int i = 0; i < 4; ++i)
                    packet << uint32_t(guildChallengeWeeklyMaximum[i]);

                for (int i = 0; i < 4; ++i)
                    packet << uint32_t(guildChallengeMaxLevelGoldReward[i]);

                for (int i = 0; i < 4; ++i)
                    packet << uint32_t(0); // progress - not yet implemented

                return true;
            }
            else if (m_protocol.isMop())
            {
                for (int i = 0; i < 6; ++i)
                    packet << uint32_t(guildChallengeWeeklyMaximum[i]);

                for (int i = 0; i < 6; ++i)
                    packet << uint32_t(guildChallengeGoldReward[i]);

                for (int i = 0; i < 6; ++i)
                    packet << uint32_t(guildChallengeMaxLevelGoldReward[i]);

                for (int i = 0; i < 6; ++i)
                    packet << uint32_t(guildChallengeXPReward[i]);

                for (int i = 0; i < 6; ++i)
                    packet << uint32_t(0); // progress - not yet implemented

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
