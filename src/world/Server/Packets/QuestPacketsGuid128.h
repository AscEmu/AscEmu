/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "QuestPacketCommon.h"

#include <cstdint>

namespace AscEmu::Packets
{
    // an item without bonuses and modifiers in the layout of the client
    inline void writeEmptyItemInstance(WorldPacket& packet, WoW::ClientProtocol const& protocol, int32_t itemId)
    {
        packet << int32_t(itemId);

        if (protocol.isWoD() || protocol.isLegion())
        {
            packet << int32_t(0);                   // random properties seed
            packet << int32_t(0);                   // random properties
            packet.writeBit(false);                 // item bonus
            packet.writeBit(false);                 // modifications
            packet.flushBits();
        }
        else if (protocol.isBfA())
        {
            packet.writeBit(false);                 // item bonus
            packet.writeBit(false);                 // modifications
            packet.flushBits();
        }
        else if (protocol.isMidnight())
        {
            packet.writeBits(0, 7);                 // modifications
            packet.flushBits();
            packet.writeBit(false);                 // item bonus
            packet.flushBits();
        }
        else
        {
            packet.writeBit(false);                 // item bonus
            packet.flushBits();
            packet.writeBits(0, 6);                 // modifications
            packet.flushBits();
        }
    }

    // 9.x and later: a reward choice with its loot type, 11.x and later with context flags
    inline void writeQuestChoiceItem(WorldPacket& packet, WoW::ClientProtocol const& protocol, QuestRewardItemEntry const& item)
    {
        packet.writeBits(0, 2);                     // loot type: item
        if (protocol.isTheWarWithinOrLater())
            packet.writeBit(false);                 // context flags
        writeEmptyItemInstance(packet, protocol, int32_t(item.itemId));
        packet << int32_t(item.count);
    }

    inline void writeQuestRewardFactions(WorldPacket& packet, QuestRewardsData const& rewards, bool withCap)
    {
        packet << int32_t(0);                       // faction flags
        for (uint8_t i = 0; i < 5; ++i)
        {
            packet << int32_t(rewards.factionId[i]);
            packet << int32_t(0);                   // value from the reward table
            packet << int32_t(rewards.factionValue[i] * 100);
            if (withCap)
                packet << int32_t(0);               // cap
        }
    }

