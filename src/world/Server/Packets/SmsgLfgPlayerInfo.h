/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "LfgPacketHelpers.h"
#include "Management/ItemProperties.hpp"
#include "Management/LFG/LFGMgr.hpp"
#include "Management/QuestProperties.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "WoWGuid.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLfgPlayerInfo : public ManagedPacket
    {
    public:
        LfgDungeonSet randomDungeons;
        LfgLockMap lock;
        uint8_t level;
        Player* player;

        SmsgLfgPlayerInfo() : SmsgLfgPlayerInfo({}, {}, 0, nullptr)
        {
        }

        SmsgLfgPlayerInfo(LfgDungeonSet randomDungeons, LfgLockMap lock, uint8_t level, Player* player) :
            ManagedPacket(SMSG_LFG_PLAYER_INFO, 0),
            randomDungeons(std::move(randomDungeons)),
            lock(std::move(lock)),
            level(level),
            player(player)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            const uint32_t rsize = uint32_t(randomDungeons.size());
            const uint32_t lsize = uint32_t(lock.size());
            return 1 + rsize * (4 + 1 + 4 + 4 + 4 + 4 + 1 + 4 + 4 + 4) + 4 + lsize * (1 + 4 + 4 + 4 + 4 + 1 + 4 + 4 + 4);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!player)
                return false;

            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                const auto expansion = m_protocol.expansion;
                packet << uint32_t(randomDungeons.size());
                Lfg::writeBlackList(packet, m_protocol, m_receiverMapId, player->getGuid(), lock);
                for (auto randomDungeon : randomDungeons)
                {
                    LfgReward const* reward = sLfgMgr.GetRandomDungeonReward(randomDungeon, level);
                    QuestProperties const* qRew = nullptr;
                    uint8_t done = 0;
                    if (reward)
                    {
                        qRew = sMySQLStore.getQuestProperties(reward->reward[0].questId);
                        if (qRew)
                        {
                            done = player->hasQuestFinished(qRew->id);
                            if (done)
                                qRew = sMySQLStore.getQuestProperties(reward->reward[1].questId);
                        }
                    }
                    const bool firstReward = qRew && !done;
                    const bool shortageEligible = player->getGroup() == nullptr;

                    const auto writeRewards = [&]()
                    {
                        if (expansion >= WoW::Expansion::_Dragonflight)
                            packet << uint8_t(0);                               // role mask
                        else
                            packet << uint32_t(0);
                        packet << int32_t(qRew ? qRew->reward_money : 0);
                        packet << int32_t(qRew ? qRew->reward_xp : 0);
                        packet << uint32_t(qRew ? qRew->GetRewardItemCount() : 0);
                        packet << uint32_t(0);                                  // currencies
                        packet << uint32_t(0);                                  // bonus currencies
                        if (qRew)
                        {
                            for (uint8_t i = 0; i < 4; ++i)
                            {
                                if (!qRew->reward_item[i])
                                    continue;
                                packet << int32_t(qRew->reward_item[i]);
                                packet << int32_t(qRew->reward_itemcount[i]);
                            }
                        }
                        // optional spell, artifact and honor values, 6.x has only one
                        packet.writeBit(false);
                        if (expansion >= WoW::Expansion::_Legion)
                        {
                            packet.writeBit(false);
                            packet.writeBit(false);
                            packet.writeBit(false);
                        }
                        packet.flushBits();
                    };

                    packet << uint32_t(randomDungeon);                          // slot
                    packet << int32_t(1);                                       // completion quantity
                    packet << int32_t(1);                                       // completion limit
                    packet << int32_t(0);                                       // completion currency
                    packet << int32_t(0);                                       // specific quantity
                    packet << int32_t(1);                                       // specific limit
                    packet << int32_t(0);                                       // overall quantity
                    packet << int32_t(1);                                       // overall limit
                    packet << int32_t(0);                                       // purse weekly quantity
                    packet << int32_t(0);                                       // purse weekly limit
                    packet << int32_t(0);                                       // purse quantity
                    packet << int32_t(0);                                       // purse limit
                    packet << int32_t(1);                                       // quantity
                    packet << uint32_t(0);                                      // completed mask
                    if (expansion >= WoW::Expansion::_Legion)
                        packet << uint32_t(0);                                  // encounter mask
                    packet << uint32_t(0);                                      // shortage rewards

                    // 12.x writes the two flags behind the rewards
                    if (expansion >= WoW::Expansion::_Midnight)
                    {
                        writeRewards();
                        packet.writeBit(firstReward);
                        packet.writeBit(shortageEligible);
                        packet.flushBits();
                    }
                    else
                    {
                        packet.writeBit(firstReward);
                        packet.writeBit(shortageEligible);
                        packet.flushBits();
                        writeRewards();
                    }
                }
                return true;
            }

            if (m_protocol.isMop())
            {
                WoWGuid playerGuid = player->getGuid();
                constexpr bool hasPlayerGuid = true;

                packet.writeBits(lock.size(), 20);                     // locksize count
                packet.writeBit(hasPlayerGuid);
                packet.writeBits(randomDungeons.size(), 17);

                ByteBuffer rewardData;
                for (auto randomDungeon : randomDungeons)
                {
                    LfgReward const* reward = sLfgMgr.GetRandomDungeonReward(randomDungeon, level);
                    QuestProperties const* qRew = nullptr;
                    uint8_t done = 0;
                    if (reward)
                    {
                        qRew = sMySQLStore.getQuestProperties(reward->reward[0].questId);
                        if (qRew)
                        {
                            done = player->hasQuestFinished(qRew->id);
                            if (done)
                                qRew = sMySQLStore.getQuestProperties(reward->reward[1].questId);
                        }
                    }

                    const bool firstReward = qRew && !done;
                    const bool shortageEligible = player->getGroup() == nullptr;
                    constexpr uint8_t shortageCount = 0;               // role shortage bonus is not implemented

                    packet.writeBit(firstReward);
                    packet.writeBit(shortageEligible);
                    packet.writeBits(0, 21);                           // Additional currency count (not supported)
                    packet.writeBits(shortageCount, 19);
                    packet.writeBits(qRew ? qRew->GetRewardItemCount() : 0, 20);
                    packet.writeBits(0, 21);                           // Reward currency count (not supported)

                    rewardData << uint32_t(qRew ? qRew->reward_xp : 0);
                    rewardData << uint32_t(0);                         // SpecificQuantity
                    rewardData << uint32_t(0);                         // PurseLimit
                    rewardData << uint32_t(qRew ? qRew->reward_money : 0);

                    if (qRew && qRew->GetRewardItemCount())
                    {
                        for (uint8_t i = 0; i < 4; ++i)
                        {
                            if (!qRew->reward_item[i])
                                continue;

                            ItemProperties const* item = sMySQLStore.getItemProperties(qRew->reward_item[i]);
                            rewardData << uint32_t(qRew->reward_itemcount[i]);
                            rewardData << uint32_t(qRew->reward_item[i]);
                            rewardData << uint32_t(item ? item->DisplayInfoID : 0);
                        }
                    }

                    rewardData << uint32_t(0);                         // OverallQuantity
                    rewardData << uint32_t(0);                         // PurseWeeklyQuantity
                    rewardData << uint32_t(1);                         // OverallLimit
                    rewardData << uint32_t(1);                         // Quantity
                    rewardData << uint32_t(0);                         // CompletionCurrencyID
                    rewardData << uint32_t(randomDungeon);             // Dungeon Entry (id + type)
                    rewardData << uint32_t(0);                         // PurseWeeklyLimit
                    rewardData << uint32_t(0);                         // Mask
                    rewardData << uint32_t(0);                         // PurseQuantity
                    rewardData << uint32_t(1);                         // CompletionLimit
                    rewardData << uint32_t(1);                         // SpecificLimit
                    rewardData << uint32_t(0);                         // CompletedMask
                    rewardData << uint32_t(1);                         // CompletionQuantity
                }

                packet.writeBit(playerGuid[5]);
                packet.writeBit(playerGuid[1]);
                packet.writeBit(playerGuid[2]);
                packet.writeBit(playerGuid[7]);
                packet.writeBit(playerGuid[3]);
                packet.writeBit(playerGuid[0]);
                packet.writeBit(playerGuid[6]);
                packet.writeBit(playerGuid[4]);

                packet.flushBits();

                packet.writeByteSeq(playerGuid[7]);
                packet.writeByteSeq(playerGuid[2]);
                packet.writeByteSeq(playerGuid[3]);
                packet.writeByteSeq(playerGuid[0]);
                packet.writeByteSeq(playerGuid[4]);
                packet.writeByteSeq(playerGuid[5]);
                packet.writeByteSeq(playerGuid[6]);
                packet.writeByteSeq(playerGuid[1]);

                packet.append(rewardData);

                for (const auto& lockEntry : lock)
                {
                    packet << uint32_t(lockEntry.first);               // Dungeon entry (id + type)
                    packet << uint32_t(lockEntry.second);               // Lock status
                    packet << uint32_t(0);                              // Current itemLevel (not tracked)
                    packet << uint32_t(0);                              // Required itemLevel (not tracked)
                }

                return true;
            }
            else if (m_protocol.isCata())
            {
                // Valor Points weekly-cap tracking and the Call to Arms role-bonus system aren't
                // implemented in AscEmu, so those fields are sent as their "not eligible"/zero
                // shape - the wire structure matches the client's expectations, the values don't.
                packet << uint8_t(randomDungeons.size());                  // Random Dungeon count

                for (auto randomDungeon : randomDungeons)
                {
                    packet << uint32_t(randomDungeon);                     // Dungeon Entry (id + type)

                    LfgReward const* reward = sLfgMgr.GetRandomDungeonReward(randomDungeon, level);
                    QuestProperties const* qRew = nullptr;
                    uint8_t done = 0;
                    if (reward)
                    {
                        qRew = sMySQLStore.getQuestProperties(reward->reward[0].questId);
                        if (qRew)
                        {
                            done = player->hasQuestFinished(qRew->id);
                            if (done)
                                qRew = sMySQLStore.getQuestProperties(reward->reward[1].questId);
                        }
                    }

                    packet << uint8_t(qRew != nullptr && !done);            // first reward still open

                    // completion, specific, overall, purse and quantity counters (13 values), not tracked
                    for (uint8_t i = 0; i < 13; ++i)
                        packet << uint32_t(0);

                    packet << uint8_t(0);                                   // Call to Arms eligible - not implemented
                    for (uint8_t i = 0; i < 3; ++i)
                        packet << uint32_t(0);                              // Call to Arms role bonus - not implemented

                    if (qRew)
                    {
                        packet << uint32_t(qRew->reward_money);
                        packet << uint32_t(qRew->reward_xp);
                        packet << uint8_t(qRew->GetRewardItemCount());
                        for (uint8_t i = 0; i < 4; ++i)
                        {
                            if (!qRew->reward_item[i])
                                continue;

                            ItemProperties const* item = sMySQLStore.getItemProperties(qRew->reward_item[i]);
                            packet << uint32_t(qRew->reward_item[i]);
                            packet << uint32_t(item ? item->DisplayInfoID : 0);
                            packet << uint32_t(qRew->reward_itemcount[i]);
                            packet << uint8_t(0);                           // an item, not a currency
                        }
                    }
                    else
                    {
                        packet << uint32_t(0);                              // Money
                        packet << uint32_t(0);                              // XP
                        packet << uint8_t(0);                               // Reward count
                    }
                }

                // BuildPlayerLockDungeonBlock: entry, status, required and current item level
                packet << uint32_t(lock.size());
                for (const auto& lockEntry : lock)
                {
                    packet << uint32_t(lockEntry.first);
                    packet << uint32_t(lockEntry.second);
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                }

                return true;
            }
            else if (m_protocol.expansion == WoW::Expansion::_WotLK)
            {
                packet << uint8_t(randomDungeons.size());                  // Random Dungeon count
                for (auto randomDungeon : randomDungeons)
                {
                    packet << uint32_t(randomDungeon);                     // Dungeon Entry (id + type)
                    LfgReward const* reward = sLfgMgr.GetRandomDungeonReward(randomDungeon, level);
                    QuestProperties const* qRew = nullptr;
                    uint8_t done = 0;
                    if (reward)
                    {
                        qRew = sMySQLStore.getQuestProperties(reward->reward[0].questId);
                        if (qRew)
                        {
                            done = player->hasQuestFinished(qRew->id);
                            if (done)
                                qRew = sMySQLStore.getQuestProperties(reward->reward[1].questId);
                        }
                    }
                    if (qRew)
                    {
                        packet << uint8_t(done);
                        packet << uint32_t(qRew->reward_money);
                        packet << uint32_t(qRew->reward_xp);
                        packet << uint32_t(reward->reward[done].variableMoney);
                        packet << uint32_t(reward->reward[done].variableXP);
                        // todo FIXME Linux: error: cast from const uint32_t* {aka const unsigned int*} to uint8_t {aka unsigned char} loses precision
                        // can someone check this now ?

                        packet << uint8_t(qRew->GetRewardItemCount());
                        for (uint8_t i = 0; i < 4; ++i)
                            if (qRew->reward_item[i] != 0)
                            {
                                ItemProperties const* item = sMySQLStore.getItemProperties(qRew->reward_item[i]);
                                packet << uint32_t(qRew->reward_item[i]);
                                packet << uint32_t(item ? item->DisplayInfoID : 0);
                                packet << uint32_t(qRew->reward_itemcount[i]);
                            }
                    }
                    else
                    {
                        packet << uint8_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint8_t(0);
                    }
                }

                // BuildPlayerLockDungeonBlock
                packet << uint32_t(lock.size());
                for (const auto& lockEntry : lock)
                {
                    packet << uint32_t(lockEntry.first);
                    packet << uint32_t(lockEntry.second);
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
