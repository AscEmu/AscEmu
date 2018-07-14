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
#include "Server/Packets/CmsgPetitionBuy.h"


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

void WorldSession::handleCharterBuy(WorldPacket& recvPacket)
{
    CmsgPetitionBuy recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    Creature* creature = _player->GetMapMgr()->GetCreature(recv_packet.creatureGuid.getGuidLow());
    if (!creature)
    {
        Disconnect();
        return;
    }

    if (!creature->isTabardDesigner())
    {
        const uint32_t arena_type = recv_packet.arenaIndex - 1;
        if (arena_type > 2)
            return;

        if (_player->m_arenaTeams[arena_type])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(SS_ALREADY_ARENA_TEAM));
            return;
        }

        ArenaTeam* arenaTeam = objmgr.GetArenaTeamByName(recv_packet.name, arena_type);
        if (arenaTeam != nullptr)
        {
            sChatHandler.SystemMessage(this, _player->GetSession()->LocalizedWorldSrv(SS_PETITION_NAME_ALREADY_USED));
            return;
        }

        if (objmgr.GetCharterByName(recv_packet.name, static_cast<CharterTypes>(recv_packet.arenaIndex)))
        {
            sChatHandler.SystemMessage(this, _player->GetSession()->LocalizedWorldSrv(SS_PETITION_NAME_ALREADY_USED));
            return;
        }

        if (_player->m_charters[recv_packet.arenaIndex])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(SS_ALREADY_ARENA_CHARTER));
            return;
        }

        if (_player->getLevel() < PLAYER_ARENA_MIN_LEVEL)
        {
            SendNotification("You must be at least level %u to buy Arena charter", PLAYER_ARENA_MIN_LEVEL);
            return;
        }

        static uint32_t item_ids[] = { CharterEntry::TwoOnTwo, CharterEntry::ThreeOnThree, CharterEntry::FiveOnFive };
        static uint32_t costs[] = { CharterCost::TwoOnTwo, CharterCost::ThreeOnThree, CharterCost::FiveOnFive };

        if (!_player->HasGold(costs[arena_type]))
        {
            SendNotification("You do not have enough gold to purchase this charter");
            return;
        }

        ItemProperties const* itemProperties = sMySQLStore.getItemProperties(item_ids[arena_type]);
        if (itemProperties == nullptr)
            return;

        const SlotResult slotResult = _player->GetItemInterface()->FindFreeInventorySlot(itemProperties);
        if (slotResult.Result == 0)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
            return;
        }

        const uint8_t error = _player->GetItemInterface()->CanReceiveItem(itemProperties, 1);
        if (error)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        }
        else
        {
            Item* item = objmgr.CreateItem(item_ids[arena_type], _player);

            Charter* charter = objmgr.CreateCharter(_player->getGuidLow(), static_cast<CharterTypes>(recv_packet.arenaIndex));
            if (item == nullptr || charter == nullptr)
                return;

            charter->GuildName = recv_packet.name;
            charter->ItemGuid = item->getGuid();

            charter->PetitionSignerCount = recv_packet.signerCount;

            item->setStackCount(1);
            item->addFlags(ITEM_FLAG_SOULBOUND);
            item->setEnchantmentId(0, charter->GetID());
            item->setPropertySeed(57813883);
            if (!_player->GetItemInterface()->AddItemToFreeSlot(item))
            {
                charter->Destroy();
                item->DeleteMe();
                return;
            }

            charter->SaveToDB();

            _player->SendItemPushResult(false, true, false, true, _player->GetItemInterface()->LastSearchItemBagSlot(),
                _player->GetItemInterface()->LastSearchItemSlot(), 1, item->getEntry(), item->getPropertySeed(), item->getRandomPropertiesId(), item->getStackCount());

            _player->ModGold(-static_cast<int32_t>(costs[arena_type]));
            _player->m_charters[recv_packet.arenaIndex] = charter;
            _player->SaveToDB(false);
        }
    }
    else
    {
        if (!_player->HasGold(1000))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_NOT_ENOUGH_MONEY);
            return;
        }

        Guild* guild = sGuildMgr.getGuildByName(recv_packet.name);
        Charter* charter = objmgr.GetCharterByName(recv_packet.name, CHARTER_TYPE_GUILD);
        if (guild != nullptr || charter != nullptr)
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(SS_GUILD_NAME_ALREADY_IN_USE));
            return;
        }

        if (_player->m_charters[CHARTER_TYPE_GUILD])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(SS_ALREADY_GUILD_CHARTER));
            return;
        }

        ItemProperties const* itemProperties = sMySQLStore.getItemProperties(CharterEntry::Guild);
        if (itemProperties == nullptr)
            return;

        const SlotResult slotResult = _player->GetItemInterface()->FindFreeInventorySlot(itemProperties);
        if (slotResult.Result == 0)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(0, 0, INV_ERR_INVENTORY_FULL);
            return;
        }

        const uint8_t error = _player->GetItemInterface()->CanReceiveItem(sMySQLStore.getItemProperties(CharterEntry::Guild), 1);
        if (error)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        }
        else
        {
            _player->PlaySoundToPlayer(recv_packet.creatureGuid, 6594);

            Item* item = objmgr.CreateItem(CharterEntry::Guild, _player);

            Charter* guildCharter = objmgr.CreateCharter(_player->getGuidLow(), CHARTER_TYPE_GUILD);
            if (item == nullptr || guildCharter == nullptr)
                return;

            guildCharter->GuildName = recv_packet.name;
            guildCharter->ItemGuid = item->getGuid();

            guildCharter->PetitionSignerCount = recv_packet.signerCount;

            item->setStackCount(1);
            item->addFlags(ITEM_FLAG_SOULBOUND);
            item->setEnchantmentId(0, guildCharter->GetID());
            item->setPropertySeed(57813883);
            if (!_player->GetItemInterface()->AddItemToFreeSlot(item))
            {
                guildCharter->Destroy();
                item->DeleteMe();
                return;
            }

            guildCharter->SaveToDB();

            _player->SendItemPushResult(false, true, false, true, _player->GetItemInterface()->LastSearchItemBagSlot(),
                _player->GetItemInterface()->LastSearchItemSlot(), 1, item->getEntry(), item->getPropertySeed(), item->getRandomPropertiesId(), item->getStackCount());

            _player->m_charters[CHARTER_TYPE_GUILD] = guildCharter;
            _player->ModGold(-1000);
            _player->SaveToDB(false);
        }
    }
}

