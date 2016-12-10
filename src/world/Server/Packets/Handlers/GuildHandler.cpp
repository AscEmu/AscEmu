/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

void WorldSession::HandleGuildQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        uint64 guildguid;
    uint64 playerGuid;
    recv_data >> guildguid;
    recv_data >> playerGuid;

    uint32 guildId = uint32(guildguid);

    Log.Debug("Opcodes", "CMSG_GUILD_QUERY [%s]: GuildId: %u Target: %u",
              _player->GetName(), guildId, Arcemu::Util::GUID_LOPART(playerGuid));

    if (Guild* guild = sGuildMgr.GetGuildById(guildId))
        if (guild->IsMember(playerGuid))
            guild->HandleQuery(this);

}

//void WorldSession::HandleCreateGuild(WorldPacket& recv_data) { }

void WorldSession::HandleInviteToGuildOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        uint32 nameLength = recv_data.readBits(7);
    std::string invitedName = recv_data.ReadString(nameLength);

    Log.Debug("GuildHandler", "CMSG_GUILD_INVITE [%s]: Invited: %s", _player->GetName(), invitedName.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleInviteMember(this, invitedName);

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

    Log.Debug("GuildHandler", "CMSG_GUILD_REMOVE [%s]: Target: %u", _player->GetName(), Arcemu::Util::GUID_LOPART(playerGuid));

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleRemoveMember(this, playerGuid);
}

void WorldSession::HandleGuildAcceptOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        Log.Debug("GuildHandler", "CMSG_GUILD_ACCEPT [%s]", _player->GetName());

    if (!GetPlayer()->GetGuildId())
        if (Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildIdInvited()))
            guild->HandleAcceptMember(this);

}

void WorldSession::HandleGuildDeclineOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        Log.Debug("GuildHandler", "CMSG_GUILD_DECLINE [%s]", _player->GetName());

    GetPlayer()->SetGuildIdInvited(0);
    GetPlayer()->SetInGuild(0);

}

//void WorldSession::HandleSetGuildInformation(WorldPacket& recv_data) { }

//void WorldSession::HandleGuildInfo(WorldPacket& recv_data) { }

void WorldSession::HandleGuildRosterOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        Log.Debug("GuildHandler", "CMSG_GUILD_ROSTER [%s]", _player->GetName());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleRoster(this);
    else
        Guild::SendCommandResult(this, GUILD_COMMAND_ROSTER, ERR_GUILD_PLAYER_NOT_IN_GUILD);
}

void WorldSession::HandleGuildPromoteOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

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

    Log.Debug("GuildHandler", "CMSG_GUILD_PROMOTE [%s]: Target: %u", _player->GetName(), Arcemu::Util::GUID_LOPART(targetGuid));

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleUpdateMemberRank(this, targetGuid, false);

}

void WorldSession::HandleGuildDemoteOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

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

    Log.Debug("GuildHandler", "CMSG_GUILD_DEMOTE [%s]: Target: %u", _player->GetName(), Arcemu::Util::GUID_LOPART(targetGuid));

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleUpdateMemberRank(this, targetGuid, true);
}

void WorldSession::HandleGuildAssignRankOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        ObjectGuid targetGuid;
    ObjectGuid setterGuid;

    uint32 rankId;
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

    Log.Debug("GuildHandler", "CMSG_GUILD_ASSIGN_MEMBER_RANK [%s]: Target: %u Rank: %u, Issuer: %u",
              _player->GetName(), Arcemu::Util::GUID_LOPART(targetGuid), rankId, Arcemu::Util::GUID_LOPART(setterGuid));

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetMemberRank(this, targetGuid, setterGuid, rankId);
}

void WorldSession::HandleGuildLeaveOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        Log.Debug("GuildHandler", "CMSG_GUILD_LEAVE [%s]", _player->GetName());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleLeaveMember(this);
}

void WorldSession::HandleGuildDisbandOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        Log.Debug("GuildHandler", "CMSG_GUILD_DISBAND [%s]", _player->GetName());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleDisband(this);

}

//void WorldSession::HandleGuildLeaderOpcode(WorldPacket& recv_data) { }

void WorldSession::HandleGuildMotdOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        uint32 motdLength = recv_data.readBits(11);
    std::string motd = recv_data.ReadString(motdLength);
    Log.Debug("GuildHandler", "CMSG_GUILD_MOTD [%s]: MOTD: %s", _player->GetName(), motd.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetMOTD(this, motd);
}

void WorldSession::HandleGuildSetNoteOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        ObjectGuid playerGuid;

    playerGuid[1] = recv_data.readBit();
    playerGuid[4] = recv_data.readBit();
    playerGuid[5] = recv_data.readBit();
    playerGuid[3] = recv_data.readBit();
    playerGuid[0] = recv_data.readBit();
    playerGuid[7] = recv_data.readBit();
    bool ispublic = recv_data.readBit();      // 0 == Officer, 1 == Public
    playerGuid[6] = recv_data.readBit();
    uint32 noteLength = recv_data.readBits(8);
    playerGuid[2] = recv_data.readBit();

    recv_data.ReadByteSeq(playerGuid[4]);
    recv_data.ReadByteSeq(playerGuid[5]);
    recv_data.ReadByteSeq(playerGuid[0]);
    recv_data.ReadByteSeq(playerGuid[3]);
    recv_data.ReadByteSeq(playerGuid[1]);
    recv_data.ReadByteSeq(playerGuid[6]);
    recv_data.ReadByteSeq(playerGuid[7]);
    std::string note = recv_data.ReadString(noteLength);
    recv_data.ReadByteSeq(playerGuid[2]);

    Log.Debug("GuildHandler", "CMSG_GUILD_SET_NOTE [%s]: Target: %u, Note: %s, Public: %u",
              _player->GetName(), Arcemu::Util::GUID_LOPART(playerGuid), note.c_str(), ispublic);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetMemberNote(this, note, playerGuid, ispublic);
}

void WorldSession::HandleGuildQueryRanksOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

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

    Log.Debug("GuildHandler", "CMSG_GUILD_QUERY_RANKS [%s]: Guild: %u",
              _player->GetName(), Arcemu::Util::GUID_LOPART(guildGuid));

    if (Guild* guild = sGuildMgr.GetGuildById(Arcemu::Util::GUID_LOPART(guildGuid)))
        if (guild->IsMember(_player->GetGUID()))
            guild->SendGuildRankInfo(this);

}

void WorldSession::HandleGuildAddRankOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        uint32 rankId;
    recv_data >> rankId;

    uint32 length = recv_data.readBits(7);
    std::string rankName = recv_data.ReadString(length);

    Log.Debug("GuildHandler", "CMSG_GUILD_ADD_RANK [%s]: Rank: %s", _player->GetName(), rankName.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleAddNewRank(this, rankName);
}

