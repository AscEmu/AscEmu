/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Group.h"
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

        // Mop-only: the recipient's own index among the members of their own subgroup
        // (used for client-side party frame layout; not meaningful pre-Mop).
        uint32_t groupPosition = 0;

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
            if (m_protocol.isMop())
            {
                return internalSerialiseMop(packet);
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
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

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }

    private:
        // Mirrors the reference Mop packet layout call-for-call, including its constant
        // true/false flags for the dungeon/raid-difficulty, loot-mode and isLFG bit fields -
        // the reference itself never varies those per-packet, so this does not either.
        bool internalSerialiseMop(WorldPacket& packet)
        {
            const WoWGuid mopGroupGuid = groupGuid;

            if (sendEmptyList)
            {
                const WoWGuid mopLeaderGuid = uint64_t(0);

                packet.writeBit(mopGroupGuid[0]);
                packet.writeBit(mopLeaderGuid[7]);
                packet.writeBit(mopLeaderGuid[1]);
                packet.writeBit(0);                    // has dungeon/raid difficulty
                packet.writeBit(mopGroupGuid[7]);
                packet.writeBit(mopLeaderGuid[6]);
                packet.writeBit(mopLeaderGuid[5]);
                packet.writeBits(0, 21);                // member count
                packet.writeBit(mopLeaderGuid[3]);
                packet.writeBit(mopLeaderGuid[0]);
                packet.writeBit(0);                    // has loot mode
                packet.writeBit(mopGroupGuid[5]);
                packet.writeBit(mopGroupGuid[2]);
                packet.writeBit(mopGroupGuid[4]);
                packet.writeBit(mopGroupGuid[1]);
                packet.writeBit(0);                    // isLFG
                packet.writeBit(mopLeaderGuid[2]);
                packet.writeBit(mopGroupGuid[6]);
                packet.writeBit(mopLeaderGuid[4]);
                packet.writeBit(mopGroupGuid[3]);
                packet.flushBits();

                packet.writeByteSeq(mopLeaderGuid[0]);
                packet.writeByteSeq(mopGroupGuid[1]);
                packet.writeByteSeq(mopLeaderGuid[4]);
                packet.writeByteSeq(mopLeaderGuid[2]);
                packet.writeByteSeq(mopGroupGuid[6]);
                packet.writeByteSeq(mopGroupGuid[4]);
                packet << uint8_t(0x10);
                packet << uint8_t(0);
                packet << int32_t(-1);
                packet.writeByteSeq(mopGroupGuid[7]);
                packet.writeByteSeq(mopLeaderGuid[3]);
                packet.writeByteSeq(mopLeaderGuid[1]);
                packet << uint32_t(updateCounter);
                packet.writeByteSeq(mopGroupGuid[0]);
                packet.writeByteSeq(mopGroupGuid[2]);
                packet.writeByteSeq(mopGroupGuid[5]);
                packet.writeByteSeq(mopGroupGuid[3]);
                packet.writeByteSeq(mopLeaderGuid[7]);
                packet << uint8_t(0);
                packet.writeByteSeq(mopLeaderGuid[5]);
                packet.writeByteSeq(mopLeaderGuid[6]);

                return true;
            }

            const WoWGuid mopLeaderGuid = leaderGuid;
            const WoWGuid mopLooterGuid = looterGuid;

            packet.writeBit(mopGroupGuid[0]);
            packet.writeBit(mopLeaderGuid[7]);
            packet.writeBit(mopLeaderGuid[1]);
            packet.writeBit(1);                        // has dungeon/raid difficulty
            packet.writeBit(mopGroupGuid[7]);
            packet.writeBit(mopLeaderGuid[6]);
            packet.writeBit(mopLeaderGuid[5]);
            packet.writeBits(members.size(), 21);

            ByteBuffer memberData;
            for (const auto& member : members)
            {
                const WoWGuid memberGuid = member.guid;

                packet.writeBit(memberGuid[1]);
                packet.writeBit(memberGuid[2]);
                packet.writeBit(memberGuid[5]);
                packet.writeBit(memberGuid[6]);
                packet.writeBits(member.name.size(), 6);
                packet.writeBit(memberGuid[7]);
                packet.writeBit(memberGuid[3]);
                packet.writeBit(memberGuid[0]);
                packet.writeBit(memberGuid[4]);

                uint8_t onlineState = member.isOnline ? MEMBER_STATUS_ONLINE : MEMBER_STATUS_OFFLINE;
                if (isBattlegroundGroup)
                    onlineState |= MEMBER_STATUS_PVP;

                memberData.writeByteSeq(memberGuid[6]);
                memberData.writeByteSeq(memberGuid[3]);
                memberData << uint8_t(member.roles);
                memberData << uint8_t(onlineState);
                memberData.writeByteSeq(memberGuid[7]);
                memberData.writeByteSeq(memberGuid[4]);
                memberData.writeByteSeq(memberGuid[1]);
                memberData.writeString(member.name);
                memberData.writeByteSeq(memberGuid[5]);
                memberData.writeByteSeq(memberGuid[2]);
                memberData << uint8_t(member.subGroup);
                memberData.writeByteSeq(memberGuid[0]);
                memberData << uint8_t(member.flags);
            }

            packet.writeBit(mopLeaderGuid[3]);
            packet.writeBit(mopLeaderGuid[0]);
            packet.writeBit(1);                        // has loot mode
            packet.writeBit(mopGroupGuid[5]);

            packet.writeBit(mopLooterGuid[6]);
            packet.writeBit(mopLooterGuid[4]);
            packet.writeBit(mopLooterGuid[5]);
            packet.writeBit(mopLooterGuid[2]);
            packet.writeBit(mopLooterGuid[1]);
            packet.writeBit(mopLooterGuid[0]);
            packet.writeBit(mopLooterGuid[7]);
            packet.writeBit(mopLooterGuid[3]);

            packet.writeBit(mopGroupGuid[2]);
            packet.writeBit(mopGroupGuid[4]);
            packet.writeBit(mopGroupGuid[1]);
            packet.writeBit(0);                        // isLFG
            packet.writeBit(mopLeaderGuid[2]);
            packet.writeBit(mopGroupGuid[6]);

            packet.writeBit(mopLeaderGuid[4]);
            packet.writeBit(mopGroupGuid[3]);
            packet.flushBits();

            packet.writeByteSeq(mopLeaderGuid[0]);

            packet << uint32_t(raidDifficulty);
            packet << uint32_t(difficulty);

            packet.append(memberData);
            packet.writeByteSeq(mopGroupGuid[1]);

            packet.writeByteSeq(mopLeaderGuid[4]);
            packet.writeByteSeq(mopLeaderGuid[2]);

            packet << uint8_t(lootMethod);
            packet.writeByteSeq(mopLooterGuid[0]);
            packet.writeByteSeq(mopLooterGuid[5]);
            packet.writeByteSeq(mopLooterGuid[4]);
            packet.writeByteSeq(mopLooterGuid[3]);
            packet.writeByteSeq(mopLooterGuid[2]);
            packet << uint8_t(lootThreshold);
            packet.writeByteSeq(mopLooterGuid[7]);
            packet.writeByteSeq(mopLooterGuid[1]);
            packet.writeByteSeq(mopLooterGuid[6]);

            packet.writeByteSeq(mopGroupGuid[6]);
            packet.writeByteSeq(mopGroupGuid[4]);
            packet << uint8_t(groupType);
            packet << uint8_t(0);
            packet << uint32_t(groupPosition);
            packet.writeByteSeq(mopGroupGuid[7]);
            packet.writeByteSeq(mopLeaderGuid[3]);
            packet.writeByteSeq(mopLeaderGuid[1]);
            packet << uint32_t(updateCounter);
            packet.writeByteSeq(mopGroupGuid[0]);
            packet.writeByteSeq(mopGroupGuid[2]);
            packet.writeByteSeq(mopGroupGuid[5]);
            packet.writeByteSeq(mopGroupGuid[3]);
            packet.writeByteSeq(mopLeaderGuid[7]);
            packet << uint8_t(0);
            packet.writeByteSeq(mopLeaderGuid[5]);
            packet.writeByteSeq(mopLeaderGuid[6]);

            return true;
        }
    };
}
