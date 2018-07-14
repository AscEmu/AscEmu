/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"
#include "GameCata/Management/GuildFinderMgr.h"
#include "Management/GuildMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Players/Player.h"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/WorldStrings.h"
#include "Server/Packets/SmsgGuildCommandResult.h"
#include "Server/Packets/MsgSaveGuildEmblem.h"
#include "Server/Packets/SmsgPetitionShowSignatures.h"

using namespace AscEmu::Packets;

//////////////////////////////////////////////////////////////////////////////////////////
// Guild

void WorldSession::HandleGuildAssignRankOpcode(WorldPacket& recvData)
{
    ObjectGuid targetGuid;
    ObjectGuid setterGuid;

    uint32_t rankId;
    recvData >> rankId;

    targetGuid[1] = recvData.readBit();
    targetGuid[7] = recvData.readBit();

    setterGuid[4] = recvData.readBit();
    setterGuid[2] = recvData.readBit();

    targetGuid[4] = recvData.readBit();
    targetGuid[5] = recvData.readBit();
    targetGuid[6] = recvData.readBit();

    setterGuid[1] = recvData.readBit();
    setterGuid[7] = recvData.readBit();

    targetGuid[2] = recvData.readBit();
    targetGuid[3] = recvData.readBit();
    targetGuid[0] = recvData.readBit();

    setterGuid[6] = recvData.readBit();
    setterGuid[3] = recvData.readBit();
    setterGuid[0] = recvData.readBit();
    setterGuid[5] = recvData.readBit();

    recvData.ReadByteSeq(targetGuid[0]);

    recvData.ReadByteSeq(setterGuid[1]);
    recvData.ReadByteSeq(setterGuid[3]);
    recvData.ReadByteSeq(setterGuid[5]);

    recvData.ReadByteSeq(targetGuid[7]);
    recvData.ReadByteSeq(targetGuid[3]);

    recvData.ReadByteSeq(setterGuid[0]);

    recvData.ReadByteSeq(targetGuid[1]);

    recvData.ReadByteSeq(setterGuid[6]);

    recvData.ReadByteSeq(targetGuid[2]);
    recvData.ReadByteSeq(targetGuid[5]);
    recvData.ReadByteSeq(targetGuid[4]);

    recvData.ReadByteSeq(setterGuid[2]);
    recvData.ReadByteSeq(setterGuid[4]);

    recvData.ReadByteSeq(targetGuid[6]);

    recvData.ReadByteSeq(setterGuid[7]);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_ASSIGN_MEMBER_RANK %s: Target: %u Rank: %u, Issuer: %u",
        _player->getName().c_str(), Arcemu::Util::GUID_LOPART(targetGuid), rankId, Arcemu::Util::GUID_LOPART(setterGuid));

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleSetMemberRank(this, targetGuid, setterGuid, rankId);
    }
}

void WorldSession::HandleGuildQueryRanksOpcode(WorldPacket& recvData)
{
    ObjectGuid guildGuid;

    guildGuid[2] = recvData.readBit();
    guildGuid[3] = recvData.readBit();
    guildGuid[0] = recvData.readBit();
    guildGuid[6] = recvData.readBit();
    guildGuid[4] = recvData.readBit();
    guildGuid[7] = recvData.readBit();
    guildGuid[5] = recvData.readBit();
    guildGuid[1] = recvData.readBit();

    recvData.ReadByteSeq(guildGuid[3]);
    recvData.ReadByteSeq(guildGuid[4]);
    recvData.ReadByteSeq(guildGuid[5]);
    recvData.ReadByteSeq(guildGuid[7]);
    recvData.ReadByteSeq(guildGuid[1]);
    recvData.ReadByteSeq(guildGuid[0]);
    recvData.ReadByteSeq(guildGuid[6]);
    recvData.ReadByteSeq(guildGuid[2]);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_QUERY_RANKS %s: Guild: %u", _player->getName().c_str(), Arcemu::Util::GUID_LOPART(guildGuid));

    if (Guild* guild = sGuildMgr.getGuildById(Arcemu::Util::GUID_LOPART(guildGuid)))
    {
        if (guild->isMember(_player->getGuid()))
        {
            guild->sendGuildRankInfo(this);
        }
    }
}

void WorldSession::HandleGuildRequestChallengeUpdate(WorldPacket& /*recv_data*/)
{
    if (Guild* guild = _player->GetGuild())
    {
        guild->handleGuildRequestChallengeUpdate(this);
    }
}

