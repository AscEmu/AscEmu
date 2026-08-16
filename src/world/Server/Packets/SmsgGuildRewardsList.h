/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Guild/GuildDefinitions.hpp"
#include <cstdint>
#include <ctime>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgGuildRewardsList : public ManagedPacket
    {
    public:
        std::vector<GuildReward> rewards;

        SmsgGuildRewardsList() : SmsgGuildRewardsList(std::vector<GuildReward>{})
        {
        }

        SmsgGuildRewardsList(std::vector<GuildReward> rewards) :
            ManagedPacket(SMSG_GUILD_REWARDS_LIST, 3 + rewards.size() * (4 + 4 + 4 + 8 + 4 + 4)),
            rewards(std::move(rewards))
        {
        }

    protected:
        size_t expectedSize() const override { return 3 + rewards.size() * (4 + 4 + 4 + 8 + 4 + 4); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.writeBits(rewards.size(), 21);
                packet.flushBits();

                for (uint32_t i = 0; i < rewards.size(); ++i)
                {
                    packet << uint32_t(rewards[i].standing);
                    packet << int32_t(rewards[i].racemask);
                    packet << uint32_t(rewards[i].entry);
                    packet << uint64_t(rewards[i].price);
                    packet << uint32_t(0);
                    packet << uint32_t(rewards[i].achievementId);
                }
                packet << uint32_t(time(nullptr));

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBits(rewards.size(), 19);

                for (uint32_t i = 0; i < rewards.size(); ++i)
                    packet.writeBits(1, 22);    // achievementcount

                packet.flushBits();

                for (uint32_t i = 0; i < rewards.size(); ++i)
                {
                    packet << uint32_t(rewards[i].achievementId);
                    packet << int32_t(rewards[i].racemask);
                    packet << uint32_t(rewards[i].entry);
                    packet << uint32_t(0);
                    packet << uint32_t(rewards[i].standing);
                    packet << uint64_t(rewards[i].price);
                }
                packet << uint32_t(time(nullptr));

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
