/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Macros/GuildMacros.hpp"
#include "Server/Definitions.h"
#include <array>
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    struct GuildRosterRankData
    {
        uint32_t rights = 0;
        uint32_t bankMoneyPerDay = 0;
        std::array<uint32_t, MAX_GUILD_BANK_TABS> bankTabRights{};
        std::array<uint32_t, MAX_GUILD_BANK_TABS> bankTabSlotsPerDay{};
    };

    struct GuildRosterMemberData
    {
        uint64_t guid = 0;
        uint8_t flags = 0;
        std::string name;
        uint32_t rankId = 0;
        uint8_t level = 0;
        uint8_t classId = 0;
        uint32_t zoneId = 0;
        uint64_t logoutTime = 0;
        bool isOnline = false;
        std::string publicNote;
        std::string officerNote;

        // Cata+ only fields
        uint32_t totalReputation = 0;
        uint64_t weekActivity = 0;
        uint32_t achievementPoints = 0;
        uint64_t totalActivity = 0;
        uint32_t weekReputationRemaining = 0;
    };

    class SmsgGuildRoster : public ManagedPacket
    {
    public:
        std::string motd;
        std::string info;
        std::vector<GuildRosterRankData> ranks;
        std::vector<GuildRosterMemberData> members;
        bool canSeeOfficerNote = false;
        uint32_t accountsNumber = 0;
        uint32_t maxRepPerWeek = 0;
        time_t createdDate = 0;

        SmsgGuildRoster() : SmsgGuildRoster("", "", {}, {}, false, 0, 0, 0)
        {
        }

        SmsgGuildRoster(std::string motd, std::string info, std::vector<GuildRosterRankData> ranks, std::vector<GuildRosterMemberData> members,
            bool canSeeOfficerNote, uint32_t accountsNumber, uint32_t maxRepPerWeek, time_t createdDate) :
            ManagedPacket(SMSG_GUILD_ROSTER, 100),
            motd(std::move(motd)), info(std::move(info)), ranks(std::move(ranks)), members(std::move(members)),
            canSeeOfficerNote(canSeeOfficerNote), accountsNumber(accountsNumber), maxRepPerWeek(maxRepPerWeek), createdDate(createdDate)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + motd.size() + 1 + info.size() + 1 + 4 + ranks.size() * (4 + 4 + MAX_GUILD_BANK_TABS * (4 + 4)) + members.size() * 50;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << uint32_t(members.size());
                packet << motd;
                packet << info;

                packet << uint32_t(ranks.size());
                for (const auto& rank : ranks)
                {
                    packet << uint32_t(rank.rights);
                    if (rank.bankMoneyPerDay == sizeof(uint32_t))
                        packet << uint32_t(sizeof(uint32_t));
                    else
                        packet << uint32_t(rank.bankMoneyPerDay);

                    for (uint8_t i = 0; i < MAX_GUILD_BANK_TABS; ++i)
                    {
                        packet << uint32_t(rank.bankTabRights[i]);
                        packet << uint32_t(rank.bankTabSlotsPerDay[i]);
                    }
                }

                for (const auto& member : members)
                {
                    packet << uint64_t(member.guid)
                        << uint8_t(member.flags)
                        << member.name
                        << uint32_t(member.rankId)
                        << uint8_t(member.level)
                        << uint8_t(member.classId)
                        << uint8_t(0)
                        << uint32_t(member.zoneId);

                    if (member.flags == 0)
                        packet << float(float(::time(nullptr) - member.logoutTime) / static_cast<uint64_t>(DAY));

                    packet << member.publicNote;

                    if (canSeeOfficerNote)
                        packet << member.officerNote;
                    else
                        packet << "";
                }

                return true;
            }
            else if (m_protocol.isCata())
            {
                ByteBuffer memberData(100);
                packet.writeBits(motd.length(), 11);
                packet.writeBits(members.size(), 18);

                for (const auto& member : members)
                {
                    const size_t pubNoteLength = member.publicNote.length();
                    const size_t offNoteLength = member.officerNote.length();

                    WoWGuid guid = member.guid;
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(0);
                    packet.writeBit(0);
                    packet.writeBits(pubNoteLength, 8);
                    packet.writeBits(offNoteLength, 8);
                    packet.writeBit(guid[0]);
                    packet.writeBits(member.name.length(), 7);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[7]);

                    memberData << uint8_t(member.classId);
                    memberData << uint32_t(member.totalReputation);
                    memberData.writeByteSeq(guid[0]);
                    memberData << uint64_t(member.weekActivity);
                    memberData << uint32_t(member.rankId);
                    memberData << uint32_t(member.achievementPoints);

                    memberData << uint32_t(0);
                    memberData << uint32_t(0);
                    memberData << uint32_t(0);
                    memberData << uint32_t(0);
                    memberData << uint32_t(0);
                    memberData << uint32_t(0);

                    memberData.writeByteSeq(guid[2]);
                    memberData << uint8_t(member.flags);
                    memberData << uint32_t(member.zoneId);
                    memberData << uint64_t(member.totalActivity);
                    memberData.writeByteSeq(guid[7]);
                    memberData << uint32_t(member.weekReputationRemaining);

                    if (pubNoteLength)
                        memberData.writeString(member.publicNote);

                    memberData.writeByteSeq(guid[3]);
                    memberData << uint8_t(member.level);
                    memberData << int32_t(0);
                    memberData.writeByteSeq(guid[5]);
                    memberData.writeByteSeq(guid[4]);
                    memberData << uint8_t(0);
                    memberData.writeByteSeq(guid[1]);
                    memberData << float(member.isOnline ? 0.0f : float(::time(nullptr) - member.logoutTime) / static_cast<uint64_t>(DAY));

                    if (offNoteLength)
                        memberData.writeString(member.officerNote);

                    memberData.writeByteSeq(guid[6]);
                    memberData.writeString(member.name);
                }

                const size_t infoLength = info.length();
                packet.writeBits(infoLength, 12);

                packet.flushBits();
                packet.append(memberData);

                if (infoLength)
                    packet.writeString(info);

                packet.writeString(motd);
                packet << uint32_t(accountsNumber);
                packet << uint32_t(maxRepPerWeek);
                packet.appendPackedTime(createdDate);
                packet << uint32_t(0);

                return true;
            }
            else if (m_protocol.isMop())
            {
                // NOTE: sends the officer note unconditionally (no permission gate)
                // for the Cata+ wire format; canSeeOfficerNote only applies to
                // the pre-Cata format above, matching the reference server exactly.
                ByteBuffer memberData(100);
                packet.writeBits(members.size(), 17);
                packet.writeBits(motd.length(), 10);

                for (const auto& member : members)
                {
                    const size_t pubNoteLength = member.publicNote.length();
                    const size_t offNoteLength = member.officerNote.length();

                    WoWGuid guid = member.guid;
                    packet.writeBits(offNoteLength, 8);
                    packet.writeBit(guid[5]);
                    packet.writeBit(0); // scroll of resurrect
                    packet.writeBits(pubNoteLength, 8);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[6]);
                    packet.writeBits(member.name.length(), 6);
                    packet.writeBit(0); // authenticator
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[2]);

                    memberData << uint8_t(member.classId);
                    memberData << uint32_t(member.totalReputation);
                    memberData.writeString(member.name);
                    memberData.writeByteSeq(guid[0]);

                    // 2 professions
                    memberData << uint32_t(0) << uint32_t(0) << uint32_t(0);
                    memberData << uint32_t(0) << uint32_t(0) << uint32_t(0);

                    memberData << uint8_t(member.level);
                    memberData << uint8_t(member.flags);
                    memberData << uint32_t(member.zoneId);
                    memberData << uint32_t(member.weekReputationRemaining);
                    memberData.writeByteSeq(guid[3]);
                    memberData << uint64_t(member.totalActivity);
                    memberData.writeString(member.officerNote);
                    memberData << float(member.isOnline ? 0.0f : float(::time(nullptr) - member.logoutTime) / static_cast<uint64_t>(DAY));
                    memberData << uint8_t(0); // gender
                    memberData << uint32_t(member.rankId);
                    memberData << uint32_t(0); // virtual realm address
                    memberData.writeByteSeq(guid[5]);
                    memberData.writeByteSeq(guid[7]);
                    memberData.writeString(member.publicNote);
                    memberData.writeByteSeq(guid[4]);
                    memberData << uint64_t(member.weekActivity);
                    memberData << uint32_t(member.achievementPoints);
                    memberData.writeByteSeq(guid[6]);
                    memberData.writeByteSeq(guid[1]);
                    memberData.writeByteSeq(guid[2]);
                }

                packet.writeBits(info.length(), 11);

                packet.flushBits();
                packet.append(memberData);

                packet << uint32_t(accountsNumber);
                packet.appendPackedTime(createdDate);
                packet.writeString(info);
                packet << uint32_t(maxRepPerWeek);
                packet.writeString(motd);
                packet << uint32_t(0);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