void WorldSession::HandleGuildQueryXPOpcode(WorldPacket& recvData)
{
    ObjectGuid guildGuid;

    guildGuid[2] = recvData.readBit();
    guildGuid[1] = recvData.readBit();
    guildGuid[0] = recvData.readBit();
    guildGuid[5] = recvData.readBit();
    guildGuid[4] = recvData.readBit();
    guildGuid[7] = recvData.readBit();
    guildGuid[6] = recvData.readBit();
    guildGuid[3] = recvData.readBit();

    recvData.ReadByteSeq(guildGuid[7]);
    recvData.ReadByteSeq(guildGuid[2]);
    recvData.ReadByteSeq(guildGuid[3]);
    recvData.ReadByteSeq(guildGuid[6]);
    recvData.ReadByteSeq(guildGuid[1]);
    recvData.ReadByteSeq(guildGuid[5]);
    recvData.ReadByteSeq(guildGuid[0]);
    recvData.ReadByteSeq(guildGuid[4]);

    uint32_t guildId = Arcemu::Util::GUID_LOPART(guildGuid);

    LogDebugFlag(LF_OPCODE, "CMSG_QUERY_GUILD_XP %s: guildId: %u", _player->getName().c_str(), guildId);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        if (guild->isMember(_player->getGuid()))
        {
            guild->sendGuildXP(this);
        }
    }
}

void WorldSession::HandleGuildRequestPartyState(WorldPacket& recvData)
{
    ObjectGuid guildGuid;

    guildGuid[0] = recvData.readBit();
    guildGuid[6] = recvData.readBit();
    guildGuid[7] = recvData.readBit();
    guildGuid[3] = recvData.readBit();
    guildGuid[5] = recvData.readBit();
    guildGuid[1] = recvData.readBit();
    guildGuid[2] = recvData.readBit();
    guildGuid[4] = recvData.readBit();

    recvData.ReadByteSeq(guildGuid[6]);
    recvData.ReadByteSeq(guildGuid[3]);
    recvData.ReadByteSeq(guildGuid[2]);
    recvData.ReadByteSeq(guildGuid[1]);
    recvData.ReadByteSeq(guildGuid[5]);
    recvData.ReadByteSeq(guildGuid[0]);
    recvData.ReadByteSeq(guildGuid[7]);
    recvData.ReadByteSeq(guildGuid[4]);

    uint32_t guildId = Arcemu::Util::GUID_LOPART(guildGuid);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        guild->handleGuildPartyRequest(this);
    }
}

void WorldSession::HandleGuildRequestMaxDailyXP(WorldPacket& recvData)
{
    ObjectGuid guid;

    guid[0] = recvData.readBit();
    guid[3] = recvData.readBit();
    guid[5] = recvData.readBit();
    guid[1] = recvData.readBit();
    guid[4] = recvData.readBit();
    guid[6] = recvData.readBit();
    guid[7] = recvData.readBit();
    guid[2] = recvData.readBit();

    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[0]);

    uint32_t guildId = Arcemu::Util::GUID_LOPART(guid);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        if (guild->isMember(_player->getGuid()))
        {
            WorldPacket data(SMSG_GUILD_MAX_DAILY_XP, 8);
            data << uint64_t(worldConfig.guild.maxXpPerDay);
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleAutoDeclineGuildInvites(WorldPacket& recvData)
{
    uint8_t enable;
    recvData >> enable;

    bool enabled = enable > 0 ? true : false;

    GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_AUTO_DECLINE_GUILD, enabled);
}

void WorldSession::HandleGuildRewardsQueryOpcode(WorldPacket& recvData)
{
    recvData.read_skip<uint32_t>();

    if (sGuildMgr.getGuildById(_player->getGuildId()))
    {
        std::vector<GuildReward> const& rewards = sGuildMgr.getGuildRewards();

        WorldPacket data(SMSG_GUILD_REWARDS_LIST, 3 + rewards.size() * (4 + 4 + 4 + 8 + 4 + 4));
        data.writeBits(rewards.size(), 21);
        data.flushBits();

        for (uint32_t i = 0; i < rewards.size(); ++i)
        {
            data << uint32_t(rewards[i].standing);
            data << int32_t(rewards[i].racemask);
            data << uint32_t(rewards[i].entry);
            data << uint64_t(rewards[i].price);
            data << uint32_t(0);
            data << uint32_t(rewards[i].achievementId);
        }
        data << uint32_t(time(nullptr));

        SendPacket(&data);
    }
}