void WorldSession::HandleGuildDelRankOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        uint32 rankId;
    recv_data >> rankId;

    Log.Debug("GuildHandler", "CMSG_GUILD_DEL_RANK [%s]: Rank: %u", _player->GetName(), rankId);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleRemoveRank(this, rankId);

}

void WorldSession::HandleGuildChangeInfoTextOpcode(WorldPacket& recv_data)
{
    uint32 length = recv_data.readBits(12);
    std::string info = recv_data.ReadString(length);

    Log.Debug("GuildHandler", "CMSG_GUILD_INFO_TEXT [%s]: %s", _player->GetName(), info.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetInfo(this, info);
}

//void WorldSession::HandleGuildSetPublicNoteOpcode(WorldPacket& recv_data) { }

//void WorldSession::HandleGuildSetOfficerNoteOpcode(WorldPacket& recv_data) { }

void WorldSession::HandleSaveGuildEmblemOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        uint64 vendorGuid;
    recv_data >> vendorGuid;

    EmblemInfo emblemInfo;
    emblemInfo.ReadPacket(recv_data);

    Log.Debug("GuildHandler", "MSG_SAVE_GUILD_EMBLEM [%s]: Guid: [" I64FMTD
              "] Style: %u, Color: %u, BorderStyle: %u, BorderColor: %u, BackgroundColor: %u"
              , _player->GetName(), vendorGuid, emblemInfo.GetStyle()
              , emblemInfo.GetColor(), emblemInfo.GetBorderStyle()
              , emblemInfo.GetBorderColor(), emblemInfo.GetBackgroundColor());

    if (GetPlayer()->GetGuild()->GetLeaderGUID() != _player->GetGUID())
        Guild::SendSaveEmblemResult(this, ERR_GUILDEMBLEM_NOTGUILDMASTER);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetEmblem(this, emblemInfo);
    else
        Guild::SendSaveEmblemResult(this, ERR_GUILDEMBLEM_NOGUILD);

}

void WorldSession::HandleGuildEventLogQueryOpcode(WorldPacket& /* recv_data */)
{
    Log.Debug("GuildHandler", "MSG_GUILD_EVENT_LOG_QUERY [%s]", _player->GetName());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendEventLog(this);
}

//Guild Bank

void WorldSession::HandleGuildBankMoneyWithdrawn(WorldPacket& /* recv_data */)
{
    Log.Debug("GuildHandler", "CMSG_GUILD_BANK_MONEY_WITHDRAWN_QUERY [%s]", _player->GetName());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendMoneyInfo(this);
}

void WorldSession::HandleGuildPermissions(WorldPacket& /* recv_data */)
{
    Log.Debug("GuildHandler", "CMSG_GUILD_PERMISSIONS [%s]", _player->GetName());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendPermissions(this);
}

