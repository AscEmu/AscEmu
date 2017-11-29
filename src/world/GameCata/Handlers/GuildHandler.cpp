/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"
#include "GameCata/Management/GuildFinderMgr.h"
#include "GameCata/Management/GuildMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Players/Player.h"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/WorldStrings.h"


//////////////////////////////////////////////////////////////////////////////////////////
// Guild

void WorldSession::HandleGuildQueryOpcode(WorldPacket& recv_data)
{
    uint64_t guildGuid;
    uint64_t playerGuid;

    recv_data >> guildGuid;
    recv_data >> playerGuid;

    uint32_t guildId = uint32_t(guildGuid);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_QUERY %s: GuildId: %u Target: %u", _player->GetName(), guildId, Arcemu::Util::GUID_LOPART(playerGuid));

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        if (guild->isMember(playerGuid))
        {
            guild->handleQuery(this);
        }
    }
}

void WorldSession::HandleInviteToGuildOpcode(WorldPacket& recv_data)
{
    uint32_t nameLength;
    std::string invitedName;

    nameLength = recv_data.readBits(7);
    invitedName = recv_data.ReadString(nameLength);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_INVITE %s: Invited: %s", _player->GetName(), invitedName.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleInviteMember(this, invitedName);
    }
}

void WorldSession::HandleGuildRemoveOpcode(WorldPacket& recv_data)
{
    ObjectGuid playerGuid;

    playerGuid[6] = recv_data.readBit();
    playerGuid[5] = recv_data.readBit();
    playerGuid[4] = recv_data.readBit();
    playerGuid[0] = recv_data.readBit();
    playerGuid[1] = recv_data.readBit();
    playerGuid[3] = recv_data.readBit();
    playerGuid[7] = recv_data.readBit();
    playerGuid[2] = recv_data.readBit();

    recv_data.ReadByteSeq(playerGuid[2]);
    recv_data.ReadByteSeq(playerGuid[6]);
    recv_data.ReadByteSeq(playerGuid[5]);
    recv_data.ReadByteSeq(playerGuid[7]);
    recv_data.ReadByteSeq(playerGuid[1]);
    recv_data.ReadByteSeq(playerGuid[4]);
    recv_data.ReadByteSeq(playerGuid[3]);
    recv_data.ReadByteSeq(playerGuid[0]);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleRemoveMember(this, playerGuid);
    }
}

void WorldSession::HandleGuildAcceptOpcode(WorldPacket& /*recv_data*/)
{
    if (!GetPlayer()->GetGuildId())
    {
        if (Guild* guild = sGuildMgr.getGuildById(GetPlayer()->GetGuildIdInvited()))
        {
            guild->handleAcceptMember(this);
        }
    }

}

void WorldSession::HandleGuildDeclineOpcode(WorldPacket& /*recv_data*/)
{
    GetPlayer()->SetGuildIdInvited(0);
    GetPlayer()->SetInGuild(0);

}

void WorldSession::HandleGuildRosterOpcode(WorldPacket& /*recv_data*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleRoster(this);
    }
    else
    {
        Guild::sendCommandResult(this, GC_TYPE_ROSTER, GC_ERROR_PLAYER_NOT_IN_GUILD);
    }
}

void WorldSession::HandleGuildPromoteOpcode(WorldPacket& recv_data)
{
    ObjectGuid targetGuid;

    targetGuid[7] = recv_data.readBit();
    targetGuid[2] = recv_data.readBit();
    targetGuid[5] = recv_data.readBit();
    targetGuid[6] = recv_data.readBit();
    targetGuid[1] = recv_data.readBit();
    targetGuid[0] = recv_data.readBit();
    targetGuid[3] = recv_data.readBit();
    targetGuid[4] = recv_data.readBit();

    recv_data.ReadByteSeq(targetGuid[0]);
    recv_data.ReadByteSeq(targetGuid[5]);
    recv_data.ReadByteSeq(targetGuid[2]);
    recv_data.ReadByteSeq(targetGuid[3]);
    recv_data.ReadByteSeq(targetGuid[6]);
    recv_data.ReadByteSeq(targetGuid[4]);
    recv_data.ReadByteSeq(targetGuid[1]);
    recv_data.ReadByteSeq(targetGuid[7]);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_PROMOTE %s: Target: %u", _player->GetName(), Arcemu::Util::GUID_LOPART(targetGuid));

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleUpdateMemberRank(this, targetGuid, false);
    }
}

void WorldSession::HandleGuildAssignRankOpcode(WorldPacket& recv_data)
{
    ObjectGuid targetGuid;
    ObjectGuid setterGuid;

    uint32_t rankId;
    recv_data >> rankId;

    targetGuid[1] = recv_data.readBit();
    targetGuid[7] = recv_data.readBit();

    setterGuid[4] = recv_data.readBit();
    setterGuid[2] = recv_data.readBit();

    targetGuid[4] = recv_data.readBit();
    targetGuid[5] = recv_data.readBit();
    targetGuid[6] = recv_data.readBit();

    setterGuid[1] = recv_data.readBit();
    setterGuid[7] = recv_data.readBit();

    targetGuid[2] = recv_data.readBit();
    targetGuid[3] = recv_data.readBit();
    targetGuid[0] = recv_data.readBit();

    setterGuid[6] = recv_data.readBit();
    setterGuid[3] = recv_data.readBit();
    setterGuid[0] = recv_data.readBit();
    setterGuid[5] = recv_data.readBit();

    recv_data.ReadByteSeq(targetGuid[0]);

    recv_data.ReadByteSeq(setterGuid[1]);
    recv_data.ReadByteSeq(setterGuid[3]);
    recv_data.ReadByteSeq(setterGuid[5]);

    recv_data.ReadByteSeq(targetGuid[7]);
    recv_data.ReadByteSeq(targetGuid[3]);

    recv_data.ReadByteSeq(setterGuid[0]);

    recv_data.ReadByteSeq(targetGuid[1]);

    recv_data.ReadByteSeq(setterGuid[6]);

    recv_data.ReadByteSeq(targetGuid[2]);
    recv_data.ReadByteSeq(targetGuid[5]);
    recv_data.ReadByteSeq(targetGuid[4]);

    recv_data.ReadByteSeq(setterGuid[2]);
    recv_data.ReadByteSeq(setterGuid[4]);

    recv_data.ReadByteSeq(targetGuid[6]);

    recv_data.ReadByteSeq(setterGuid[7]);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_ASSIGN_MEMBER_RANK %s: Target: %u Rank: %u, Issuer: %u",
        _player->GetName(), Arcemu::Util::GUID_LOPART(targetGuid), rankId, Arcemu::Util::GUID_LOPART(setterGuid));

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleSetMemberRank(this, targetGuid, setterGuid, rankId);
    }
}

void WorldSession::HandleGuildDemoteOpcode(WorldPacket& recv_data)
{
    ObjectGuid targetGuid;

    targetGuid[7] = recv_data.readBit();
    targetGuid[1] = recv_data.readBit();
    targetGuid[5] = recv_data.readBit();
    targetGuid[6] = recv_data.readBit();
    targetGuid[2] = recv_data.readBit();
    targetGuid[3] = recv_data.readBit();
    targetGuid[0] = recv_data.readBit();
    targetGuid[4] = recv_data.readBit();

    recv_data.ReadByteSeq(targetGuid[1]);
    recv_data.ReadByteSeq(targetGuid[2]);
    recv_data.ReadByteSeq(targetGuid[7]);
    recv_data.ReadByteSeq(targetGuid[5]);
    recv_data.ReadByteSeq(targetGuid[6]);
    recv_data.ReadByteSeq(targetGuid[0]);
    recv_data.ReadByteSeq(targetGuid[4]);
    recv_data.ReadByteSeq(targetGuid[3]);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_DEMOTE %s: Target: %u", _player->GetName(), Arcemu::Util::GUID_LOPART(targetGuid));

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleUpdateMemberRank(this, targetGuid, true);
    }
}

void WorldSession::HandleGuildLeaveOpcode(WorldPacket& /*recv_data*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleLeaveMember(this);
    }
}

void WorldSession::HandleGuildDisbandOpcode(WorldPacket& /*recv_data*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleDisband(this);
    }
}

void WorldSession::HandleGuildLeaderOpcode(WorldPacket& recvData)
{
    uint8_t nameLength = static_cast<uint8_t>(recvData.readBits(7));
    /*bool inactive = */recvData.readBit();                 // bool inactive?
    std::string playerName = recvData.ReadString(nameLength);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleSetNewGuildMaster(this, playerName);
    }
}