void WorldSession::HandleGuildQueryNewsOpcode(WorldPacket& recvData)
{
    recvData.read_skip<uint32_t>();

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->sendNewsUpdate(this);
    }
}

void WorldSession::HandleGuildNewsUpdateStickyOpcode(WorldPacket& recvData)
{
    uint32_t newsId;
    bool isSticky;

    ObjectGuid guid;

    recvData >> newsId;

    guid[2] = recvData.readBit();
    guid[4] = recvData.readBit();
    guid[3] = recvData.readBit();
    guid[0] = recvData.readBit();

    isSticky = recvData.readBit();

    guid[6] = recvData.readBit();
    guid[7] = recvData.readBit();
    guid[1] = recvData.readBit();
    guid[5] = recvData.readBit();

    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[4]);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleNewsSetSticky(this, newsId, isSticky);
    }
}

void WorldSession::HandleGuildSetGuildMaster(WorldPacket& recvData)
{
    uint8_t nameLength = static_cast<uint8_t>(recvData.readBits(7));

    recvData.readBit();

    std::string playerName = recvData.ReadString(nameLength);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleSetNewGuildMaster(this, playerName);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// GuildFinder
void WorldSession::HandleGuildFinderAddRecruit(WorldPacket& recvData)
{
    if (sGuildFinderMgr.getAllMembershipRequestsForPlayer(GetPlayer()->getGuidLow()).size() == 10)
    {
        return;
    }

    uint32_t classRoles = 0;
    uint32_t availability = 0;
    uint32_t guildInterests = 0;

    recvData >> classRoles;
    recvData >> guildInterests;
    recvData >> availability;

    ObjectGuid guid;

    guid[3] = recvData.readBit();
    guid[0] = recvData.readBit();
    guid[6] = recvData.readBit();
    guid[1] = recvData.readBit();

    uint16_t commentLength = static_cast<uint16_t>(recvData.readBits(11));

    guid[5] = recvData.readBit();
    guid[4] = recvData.readBit();
    guid[7] = recvData.readBit();

    uint8_t nameLength = static_cast<uint8_t>(recvData.readBits(7));

    guid[2] = recvData.readBit();

    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[5]);

    std::string comment = recvData.ReadString(commentLength);
    std::string playerName = recvData.ReadString(nameLength);

    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[3]);

    uint32_t guildLowGuid = Arcemu::Util::GUID_LOPART(uint64_t(guid));

    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
    {
        return;
    }

    if (!(availability & AVAILABILITY_ALWAYS) || availability > AVAILABILITY_ALWAYS)
    {
        return;
    }

    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
    {
        return;
    }

    MembershipRequest request = MembershipRequest(GetPlayer()->getGuidLow(), guildLowGuid, availability, classRoles, guildInterests, comment, time(nullptr));
    sGuildFinderMgr.addMembershipRequest(guildLowGuid, request);
}