// Called when clicking on Guild bank gameobject
void WorldSession::HandleGuildBankerActivate(WorldPacket& recv_data)
{
    uint64 guid;
    bool sendAllSlots;
    recv_data >> guid;
    recv_data >> sendAllSlots;

    Log.Debug("GuildHandler", "CMSG_GUILD_BANKER_ACTIVATE [%s]: Go: [" I64FMTD "] AllSlots: %u"
              , _player->GetName(), guid, sendAllSlots);

    Guild* const guild = GetPlayer()->GetGuild();
    if (!guild)
    {
        Guild::SendCommandResult(this, GUILD_COMMAND_VIEW_TAB, ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    guild->SendBankList(this, 0, true, true);
}

// Called when opening guild bank tab only (first one)
void WorldSession::HandleGuildBankQueryTab(WorldPacket& recv_data)
{
    uint64 guid;
    uint8 tabId;
    bool sendAllSlots;

    recv_data >> guid;
    recv_data >> tabId;
    recv_data >> sendAllSlots;

    Log.Debug("Guild Handler", "CMSG_GUILD_BANK_QUERY_TAB [%s]: Go: [" I64FMTD "], TabId: %u, AllSlots: %u"
              , _player->GetName(), guid, tabId, sendAllSlots);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendBankList(this, tabId, true, false);
}

void WorldSession::HandleGuildBankDepositMoney(WorldPacket& recv_data)
{
    uint64 guid;
    uint64 money;
    recv_data >> guid;
    recv_data >> money;

    Log.Debug("GuildHandler", "CMSG_GUILD_BANK_DEPOSIT_MONEY [%s]: Go: [" I64FMTD "], money: " I64FMTD,
              _player->GetName(), guid, money);

    if (money && GetPlayer()->HasGold(money))
        if (Guild* guild = GetPlayer()->GetGuild())
            guild->HandleMemberDepositMoney(this, money);
}

void WorldSession::HandleGuildBankWithdrawMoney(WorldPacket& recv_data)
{
    uint64 guid;
    uint64 money;
    recv_data >> guid;
    recv_data >> money;

    Log.Debug("GuildHandler", "CMSG_GUILD_BANK_WITHDRAW_MONEY [%s]: Go: [" I64FMTD "], money: " I64FMTD,
              _player->GetName(), guid, money);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleMemberWithdrawMoney(this, money);
}

void WorldSession::HandleGuildBankSwapItems(WorldPacket& recv_data)
{
    Log.Debug("GuildHandler", "CMSG_GUILD_BANK_SWAP_ITEMS [%s]", _player->GetName());

    uint64 GoGuid;
    recv_data >> GoGuid;

    Guild* guild = GetPlayer()->GetGuild();
    if (!guild)
        return;

    uint8 bankToBank;
    recv_data >> bankToBank;

    uint8 tabId;
    uint8 slotId;
    uint32 itemEntry;
    uint32 splitedAmount = 0;

    if (bankToBank)
    {
        uint8 destTabId;
        recv_data >> destTabId;

        uint8 destSlotId;
        recv_data >> destSlotId;

        uint32 destItemEntry;
        recv_data >> destItemEntry;

        recv_data >> tabId;
        recv_data >> slotId;
        recv_data >> itemEntry;
        recv_data.read_skip<uint8>();                       // Always 0
        recv_data >> splitedAmount;

        guild->SwapItems(GetPlayer(), tabId, slotId, destTabId, destSlotId, splitedAmount);
    }
    else
    {
        uint8 playerBag = 0;
        uint8 playerSlotId = 255;
        uint8 toChar = 1;

        recv_data >> tabId;
        recv_data >> slotId;
        recv_data >> itemEntry;

        uint8 autoStore;
        recv_data >> autoStore;
        if (autoStore)
        {
            recv_data.read_skip<uint32>();                  // autoStoreCount
            recv_data.read_skip<uint8>();                   // ToChar (?), always and expected to be 1 (autostore only triggered in Bank -> Char)
            recv_data.read_skip<uint32>();                  // Always 0
        }
        else
        {
            recv_data >> playerBag;
            recv_data >> playerSlotId;
            recv_data >> toChar;
            recv_data >> splitedAmount;
        }

        bool to_char = toChar > 0 ? true : false;

        // Player <-> Bank todo
        guild->SwapItemsWithInventory(GetPlayer(), to_char, tabId, slotId, playerBag, playerSlotId, splitedAmount);
    }
}

void WorldSession::HandleGuildBankBuyTab(WorldPacket& recv_data)
{
    uint64 guid;
    recv_data >> guid;

    uint8 tabId;
    recv_data >> tabId;

    Log.Debug("GuildHandler", "CMSG_GUILD_BANK_BUY_TAB [%s]: Go: [" I64FMTD "], TabId: %u", _player->GetName(), guid, tabId);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleBuyBankTab(this, tabId);
}

void WorldSession::HandleGuildBankUpdateTab(WorldPacket& recv_data)
{
    uint64 guid;
    uint8 tabId;
    std::string name, icon;

    recv_data >> guid;
    recv_data >> tabId;
    recv_data >> name;
    recv_data >> icon;

    Log.Debug("GuildHandler", "CMSG_GUILD_BANK_UPDATE_TAB [%s]: Go: [" I64FMTD "], TabId: %u, Name: %s, Icon: %s"
              , _player->GetName(), guid, tabId, name.c_str(), icon.c_str());
    if (!name.empty() && !icon.empty())
        if (Guild* guild = GetPlayer()->GetGuild())
            guild->HandleSetBankTabInfo(this, tabId, name, icon);
}

void WorldSession::HandleGuildBankLogQuery(WorldPacket& recv_data)
{
    uint32 tabId;
    recv_data >> tabId;

    Log.Debug("GuildHandler", "MSG_GUILD_BANK_LOG_QUERY [%s]: TabId: %u", _player->GetName(), tabId);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendBankLog(this, tabId);
}

void WorldSession::HandleQueryGuildBankTabText(WorldPacket &recv_data)
{
    uint8 tabId;
    recv_data >> tabId;

    Log.Debug("GuildHandler", "MSG_QUERY_GUILD_BANK_TEXT [%s]: TabId: %u", _player->GetName(), tabId);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendBankTabText(this, tabId);
}

void WorldSession::HandleSetGuildBankTabText(WorldPacket& recv_data)
{
    uint32 tabId;
    recv_data >> tabId;

    uint32 textLen = recv_data.readBits(14);
    std::string text = recv_data.ReadString(textLen);

    Log.Debug("GuildHandler", "CMSG_SET_GUILD_BANK_TEXT [%s]: TabId: %u, Text: %s", _player->GetName(), tabId, text.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SetBankTabText(tabId, text);
}

void WorldSession::HandleGuildQueryXPOpcode(WorldPacket& recv_data)
{
    Log.Debug("GuildHandler", "WORLD: Received CMSG_QUERY_GUILD_XP");

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

    Log.Debug("GuildHandler", "CMSG_QUERY_GUILD_XP [%s]: Guild: %u", _player->GetName(), Arcemu::Util::GUID_LOPART(guildGuid));

    uint32 guildId = uint32(guildGuid);

    if (Guild* guild = sGuildMgr.GetGuildById(guildId))
        if (guild->IsMember(_player->GetGUID()))
            guild->SendGuildXP(this);
}

void WorldSession::HandleGuildSetRankPermissionsOpcode(WorldPacket& recv_data)
{
    Guild* guild = GetPlayer()->GetGuild();
    if (!guild)
        return;


    uint32 oldRankId;
    uint32 newRankId;
    uint32 oldRights;
    uint32 newRights;
    uint32 moneyPerDay;

    recv_data >> oldRankId;
    recv_data >> oldRights;
    recv_data >> newRights;

    GuildBankRightsAndSlotsVec rightsAndSlots(GUILD_BANK_MAX_TABS);
    for (uint8 tabId = 0; tabId < GUILD_BANK_MAX_TABS; ++tabId)
    {
        uint32 bankRights;
        uint32 slots;

        recv_data >> bankRights;
        recv_data >> slots;

        rightsAndSlots[tabId] = GuildBankRightsAndSlots(tabId, uint8(bankRights), slots);
    }

    recv_data >> moneyPerDay;
    recv_data >> newRankId;
    uint32 nameLength = recv_data.readBits(7);
    std::string rankName = recv_data.ReadString(nameLength);

    Log.Debug("GuildHandler", "CMSG_GUILD_SET_RANK_PERMISSIONS [%s]: Rank: %s (%u)", _player->GetName(), rankName.c_str(), newRankId);

    guild->HandleSetRankInfo(this, newRankId, rankName, newRights, moneyPerDay, rightsAndSlots);
}

void WorldSession::HandleGuildRequestPartyState(WorldPacket& recv_data)
{
    Log.Debug("GuildHandler", "WORLD: Received CMSG_GUILD_REQUEST_PARTY_STATE");

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

    uint32 guildId = uint32(guildGuid);

    if (Guild* guild = sGuildMgr.GetGuildById(guildId))
        guild->HandleGuildPartyRequest(this);
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

    uint32 guildId = uint32(guid);

    if (Guild* guild = sGuildMgr.GetGuildById(guildId))
    {
        if (guild->IsMember(_player->GetGUID()))
        {
            WorldPacket data(SMSG_GUILD_MAX_DAILY_XP, 8);
            data << uint64(7807500 /*Daily XP Cap Todo*/);
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleAutoDeclineGuildInvites(WorldPacket& recv_data)
{
    uint8 enable;
    recv_data >> enable;

    bool enabled = enable > 0 ? true : false;

    GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_AUTO_DECLINE_GUILD, enabled);
}

void WorldSession::HandleGuildRewardsQueryOpcode(WorldPacket& recv_data)
{
    recv_data.read_skip<uint32>(); // Unk

    if (sGuildMgr.GetGuildById(_player->GetGuildId()))
    {
        std::vector<GuildReward> const& rewards = sGuildMgr.GetGuildRewards();

        WorldPacket data(SMSG_GUILD_REWARDS_LIST, 3 + rewards.size() * (4 + 4 + 4 + 8 + 4 + 4));
        data.writeBits(rewards.size(), 21);
        data.flushBits();

        for (uint32 i = 0; i < rewards.size(); i++)
        {
            data << uint32(rewards[i].Standing);
            data << int32(rewards[i].Racemask);
            data << uint32(rewards[i].Entry);
            data << uint64(rewards[i].Price);
            data << uint32(0); // Unused
            data << uint32(rewards[i].AchievementId);
        }
        data << uint32(time(NULL));
        SendPacket(&data);
    }
}

void WorldSession::HandleGuildQueryNewsOpcode(WorldPacket& recv_data)
{
    recv_data.read_skip<uint32>();
    Log.Debug("GuildHandler", "CMSG_GUILD_QUERY_NEWS [%s]", _player->GetName());
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendNewsUpdate(this);
}


void WorldSession::HandleGuildNewsUpdateStickyOpcode(WorldPacket& recv_data)
{
    uint32 newsId;
    bool sticky;
    ObjectGuid guid;

    recv_data >> newsId;

    guid[2] = recv_data.readBit();
    guid[4] = recv_data.readBit();
    guid[3] = recv_data.readBit();
    guid[0] = recv_data.readBit();
    sticky = recv_data.readBit();
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
        guild->HandleNewsSetSticky(this, newsId, sticky);
}

void WorldSession::HandleGuildSetGuildMaster(WorldPacket& recv_data)
{
    uint8 nameLength = recv_data.readBits(7);

    recv_data.readBit();
    std::string playerName = recv_data.ReadString(nameLength);
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetNewGuildMaster(this, playerName);
}

// Charter part
void WorldSession::HandleCharterBuyOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        uint8 error;

    // Arena team charters are full of crap
    uint64 creature_guid;
    uint32 crap;
    uint64 crap2;
    std::string name, UnkString;
    //uint32 crap3, crap4, crap5, crap6, crap7, crap8, crap9;
    uint32 Data[7];
    uint16 crap10;
    uint32 crap11;
    uint32 crap12, PetitionSignerCount;
    std::string crap13;
    uint32 arena_index;


    recv_data >> creature_guid;
    recv_data >> crap >> crap2;
    recv_data >> name;
    recv_data >> UnkString;

    for (uint8 i = 0; i < 7; ++i)
        recv_data >> Data[i];

    recv_data >> crap10;
    recv_data >> crap11;
    recv_data >> crap12;
    recv_data >> PetitionSignerCount;

    for (uint32 s = 0; s < 10; ++s)
        recv_data >> crap13;

    recv_data >> arena_index;

    Creature* crt = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(creature_guid));
    if (!crt)
    {
        Disconnect();
        return;
    }

    if (!crt->isTabardDesigner())
    {
        uint32 arena_type = arena_index - 1;
        if (arena_type > 2)
            return;

        if (_player->m_arenaTeams[arena_type])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(71));
            return;
        }

        ArenaTeam* t = objmgr.GetArenaTeamByName(name, arena_type);
        if (t != NULL)
        {
            sChatHandler.SystemMessage(this, _player->GetSession()->LocalizedWorldSrv(72));
            return;
        }

        if (objmgr.GetCharterByName(name, (CharterTypes)arena_index))
        {
            sChatHandler.SystemMessage(this, _player->GetSession()->LocalizedWorldSrv(72));
            return;
        }

        if (_player->m_charters[arena_index])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(73));
            return;
        }

        if (_player->getLevel() < PLAYER_ARENA_MIN_LEVEL)
        {
            ///\todo Replace by LocalizedWorldSrv(..)
            SendNotification("You must be at least level %u to buy Arena charter", PLAYER_ARENA_MIN_LEVEL);
            return;
        }

        static uint32 item_ids[] = { ARENA_TEAM_CHARTER_2v2, ARENA_TEAM_CHARTER_3v3, ARENA_TEAM_CHARTER_5v5 };
        static uint32 costs[] = { ARENA_TEAM_CHARTER_2v2_COST, ARENA_TEAM_CHARTER_3v3_COST, ARENA_TEAM_CHARTER_5v5_COST };

        if (!_player->HasGold(costs[arena_type]))
            return;            // error message needed here

        ItemProperties const* ip = sMySQLStore.GetItemProperties(item_ids[arena_type]);
        ARCEMU_ASSERT(ip != NULL);
        SlotResult res = _player->GetItemInterface()->FindFreeInventorySlot(ip);
        if (res.Result == 0)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(0, 0, INV_ERR_INVENTORY_FULL);
            return;
        }

        error = _player->GetItemInterface()->CanReceiveItem(ip, 1);
        if (error)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, error);
        }
        else
        {
            // Create the item and charter
            Item* i = objmgr.CreateItem(item_ids[arena_type], _player);
            Charter* c = objmgr.CreateCharter(_player->GetLowGUID(), (CharterTypes)arena_index);
            if (i == NULL || c == NULL)
                return;

            c->GuildName = name;
            c->ItemGuid = i->GetGUID();

            c->UnkString = UnkString;
            c->Unk1 = crap10;
            c->Unk2 = crap11;
            c->Unk3 = crap12;
            c->PetitionSignerCount = PetitionSignerCount;
            memcpy(c->Data, Data, sizeof(Data));

            i->SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);
            i->SoulBind();
            i->SetEnchantmentId(0, c->GetID());
            i->SetItemRandomSuffixFactor(57813883);
            if (!_player->GetItemInterface()->AddItemToFreeSlot(i))
            {
                c->Destroy();
                i->DeleteMe();
                return;
            }

            c->SaveToDB();

            _player->SendItemPushResult(false, true, false, true, _player->GetItemInterface()->LastSearchItemBagSlot(), _player->GetItemInterface()->LastSearchItemSlot(), 1, i->GetEntry(), i->GetItemRandomSuffixFactor(), i->GetItemRandomPropertyId(), i->GetStackCount());

            _player->ModGold(-(int32)costs[arena_type]);
            _player->m_charters[arena_index] = c;
            _player->SaveToDB(false);
        }
    }
    else
    {
        if (!_player->HasGold(1000))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_NOT_ENOUGH_MONEY);
            return;
        }

        Guild* guild = sGuildMgr.GetGuildByName(name);
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

        ItemProperties const* ip = sMySQLStore.GetItemProperties(ITEM_ENTRY_GUILD_CHARTER);
        ARCEMU_ASSERT(ip != NULL);
        SlotResult res = _player->GetItemInterface()->FindFreeInventorySlot(ip);
        if (res.Result == 0)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(0, 0, INV_ERR_INVENTORY_FULL);
            return;
        }

        error = _player->GetItemInterface()->CanReceiveItem(sMySQLStore.GetItemProperties(ITEM_ENTRY_GUILD_CHARTER), 1);
        if (error)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, error);
        }
        else
        {
            // Meh...
            WorldPacket data(SMSG_PLAY_OBJECT_SOUND, 12);
            data << uint32(0x000019C2);
            data << creature_guid;
            SendPacket(&data);

            // Create the item and charter
            Item* item = objmgr.CreateItem(ITEM_ENTRY_GUILD_CHARTER, _player);
            charter = objmgr.CreateCharter(_player->GetLowGUID(), CHARTER_TYPE_GUILD);
            if (item == nullptr || charter == nullptr)
                return;

            charter->GuildName = name;
            charter->ItemGuid = item->GetGUID();

            charter->UnkString = UnkString;
            charter->Unk1 = crap10;
            charter->Unk2 = crap11;
            charter->Unk3 = crap12;
            charter->PetitionSignerCount = PetitionSignerCount;
            memcpy(charter->Data, Data, sizeof(Data));

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

            _player->SendItemPushResult(false, true, false, true, _player->GetItemInterface()->LastSearchItemBagSlot(), _player->GetItemInterface()->LastSearchItemSlot(), 1, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());

            _player->m_charters[CHARTER_TYPE_GUILD] = charter;
            _player->ModGold(-1000);
            _player->SaveToDB(false);
        }
    }
}

