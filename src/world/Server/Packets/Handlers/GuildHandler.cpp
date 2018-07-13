/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgGuildQuery.h"
#include "Server/Packets/SmsgGuildCommandResult.h"
#include "Server/Packets/CmsgGuildInvite.h"
#include "Management/GuildMgr.h"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/SmsgGuildInfo.h"
#include "Server/Packets/MsgSaveGuildEmblem.h"
#include "Server/Packets/CmsgGuildBankBuyTab.h"
#include "Server/Packets/MsgGuildBankLogQuery.h"
#include "Server/Packets/CmsgSetGuildBankText.h"
#include "Server/Packets/CmsgGuildLeader.h"
#include "Server/Packets/CmsgGuildMotd.h"
#include "Server/Packets/CmsgGuildAddRank.h"
#include "Server/Packets/CmsgGuildInfoText.h"
#include "Server/Packets/CmsgGuildRemove.h"
#include "Server/Packets/CmsgGuildPromote.h"
#include "Server/Packets/CmsgGuildDemote.h"
#include "Server/Packets/CmsgGuildSetPublicNote.h"
#include "Server/Packets/CmsgGuildSetOfficerNote.h"
#include "Server/Packets/CmsgGuildSetNote.h"
#include "Server/Packets/CmsgGuildDelRank.h"
#include "Server/Packets/CmsgGuildBankWithdrawMoney.h"
#include "Server/Packets/CmsgGuildBankDepositMoney.h"
#include "Server/Packets/CmsgGuildBankUpdateTab.h"
#include "Server/Packets/CmsgGuildBankSwapItems.h"


using namespace AscEmu::Packets;

void WorldSession::handleGuildQuery(WorldPacket& recvPacket)
{
    CmsgGuildQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto guild = sGuildMgr.getGuildById(uint32_t(recv_packet.guildId));
    if (guild == nullptr)
        return;

#if VERSION_STRING != Cata
    guild->handleQuery(this);
#else

    if (guild->isMember(recv_packet.playerGuid))
        guild->handleQuery(this);
#endif
}

void WorldSession::handleInviteToGuild(WorldPacket& recvPacket)
{
    CmsgGuildInvite recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    sendGuildInvitePacket(recv_packet.name);
}

#if VERSION_STRING != Cata
void WorldSession::handleGuildInfo(WorldPacket& /*recvPacket*/)
{
    if (const auto guild = GetPlayer()->GetGuild())
        SendPacket(SmsgGuildInfo(guild->getName(), guild->getCreatedDate(), guild->getMembersCount(), guild->getAccountCount()).serialise().get());
}
#endif

void WorldSession::handleSaveGuildEmblem(WorldPacket& recvPacket)
{
    MsgSaveGuildEmblem recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "MSG_SAVE_GUILD_EMBLEM %s: vendorGuid: %u style: %u, color: %u, borderStyle: %u, borderColor: %u, backgroundColor: %u",
        _player->getName().c_str(), recv_packet.guid.getGuidLow(), recv_packet.emblemInfo.getStyle(), recv_packet.emblemInfo.getColor(),
        recv_packet.emblemInfo.getBorderStyle(), recv_packet.emblemInfo.getBorderColor(), recv_packet.emblemInfo.getBackgroundColor());

    Guild* guild = _player->GetGuild();
    if (guild == nullptr)
    {
        SendPacket(MsgSaveGuildEmblem(GEM_ERROR_NOGUILD).serialise().get());
        return;
    }

    if (guild->getLeaderGUID() != _player->getGuid())
    {
        SendPacket(MsgSaveGuildEmblem(GEM_ERROR_NOTGUILDMASTER).serialise().get());
        return;
    }

    guild->handleSetEmblem(this, recv_packet.emblemInfo);
}

void WorldSession::handleGuildAccept(WorldPacket& /*recvPacket*/)
{
    if (!GetPlayer()->getGuildId())
        if (Guild* guild = sGuildMgr.getGuildById(GetPlayer()->GetGuildIdInvited()))
            guild->handleAcceptMember(this);
}

void WorldSession::handleGuildDecline(WorldPacket& /*recvPacket*/)
{
    GetPlayer()->SetGuildIdInvited(0);
    GetPlayer()->setGuildId(0);
}

void WorldSession::handleGuildRoster(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleRoster(this);
    else
        SendPacket(SmsgGuildCommandResult(GC_TYPE_ROSTER, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
}

void WorldSession::handleGuildLeave(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleLeaveMember(this);
}

void WorldSession::handleGuildDisband(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleDisband(this);
}

void WorldSession::handleGuildLog(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->sendEventLog(this);
}

void WorldSession::handleGuildPermissions(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->sendPermissions(this);
}

void WorldSession::handleGuildBankBuyTab(WorldPacket& recvPacket)
{
    CmsgGuildBankBuyTab recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleBuyBankTab(this, recv_packet.tabId);
}

void WorldSession::handleGuildBankLogQuery(WorldPacket& recvPacket)
{
    MsgGuildBankLogQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->sendBankLog(this, recv_packet.tabId);
}

void WorldSession::handleSetGuildBankText(WorldPacket& recvPacket)
{
    CmsgSetGuildBankText recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->setBankTabText(static_cast<uint8_t>(recv_packet.tabId), recv_packet.text);
}

void WorldSession::handleGuildLeader(WorldPacket& recvPacket)
{
    CmsgGuildLeader recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(recv_packet.name.c_str());
    if (targetPlayerInfo == nullptr)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, recv_packet.name, GC_ERROR_PLAYER_NOT_FOUND_S).serialise().get());
        return;
    }

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleSetNewGuildMaster(this, targetPlayerInfo->name);
}