void WorldSession::HandleGuildMotdOpcode(WorldPacket& recv_data)
{
    uint32_t motdLength;
    std::string motd;

    motdLength = recv_data.readBits(11);
    motd = recv_data.ReadString(motdLength);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_MOTD %s: MOTD: %s", _player->GetName(), motd.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleSetMOTD(this, motd);
    }
}

void WorldSession::HandleGuildSetNoteOpcode(WorldPacket& recv_data)
{
    bool ispublic;          // 0 officer, 1 public
    uint32_t noteLength;
    std::string note;

    ObjectGuid playerGuid;

    playerGuid[1] = recv_data.readBit();
    playerGuid[4] = recv_data.readBit();
    playerGuid[5] = recv_data.readBit();
    playerGuid[3] = recv_data.readBit();
    playerGuid[0] = recv_data.readBit();
    playerGuid[7] = recv_data.readBit();

    ispublic = recv_data.readBit();

    playerGuid[6] = recv_data.readBit();

    noteLength = recv_data.readBits(8);

    playerGuid[2] = recv_data.readBit();

    recv_data.ReadByteSeq(playerGuid[4]);
    recv_data.ReadByteSeq(playerGuid[5]);
    recv_data.ReadByteSeq(playerGuid[0]);
    recv_data.ReadByteSeq(playerGuid[3]);
    recv_data.ReadByteSeq(playerGuid[1]);
    recv_data.ReadByteSeq(playerGuid[6]);
    recv_data.ReadByteSeq(playerGuid[7]);

    note = recv_data.ReadString(noteLength);

    recv_data.ReadByteSeq(playerGuid[2]);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_SET_NOTE %s: Target: %u, Note: %s, Public: %u",
        _player->GetName(), Arcemu::Util::GUID_LOPART(playerGuid), note.c_str(), ispublic);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleSetMemberNote(this, note, playerGuid, ispublic);
    }
}

void WorldSession::HandleGuildQueryRanksOpcode(WorldPacket& recv_data)
{
    ObjectGuid guildGuid;

    guildGuid[2] = recv_data.readBit();
    guildGuid[3] = recv_data.readBit();
    guildGuid[0] = recv_data.readBit();
    guildGuid[6] = recv_data.readBit();
    guildGuid[4] = recv_data.readBit();
    guildGuid[7] = recv_data.readBit();
    guildGuid[5] = recv_data.readBit();
    guildGuid[1] = recv_data.readBit();

    recv_data.ReadByteSeq(guildGuid[3]);
    recv_data.ReadByteSeq(guildGuid[4]);
    recv_data.ReadByteSeq(guildGuid[5]);
    recv_data.ReadByteSeq(guildGuid[7]);
    recv_data.ReadByteSeq(guildGuid[1]);
    recv_data.ReadByteSeq(guildGuid[0]);
    recv_data.ReadByteSeq(guildGuid[6]);
    recv_data.ReadByteSeq(guildGuid[2]);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_QUERY_RANKS %s: Guild: %u", _player->GetName(), Arcemu::Util::GUID_LOPART(guildGuid));

    if (Guild* guild = sGuildMgr.getGuildById(Arcemu::Util::GUID_LOPART(guildGuid)))
    {
        if (guild->isMember(_player->GetGUID()))
        {
            guild->sendGuildRankInfo(this);
        }
    }
}

void WorldSession::HandleGuildAddRankOpcode(WorldPacket& recv_data)
{
    uint32_t rankId;
    uint32_t length;
    std::string rankName;

    recv_data >> rankId;

    length = recv_data.readBits(7);
    rankName = recv_data.ReadString(length);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_ADD_RANK %s: Rank: %s", _player->GetName(), rankName.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleAddNewRank(this, rankName);
    }
}