void SendShowSignatures(Charter* c, uint64 i, Player* p)
{
    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, 100);
    data << i;
    data << (uint64)c->GetLeader();
    data << c->GetID();
    data << uint8(c->SignatureCount);
    for (uint32 j = 0; j < c->Slots; ++j)
    {
        if (c->Signatures[j] == 0) continue;
        data << uint64(c->Signatures[j]) << uint32(1);
    }
    data << uint8(0);
    p->GetSession()->SendPacket(&data);
}

void WorldSession::HandleCharterShowSignaturesOpcode(WorldPacket& recv_data)
{
    Charter* pCharter;
    uint64 item_guid;
    recv_data >> item_guid;
    pCharter = objmgr.GetCharterByItemGuid(item_guid);

    if (pCharter)
        SendShowSignatures(pCharter, item_guid, _player);
}

void WorldSession::HandleCharterQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint32 charter_id;
    uint64 item_guid;
    recv_data >> charter_id;
    recv_data >> item_guid;

    Charter* charter = objmgr.GetCharterByItemGuid(item_guid);
    if (charter == nullptr)
        return;

    WorldPacket data(SMSG_PETITION_QUERY_RESPONSE, 100);
    data << charter_id;
    data << (uint64)charter->LeaderGuid;
    data << charter->GuildName;
    data << uint8(0);
    if (charter->CharterType == CHARTER_TYPE_GUILD)
    {
        data << uint32(9);
        data << uint32(9);
    }
    else
    {
        data << uint32(charter->Slots);
        data << uint32(charter->Slots);
    }

    data << uint32(0);                                      // 4
    data << uint32(0);                                      // 5
    data << uint32(0);                                      // 6
    data << uint32(0);                                      // 7
    data << uint32(0);                                      // 8
    data << uint16(0);                                      // 9 2 bytes field

    if (charter->CharterType == CHARTER_TYPE_GUILD)
    {
        data << uint32(1);                                  // 10 min level to sign a guild charter
        data << uint32(80);                                    // 11 max level to sign a guild charter
    }
    else
    {
        data << uint32(80);                                 // 10 min level to sign an arena charter
        data << uint32(80);                                    // 11 max level to sign an arena charter
    }

    data << uint32(0);                                      // 12
    data << uint32(0);                                      // 13 count of next strings?
    data << uint32(0);                                      // 14
    data << uint32(0);
    data << uint16(0);

    if (charter->CharterType == CHARTER_TYPE_GUILD)
        data << uint32(0);
    else
        data << uint32(1);

    SendPacket(&data);
}