#if VERSION_STRING == Cata
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
        guild->handleSetMemberRank(this, targetGuid, setterGuid, rankId);
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
            guild->sendGuildRankInfo(this);
    }
}

void WorldSession::HandleGuildRequestChallengeUpdate(WorldPacket& /*recv_data*/)
{
    if (Guild* guild = _player->GetGuild())
        guild->handleGuildRequestChallengeUpdate(this);
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
            guild->sendGuildXP(this);
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
        guild->handleGuildPartyRequest(this);
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
        guild->sendNewsUpdate(this);
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
        guild->handleNewsSetSticky(this, newsId, isSticky);
}

void WorldSession::HandleGuildSetGuildMaster(WorldPacket& recvData)
{
    uint8_t nameLength = static_cast<uint8_t>(recvData.readBits(7));

    recvData.readBit();

    std::string playerName = recvData.ReadString(nameLength);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->handleSetNewGuildMaster(this, playerName);
}

//////////////////////////////////////////////////////////////////////////////////////////
// GuildFinder
void WorldSession::HandleGuildFinderAddRecruit(WorldPacket& recvData)
{
    if (sGuildFinderMgr.getAllMembershipRequestsForPlayer(GetPlayer()->getGuidLow()).size() == 10)
        return;

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
        return;

    if (!(availability & AVAILABILITY_ALWAYS) || availability > AVAILABILITY_ALWAYS)
        return;

    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
        return;

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
        return;

    if (!(availability & AVAILABILITY_ALWAYS) || availability > AVAILABILITY_ALWAYS)
        return;

    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
        return;

    if (playerLevel > worldConfig.player.playerLevelCap || playerLevel < 1)
        return;

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
        return;

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
            isGuildMaster = false;
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
        level = ANY_FINDER_LEVEL;

    uint32_t length = recv_data.readBits(11);
    bool listed = recv_data.readBit();
    std::string comment = recv_data.ReadString(length);

    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
        return;

    if (!(availability & AVAILABILITY_ALWAYS) || availability > AVAILABILITY_ALWAYS)
        return;

    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
        return;

    if (!(level & ALL_GUILDFINDER_LEVELS) || level > ALL_GUILDFINDER_LEVELS)
        return;

    Player* player = GetPlayer();
    if (!player->getGuildId())
        return;

    if (Guild* guild = sGuildMgr.getGuildById(player->getGuildId()))
    {
        if (guild->getLeaderGUID() != player->getGuid())
            return;
    }

    LFGuildSettings settings(listed, player->GetTeam(), player->getGuildId(), static_cast<uint8_t>(classRoles), static_cast<uint8_t>(availability), static_cast<uint8_t>(guildInterests), static_cast<uint8_t>(level), comment);
    sGuildFinderMgr.setGuildSettings(player->getGuildId(), settings);
}

#endif