void WorldSession::HandleGuildDelRankOpcode(WorldPacket& recvData)
{
    uint32_t rankId;
    recvData >> rankId;

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_DEL_RANK %s: Rank: %u", _player->GetName(), rankId);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleRemoveRank(this, static_cast<uint8_t>(rankId));
    }
}

void WorldSession::HandleGuildChangeInfoTextOpcode(WorldPacket& recvData)
{
    std::string info;

    uint32_t length = static_cast<uint32_t>(recvData.readBits(12));
    info = recvData.ReadString(length);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_INFO_TEXT %s: %s", _player->GetName(), info.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleSetInfo(this, info);
    }
}

void WorldSession::HandleSaveGuildEmblemOpcode(WorldPacket& recv_data)
{
    uint64_t vendorGuid;
    EmblemInfo emblemInfo;

    recv_data >> vendorGuid;
    emblemInfo.readEmblemInfoFromPacket(recv_data);

    LogDebugFlag(LF_OPCODE, "MSG_SAVE_GUILD_EMBLEM %s: vendorGuid: %u style: %u, color: %u, borderStyle: %u, borderColor: %u, backgroundColor: %u", _player->GetName(),
        Arcemu::Util::GUID_LOPART(vendorGuid), emblemInfo.getStyle(), emblemInfo.getColor(), emblemInfo.getBorderStyle(), emblemInfo.getBorderColor(), emblemInfo.getBackgroundColor());

    if (GetPlayer()->GetGuild()->getLeaderGUID() != _player->GetGUID())
    {
        Guild::sendSaveEmblemResult(this, GEM_ERROR_NOTGUILDMASTER);
    }

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleSetEmblem(this, emblemInfo);
    }
    else
    {
        Guild::sendSaveEmblemResult(this, GEM_ERROR_NOGUILD);
    }
}

void WorldSession::HandleGuildEventLogQueryOpcode(WorldPacket& /*recv_data*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->sendEventLog(this);
    }
}

void WorldSession::HandleGuildRequestChallengeUpdate(WorldPacket& /*recv_data*/)
{
    if (Guild* guild = _player->GetGuild())
    {
        guild->handleGuildRequestChallengeUpdate(this);
    }
}