void WorldSession::HandleCharterOfferOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint32 shit;
    uint64 item_guid;
    uint64 target_guid;

    recv_data >> shit;
    recv_data >> item_guid;
    recv_data >> target_guid;

    Player* pTarget = _player->GetMapMgr()->GetPlayer((uint32)target_guid);
    Charter* charter = objmgr.GetCharterByItemGuid(item_guid);

    if (!charter)
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(76));
        return;
    }

    if (pTarget == 0 || pTarget->GetTeam() != _player->GetTeam() || (pTarget == _player && !sWorld.interfaction_guild))
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(77));
        return;
    }

    if (!pTarget->CanSignCharter(charter, _player))
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(78));
        return;
    }

    SendShowSignatures(charter, item_guid, pTarget);
}

void WorldSession::HandleCharterSignOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 item_guid;
    recv_data >> item_guid;

    Charter* charter = objmgr.GetCharterByItemGuid(item_guid);
    if (charter == nullptr)
        return;

    for (uint32 i = 0; i < charter->SignatureCount; ++i)
    {
        if (charter->Signatures[i] == _player->GetGUID())
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(79));
            return;
        }
    }

    if (charter->IsFull())
        return;

    charter->AddSignature(_player->GetLowGUID());
    charter->SaveToDB();
    _player->m_charters[charter->CharterType] = charter;
    _player->SaveToDB(false);

    Player* player = _player->GetMapMgr()->GetPlayer(charter->GetLeader());
    if (player == nullptr)
        return;

    WorldPacket data(SMSG_PETITION_SIGN_RESULTS, 100);
    data << item_guid;
    data << _player->GetGUID();
    data << uint32(0);
    player->GetSession()->SendPacket(&data);

    data.clear();

    data << item_guid;
    data << (uint64)charter->GetLeader();
    data << uint32(0);
    SendPacket(&data);
}

void WorldSession::HandleCharterDeclineOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 item_guid;
    recv_data >> item_guid;

    Charter* charter = objmgr.GetCharterByItemGuid(item_guid);
    if (!charter)
        return;

    Player* owner = objmgr.GetPlayer(charter->GetLeader());
    if (owner)
    {
        WorldPacket data(MSG_PETITION_DECLINE, 8);
        data << _player->GetGUID();
        owner->GetSession()->SendPacket(&data);
    }
}

void WorldSession::HandleCharterTurnInCharterOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 mooguid;
    recv_data >> mooguid;

    Charter* charter = objmgr.GetCharterByItemGuid(mooguid);
    if (!charter)
        return;

    if (charter->CharterType == CHARTER_TYPE_GUILD)
    {
        Charter* player_charter = _player->m_charters[CHARTER_TYPE_GUILD];
        if (player_charter == nullptr)
            return;

        if (player_charter->SignatureCount < 9 && Config.MainConfig.GetBoolDefault("Server", "RequireAllSignatures", false))
        {
            Guild::SendTurnInPetitionResult(this, ERR_PETITION_NOT_ENOUGH_SIGNATURES);
            return;
        }

        // don't know hacky or not but only solution for now
        // If everything is fine create guild

        Guild* pGuild = new Guild;
        pGuild->Create(_player, player_charter->GuildName);

        // Destroy the charter
        _player->m_charters[CHARTER_TYPE_GUILD] = 0;
        player_charter->Destroy();

        _player->GetItemInterface()->RemoveItemAmt(ITEM_ENTRY_GUILD_CHARTER, 1);
        sHookInterface.OnGuildCreate(_player, pGuild);
    }
    else
    {
        ///\todo Arena charter -Replace with correct messages */
        uint32 icon;
        uint32 iconcolor;
        uint32 bordercolor;
        uint32 border;
        uint32 background;

        recv_data >> iconcolor;
        recv_data >> icon;
        recv_data >> bordercolor;
        recv_data >> border;
        recv_data >> background;

        uint32 type;
        switch (charter->CharterType)
        {
            case CHARTER_TYPE_ARENA_2V2:
                type = ARENA_TEAM_TYPE_2V2;
                break;

            case CHARTER_TYPE_ARENA_3V3:
                type = ARENA_TEAM_TYPE_3V3;
                break;

            case CHARTER_TYPE_ARENA_5V5:
                type = ARENA_TEAM_TYPE_5V5;
                break;

            default:
                SendNotification("Internal Error");
                return;
        }

        if (_player->m_arenaTeams[charter->CharterType - 1] != NULL)
        {
            sChatHandler.SystemMessage(this, LocalizedWorldSrv(Worldstring::SS_ALREADY_ARENA_TEAM));
            return;
        }

        if (charter->SignatureCount < charter->GetNumberOfSlotsByType() && Config.MainConfig.GetBoolDefault("Server", "RequireAllSignatures", false))
        {
            Guild::SendTurnInPetitionResult(this, ERR_PETITION_NOT_ENOUGH_SIGNATURES);
            return;
        }

        ArenaTeam* team = new ArenaTeam(type, objmgr.GenerateArenaTeamId());
        team->m_name = charter->GuildName;
        team->m_emblemColour = iconcolor;
        team->m_emblemStyle = icon;
        team->m_borderColour = bordercolor;
        team->m_borderStyle = border;
        team->m_backgroundColour = background;
        team->m_leader = _player->GetLowGUID();
        team->m_stat_rating = 1500;

        objmgr.AddArenaTeam(team);
        objmgr.UpdateArenaTeamRankings();
        team->AddMember(_player->m_playerInfo);


        /* Add the members */
        for (uint32 i = 0; i < charter->SignatureCount; ++i)
        {
            PlayerInfo* info = objmgr.GetPlayerInfo(charter->Signatures[i]);
            if (info)
            {
                team->AddMember(info);
            }
        }

        _player->GetItemInterface()->SafeFullRemoveItemByGuid(mooguid);
        _player->m_charters[charter->CharterType] = NULL;
        charter->Destroy();
    }

    Guild::SendTurnInPetitionResult(this, ERR_PETITION_OK);
}

void WorldSession::HandleCharterRenameOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 guid;
    std::string name;

    recv_data >> guid;
    recv_data >> name;

    Charter* charter = objmgr.GetCharterByItemGuid(guid);
    if (charter == nullptr)
        return;

    Guild* guild = sGuildMgr.GetGuildByName(name);
    Charter* guild_charter = objmgr.GetCharterByName(name, (CharterTypes)charter->CharterType);
    if (guild_charter || guild)
    {
        SendNotification("That name is in use by another guild.");
        return;
    }

    guild_charter = charter;
    guild_charter->GuildName = name;
    guild_charter->SaveToDB();

    WorldPacket data(MSG_PETITION_RENAME, 100);
    data << guid;
    data << name;
    SendPacket(&data);
}

// GuildFinder

void WorldSession::HandleGuildFinderAddRecruit(WorldPacket& recvPacket)
{
    Log.Debug("GuildHandler", "WORLD: Received CMSG_LF_GUILD_ADD_RECRUIT");

    if (sGuildFinderMgr.GetAllMembershipRequestsForPlayer(GetPlayer()->GetLowGUID()).size() == 10)
        return;

    uint32 classRoles = 0;
    uint32 availability = 0;
    uint32 guildInterests = 0;

    recvPacket >> classRoles;
    recvPacket >> guildInterests;
    recvPacket >> availability;

    ObjectGuid guid;

    guid[3] = recvPacket.readBit();
    guid[0] = recvPacket.readBit();
    guid[6] = recvPacket.readBit();
    guid[1] = recvPacket.readBit();
    uint16 commentLength = recvPacket.readBits(11);
    guid[5] = recvPacket.readBit();
    guid[4] = recvPacket.readBit();
    guid[7] = recvPacket.readBit();
    uint8 nameLength = recvPacket.readBits(7);
    guid[2] = recvPacket.readBit();

    recvPacket.ReadByteSeq(guid[4]);
    recvPacket.ReadByteSeq(guid[5]);
    std::string comment = recvPacket.ReadString(commentLength);
    std::string playerName = recvPacket.ReadString(nameLength);
    recvPacket.ReadByteSeq(guid[7]);
    recvPacket.ReadByteSeq(guid[2]);
    recvPacket.ReadByteSeq(guid[0]);
    recvPacket.ReadByteSeq(guid[6]);
    recvPacket.ReadByteSeq(guid[1]);
    recvPacket.ReadByteSeq(guid[3]);

    uint32 guildLowGuid = Arcemu::Util::GUID_LOPART(uint64(guid));

    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
        return;
    if (!(availability & AVAILABILITY_ALWAYS) || availability > AVAILABILITY_ALWAYS)
        return;
    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
        return;

    MembershipRequest request = MembershipRequest(GetPlayer()->GetLowGUID(), guildLowGuid, availability, classRoles, guildInterests, comment, time(NULL));
    sGuildFinderMgr.AddMembershipRequest(guildLowGuid, request);
}

