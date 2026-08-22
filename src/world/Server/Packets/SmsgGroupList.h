/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    // A single other-member entry as seen from the perspective of the recipient (i.e. the
    // recipient itself is never included - see SmsgGroupList::requesterSubGroup/requesterFlags
    // for the recipient's own data).
    struct SmsgGroupListMember
    {
        std::string name;
        uint64_t guid = 0;
        bool isOnline = false;
        uint8_t subGroup = 0;
        uint8_t flags = 0;
        uint8_t roles = 0;
    };

    class SmsgGroupList : public ManagedPacket
    {
    public:
        bool sendEmptyList;

        // Full member-list fields (used when sendEmptyList == false). This packet is
        // personalized per recipient (Group::Update in Management/Group.cpp builds and sends
        // one instance per online group member), so requesterSubGroup/requesterFlags reflect
        // the recipient rather than a fixed value.
        uint8_t groupType = 0;
        uint8_t requesterSubGroup = 0;
        uint8_t requesterFlags = 0;
        bool isBattlegroundGroup = false;
        bool isLfgGroup = false;
        uint8_t lfgState = 0;
        uint32_t lfgDungeon = 0;
        uint64_t groupGuid = 0;
        uint32_t updateCounter = 0;
        uint32_t memberCount = 0;
        std::vector<SmsgGroupListMember> members;
        bool hasLeader = false;
        uint64_t leaderGuid = 0;
        uint8_t lootMethod = 0;
        bool hasLooter = false;
        uint64_t looterGuid = 0;
        uint8_t lootThreshold = 0;
        uint8_t difficulty = 0;
        uint8_t raidDifficulty = 0;

        SmsgGroupList(bool sendEmptyList = true) :
            ManagedPacket(SMSG_GROUP_LIST, 28),
            sendEmptyList(sendEmptyList)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (sendEmptyList)
                return m_minimum_size;

            return 50 + (members.size() + 1) * 20;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            // Mop uses a bit-packed layout for this opcode that differs substantially from
            // the Cata/WotLK/TBC/Classic format below; not implemented yet, so refuse to send
            // a mis-formatted packet rather than desyncing the client's byte stream.
            if (m_protocol.isMop())
                return false;

            if (sendEmptyList)
            {
                if (m_protocol.expansion <= WoW::Expansion::_TBC)
                {
                    packet << uint64_t(0) << uint64_t(0) << uint64_t(0);
                }

                if (m_protocol.expansion > WoW::Expansion::_TBC)
                {
                    packet << uint8_t(0x10) << uint8_t(0) << uint8_t(0) << uint8_t(0);
                    packet << uint64_t(0) << uint32_t(0) << uint32_t(0) << uint64_t(0);
                }

                return true;
            }

            packet << groupType;
            packet << requesterSubGroup;
            packet << requesterFlags;

            // if the leader is in a BG, then the group is a BG group
            packet << uint8_t(isBattlegroundGroup ? 1 : 0);

            if (isLfgGroup)
            {
                packet << lfgState;
                packet << lfgDungeon;

                if (m_protocol.expansion >= WoW::Expansion::_Cata)
                    packet << uint8_t(0); // unk
            }

            packet << groupGuid;          // Group guid
            packet << updateCounter;      // 3.3 - increments every time a group list update is being sent to client
            packet << memberCount;        // we don't include self

            for (const auto& member : members)
            {
                packet << member.name;
                packet << member.guid;
                packet << uint8_t(member.isOnline ? 1 : 0);
                packet << member.subGroup;
                packet << member.flags;
                packet << member.roles;   // Player roles
            }

            if (hasLeader)
                packet << leaderGuid;
            else
                packet << uint64_t(0);

            packet << lootMethod;

            if (hasLooter)
                packet << looterGuid;
            else
                packet << uint64_t(0);

            packet << lootThreshold;
            packet << difficulty;
            packet << raidDifficulty;
            packet << uint8_t(0);   // 3.3 - unk

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
