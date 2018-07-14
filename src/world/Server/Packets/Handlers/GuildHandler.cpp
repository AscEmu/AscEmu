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
#include "Server/Packets/MsgQueryGuildBankText.h"
#include "Server/Packets/CmsgGuildBankQueryText.h"
#include "Server/Packets/CmsgGuildBankQueryTab.h"
#include "Server/Packets/CmsgGuildBankerActivate.h"
#include "Server/Packets/CmsgGuildSetRank.h"
#include "Map/MapMgr.h"
#include "Server/Packets/CmsgPetitionShowSignatures.h"
#include "Server/Packets/SmsgPetitionShowSignatures.h"
#include "Server/Packets/CmsgOfferPetition.h"
#include "Server/Packets/SmsgPetitionSignResult.h"
#include "Server/Packets/CmsgPetitionSign.h"
#include "Server/Packets/MsgPetitionDecline.h"
#include "Server/Packets/MsgPetitionRename.h"
#include "Server/Packets/CmsgTurnInPetition.h"
#include "Server/Packets/CmsgPetitionQuery.h"
#include "Server/Packets/SmsgPetitionQueryResponse.h"


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

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->sendGuildInvitePacket(GetPlayer()->GetSession(), recv_packet.name);
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

#if VERSION_STRING != Cata
void WorldSession::handleGuildBankQueryText(WorldPacket& recvPacket)
{
    MsgQueryGuildBankText recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->sendBankTabText(this, recv_packet.tabId);
}
#else
void WorldSession::handleQueryGuildBankTabText(WorldPacket& recvPacket)
{
    CmsgGuildBankQueryText recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->sendBankTabText(this, recv_packet.tabId);
}
#endif

void WorldSession::handleGuildBankQueryTab(WorldPacket& recvPacket)
{
    CmsgGuildBankQueryTab recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    Guild* guild = GetPlayer()->GetGuild();
    if (guild == nullptr)
        return;

    GuildBankTab* pTab = guild->getBankTab(recv_packet.tabId);
    if (pTab == nullptr)
        return;

#if VERSION_STRING != Cata
    guild->sendBankList(this, recv_packet.tabId, false, true);
#else
    guild->sendBankList(this, recv_packet.tabId, true, false);
#endif
}

void WorldSession::handleGuildBankerActivate(WorldPacket& recvPacket)
{
    CmsgGuildBankerActivate recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto gameObject = GetPlayer()->GetMapMgr()->GetGameObject(recv_packet.guid.getGuidLow());
    if (gameObject == nullptr)
        return;

    Guild* guild = GetPlayer()->GetGuild();
    if (guild == nullptr)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_VIEW_TAB, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

#if VERSION_STRING != Cata
    guild->sendBankList(this, 0, false, false);
#else
    guild->sendBankList(this, 0, true, true);
#endif
}

void WorldSession::handleGuildBankMoneyWithdrawn(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->sendMoneyInfo(this);
}

void WorldSession::handleGuildSetRank(WorldPacket& recvPacket)
{
    CmsgGuildSetRank recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleSetRankInfo(this, recv_packet.newRankId, recv_packet.rankName, recv_packet.newRights, recv_packet.moneyPerDay, recv_packet._rightsAndSlots);
}


void WorldSession::handleCharterShowSignatures(WorldPacket& recvPacket)
{
    CmsgPetitionShowSignatures recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Charter* charter = objmgr.GetCharterByItemGuid(recv_packet.itemGuid))
        _player->GetSession()->SendPacket(SmsgPetitionShowSignatures(recv_packet.itemGuid, charter->GetLeader(), charter->GetID(), charter->SignatureCount,
            charter->Slots, charter->Signatures).serialise().get());
}

void WorldSession::handleCharterOffer(WorldPacket& recvPacket)
{
    CmsgOfferPetition recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    Player* pTarget = _player->GetMapMgr()->GetPlayer(recv_packet.playerGuid.getGuidLow());
    Charter* pCharter = objmgr.GetCharterByItemGuid(recv_packet.itemGuid);
    if (pCharter != nullptr)
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(76));
        return;
    }

    if (pTarget == nullptr || pTarget->GetTeam() != _player->GetTeam() || (pTarget == _player && !worldConfig.player.isInterfactionGuildEnabled))
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(77));
        return;
    }

    if (!pTarget->CanSignCharter(pCharter, _player))
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(78));
        return;
    }

    pTarget->GetSession()->SendPacket(SmsgPetitionShowSignatures(recv_packet.itemGuid, pCharter->GetLeader(), pCharter->GetID(), pCharter->SignatureCount,
        pCharter->Slots, pCharter->Signatures).serialise().get());
}

namespace PetitionSignResult
{
    enum
    {
        OK = 0,
        AlreadySigned = 1
    };
}