    inline void writeQuestRewards(WorldPacket& packet, WoW::ClientProtocol const& protocol, QuestRewardsData const& rewards)
    {
        if (protocol.isWoD())
        {
            packet << int32_t(rewards.choiceItemCount);
            for (auto const& item : rewards.choiceItems)
                packet << int32_t(item.itemId) << int32_t(item.count);
            packet << int32_t(rewards.itemCount);
            for (auto const& item : rewards.items)
                packet << int32_t(item.itemId) << int32_t(item.count);
            packet << int32_t(rewards.money);
            packet << int32_t(rewards.xp);
            packet << int32_t(rewards.title);
            packet << int32_t(rewards.talents);
            writeQuestRewardFactions(packet, rewards, false);
            packet << int32_t(rewards.displaySpell);
            packet << int32_t(rewards.spell);
            for (uint8_t i = 0; i < 4; ++i)
                packet << int32_t(rewards.currencyId[i]) << int32_t(rewards.currencyCount[i]);
            packet << int32_t(0);                   // skill line
            packet << int32_t(0);                   // skill ups
            packet.writeBit(false);
            packet.flushBits();
            return;
        }

        if (protocol.isLegion())
        {
            packet << int32_t(rewards.choiceItemCount);
            for (auto const& item : rewards.choiceItems)
                packet << int32_t(item.itemId) << int32_t(item.count);
            packet << int32_t(rewards.itemCount);
            for (auto const& item : rewards.items)
                packet << int32_t(item.itemId) << int32_t(item.count);
            packet << int32_t(rewards.money);
            packet << int32_t(rewards.xp);
            packet << int64_t(0);                   // artifact xp
            packet << int32_t(0);                   // artifact category
            packet << int32_t(rewards.honor);
            packet << int32_t(rewards.title);
            writeQuestRewardFactions(packet, rewards, true);
            packet << int32_t(rewards.displaySpell);
            packet << int32_t(0);
            packet << int32_t(0);
            packet << int32_t(rewards.spell);
            for (uint8_t i = 0; i < 4; ++i)
                packet << int32_t(rewards.currencyId[i]) << int32_t(rewards.currencyCount[i]);
            packet << int32_t(0);                   // skill line
            packet << int32_t(0);                   // skill ups
            packet << int32_t(0);                   // reward id
            packet.writeBit(false);                 // boost spell
            packet.flushBits();
            return;
        }

        if (protocol.isTheWarWithinOrLater())
        {
            // 11.x: items first, currencies behind the treasure pickers, choices at the end;
            // 12.x: items, currencies and choices first
            for (auto const& item : rewards.items)
            {
                packet << int32_t(item.itemId) << int32_t(item.count);
                packet.writeBit(false);             // context flags
                packet.flushBits();
            }

            auto writeCurrencies = [&]()
            {
                for (uint8_t i = 0; i < 4; ++i)
                {
                    packet << int32_t(rewards.currencyId[i]) << int32_t(rewards.currencyCount[i]);
                    packet << int32_t(0);           // bonus quantity
                    packet.writeBit(false);         // context flags
                    packet.flushBits();
                }
            };

            if (protocol.isMidnight())
            {
                writeCurrencies();
                packet << int32_t(rewards.choiceItemCount);
                for (auto const& item : rewards.choiceItems)
                    writeQuestChoiceItem(packet, protocol, item);
                packet << int32_t(rewards.itemCount);
            }
            else
            {
                packet << int32_t(rewards.choiceItemCount);
                packet << int32_t(rewards.itemCount);
            }

            packet << int32_t(rewards.money);
            packet << int32_t(rewards.xp);
            packet << int64_t(0);                   // artifact xp
            packet << int32_t(0);                   // artifact category
            packet << int32_t(rewards.honor);
            packet << int32_t(rewards.title);
            writeQuestRewardFactions(packet, rewards, true);
            packet << int32_t(rewards.displaySpell);
            packet << int32_t(0);
            packet << int32_t(0);
            packet << int32_t(rewards.spell);
            packet << int32_t(0);                   // skill line
            packet << int32_t(0);                   // skill ups
            packet << uint32_t(0);                  // treasure pickers

            if (protocol.isMidnight())
            {
                packet.writeBit(false);             // boost spell
                packet.flushBits();
                return;
            }

            writeCurrencies();
            packet.writeBit(false);                 // boost spell
            packet.flushBits();
            for (auto const& item : rewards.choiceItems)
                writeQuestChoiceItem(packet, protocol, item);
            return;
        }

        // 8.x to 10.x
        packet << int32_t(rewards.choiceItemCount);
        packet << int32_t(rewards.itemCount);
        for (auto const& item : rewards.items)
            packet << int32_t(item.itemId) << int32_t(item.count);
        packet << int32_t(rewards.money);
        packet << int32_t(rewards.xp);
        packet << int64_t(0);                       // artifact xp
        packet << int32_t(0);                       // artifact category
        packet << int32_t(rewards.honor);
        packet << int32_t(rewards.title);
        writeQuestRewardFactions(packet, rewards, true);
        packet << int32_t(rewards.displaySpell);
        packet << int32_t(0);
        packet << int32_t(0);
        packet << int32_t(rewards.spell);
        for (uint8_t i = 0; i < 4; ++i)
            packet << int32_t(rewards.currencyId[i]) << int32_t(rewards.currencyCount[i]);
        packet << int32_t(0);                       // skill line
        packet << int32_t(0);                       // skill ups
        packet << int32_t(0);                       // treasure picker
        for (auto const& item : rewards.choiceItems)
        {
            if (protocol.isBfA())
            {
                writeEmptyItemInstance(packet, protocol, int32_t(item.itemId));
                packet << int32_t(item.count);
            }
            else
            {
                writeQuestChoiceItem(packet, protocol, item);
            }
        }
        packet.writeBit(false);                     // boost spell
        packet.flushBits();
    }

    // 6.x and later: the quest flags; 10.x sends three, 11.x and later four values
    inline void writeQuestFlags(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t flags)
    {
        packet << uint32_t(flags);
        packet << uint32_t(0);
        if (protocol.isDragonflight() || protocol.isTheWarWithinOrLater())
            packet << uint32_t(0);
        if (protocol.isTheWarWithinOrLater())
            packet << uint32_t(0);
    }
}