void WorldSession::HandleGuildFinderBrowse(WorldPacket& recvPacket)
{
    Log.Debug("GuildHandler", "WORLD: Received CMSG_LF_GUILD_BROWSE");
    uint32 classRoles = 0;
    uint32 availability = 0;
    uint32 guildInterests = 0;
    uint32 playerLevel = 0; // Raw player level (1-85), do they use MAX_FINDER_LEVEL when on level 85 ?

    recvPacket >> classRoles >> availability >> guildInterests >> playerLevel;

    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
        return;
    if (!(availability & AVAILABILITY_ALWAYS) || availability > AVAILABILITY_ALWAYS)
        return;
    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
        return;
    if (playerLevel > 85 /*Level CAP */ || playerLevel < 1)
        return;

    Player* player = GetPlayer();

    LFGuildPlayer settings(player->GetLowGUID(), classRoles, availability, guildInterests, ANY_FINDER_LEVEL);
    LFGuildStore guildList = sGuildFinderMgr.GetGuildsMatchingSetting(settings, player->GetTeamReal());
    uint32 guildCount = guildList.size();

    if (guildCount == 0)
    {
        WorldPacket packet(SMSG_LF_GUILD_BROWSE_UPDATED, 0);
        player->SendMessageToSet(&packet, false);
        return;
    }

    ByteBuffer bufferData(65 * guildCount);
    WorldPacket data(SMSG_LF_GUILD_BROWSE_UPDATED, 3 + guildCount * 65); // Estimated size
    data.writeBits(guildCount, 19);

    for (LFGuildStore::const_iterator itr = guildList.begin(); itr != guildList.end(); ++itr)
    {
        LFGuildSettings guildSettings = itr->second;
        Guild* guild = sGuildMgr.GetGuildById(itr->first);

        ObjectGuid guildGUID = ObjectGuid(guild->GetGUID());

        data.writeBit(guildGUID[7]);
        data.writeBit(guildGUID[5]);
        data.writeBits(guild->GetName().size(), 8);
        data.writeBit(guildGUID[0]);
        data.writeBits(guildSettings.GetComment().size(), 11);
        data.writeBit(guildGUID[4]);
        data.writeBit(guildGUID[1]);
        data.writeBit(guildGUID[2]);
        data.writeBit(guildGUID[6]);
        data.writeBit(guildGUID[3]);

        bufferData << uint32(guild->GetEmblemInfo().GetColor());
        bufferData << uint32(guild->GetEmblemInfo().GetBorderStyle()); // Guessed
        bufferData << uint32(guild->GetEmblemInfo().GetStyle());

        bufferData.WriteString(guildSettings.GetComment());

        bufferData << uint8(0); // Unk

        bufferData.WriteByteSeq(guildGUID[5]);

        bufferData << uint32(guildSettings.GetInterests());

        bufferData.WriteByteSeq(guildGUID[6]);
        bufferData.WriteByteSeq(guildGUID[4]);

        bufferData << uint32(guild->GetLevel());

        bufferData.WriteString(guild->GetName());

        bufferData << uint32(0); // Achievmentspoints Todo

        bufferData.WriteByteSeq(guildGUID[7]);

        bufferData << uint8(sGuildFinderMgr.HasRequest(player->GetLowGUID(), guild->GetGUID())); // Request pending

        bufferData.WriteByteSeq(guildGUID[2]);
        bufferData.WriteByteSeq(guildGUID[0]);

        bufferData << uint32(guildSettings.GetAvailability());

        bufferData.WriteByteSeq(guildGUID[1]);

        bufferData << uint32(guild->GetEmblemInfo().GetBackgroundColor());
        bufferData << uint32(0); // Unk Int 2 (+ 128) // Always 0 or 1
        bufferData << uint32(guild->GetEmblemInfo().GetBorderColor());
        bufferData << uint32(guildSettings.GetClassRoles());

        bufferData.WriteByteSeq(guildGUID[3]);
        bufferData << uint32(guild->GetMembersCount());
    }

    data.flushBits();
    data.append(bufferData);

    player->SendMessageToSet(&data, false);
}

void WorldSession::HandleGuildFinderDeclineRecruit(WorldPacket& recvPacket)
{
    Log.Debug("GuildHandler", "WORLD: Received CMSG_LF_GUILD_DECLINE_RECRUIT");

    ObjectGuid playerGuid;

    playerGuid[1] = recvPacket.readBit();
    playerGuid[4] = recvPacket.readBit();
    playerGuid[5] = recvPacket.readBit();
    playerGuid[2] = recvPacket.readBit();
    playerGuid[6] = recvPacket.readBit();
    playerGuid[7] = recvPacket.readBit();
    playerGuid[0] = recvPacket.readBit();
    playerGuid[3] = recvPacket.readBit();

    recvPacket.ReadByteSeq(playerGuid[5]);
    recvPacket.ReadByteSeq(playerGuid[7]);
    recvPacket.ReadByteSeq(playerGuid[2]);
    recvPacket.ReadByteSeq(playerGuid[3]);
    recvPacket.ReadByteSeq(playerGuid[4]);
    recvPacket.ReadByteSeq(playerGuid[1]);
    recvPacket.ReadByteSeq(playerGuid[0]);
    recvPacket.ReadByteSeq(playerGuid[6]);

    if (!IS_PLAYER_GUID(playerGuid))
        return;

    sGuildFinderMgr.RemoveMembershipRequest(Arcemu::Util::GUID_LOPART(playerGuid), GetPlayer()->GetGuildId());
}

void WorldSession::HandleGuildFinderGetApplications(WorldPacket& /*recvPacket*/)
{
    Log.Debug("GuildHandler", "Received CMSG_LF_GUILD_GET_APPLICATIONS"); // Empty opcode

    std::list<MembershipRequest> applicatedGuilds = sGuildFinderMgr.GetAllMembershipRequestsForPlayer(GetPlayer()->GetLowGUID());
    uint32 applicationsCount = applicatedGuilds.size();
    WorldPacket data(SMSG_LF_GUILD_MEMBERSHIP_LIST_UPDATED, 7 + 54 * applicationsCount);
    data.writeBits(applicationsCount, 20);

    if (applicationsCount > 0)
    {
        ByteBuffer bufferData(54 * applicationsCount);
        for (std::list<MembershipRequest>::const_iterator itr = applicatedGuilds.begin(); itr != applicatedGuilds.end(); ++itr)
        {
            Guild* guild = sGuildMgr.GetGuildById(itr->GetGuildId());
            LFGuildSettings guildSettings = sGuildFinderMgr.GetGuildSettings(itr->GetGuildId());
            MembershipRequest request = *itr;

            ObjectGuid guildGuid = ObjectGuid(guild->GetGUID());

            data.writeBit(guildGuid[1]);
            data.writeBit(guildGuid[0]);
            data.writeBit(guildGuid[5]);
            data.writeBits(request.GetComment().size(), 11);
            data.writeBit(guildGuid[3]);
            data.writeBit(guildGuid[7]);
            data.writeBit(guildGuid[4]);
            data.writeBit(guildGuid[6]);
            data.writeBit(guildGuid[2]);
            data.writeBits(guild->GetName().size(), 8);

            bufferData.WriteByteSeq(guildGuid[2]);
            bufferData.WriteString(request.GetComment());
            bufferData.WriteByteSeq(guildGuid[5]);
            bufferData.WriteString(guild->GetName());

            bufferData << uint32(guildSettings.GetAvailability());
            bufferData << uint32(request.GetExpiryTime() - time(NULL)); // Time left to application expiry (seconds)

            bufferData.WriteByteSeq(guildGuid[0]);
            bufferData.WriteByteSeq(guildGuid[6]);
            bufferData.WriteByteSeq(guildGuid[3]);
            bufferData.WriteByteSeq(guildGuid[7]);

            bufferData << uint32(guildSettings.GetClassRoles());

            bufferData.WriteByteSeq(guildGuid[4]);
            bufferData.WriteByteSeq(guildGuid[1]);

            bufferData << uint32(time(NULL) - request.GetSubmitTime()); // Time since application (seconds)
            bufferData << uint32(guildSettings.GetInterests());
        }

        data.flushBits();
        data.append(bufferData);
    }
    data << uint32(10 - sGuildFinderMgr.CountRequestsFromPlayer(GetPlayer()->GetLowGUID())); // Applications count left

    GetPlayer()->SendMessageToSet(&data, false);
}

