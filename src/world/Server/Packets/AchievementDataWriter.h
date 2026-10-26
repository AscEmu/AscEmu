/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AchievementDataEntries.h"
#include "ManagedPacket.h"

#include <vector>

namespace AscEmu::Packets
{
    // the achievement lists of the 6.x and later clients
    namespace AchievementWire
    {
        // an account wide achievement names no earner and no realm
        inline void writeEarned(WorldPacket& packet, WoW::ClientProtocol const& protocol, WoWGuid const& player, CompletedAchievementEntry const& entry)
        {
            uint32_t const realm = entry.accountWide ? 0 : protocol.getVirtualRealmAddress();
            packet << entry.achievementId;
            packet.appendPackedTime(entry.date);
            packet << (entry.accountWide ? WoWGuid128() : player.toGuid128(protocol.realmId, 0));
            packet << realm << realm;
        }

        inline void writeProgress(WorldPacket& packet, WoW::ClientProtocol const& protocol, WoWGuid128 const& owner, CriteriaProgressEntry const& entry)
        {
            packet << entry.criteriaId << uint64_t(entry.counter) << owner;

            // 10.x: an unused value and the flags in front of the date, 11.x and later: flags and state flags
            if (protocol.expansion >= WoW::Expansion::_Dragonflight)
                packet << uint32_t(0) << uint32_t(0);

            packet.appendPackedTime(entry.date);

            // the time since the start and the creation of the criteria, timed criteria are not tracked
            if (protocol.expansion >= WoW::Expansion::_Shadowlands)
                packet << int64_t(0) << int64_t(0);
            else
                packet << uint32_t(0) << uint32_t(0);

            if (protocol.expansion < WoW::Expansion::_Dragonflight)
                packet.writeBits(0, 4);                 // flags

            // 8.x and later: the recruit a friend acceptance (dynamic id from 11.x on)
            if (protocol.expansion >= WoW::Expansion::_BfA)
                packet.writeBit(false);
            packet.flushBits();
        }

        inline void writeAll(WorldPacket& packet, WoW::ClientProtocol const& protocol, WoWGuid const& player,
            std::vector<CompletedAchievementEntry> const& completed, std::vector<CriteriaProgressEntry> const& progress)
        {
            WoWGuid128 const owner = player.toGuid128(protocol.realmId, 0);
            packet << uint32_t(completed.size()) << uint32_t(progress.size());
            for (auto const& entry : completed)
                writeEarned(packet, protocol, player, entry);
            for (auto const& entry : progress)
                writeProgress(packet, protocol, owner, entry);
        }
    }
}