void WorldSession::handleCharterSign(WorldPacket& recvPacket)
{
    CmsgPetitionSign recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Charter* charter = objmgr.GetCharterByItemGuid(recv_packet.itemGuid))
    {
        for (uint32_t i = 0; i < charter->SignatureCount; ++i)
        {
            if (charter->Signatures[i] == _player->getGuid())
            {
                SendNotification(_player->GetSession()->LocalizedWorldSrv(79));
                SendPacket(SmsgPetitionSignResult(recv_packet.itemGuid, _player->getGuid(), PetitionSignResult::AlreadySigned).serialise().get());
                return;
            }
        }

        if (charter->IsFull())
            return;

        charter->AddSignature(_player->getGuidLow());
        charter->SaveToDB();
        _player->m_charters[charter->CharterType] = charter;
        _player->SaveToDB(false);

        Player* player = _player->GetMapMgr()->GetPlayer(charter->GetLeader());
        if (player == nullptr)
            return;

        player->SendPacket(SmsgPetitionSignResult(recv_packet.itemGuid, _player->getGuid(), PetitionSignResult::OK).serialise().get());
        SendPacket(SmsgPetitionSignResult(recv_packet.itemGuid, uint64_t(charter->GetLeader()), PetitionSignResult::OK).serialise().get());
    }
}

void WorldSession::handleCharterDecline(WorldPacket& recvPacket)
{
    MsgPetitionDecline recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    Charter* charter = objmgr.GetCharterByItemGuid(recv_packet.itemGuid);
    if (charter == nullptr)
        return;

    Player* player = objmgr.GetPlayer(charter->GetLeader());
    if (player)
        player->GetSession()->SendPacket(MsgPetitionDecline(_player->getGuid()).serialise().get());
}

void WorldSession::handleCharterRename(WorldPacket& recvPacket)
{
    MsgPetitionRename recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    Charter* charter1 = objmgr.GetCharterByItemGuid(recv_packet.itemGuid);
    if (charter1 == nullptr)
        return;

    Guild* guild = sGuildMgr.getGuildByName(recv_packet.name);
    Charter* charter = objmgr.GetCharterByName(recv_packet.name, static_cast<CharterTypes>(charter1->CharterType));
    if (charter || guild)
    {
        SendNotification("That name is in use by another guild.");
        return;
    }

    charter = charter1;
    charter->GuildName = recv_packet.name;
    charter->SaveToDB();

    SendPacket(MsgPetitionRename(recv_packet.itemGuid, recv_packet.name).serialise().get());
}

void WorldSession::handleCharterTurnInCharter(WorldPacket& recvPacket)
{
    CmsgTurnInPetition recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto charter = objmgr.GetCharterByItemGuid(recv_packet.itemGuid);
    if (charter == nullptr)
        return;

    if (charter->CharterType == CHARTER_TYPE_GUILD)
    {
        const auto playerCharter = _player->m_charters[CHARTER_TYPE_GUILD];
        if (playerCharter == nullptr)
            return;

        if (playerCharter->SignatureCount < 9 && worldConfig.server.requireAllSignatures)
        {
            Guild::sendTurnInPetitionResult(this, PETITION_ERROR_NEED_MORE_SIGNATURES);
            return;
        }

        const auto guild = new Guild;
        if (!guild->create(_player, playerCharter->GuildName))
        {
            delete guild;
            return;
        }

        _player->m_charters[CHARTER_TYPE_GUILD] = nullptr;
        playerCharter->Destroy();

        _player->GetItemInterface()->RemoveItemAmt(CharterEntry::Guild, 1);
        sHookInterface.OnGuildCreate(_player, guild);
    }
    else
    {
        uint16_t type;

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

        if (_player->m_arenaTeams[charter->CharterType - 1] != nullptr)
        {
            sChatHandler.SystemMessage(this, LocalizedWorldSrv(SS_ALREADY_ARENA_TEAM));
            return;
        }

        if (charter->SignatureCount < charter->GetNumberOfSlotsByType() && worldConfig.server.requireAllSignatures)
        {
            Guild::sendTurnInPetitionResult(this, PETITION_ERROR_NEED_MORE_SIGNATURES);
            return;
        }

        const auto arenaTeam = new ArenaTeam(type, objmgr.GenerateArenaTeamId());
        arenaTeam->m_name = charter->GuildName;
        arenaTeam->m_emblemColour = recv_packet.iconColor;
        arenaTeam->m_emblemStyle = recv_packet.icon;
        arenaTeam->m_borderColour = recv_packet.borderColor;
        arenaTeam->m_borderStyle = recv_packet.border;
        arenaTeam->m_backgroundColour = recv_packet.background;
        arenaTeam->m_leader = _player->getGuidLow();
        arenaTeam->m_stat_rating = 1500;

        objmgr.AddArenaTeam(arenaTeam);
        objmgr.UpdateArenaTeamRankings();
        arenaTeam->AddMember(_player->m_playerInfo);

        for (uint32_t i = 0; i < charter->SignatureCount; ++i)
            if (PlayerInfo* info = objmgr.GetPlayerInfo(charter->Signatures[i]))
                arenaTeam->AddMember(info);

        _player->GetItemInterface()->SafeFullRemoveItemByGuid(recv_packet.itemGuid);
        _player->m_charters[charter->CharterType] = nullptr;
        charter->Destroy();
    }

    Guild::sendTurnInPetitionResult(this, PETITION_ERROR_OK);
}

void WorldSession::handleCharterQuery(WorldPacket& recvPacket)
{
    CmsgPetitionQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (Charter* charter = objmgr.GetCharterByItemGuid(recv_packet.itemGuid))
        SendPacket(SmsgPetitionQueryResponse(recv_packet.charterId, static_cast<uint64>(charter->LeaderGuid),
            charter->GuildName, charter->CharterType, charter->Slots).serialise().get());
}