void WorldSession::HandleGuildFinderBrowse(WorldPacket& recv_data)
{
    uint32_t classRoles = 0;
    uint32_t availability = 0;
    uint32_t guildInterests = 0;
    uint32_t playerLevel = 0;

    recv_data >> classRoles;
    recv_data >> availability;
    recv_data >> guildInterests;
    recv_data >> playerLevel;

    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
    {
        return;
    }

    if (!(availability & AVAILABILITY_ALWAYS) || availability > AVAILABILITY_ALWAYS)
    {
        return;
    }

    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
    {
        return;
    }

    if (playerLevel > worldConfig.player.playerLevelCap || playerLevel < 1)
    {
        return;
    }

    Player* player = GetPlayer();

    LFGuildPlayer settings(player->getGuidLow(), static_cast<uint8_t>(classRoles), static_cast<uint8_t>(availability), static_cast<uint8_t>(guildInterests), ANY_FINDER_LEVEL);
    LFGuildStore guildList = sGuildFinderMgr.getGuildsMatchingSetting(settings, player->GetTeam());
    uint32_t guildCount = static_cast<uint32_t>(guildList.size());

    if (guildCount == 0)
    {
        WorldPacket packet(SMSG_LF_GUILD_BROWSE_UPDATED, 0);
        player->SendPacket(&packet);
        return;
    }

    ByteBuffer bufferData(65 * guildCount);
    WorldPacket data(SMSG_LF_GUILD_BROWSE_UPDATED, 3 + guildCount * 65);
    data.writeBits(guildCount, 19);

    for (LFGuildStore::const_iterator itr = guildList.begin(); itr != guildList.end(); ++itr)
    {
        LFGuildSettings guildSettings = itr->second;
        Guild* guild = sGuildMgr.getGuildById(itr->first);

        ObjectGuid guildGUID = guild->getGUID();

        data.writeBit(guildGUID[7]);
        data.writeBit(guildGUID[5]);

        data.writeBits(guild->getName().size(), 8);

        data.writeBit(guildGUID[0]);

        data.writeBits(guildSettings.getComment().size(), 11);

        data.writeBit(guildGUID[4]);
        data.writeBit(guildGUID[1]);
        data.writeBit(guildGUID[2]);
        data.writeBit(guildGUID[6]);
        data.writeBit(guildGUID[3]);

        bufferData << uint32_t(guild->getEmblemInfo().getColor());
        bufferData << uint32_t(guild->getEmblemInfo().getBorderStyle());
        bufferData << uint32_t(guild->getEmblemInfo().getStyle());

        bufferData.WriteString(guildSettings.getComment());

        bufferData << uint8_t(0);

        bufferData.WriteByteSeq(guildGUID[5]);

        bufferData << uint32_t(guildSettings.getInterests());

        bufferData.WriteByteSeq(guildGUID[6]);
        bufferData.WriteByteSeq(guildGUID[4]);

        bufferData << uint32_t(guild->getLevel());

        bufferData.WriteString(guild->getName());

        bufferData << uint32_t(0); // Achievment

        bufferData.WriteByteSeq(guildGUID[7]);

        bufferData << uint8_t(sGuildFinderMgr.hasRequest(player->getGuidLow(), guild->getId()));

        bufferData.WriteByteSeq(guildGUID[2]);
        bufferData.WriteByteSeq(guildGUID[0]);

        bufferData << uint32_t(guildSettings.getAvailability());

        bufferData.WriteByteSeq(guildGUID[1]);

        bufferData << uint32_t(guild->getEmblemInfo().getBackgroundColor());
        bufferData << uint32_t(0); 
        bufferData << uint32_t(guild->getEmblemInfo().getBorderColor());
        bufferData << uint32_t(guildSettings.getClassRoles());

        bufferData.WriteByteSeq(guildGUID[3]);
        bufferData << uint32_t(guild->getMembersCount());
    }

    data.flushBits();
    data.append(bufferData);

    player->SendPacket(&data);
}

void WorldSession::HandleGuildFinderDeclineRecruit(WorldPacket& recv_data)
{
    ObjectGuid playerGuid;

    playerGuid[1] = recv_data.readBit();
    playerGuid[4] = recv_data.readBit();
    playerGuid[5] = recv_data.readBit();
    playerGuid[2] = recv_data.readBit();
    playerGuid[6] = recv_data.readBit();
    playerGuid[7] = recv_data.readBit();
    playerGuid[0] = recv_data.readBit();
    playerGuid[3] = recv_data.readBit();

    recv_data.ReadByteSeq(playerGuid[5]);
    recv_data.ReadByteSeq(playerGuid[7]);
    recv_data.ReadByteSeq(playerGuid[2]);
    recv_data.ReadByteSeq(playerGuid[3]);
    recv_data.ReadByteSeq(playerGuid[4]);
    recv_data.ReadByteSeq(playerGuid[1]);
    recv_data.ReadByteSeq(playerGuid[0]);
    recv_data.ReadByteSeq(playerGuid[6]);

    if (!IS_PLAYER_GUID(playerGuid))
    {
        return;
    }

    sGuildFinderMgr.removeMembershipRequest(Arcemu::Util::GUID_LOPART(playerGuid), GetPlayer()->getGuildId());
}