// Lists all recruits for a guild - Misses times
void WorldSession::HandleGuildFinderGetRecruits(WorldPacket& recvPacket)
{
    Log.Debug("GuildHandler", "Received CMSG_LF_GUILD_GET_RECRUITS");

    uint32 unkTime = 0;
    recvPacket >> unkTime;

    Player* player = GetPlayer();
    if (!player->GetGuildId())
        return;

    std::vector<MembershipRequest> recruitsList = sGuildFinderMgr.GetAllMembershipRequestsForGuild(player->GetGuildId());
    uint32 recruitCount = recruitsList.size();

    ByteBuffer dataBuffer(53 * recruitCount);
    WorldPacket data(SMSG_LF_GUILD_RECRUIT_LIST_UPDATED, 7 + 26 * recruitCount + 53 * recruitCount);
    data.writeBits(recruitCount, 20);

    for (std::vector<MembershipRequest>::const_iterator itr = recruitsList.begin(); itr != recruitsList.end(); ++itr)
    {
        MembershipRequest request = *itr;
        ObjectGuid playerGuid(MAKE_NEW_GUID(request.GetPlayerGUID(), 0, HIGHGUID_TYPE_PLAYER));

        PlayerInfo* info = objmgr.GetPlayerInfo(request.GetPlayerGUID());
        std::string name = info->name;

        data.writeBits(request.GetComment().size(), 11);
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

        dataBuffer << int32(time(NULL) <= request.GetExpiryTime());

        dataBuffer.WriteByteSeq(playerGuid[3]);
        dataBuffer.WriteByteSeq(playerGuid[0]);
        dataBuffer.WriteByteSeq(playerGuid[1]);

        dataBuffer << int32(info->lastLevel);

        dataBuffer.WriteByteSeq(playerGuid[6]);
        dataBuffer.WriteByteSeq(playerGuid[7]);
        dataBuffer.WriteByteSeq(playerGuid[2]);

        dataBuffer << int32(time(NULL) - request.GetSubmitTime()); // Time in seconds since application submitted.
        dataBuffer << int32(request.GetAvailability());
        dataBuffer << int32(request.GetClassRoles());
        dataBuffer << int32(request.GetInterests());
        dataBuffer << int32(request.GetExpiryTime() - time(NULL)); // TIme in seconds until application expires.

        dataBuffer.WriteString(name);
        dataBuffer.WriteString(request.GetComment());

        dataBuffer << int32(info->cl);

        dataBuffer.WriteByteSeq(playerGuid[5]);
    }

    data.flushBits();
    data.append(dataBuffer);
    data << uint32(time(NULL)); // Unk time

    player->SendMessageToSet(&data, false);
}

void WorldSession::HandleGuildFinderPostRequest(WorldPacket& /*recvPacket*/)
{
    Log.Debug("GuildHandler", "Received CMSG_LF_GUILD_POST_REQUEST"); // Empty opcode

    Player* player = GetPlayer();

    if (!player->GetGuildId()) // Player must be in guild
        return;

    bool isGuildMaster = true;
    if (Guild* guild = sGuildMgr.GetGuildById(player->GetGuildId()))
        if (guild->GetLeaderGUID() != player->GetGUID())
            isGuildMaster = false;

    LFGuildSettings settings = sGuildFinderMgr.GetGuildSettings(player->GetGuildId());

    WorldPacket data(SMSG_LF_GUILD_POST_UPDATED, 35);
    data.writeBit(isGuildMaster); // Guessed

    if (isGuildMaster)
    {
        data.writeBit(settings.IsListed());
        data.writeBits(settings.GetComment().size(), 11);
        data << uint32(settings.GetLevel());
        data.WriteString(settings.GetComment());
        data << uint32(0); // Unk Int32
        data << uint32(settings.GetAvailability());
        data << uint32(settings.GetClassRoles());
        data << uint32(settings.GetInterests());
    }
    else
        data.flushBits();
    player->SendMessageToSet(&data, false);
}

void WorldSession::HandleGuildFinderRemoveRecruit(WorldPacket& recvPacket)
{
    Log.Debug("GuildHandler", "Received CMSG_LF_GUILD_REMOVE_RECRUIT");

    ObjectGuid guildGuid;

    guildGuid[0] = recvPacket.readBit();
    guildGuid[4] = recvPacket.readBit();
    guildGuid[3] = recvPacket.readBit();
    guildGuid[5] = recvPacket.readBit();
    guildGuid[7] = recvPacket.readBit();
    guildGuid[6] = recvPacket.readBit();
    guildGuid[2] = recvPacket.readBit();
    guildGuid[1] = recvPacket.readBit();

    recvPacket.ReadByteSeq(guildGuid[4]);
    recvPacket.ReadByteSeq(guildGuid[0]);
    recvPacket.ReadByteSeq(guildGuid[3]);
    recvPacket.ReadByteSeq(guildGuid[6]);
    recvPacket.ReadByteSeq(guildGuid[5]);
    recvPacket.ReadByteSeq(guildGuid[1]);
    recvPacket.ReadByteSeq(guildGuid[2]);
    recvPacket.ReadByteSeq(guildGuid[7]);

    sGuildFinderMgr.RemoveMembershipRequest(GetPlayer()->GetLowGUID(), Arcemu::Util::GUID_LOPART(guildGuid));
}

// Sent any time a guild master sets an option in the interface and when listing / unlisting his guild
void WorldSession::HandleGuildFinderSetGuildPost(WorldPacket& recvPacket)
{
    Log.Debug("GuildHandler", "WORLD: Received CMSG_LF_GUILD_SET_GUILD_POST");

    uint32 classRoles = 0;
    uint32 availability = 0;
    uint32 guildInterests = 0;
    uint32 level = 0;

    recvPacket >> level >> availability >> guildInterests >> classRoles;
    // Level sent is zero if untouched, force to any (from interface). Idk why
    if (!level)
        level = ANY_FINDER_LEVEL;

    uint16 length = recvPacket.readBits(11);
    bool listed = recvPacket.readBit();
    std::string comment = recvPacket.ReadString(length);

    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
        return;
    if (!(availability & AVAILABILITY_ALWAYS) || availability > AVAILABILITY_ALWAYS)
        return;
    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
        return;
    if (!(level & ALL_GUILDFINDER_LEVELS) || level > ALL_GUILDFINDER_LEVELS)
        return;

    Player* player = GetPlayer();

    if (!player->GetGuildId()) // Player must be in guild
        return;

    if (Guild* guild = sGuildMgr.GetGuildById(player->GetGuildId())) // Player must be guild master
        if (guild->GetLeaderGUID() != player->GetGUID())
            return;

    LFGuildSettings settings(listed, player->GetTeamReal(), player->GetGuildId(), classRoles, availability, guildInterests, level, comment);
    sGuildFinderMgr.SetGuildSettings(player->GetGuildId(), settings);
}