void WorldSession::handleGuildMotd(WorldPacket& recvPacket)
{
    CmsgGuildMotd recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleSetMOTD(this, recv_packet.message);
}

void WorldSession::handleGuildAddRank(WorldPacket& recvPacket)
{
    CmsgGuildAddRank recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleAddNewRank(this, recv_packet.name);
}

void WorldSession::handleSetGuildInfo(WorldPacket& recvPacket)
{
    CmsgGuildInfoText recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleSetInfo(this, recv_packet.text);
}

void WorldSession::handleGuildRemove(WorldPacket& recvPacket)
{
    CmsgGuildRemove recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

#if VERSION_STRING != Cata
    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(recv_packet.name.c_str());
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleRemoveMember(this, targetPlayerInfo->guid);
#else
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleRemoveMember(this, recv_packet.guid);

#endif
}

void WorldSession::handleGuildPromote(WorldPacket& recvPacket)
{
    CmsgGuildPromote recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

#if VERSION_STRING != Cata
    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(recv_packet.name.c_str());
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleUpdateMemberRank(this, targetPlayerInfo->guid, false);
#else
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleUpdateMemberRank(this, recv_packet.guid, false);

#endif
}

void WorldSession::handleGuildDemote(WorldPacket& recvPacket)
{
    CmsgGuildDemote recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

#if VERSION_STRING != Cata
    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(recv_packet.name.c_str());
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleUpdateMemberRank(this, targetPlayerInfo->guid, true);
#else
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleUpdateMemberRank(this, recv_packet.guid, true);
#endif
}

#if VERSION_STRING != Cata
void WorldSession::handleGuildSetPublicNote(WorldPacket& recvPacket)
{
    CmsgGuildSetPublicNote recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(recv_packet.targetName.c_str());
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleSetMemberNote(this, recv_packet.note, targetPlayerInfo->guid, true);
}

void WorldSession::handleGuildSetOfficerNote(WorldPacket& recvPacket)
{
    CmsgGuildSetOfficerNote recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(recv_packet.targetName.c_str());
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleSetMemberNote(this, recv_packet.note, targetPlayerInfo->guid, false);
}
#else
void WorldSession::handleGuildSetNoteOpcode(WorldPacket& recvPacket)
{
    CmsgGuildSetNote recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleSetMemberNote(this, recv_packet.note, recv_packet.guid, recv_packet.isPublic);
}
#endif

#if VERSION_STRING != Cata
void WorldSession::handleGuildDelRank(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleRemoveLowestRank(this);
}
#else
void WorldSession::handleGuildDelRank(WorldPacket& recvPacket)
{
    CmsgGuildDelRank recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleRemoveRank(this, static_cast<uint8_t>(recv_packet.rankId));
}
#endif

void WorldSession::handleGuildBankWithdrawMoney(WorldPacket& recvPacket)
{
    CmsgGuildBankWithdrawMoney recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleMemberWithdrawMoney(this, recv_packet.money);
}

void WorldSession::handleGuildBankDepositMoney(WorldPacket& recvPacket)
{
    CmsgGuildBankDepositMoney recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    //\todo HasGold requires an uint32_t
    if (recv_packet.money && GetPlayer()->HasGold(static_cast<uint32_t>(recv_packet.money)))
        if (Guild* guild = GetPlayer()->GetGuild())
            guild->handleMemberDepositMoney(this, recv_packet.money);
}

void WorldSession::handleGuildBankUpdateTab(WorldPacket& recvPacket)
{
    CmsgGuildBankUpdateTab recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (!recv_packet.tabName.empty() && !recv_packet.tabIcon.empty())
        if (Guild* guild = GetPlayer()->GetGuild())
            guild->handleSetBankTabInfo(this, recv_packet.slot, recv_packet.tabName, recv_packet.tabIcon);
}

void WorldSession::handleGuildBankSwapItems(WorldPacket& recvPacket)
{
    Guild* guild = GetPlayer()->GetGuild();
    if (guild == nullptr)
    {
        recvPacket.rfinish();
        return;
    }

    CmsgGuildBankSwapItems recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (recv_packet.bankToBank)
        guild->swapItems(GetPlayer(), recv_packet.tabId, recv_packet.slotId, recv_packet.destTabId, recv_packet.destSlotId, recv_packet.splitedAmount);
    else
        guild->swapItemsWithInventory(GetPlayer(), recv_packet.toChar, recv_packet.tabId, recv_packet.slotId, recv_packet.playerBag, recv_packet.playerSlotId, recv_packet.splitedAmount);
}