void WorldSession::HandleGuildFinderGetApplications(WorldPacket& /*recv_data*/)
{
    std::list<MembershipRequest> applicatedGuilds = sGuildFinderMgr.getAllMembershipRequestsForPlayer(GetPlayer()->getGuidLow());
    uint32_t applicationsCount = static_cast<uint32_t>(applicatedGuilds.size());
    WorldPacket data(SMSG_LF_GUILD_MEMBERSHIP_LIST_UPDATED, 7 + 54 * applicationsCount);
    data.writeBits(applicationsCount, 20);

    if (applicationsCount > 0)
    {
        ByteBuffer bufferData(54 * applicationsCount);
        for (std::list<MembershipRequest>::const_iterator itr = applicatedGuilds.begin(); itr != applicatedGuilds.end(); ++itr)
        {
            Guild* guild = sGuildMgr.getGuildById(itr->getGuildId());
            LFGuildSettings guildSettings = sGuildFinderMgr.getGuildSettings(itr->getGuildId());
            MembershipRequest request = *itr;

            ObjectGuid guildGuid = ObjectGuid(guild->getGUID());

            data.writeBit(guildGuid[1]);
            data.writeBit(guildGuid[0]);
            data.writeBit(guildGuid[5]);

            data.writeBits(request.getComment().size(), 11);

            data.writeBit(guildGuid[3]);
            data.writeBit(guildGuid[7]);
            data.writeBit(guildGuid[4]);
            data.writeBit(guildGuid[6]);
            data.writeBit(guildGuid[2]);

            data.writeBits(guild->getName().size(), 8);

            bufferData.WriteByteSeq(guildGuid[2]);

            bufferData.WriteString(request.getComment());

            bufferData.WriteByteSeq(guildGuid[5]);

            bufferData.WriteString(guild->getName());

            bufferData << uint32_t(guildSettings.getAvailability());
            bufferData << uint32_t(request.getExpiryTime() - time(nullptr));

            bufferData.WriteByteSeq(guildGuid[0]);
            bufferData.WriteByteSeq(guildGuid[6]);
            bufferData.WriteByteSeq(guildGuid[3]);
            bufferData.WriteByteSeq(guildGuid[7]);

            bufferData << uint32_t(guildSettings.getClassRoles());

            bufferData.WriteByteSeq(guildGuid[4]);
            bufferData.WriteByteSeq(guildGuid[1]);

            bufferData << uint32_t(time(nullptr) - request.getSubmitTime());
            bufferData << uint32_t(guildSettings.getInterests());
        }

        data.flushBits();
        data.append(bufferData);
    }
    data << uint32_t(10 - sGuildFinderMgr.countRequestsFromPlayer(GetPlayer()->getGuidLow()));

    GetPlayer()->SendPacket(&data);
}

void WorldSession::HandleGuildFinderGetRecruits(WorldPacket& recv_data)
{
    uint32_t unkTime = 0;
    recv_data >> unkTime;

    Player* player = GetPlayer();
    if (!player->getGuildId())
        return;

    std::vector<MembershipRequest> recruitsList = sGuildFinderMgr.getAllMembershipRequestsForGuild(player->getGuildId());
    uint32_t recruitCount = static_cast<uint32_t>(recruitsList.size());

    ByteBuffer dataBuffer(53 * recruitCount);
    WorldPacket data(SMSG_LF_GUILD_RECRUIT_LIST_UPDATED, 7 + 26 * recruitCount + 53 * recruitCount);
    data.writeBits(recruitCount, 20);

    for (std::vector<MembershipRequest>::const_iterator itr = recruitsList.begin(); itr != recruitsList.end(); ++itr)
    {
        MembershipRequest request = *itr;
        ObjectGuid playerGuid(MAKE_NEW_GUID(request.getPlayerGUID(), 0, HIGHGUID_TYPE_PLAYER));

        PlayerInfo* info = objmgr.GetPlayerInfo(request.getPlayerGUID());
        std::string name = info->name;

        data.writeBits(request.getComment().size(), 11);

        data.writeBit(playerGuid[2]);
        data.writeBit(playerGuid[4]);
        data.writeBit(playerGuid[3]);
        data.writeBit(playerGuid[7]);
        data.writeBit(playerGuid[0]);

        data.writeBits(name.size(), 7);

        data.writeBit(playerGuid[5]);
        data.writeBit(playerGuid[1]);
        data.writeBit(playerGuid[6]);

        dataBuffer.WriteByteSeq(playerGuid[4]);

        dataBuffer << int32_t(time(nullptr) <= request.getExpiryTime());

        dataBuffer.WriteByteSeq(playerGuid[3]);
        dataBuffer.WriteByteSeq(playerGuid[0]);
        dataBuffer.WriteByteSeq(playerGuid[1]);

        dataBuffer << int32_t(info->lastLevel);

        dataBuffer.WriteByteSeq(playerGuid[6]);
        dataBuffer.WriteByteSeq(playerGuid[7]);
        dataBuffer.WriteByteSeq(playerGuid[2]);

        dataBuffer << int32_t(time(nullptr) - request.getSubmitTime());
        dataBuffer << int32_t(request.getAvailability());
        dataBuffer << int32_t(request.getClassRoles());
        dataBuffer << int32_t(request.getInterests());
        dataBuffer << int32_t(request.getExpiryTime() - time(nullptr));

        dataBuffer.WriteString(name);
        dataBuffer.WriteString(request.getComment());

        dataBuffer << int32_t(info->cl);

        dataBuffer.WriteByteSeq(playerGuid[5]);
    }

    data.flushBits();
    data.append(dataBuffer);
    data << uint32_t(time(nullptr));

    player->SendPacket(&data);
}