void WorldSession::HandleGuildPermissions(WorldPacket& /*recv_data*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->sendPermissions(this);
    }
}

void WorldSession::HandleGuildQueryXPOpcode(WorldPacket& recv_data)
{
    ObjectGuid guildGuid;

    guildGuid[2] = recv_data.readBit();
    guildGuid[1] = recv_data.readBit();
    guildGuid[0] = recv_data.readBit();
    guildGuid[5] = recv_data.readBit();
    guildGuid[4] = recv_data.readBit();
    guildGuid[7] = recv_data.readBit();
    guildGuid[6] = recv_data.readBit();
    guildGuid[3] = recv_data.readBit();

    recv_data.ReadByteSeq(guildGuid[7]);
    recv_data.ReadByteSeq(guildGuid[2]);
    recv_data.ReadByteSeq(guildGuid[3]);
    recv_data.ReadByteSeq(guildGuid[6]);
    recv_data.ReadByteSeq(guildGuid[1]);
    recv_data.ReadByteSeq(guildGuid[5]);
    recv_data.ReadByteSeq(guildGuid[0]);
    recv_data.ReadByteSeq(guildGuid[4]);

    uint32_t guildId = Arcemu::Util::GUID_LOPART(guildGuid);

    LogDebugFlag(LF_OPCODE, "CMSG_QUERY_GUILD_XP %s: guildId: %u", _player->GetName(), guildId);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        if (guild->isMember(_player->GetGUID()))
        {
            guild->sendGuildXP(this);
        }
    }
}

void WorldSession::HandleGuildSetRankPermissionsOpcode(WorldPacket& recv_data)
{
    Guild* guild = GetPlayer()->GetGuild();
    if (guild == nullptr)
    {
        return;
    }

    uint32_t oldRankId;
    uint32_t newRankId;
    uint32_t oldRights;
    uint32_t newRights;
    uint32_t moneyPerDay;

    recv_data >> oldRankId;
    recv_data >> oldRights;
    recv_data >> newRights;

    GuildBankRightsAndSlotsVec rightsAndSlots(MAX_GUILD_BANK_TABS);
    for (uint8_t tabId = 0; tabId < MAX_GUILD_BANK_TABS; ++tabId)
    {
        uint32_t bankRights;
        uint32_t slots;

        recv_data >> bankRights;
        recv_data >> slots;

        rightsAndSlots[tabId] = GuildBankRightsAndSlots(tabId, uint8_t(bankRights), slots);
    }

    recv_data >> moneyPerDay;
    recv_data >> newRankId;

    uint32_t nameLength = recv_data.readBits(7);
    std::string rankName = recv_data.ReadString(nameLength);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_SET_RANK_PERMISSIONS %s: rank: %s (%u)", _player->GetName(), rankName.c_str(), newRankId);

    guild->handleSetRankInfo(this, static_cast<uint8_t>(newRankId), rankName, newRights, moneyPerDay, rightsAndSlots);
}

void WorldSession::HandleGuildRequestPartyState(WorldPacket& recv_data)
{
    ObjectGuid guildGuid;

    guildGuid[0] = recv_data.readBit();
    guildGuid[6] = recv_data.readBit();
    guildGuid[7] = recv_data.readBit();
    guildGuid[3] = recv_data.readBit();
    guildGuid[5] = recv_data.readBit();
    guildGuid[1] = recv_data.readBit();
    guildGuid[2] = recv_data.readBit();
    guildGuid[4] = recv_data.readBit();

    recv_data.ReadByteSeq(guildGuid[6]);
    recv_data.ReadByteSeq(guildGuid[3]);
    recv_data.ReadByteSeq(guildGuid[2]);
    recv_data.ReadByteSeq(guildGuid[1]);
    recv_data.ReadByteSeq(guildGuid[5]);
    recv_data.ReadByteSeq(guildGuid[0]);
    recv_data.ReadByteSeq(guildGuid[7]);
    recv_data.ReadByteSeq(guildGuid[4]);

    uint32_t guildId = Arcemu::Util::GUID_LOPART(guildGuid);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        guild->handleGuildPartyRequest(this);
    }
}

void WorldSession::HandleGuildRequestMaxDailyXP(WorldPacket& recv_data)
{
    ObjectGuid guid;

    guid[0] = recv_data.readBit();
    guid[3] = recv_data.readBit();
    guid[5] = recv_data.readBit();
    guid[1] = recv_data.readBit();
    guid[4] = recv_data.readBit();
    guid[6] = recv_data.readBit();
    guid[7] = recv_data.readBit();
    guid[2] = recv_data.readBit();

    recv_data.ReadByteSeq(guid[7]);
    recv_data.ReadByteSeq(guid[4]);
    recv_data.ReadByteSeq(guid[3]);
    recv_data.ReadByteSeq(guid[5]);
    recv_data.ReadByteSeq(guid[1]);
    recv_data.ReadByteSeq(guid[2]);
    recv_data.ReadByteSeq(guid[6]);
    recv_data.ReadByteSeq(guid[0]);

    uint32_t guildId = Arcemu::Util::GUID_LOPART(guid);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        if (guild->isMember(_player->GetGUID()))
        {
            WorldPacket data(SMSG_GUILD_MAX_DAILY_XP, 8);
            data << uint64_t(worldConfig.guild.maxXpPerDay);
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleAutoDeclineGuildInvites(WorldPacket& recv_data)
{
    uint8_t enable;
    recv_data >> enable;

    bool enabled = enable > 0 ? true : false;

    GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_AUTO_DECLINE_GUILD, enabled);
}

void WorldSession::HandleGuildRewardsQueryOpcode(WorldPacket& recv_data)
{
    recv_data.read_skip<uint32_t>();

    if (sGuildMgr.getGuildById(_player->GetGuildId()))
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

void WorldSession::HandleGuildQueryNewsOpcode(WorldPacket& recv_data)
{
    recv_data.read_skip<uint32_t>();

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->sendNewsUpdate(this);
    }
}

void WorldSession::HandleGuildNewsUpdateStickyOpcode(WorldPacket& recv_data)
{
    uint32_t newsId;
    bool isSticky;

    ObjectGuid guid;

    recv_data >> newsId;

    guid[2] = recv_data.readBit();
    guid[4] = recv_data.readBit();
    guid[3] = recv_data.readBit();
    guid[0] = recv_data.readBit();

    isSticky = recv_data.readBit();

    guid[6] = recv_data.readBit();
    guid[7] = recv_data.readBit();
    guid[1] = recv_data.readBit();
    guid[5] = recv_data.readBit();

    recv_data.ReadByteSeq(guid[6]);
    recv_data.ReadByteSeq(guid[2]);
    recv_data.ReadByteSeq(guid[1]);
    recv_data.ReadByteSeq(guid[0]);
    recv_data.ReadByteSeq(guid[5]);
    recv_data.ReadByteSeq(guid[3]);
    recv_data.ReadByteSeq(guid[7]);
    recv_data.ReadByteSeq(guid[4]);

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
// Guild Bank

void WorldSession::HandleGuildBankMoneyWithdrawn(WorldPacket& /*recv_data*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->sendMoneyInfo(this);
    }
}

void WorldSession::HandleGuildBankerActivate(WorldPacket& recv_data)
{
    uint64_t bankGuid;
    bool sendAllSlots;

    recv_data >> bankGuid;
    recv_data >> sendAllSlots;

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_BANKER_ACTIVATE %s: gameobject: %u allSlots: %u",
        _player->GetName(), Arcemu::Util::GUID_LOPART(bankGuid), sendAllSlots);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->sendBankList(this, 0, true, true);
    }
    else
    {
        Guild::sendCommandResult(this, GC_TYPE_VIEW_TAB, GC_ERROR_PLAYER_NOT_IN_GUILD);
        return;
    }
}

void WorldSession::HandleGuildBankQueryTab(WorldPacket& recv_data)
{
    uint64_t bankGuid;
    uint8_t tabId;
    bool sendAllSlots;

    recv_data >> bankGuid;
    recv_data >> tabId;
    recv_data >> sendAllSlots;

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_BANK_QUERY_TAB %s: gameobject: %u, tabId: %u, allSlots: %u",
        _player->GetName(), Arcemu::Util::GUID_LOPART(bankGuid), tabId, sendAllSlots);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->sendBankList(this, tabId, true, false);
    }
}

void WorldSession::HandleGuildBankDepositMoney(WorldPacket& recv_data)
{
    uint64_t bankGuid;
    uint64_t money;

    recv_data >> bankGuid;
    recv_data >> money;

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_BANK_DEPOSIT_MONEY %s: gameobject: %u, money: " I64FMTD,
        _player->GetName(), Arcemu::Util::GUID_LOPART(bankGuid), money);

    if (money && GetPlayer()->HasGold(static_cast<uint32_t>(money)))
    {
        if (Guild* guild = GetPlayer()->GetGuild())
        {
            guild->handleMemberDepositMoney(this, money);
        }
    }
}

void WorldSession::HandleGuildBankWithdrawMoney(WorldPacket& recv_data)
{
    uint64_t bankGuid;
    uint64_t money;

    recv_data >> bankGuid;
    recv_data >> money;

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_BANK_WITHDRAW_MONEY %s: gameobject: %u, money: " I64FMTD,
        _player->GetName(), Arcemu::Util::GUID_LOPART(bankGuid), money);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleMemberWithdrawMoney(this, money);
    }
}

void WorldSession::HandleGuildBankSwapItems(WorldPacket& recv_data)
{
    Guild* guild = GetPlayer()->GetGuild();
    if (guild == nullptr)
    {
        recv_data.rfinish();
        return;
    }

    uint64_t bankGuid;
    recv_data >> bankGuid;


    uint8_t bankToBank;
    recv_data >> bankToBank;

    uint8_t tabId;
    uint8_t slotId;
    uint32_t itemEntry;
    uint32_t splitedAmount = 0;

    if (bankToBank)
    {
        uint8_t destTabId;
        recv_data >> destTabId;

        uint8_t destSlotId;
        recv_data >> destSlotId;

        uint32_t destItemEntry;
        recv_data >> destItemEntry;

        recv_data >> tabId;
        recv_data >> slotId;
        recv_data >> itemEntry;

        recv_data.read_skip<uint8_t>();

        recv_data >> splitedAmount;

        guild->swapItems(GetPlayer(), tabId, slotId, destTabId, destSlotId, splitedAmount);
    }
    else
    {
        uint8_t playerBag = 0;
        uint8_t playerSlotId = UNDEFINED_TAB_SLOT;
        uint8_t toCharNum = 1;

        recv_data >> tabId;
        recv_data >> slotId;
        recv_data >> itemEntry;

        uint8_t autoStore;
        recv_data >> autoStore;

        if (autoStore)
        {
            recv_data.read_skip<uint32_t>();
            recv_data.read_skip<uint8_t>();
            recv_data.read_skip<uint32_t>();
        }
        else
        {
            recv_data >> playerBag;
            recv_data >> playerSlotId;
            recv_data >> toCharNum;
            recv_data >> splitedAmount;
        }

        bool toChar = toCharNum > 0 ? true : false;

        guild->swapItemsWithInventory(GetPlayer(), toChar, tabId, slotId, playerBag, playerSlotId, splitedAmount);
    }
}

void WorldSession::HandleGuildBankBuyTab(WorldPacket& recv_data)
{
    uint64_t bankGuid;
    uint8_t tabId;

    recv_data >> bankGuid;
    recv_data >> tabId;

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_BANK_BUY_TAB %s: gameobject: %u, TabId: %u", _player->GetName(), Arcemu::Util::GUID_LOPART(bankGuid), tabId);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->handleBuyBankTab(this, tabId);
    }
}

void WorldSession::HandleGuildBankUpdateTab(WorldPacket& recv_data)
{
    uint64_t bankGuid;
    uint8_t tabId;
    std::string name;
    std::string icon;

    recv_data >> bankGuid;
    recv_data >> tabId;
    recv_data >> name;
    recv_data >> icon;

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_BANK_UPDATE_TAB %s: gameobject: %u, tabId: %u, name: %s, icon: %s",
        _player->GetName(), Arcemu::Util::GUID_LOPART(bankGuid), tabId, name.c_str(), icon.c_str());

    if (name.empty() == false && icon.empty() == false)
    {
        if (Guild* guild = GetPlayer()->GetGuild())
        {
            guild->handleSetBankTabInfo(this, tabId, name, icon);
        }
    }
}

void WorldSession::HandleGuildBankLogQuery(WorldPacket& recvData)
{
    uint32_t tabId;
    recvData >> tabId;

    LogDebugFlag(LF_OPCODE, "MSG_GUILD_BANK_LOG_QUERY %s: tabId: %u", _player->GetName(), tabId);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->sendBankLog(this, static_cast<uint8_t>(tabId));
    }
}

void WorldSession::HandleQueryGuildBankTabText(WorldPacket &recv_data)
{
    uint8_t tabId;
    recv_data >> tabId;

    LogDebugFlag(LF_OPCODE, "MSG_QUERY_GUILD_BANK_TEXT %s: tabId: %u", _player->GetName(), tabId);

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->sendBankTabText(this, tabId);
    }
}

void WorldSession::HandleSetGuildBankTabText(WorldPacket& recvData)
{
    uint32_t tabId;
    std::string text;

    recvData >> tabId;

    uint32_t textLen = recvData.readBits(14);
    text = recvData.ReadString(textLen);

    LogDebugFlag(LF_OPCODE, "CMSG_SET_GUILD_BANK_TEXT %s: tabId: %u, text: %s", _player->GetName(), tabId, text.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
    {
        guild->setBankTabText(static_cast<uint8_t>(tabId), text);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Charter

void WorldSession::HandleCharterBuyOpcode(WorldPacket& recv_data)
{
    uint8_t error;

    uint64_t creatureGuid;
    uint32_t unk1;
    uint64_t unk2;
    std::string name;
    std::string unkString;
    uint32_t charterData[7];
    uint16_t unk3;
    uint32_t unk4;
    uint32_t unk5;
    uint32_t petitionSignerCount;
    std::string unkString2;
    uint32_t arenaIndex;

    recv_data >> creatureGuid;
    recv_data >> unk1;
    recv_data >> unk2;
    recv_data >> name;
    recv_data >> unkString;

    for (uint8_t i = 0; i < 7; ++i)
    {
        recv_data >> charterData[i];
    }

    recv_data >> unk3;
    recv_data >> unk4;
    recv_data >> unk5;
    recv_data >> petitionSignerCount;

    for (uint32_t s = 0; s < 10; ++s)
    {
        recv_data >> unkString2;
    }

    recv_data >> arenaIndex;

    Creature* creature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(creatureGuid));
    if (creature == nullptr)
    {
        Disconnect();
        return;
    }

    if (!creature->isTabardDesigner())
    {
        uint32_t arena_type = arenaIndex - 1;
        if (arena_type > 2)
        {
            return;
        }

        if (_player->m_arenaTeams[arena_type])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(71));
            return;
        }

        ArenaTeam* arenTeam = objmgr.GetArenaTeamByName(name, arena_type);
        if (arenTeam != nullptr)
        {
            sChatHandler.SystemMessage(this, _player->GetSession()->LocalizedWorldSrv(72));
            return;
        }

        if (objmgr.GetCharterByName(name, (CharterTypes)arenaIndex))
        {
            sChatHandler.SystemMessage(this, _player->GetSession()->LocalizedWorldSrv(72));
            return;
        }

        if (_player->m_charters[arenaIndex])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(73));
            return;
        }

        if (_player->getLevel() < PLAYER_ARENA_MIN_LEVEL)
        {
            //\todo Replace by LocalizedWorldSrv(..)
            SendNotification("You must be at least level %u to buy Arena charter", PLAYER_ARENA_MIN_LEVEL);
            return;
        }

        static uint32_t item_ids[] = { ARENA_TEAM_CHARTER_2v2, ARENA_TEAM_CHARTER_3v3, ARENA_TEAM_CHARTER_5v5 };
        static uint32_t costs[] = { ARENA_TEAM_CHARTER_2v2_COST, ARENA_TEAM_CHARTER_3v3_COST, ARENA_TEAM_CHARTER_5v5_COST };

        if (!_player->HasGold(costs[arena_type]))
        {
            return;
        }

        ItemProperties const* itemProperties = sMySQLStore.getItemProperties(item_ids[arena_type]);
        ARCEMU_ASSERT(itemProperties != nullptr);
        SlotResult res = _player->GetItemInterface()->FindFreeInventorySlot(itemProperties);
        if (res.Result == 0)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(0, 0, INV_ERR_INVENTORY_FULL);
            return;
        }

        error = _player->GetItemInterface()->CanReceiveItem(itemProperties, 1);
        if (error)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, error);
        }
        else
        {
            Item* item = objmgr.CreateItem(item_ids[arena_type], _player);
            Charter* charter = objmgr.CreateCharter(_player->GetLowGUID(), (CharterTypes)arenaIndex);
            if (item == nullptr || charter == nullptr)
            {
                return;
            }

            charter->GuildName = name;
            charter->ItemGuid = item->GetGUID();

            charter->UnkString = unkString;
            charter->Unk1 = unk3;
            charter->Unk2 = unk4;
            charter->Unk3 = unk5;
            charter->PetitionSignerCount = petitionSignerCount;
            memcpy(charter->Data, charterData, sizeof(charterData));

            item->setUInt32Value(ITEM_FIELD_STACK_COUNT, 1);
            item->SoulBind();
            item->SetEnchantmentId(0, charter->GetID());
            item->SetItemRandomSuffixFactor(57813883);
            if (!_player->GetItemInterface()->AddItemToFreeSlot(item))
            {
                charter->Destroy();
                item->DeleteMe();
                return;
            }

            charter->SaveToDB();

            _player->SendItemPushResult(false, true, false, true, _player->GetItemInterface()->LastSearchItemBagSlot(), _player->GetItemInterface()->LastSearchItemSlot(), 1, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());

            _player->ModGold(-(int32_t)costs[arena_type]);
            _player->m_charters[arenaIndex] = charter;
            _player->SaveToDB(false);
        }
    }
    else
    {
        if (_player->HasGold(1000) == false)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_NOT_ENOUGH_MONEY);
            return;
        }

        Guild* guild = sGuildMgr.getGuildByName(name);
        Charter* charter = objmgr.GetCharterByName(name, CHARTER_TYPE_GUILD);
        if (guild != nullptr || charter != nullptr)
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(74));
            return;
        }

        if (_player->m_charters[CHARTER_TYPE_GUILD])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(75));
            return;
        }

        ItemProperties const* itemProperties = sMySQLStore.getItemProperties(ITEM_ENTRY_GUILD_CHARTER);
        ARCEMU_ASSERT(itemProperties != nullptr);
        SlotResult slotResult = _player->GetItemInterface()->FindFreeInventorySlot(itemProperties);
        if (slotResult.Result == 0)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(0, 0, INV_ERR_INVENTORY_FULL);
            return;
        }

        error = _player->GetItemInterface()->CanReceiveItem(sMySQLStore.getItemProperties(ITEM_ENTRY_GUILD_CHARTER), 1);
        if (error)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, error);
        }
        else
        {
            WorldPacket data(SMSG_PLAY_OBJECT_SOUND, 12);
            data << uint32_t(0x000019C2);
            data << creatureGuid;
            SendPacket(&data);

            Item* item = objmgr.CreateItem(ITEM_ENTRY_GUILD_CHARTER, _player);
            charter = objmgr.CreateCharter(_player->GetLowGUID(), CHARTER_TYPE_GUILD);
            if (item == nullptr || charter == nullptr)
            {
                return;
            }

            charter->GuildName = name;
            charter->ItemGuid = item->GetGUID();

            charter->UnkString = unkString;
            charter->Unk1 = unk3;
            charter->Unk2 = unk4;
            charter->Unk3 = unk5;
            charter->PetitionSignerCount = petitionSignerCount;
            memcpy(charter->Data, charterData, sizeof(charterData));

            item->SetStackCount(1);
            item->SoulBind();
            item->SetEnchantmentId(0, charter->GetID());
            item->SetItemRandomSuffixFactor(57813883);
            if (!_player->GetItemInterface()->AddItemToFreeSlot(item))
            {
                charter->Destroy();
                item->DeleteMe();
                return;
            }

            charter->SaveToDB();

            _player->SendItemPushResult(false, true, false, true, _player->GetItemInterface()->LastSearchItemBagSlot(), _player->GetItemInterface()->LastSearchItemSlot(),
                1, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());

            _player->m_charters[CHARTER_TYPE_GUILD] = charter;
            _player->ModGold(-1000);
            _player->SaveToDB(false);
        }
    }
}

void SendShowSignatures(Charter* charter, uint64_t itemGuid, Player* player)
{
    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, 100);
    data << uint64_t(itemGuid);
    data << uint64_t(charter->GetLeader());
    data << uint32_t(charter->GetID());
    data << uint8_t(charter->SignatureCount);
    for (uint32_t j = 0; j < charter->Slots; ++j)
    {
        if (charter->Signatures[j] == 0)
        {
            continue;
        }

        data << uint64_t(charter->Signatures[j]);
        data << uint32_t(1);
    }
    data << uint8_t(0);
    player->GetSession()->SendPacket(&data);
}

void WorldSession::HandleCharterShowSignaturesOpcode(WorldPacket& recv_data)
{
    uint64_t itemGuid;
    recv_data >> itemGuid;

    if (Charter* pCharter = objmgr.GetCharterByItemGuid(itemGuid))
    {
        SendShowSignatures(pCharter, itemGuid, _player);
    }
}

void WorldSession::HandleCharterQueryOpcode(WorldPacket& recv_data)
{
    uint32_t charterId;
    uint64_t itemGuid;

    recv_data >> charterId;
    recv_data >> itemGuid;

    Charter* charter = objmgr.GetCharterByItemGuid(itemGuid);
    if (charter == nullptr)
    {
        return;
    }

    WorldPacket data(SMSG_PETITION_QUERY_RESPONSE, 100);
    data << charterId;

    data << uint64_t(charter->LeaderGuid);
    data << charter->GuildName;

    data << uint8_t(0);

    if (charter->CharterType == CHARTER_TYPE_GUILD)
    {
        data << uint32_t(9);
        data << uint32_t(9);
    }
    else
    {
        data << uint32_t(charter->Slots);
        data << uint32_t(charter->Slots);
    }

    data << uint32_t(0);            // 4
    data << uint32_t(0);            // 5
    data << uint32_t(0);            // 6
    data << uint32_t(0);            // 7
    data << uint32_t(0);            // 8
    data << uint16_t(0);            // 9 2 bytes field

    if (charter->CharterType == CHARTER_TYPE_GUILD)
    {
        data << uint32_t(1);        // 10 min level to sign a guild charter
        data << uint32_t(80);       // 11 max level to sign a guild charter
    }
    else
    {
        data << uint32_t(80);       // 10 min level to sign an arena charter
        data << uint32_t(80);       // 11 max level to sign an arena charter
    }

    data << uint32_t(0);            // 12
    data << uint32_t(0);            // 13 count of next strings?
    data << uint32_t(0);            // 14
    data << uint32_t(0);            // 15
    data << uint16_t(0);            // 16

    if (charter->CharterType == CHARTER_TYPE_GUILD)
    {
        data << uint32_t(0);
    }
    else
    {
        data << uint32_t(1);
    }

    SendPacket(&data);
}

void WorldSession::HandleCharterOfferOpcode(WorldPacket& recv_data)
{
    uint32_t unk;
    uint64_t itemGuid;
    uint64_t targetGuid;

    recv_data >> unk;
    recv_data >> itemGuid;
    recv_data >> targetGuid;

    
    Charter* charter = objmgr.GetCharterByItemGuid(itemGuid);
    if (charter == nullptr)
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(76));
        return;
    }

    Player* pTarget = _player->GetMapMgr()->GetPlayer((uint32_t)targetGuid);
    if (pTarget == nullptr || pTarget->GetTeam() != _player->GetTeam() ||
        (pTarget == _player && worldConfig.player.isInterfactionGuildEnabled == false))
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(77));
        return;
    }

    if (pTarget->CanSignCharter(charter, _player) == false)
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(78));
        return;
    }

    SendShowSignatures(charter, itemGuid, pTarget);
}

void WorldSession::HandleCharterSignOpcode(WorldPacket& recv_data)
{
    uint64_t itemGuid;
    recv_data >> itemGuid;

    Charter* charter = objmgr.GetCharterByItemGuid(itemGuid);
    if (charter == nullptr)
    {
        return;
    }

    for (uint32_t i = 0; i < charter->SignatureCount; ++i)
    {
        if (charter->Signatures[i] == _player->GetGUID())
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(79));
            return;
        }
    }

    if (charter->IsFull())
    {
        return;
    }

    charter->AddSignature(_player->GetLowGUID());
    charter->SaveToDB();
    _player->m_charters[charter->CharterType] = charter;
    _player->SaveToDB(false);

    Player* player = _player->GetMapMgr()->GetPlayer(charter->GetLeader());
    if (player == nullptr)
    {
        return;
    }

    WorldPacket data(SMSG_PETITION_SIGN_RESULTS, 100);
    data << uint64_t(itemGuid);
    data << uint64_t(_player->GetGUID());
    data << uint32_t(0);
    player->GetSession()->SendPacket(&data);

    data.clear();

    data << uint64_t(itemGuid);
    data << uint64_t(charter->GetLeader());
    data << uint32_t(0);

    SendPacket(&data);
}

void WorldSession::HandleCharterDeclineOpcode(WorldPacket& recv_data)
{
    uint64_t itemGuid;
    recv_data >> itemGuid;

    Charter* charter = objmgr.GetCharterByItemGuid(itemGuid);
    if (charter == nullptr)
    {
        return;
    }

    if (Player* owner = objmgr.GetPlayer(charter->GetLeader()))
    {
        WorldPacket data(MSG_PETITION_DECLINE, 8);
        data << uint64_t(_player->GetGUID());
        owner->GetSession()->SendPacket(&data);
    }
}

void WorldSession::HandleCharterTurnInCharterOpcode(WorldPacket& recv_data)
{
    uint64_t charterGuid;
    recv_data >> charterGuid;

    Charter* charter = objmgr.GetCharterByItemGuid(charterGuid);
    if (charter == nullptr)
    {
        return;
    }

    if (charter->CharterType == CHARTER_TYPE_GUILD)
    {
        Charter* playerCharter = _player->m_charters[CHARTER_TYPE_GUILD];
        if (playerCharter == nullptr)
        {
            return;
        }

        if (playerCharter->SignatureCount < 9)
        {
            Guild::sendTurnInPetitionResult(this, PETITION_ERROR_NEED_MORE_SIGNATURES);
            return;
        }

        Guild* guild = new Guild;
        guild->create(_player, playerCharter->GuildName);

        _player->m_charters[CHARTER_TYPE_GUILD] = 0;
        playerCharter->Destroy();

        _player->GetItemInterface()->RemoveItemAmt(ITEM_ENTRY_GUILD_CHARTER, 1);
        sHookInterface.OnGuildCreate(_player, guild);
    }
    else
    {
        uint32_t icon;
        uint32_t iconcolor;
        uint32_t bordercolor;
        uint32_t border;
        uint32_t background;

        recv_data >> iconcolor;
        recv_data >> icon;
        recv_data >> bordercolor;
        recv_data >> border;
        recv_data >> background;

        uint32_t type;
        switch (charter->CharterType)
        {
            case CHARTER_TYPE_ARENA_2V2:
            {
                type = ARENA_TEAM_TYPE_2V2;
            } break;
            case CHARTER_TYPE_ARENA_3V3:
            {
                type = ARENA_TEAM_TYPE_3V3;
            } break;

            case CHARTER_TYPE_ARENA_5V5:
            {
                type = ARENA_TEAM_TYPE_5V5;
            } break;
            default:
            {
                SendNotification("Internal Error");
                return;
            }
        }

        if (_player->m_arenaTeams[charter->CharterType - 1] != nullptr)
        {
            sChatHandler.SystemMessage(this, LocalizedWorldSrv(ServerString::SS_ALREADY_ARENA_TEAM));
            return;
        }

        if (charter->SignatureCount < charter->GetNumberOfSlotsByType())
        {
            Guild::sendTurnInPetitionResult(this, PETITION_ERROR_NEED_MORE_SIGNATURES);
            return;
        }

        ArenaTeam* arenaTeam = new ArenaTeam(static_cast<uint16_t>(type), objmgr.GenerateArenaTeamId());
        arenaTeam->m_name = charter->GuildName;
        arenaTeam->m_emblemColour = iconcolor;
        arenaTeam->m_emblemStyle = icon;
        arenaTeam->m_borderColour = bordercolor;
        arenaTeam->m_borderStyle = border;
        arenaTeam->m_backgroundColour = background;
        arenaTeam->m_leader = _player->GetLowGUID();
        arenaTeam->m_stat_rating = 1500;

        objmgr.AddArenaTeam(arenaTeam);
        objmgr.UpdateArenaTeamRankings();
        arenaTeam->AddMember(_player->m_playerInfo);

        for (uint32_t i = 0; i < charter->SignatureCount; ++i)
        {
            if (PlayerInfo* info = objmgr.GetPlayerInfo(charter->Signatures[i]))
            {
                arenaTeam->AddMember(info);
            }
        }

        _player->GetItemInterface()->SafeFullRemoveItemByGuid(charterGuid);
        _player->m_charters[charter->CharterType] = nullptr;
        charter->Destroy();
    }

    Guild::sendTurnInPetitionResult(this, PETITION_ERROR_OK);
}

void WorldSession::HandleCharterRenameOpcode(WorldPacket& recv_data)
{
    uint64_t charterGuid;
    std::string name;

    recv_data >> charterGuid;
    recv_data >> name;

    Charter* charter = objmgr.GetCharterByItemGuid(charterGuid);
    if (charter == nullptr)
    {
        return;
    }

    Guild* guild = sGuildMgr.getGuildByName(name);
    Charter* guildCharter = objmgr.GetCharterByName(name, (CharterTypes)charter->CharterType);
    if (guildCharter || guild)
    {
        SendNotification("Guild name already in use.");
        return;
    }

    guildCharter = charter;
    guildCharter->GuildName = name;
    guildCharter->SaveToDB();

    WorldPacket data(MSG_PETITION_RENAME, 100);
    data << uint64_t(charterGuid);
    data << name;

    SendPacket(&data);
}

//////////////////////////////////////////////////////////////////////////////////////////
// GuildFinder
void WorldSession::HandleGuildFinderAddRecruit(WorldPacket& recvData)
{
    if (sGuildFinderMgr.getAllMembershipRequestsForPlayer(GetPlayer()->GetLowGUID()).size() == 10)
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

    MembershipRequest request = MembershipRequest(GetPlayer()->GetLowGUID(), guildLowGuid, availability, classRoles, guildInterests, comment, time(nullptr));
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

    LFGuildPlayer settings(player->GetLowGUID(), static_cast<uint8_t>(classRoles), static_cast<uint8_t>(availability), static_cast<uint8_t>(guildInterests), ANY_FINDER_LEVEL);
    LFGuildStore guildList = sGuildFinderMgr.getGuildsMatchingSetting(settings, player->GetTeamReal());
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

        bufferData << uint8_t(sGuildFinderMgr.hasRequest(player->GetLowGUID(), guild->getId()));

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

    sGuildFinderMgr.removeMembershipRequest(Arcemu::Util::GUID_LOPART(playerGuid), GetPlayer()->GetGuildId());
}

void WorldSession::HandleGuildFinderGetApplications(WorldPacket& /*recv_data*/)
{
    std::list<MembershipRequest> applicatedGuilds = sGuildFinderMgr.getAllMembershipRequestsForPlayer(GetPlayer()->GetLowGUID());
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
    data << uint32_t(10 - sGuildFinderMgr.countRequestsFromPlayer(GetPlayer()->GetLowGUID()));

    GetPlayer()->SendPacket(&data);
}

