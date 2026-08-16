/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/ItemProperties.hpp"
#include "Management/LFG/LFGMgr.hpp"
#include "Management/QuestProperties.hpp"
#include "Storage/MySQLDataStore.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLfgPlayerReward : public ManagedPacket
    {
    public:
        uint32_t randomDungeonEntry;
        uint32_t dungeonEntry;
        uint8_t done;
        LfgReward const* reward;
        QuestProperties const* qReward;

        SmsgLfgPlayerReward() : SmsgLfgPlayerReward(0, 0, 0, nullptr, nullptr)
        {
        }

        SmsgLfgPlayerReward(uint32_t randomDungeonEntry, uint32_t dungeonEntry, uint8_t done, LfgReward const* reward, QuestProperties const* qReward) :
            ManagedPacket(SMSG_LFG_PLAYER_REWARD, 0),
            randomDungeonEntry(randomDungeonEntry),
            dungeonEntry(dungeonEntry),
            done(done),
            reward(reward),
            qReward(qReward)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            const uint8_t itemNum = qReward ? uint8_t(qReward->GetRewardItemCount()) : 0;
            return 4 + 4 + 1 + 4 + 4 + 4 + 4 + 4 + 1 + itemNum * (4 + 4 + 4 + 1);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!qReward)
                return false;

            if (m_protocol.isMop())
            {
                // BuildQuestReward: currency rewards are combined with item rewards into a single
                // count/list, but AscEmu's QuestProperties doesn't track quest currency rewards, so
                // only the item half is ever emitted here (documented gap, not a format error).
                const uint8_t rewCount = uint8_t(qReward->GetRewardItemCount());

                packet << uint32_t(randomDungeonEntry);                           // Random Dungeon Finished
                packet << uint32_t(dungeonEntry);                                 // Dungeon Finished
                packet << uint32_t(qReward->reward_money);                        // Money reward
                packet << uint32_t(qReward->reward_xp);                           // XP reward
                packet << uint8_t(rewCount);

                if (rewCount)
                {
                    for (uint8_t i = 0; i < 4; ++i)
                    {
                        if (!qReward->reward_item[i])
                            continue;

                        auto itemProperties = sMySQLStore.getItemProperties(qReward->reward_item[i]);

                        packet << uint32_t(qReward->reward_item[i]);
                        packet << uint32_t(itemProperties ? itemProperties->DisplayInfoID : 0);
                        packet << uint32_t(qReward->reward_itemcount[i]);
                        packet << uint8_t(0);                                     // Is currency
                    }
                }
                return true;
            }
            else if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                const uint8_t itemNum = uint8_t(qReward->GetRewardItemCount());

                packet << uint32_t(randomDungeonEntry);                               // Random Dungeon Finished
                packet << uint32_t(dungeonEntry);                                     // Dungeon Finished
                packet << uint8_t(done);
                packet << uint32_t(1);
                packet << uint32_t(qReward->reward_money);
                packet << uint32_t(qReward->reward_xp);
                packet << uint32_t(reward->reward[done].variableMoney);
                packet << uint32_t(reward->reward[done].variableXP);
                packet << uint8_t(itemNum);

                if (itemNum)
                {
                    for (uint8_t i = 0; i < 4; ++i)
                    {
                        if (!qReward->reward_item[i])
                            continue;

                        auto itemProperties = sMySQLStore.getItemProperties(qReward->reward_item[i]);

                        packet << uint32_t(qReward->reward_item[i]);
                        packet << uint32_t(itemProperties ? itemProperties->DisplayInfoID : 0);
                        packet << uint32_t(qReward->reward_itemcount[i]);
                    }
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