void WorldSession::HandleGuildFinderPostRequest(WorldPacket& /*recv_data*/)
{
    Player* player = GetPlayer();
    if (!player->getGuildId())
        return;

    bool isGuildMaster = true;
    if (Guild* guild = sGuildMgr.getGuildById(player->getGuildId()))
    {
        if (guild->getLeaderGUID() != player->getGuid())
        {
            isGuildMaster = false;
        }
    }

    LFGuildSettings settings = sGuildFinderMgr.getGuildSettings(player->getGuildId());

    WorldPacket data(SMSG_LF_GUILD_POST_UPDATED, 35);
    data.writeBit(isGuildMaster);

    if (isGuildMaster)
    {
        data.writeBits(settings.getComment().size(), 11);

        data.writeBit(settings.isListed());

        data << uint32_t(settings.getLevel());

        data.WriteString(settings.getComment());

        data << uint32_t(0);

        data << uint32_t(settings.getAvailability());
        data << uint32_t(settings.getClassRoles());
        data << uint32_t(settings.getInterests());
    }
    else
    {
        data.flushBits();
    }

    player->GetSession()->SendPacket(&data);
}

void WorldSession::HandleGuildFinderRemoveRecruit(WorldPacket& recv_data)
{
    ObjectGuid guildGuid;

    guildGuid[0] = recv_data.readBit();
    guildGuid[4] = recv_data.readBit();
    guildGuid[3] = recv_data.readBit();
    guildGuid[5] = recv_data.readBit();
    guildGuid[7] = recv_data.readBit();
    guildGuid[6] = recv_data.readBit();
    guildGuid[2] = recv_data.readBit();
    guildGuid[1] = recv_data.readBit();

    recv_data.ReadByteSeq(guildGuid[4]);
    recv_data.ReadByteSeq(guildGuid[0]);
    recv_data.ReadByteSeq(guildGuid[3]);
    recv_data.ReadByteSeq(guildGuid[6]);
    recv_data.ReadByteSeq(guildGuid[5]);
    recv_data.ReadByteSeq(guildGuid[1]);
    recv_data.ReadByteSeq(guildGuid[2]);
    recv_data.ReadByteSeq(guildGuid[7]);

    sGuildFinderMgr.removeMembershipRequest(Arcemu::Util::GUID_LOPART(GetPlayer()->getGuid()), Arcemu::Util::GUID_LOPART(guildGuid));
}

void WorldSession::HandleGuildFinderSetGuildPost(WorldPacket& recv_data)
{
    uint32_t classRoles = 0;
    uint32_t availability = 0;
    uint32_t guildInterests = 0;
    uint32_t level = 0;

    recv_data >> level;
    recv_data >> availability;
    recv_data >> guildInterests;
    recv_data >> classRoles;

    if (level == 0)
    {
        level = ANY_FINDER_LEVEL;
    }

    uint32_t length = recv_data.readBits(11);
    bool listed = recv_data.readBit();
    std::string comment = recv_data.ReadString(length);

    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
    {
        return;
    }

    if (!(availability & AVAILABILITY_ALWAYS) || availability > AVAILABILITY_ALWAYS)
    {
        return;
    }

    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
    {
        return;
    }

    if (!(level & ALL_GUILDFINDER_LEVELS) || level > ALL_GUILDFINDER_LEVELS)
    {
        return;
    }

    Player* player = GetPlayer();
    if (!player->getGuildId())
        return;

    if (Guild* guild = sGuildMgr.getGuildById(player->getGuildId()))
    {
        if (guild->getLeaderGUID() != player->getGuid())
        {
            return;
        }
    }

    LFGuildSettings settings(listed, player->GetTeam(), player->getGuildId(), static_cast<uint8_t>(classRoles), static_cast<uint8_t>(availability), static_cast<uint8_t>(guildInterests), static_cast<uint8_t>(level), comment);
    sGuildFinderMgr.setGuildSettings(player->getGuildId(), settings);
}