void WorldSession::HandleGuildFinderGetRecruits(WorldPacket& recv_data)
{
    uint32_t unkTime = 0;
    recv_data >> unkTime;

    Player* player = GetPlayer();
    if (!player->GetGuildId())
    {
        return;
    }

    std::vector<MembershipRequest> recruitsList = sGuildFinderMgr.getAllMembershipRequestsForGuild(player->GetGuildId());
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
    if (!player->GetGuildId())
    {
        return;
    }

    bool isGuildMaster = true;
    if (Guild* guild = sGuildMgr.getGuildById(player->GetGuildId()))
    {
        if (guild->getLeaderGUID() != player->GetGUID())
        {
            isGuildMaster = false;
        }
    }

    LFGuildSettings settings = sGuildFinderMgr.getGuildSettings(player->GetGuildId());

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

    sGuildFinderMgr.removeMembershipRequest(Arcemu::Util::GUID_LOPART(GetPlayer()->GetGUID()), Arcemu::Util::GUID_LOPART(guildGuid));
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
    if (!player->GetGuildId())
    {
        return;
    }

    if (Guild* guild = sGuildMgr.getGuildById(player->GetGuildId()))
    {
        if (guild->getLeaderGUID() != player->GetGUID())
        {
            return;
        }
    }

    LFGuildSettings settings(listed, player->GetTeamReal(), player->GetGuildId(), static_cast<uint8_t>(classRoles), static_cast<uint8_t>(availability), static_cast<uint8_t>(guildInterests), static_cast<uint8_t>(level), comment);
    sGuildFinderMgr.setGuildSettings(player->GetGuildId(), settings);
}
