/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Storage/MySQLDataStore.hpp"
#include "scripts/InstanceScripts/Setup.h"
#include "Storage/WorldStrings.h"

#if VERSION_STRING == Cata
#include "GameCata/Management/GuildFinderMgr.h"
#elif VERSION_STRING == Mop
#include "GameMop/Management/GuildFinderMgr.h"
#endif

using namespace AscEmu::Packets;

void WorldSession::handleGuildQuery(WorldPacket& recvPacket)
{
    CmsgGuildQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto guild = sGuildMgr.getGuildById(uint32_t(srlPacket.guildId));
    if (guild == nullptr)
        return;

#if VERSION_STRING < Cata
    guild->handleQuery(this);
#else

    if (guild->isMember(srlPacket.playerGuid))
        guild->handleQuery(this);
#endif
}

void WorldSession::handleInviteToGuild(WorldPacket& recvPacket)
{
    CmsgGuildInvite srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->sendGuildInvitePacket(_player->GetSession(), srlPacket.name);
}

#if VERSION_STRING < Cata
void WorldSession::handleGuildInfo(WorldPacket& /*recvPacket*/)
{
    if (const auto guild = _player->GetGuild())
        SendPacket(SmsgGuildInfo(guild->getName(), guild->getCreatedDate(), guild->getMembersCount(), guild->getAccountCount()).serialise().get());
}
#endif

void WorldSession::handleSaveGuildEmblem(WorldPacket& recvPacket)
{
    MsgSaveGuildEmblem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "MSG_SAVE_GUILD_EMBLEM %s: vendorGuid: %u style: %u, color: %u, borderStyle: %u, borderColor: %u, backgroundColor: %u",
        _player->getName().c_str(), srlPacket.guid.getGuidLow(), srlPacket.emblemInfo.getStyle(), srlPacket.emblemInfo.getColor(),
        srlPacket.emblemInfo.getBorderStyle(), srlPacket.emblemInfo.getBorderColor(), srlPacket.emblemInfo.getBackgroundColor());

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

    guild->handleSetEmblem(this, srlPacket.emblemInfo);
}

void WorldSession::handleGuildAccept(WorldPacket& /*recvPacket*/)
{
    if (!_player->getGuildId())
        if (Guild* guild = sGuildMgr.getGuildById(_player->GetGuildIdInvited()))
            guild->handleAcceptMember(this);
}

void WorldSession::handleGuildDecline(WorldPacket& /*recvPacket*/)
{
    _player->SetGuildIdInvited(0);
    _player->setGuildId(0);
}

void WorldSession::handleGuildRoster(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->GetGuild())
        guild->handleRoster(this);
    else
        SendPacket(SmsgGuildCommandResult(GC_TYPE_ROSTER, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
}

void WorldSession::handleGuildLeave(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->GetGuild())
        guild->handleLeaveMember(this);
}

void WorldSession::handleGuildDisband(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->GetGuild())
        guild->handleDisband(this);
}

void WorldSession::handleGuildLog(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->GetGuild())
        guild->sendEventLog(this);
}

void WorldSession::handleGuildPermissions(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->GetGuild())
        guild->sendPermissions(this);
}

void WorldSession::handleGuildBankBuyTab(WorldPacket& recvPacket)
{
    CmsgGuildBankBuyTab srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleBuyBankTab(this, srlPacket.tabId);
}

void WorldSession::handleGuildBankLogQuery(WorldPacket& recvPacket)
{
    MsgGuildBankLogQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->sendBankLog(this, srlPacket.tabId);
}

void WorldSession::handleSetGuildBankText(WorldPacket& recvPacket)
{
    CmsgSetGuildBankText srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->setBankTabText(static_cast<uint8_t>(srlPacket.tabId), srlPacket.text);
}

void WorldSession::handleGuildLeader(WorldPacket& recvPacket)
{
    CmsgGuildLeader srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(srlPacket.name.c_str());
    if (targetPlayerInfo == nullptr)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, srlPacket.name, GC_ERROR_PLAYER_NOT_FOUND_S).serialise().get());
        return;
    }

    if (Guild* guild = _player->GetGuild())
        guild->handleSetNewGuildMaster(this, targetPlayerInfo->name);
}

void WorldSession::handleGuildMotd(WorldPacket& recvPacket)
{
    CmsgGuildMotd srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleSetMOTD(this, srlPacket.message);
}

void WorldSession::handleGuildAddRank(WorldPacket& recvPacket)
{
    CmsgGuildAddRank srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleAddNewRank(this, srlPacket.name);
}

void WorldSession::handleSetGuildInfo(WorldPacket& recvPacket)
{
    CmsgGuildInfoText srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleSetInfo(this, srlPacket.text);
}

void WorldSession::handleGuildRemove(WorldPacket& recvPacket)
{
    CmsgGuildRemove srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

#if VERSION_STRING < Cata
    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(srlPacket.name.c_str());
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleRemoveMember(this, targetPlayerInfo->guid);
#else
    if (Guild* guild = _player->GetGuild())
        guild->handleRemoveMember(this, srlPacket.guid);

#endif
}

void WorldSession::handleGuildPromote(WorldPacket& recvPacket)
{
    CmsgGuildPromote srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

#if VERSION_STRING < Cata
    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(srlPacket.name.c_str());
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleUpdateMemberRank(this, targetPlayerInfo->guid, false);
#else
    if (Guild* guild = _player->GetGuild())
        guild->handleUpdateMemberRank(this, srlPacket.guid, false);

#endif
}

void WorldSession::handleGuildDemote(WorldPacket& recvPacket)
{
    CmsgGuildDemote srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

#if VERSION_STRING < Cata
    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(srlPacket.name.c_str());
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleUpdateMemberRank(this, targetPlayerInfo->guid, true);
#else
    if (Guild* guild = _player->GetGuild())
        guild->handleUpdateMemberRank(this, srlPacket.guid, true);
#endif
}

#if VERSION_STRING < Cata
void WorldSession::handleGuildSetPublicNote(WorldPacket& recvPacket)
{
    CmsgGuildSetPublicNote srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(srlPacket.targetName.c_str());
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleSetMemberNote(this, srlPacket.note, targetPlayerInfo->guid, true);
}

void WorldSession::handleGuildSetOfficerNote(WorldPacket& recvPacket)
{
    CmsgGuildSetOfficerNote srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto targetPlayerInfo = objmgr.GetPlayerInfoByName(srlPacket.targetName.c_str());
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleSetMemberNote(this, srlPacket.note, targetPlayerInfo->guid, false);
}
#else
void WorldSession::handleGuildSetNoteOpcode(WorldPacket& recvPacket)
{
    CmsgGuildSetNote srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleSetMemberNote(this, srlPacket.note, srlPacket.guid, srlPacket.isPublic);
}
#endif

#if VERSION_STRING < Cata
void WorldSession::handleGuildDelRank(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->GetGuild())
        guild->handleRemoveLowestRank(this);
}
#else
void WorldSession::handleGuildDelRank(WorldPacket& recvPacket)
{
    CmsgGuildDelRank srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleRemoveRank(this, static_cast<uint8_t>(srlPacket.rankId));
}
#endif

void WorldSession::handleGuildBankWithdrawMoney(WorldPacket& recvPacket)
{
    CmsgGuildBankWithdrawMoney srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleMemberWithdrawMoney(this, srlPacket.money);
}

void WorldSession::handleGuildBankDepositMoney(WorldPacket& recvPacket)
{
    CmsgGuildBankDepositMoney srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    //\todo HasGold requires an uint32_t
    if (srlPacket.money && _player->hasEnoughCoinage(srlPacket.money))
        if (Guild* guild = _player->GetGuild())
            guild->handleMemberDepositMoney(this, srlPacket.money);
}

void WorldSession::handleGuildBankUpdateTab(WorldPacket& recvPacket)
{
    CmsgGuildBankUpdateTab srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!srlPacket.tabName.empty() && !srlPacket.tabIcon.empty())
        if (Guild* guild = _player->GetGuild())
            guild->handleSetBankTabInfo(this, srlPacket.slot, srlPacket.tabName, srlPacket.tabIcon);
}

void WorldSession::handleGuildBankSwapItems(WorldPacket& recvPacket)
{
    Guild* guild = _player->GetGuild();
    if (guild == nullptr)
    {
        recvPacket.rfinish();
        return;
    }

    CmsgGuildBankSwapItems srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.bankToBank)
        guild->swapItems(_player, srlPacket.tabId, srlPacket.slotId, srlPacket.destTabId, srlPacket.destSlotId, srlPacket.splitedAmount);
    else
        guild->swapItemsWithInventory(_player, srlPacket.toChar, srlPacket.tabId, srlPacket.slotId, srlPacket.playerBag, srlPacket.playerSlotId, srlPacket.splitedAmount);
}

#if VERSION_STRING < Cata
void WorldSession::handleGuildBankQueryText(WorldPacket& recvPacket)
{
    MsgQueryGuildBankText srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->sendBankTabText(this, srlPacket.tabId);
}
#else
void WorldSession::handleQueryGuildBankTabText(WorldPacket& recvPacket)
{
    CmsgGuildBankQueryText srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->sendBankTabText(this, srlPacket.tabId);
}
#endif

void WorldSession::handleGuildBankQueryTab(WorldPacket& recvPacket)
{
    CmsgGuildBankQueryTab srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    Guild* guild = _player->GetGuild();
    if (guild == nullptr)
        return;

    GuildBankTab* pTab = guild->getBankTab(srlPacket.tabId);
    if (pTab == nullptr)
        return;

#if VERSION_STRING < Cata
    guild->sendBankList(this, srlPacket.tabId, false, true);
#else
    guild->sendBankList(this, srlPacket.tabId, true, false);
#endif
}

void WorldSession::handleGuildBankerActivate(WorldPacket& recvPacket)
{
    CmsgGuildBankerActivate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto gameObject = _player->GetMapMgr()->GetGameObject(srlPacket.guid.getGuidLow());
    if (gameObject == nullptr)
        return;

    Guild* guild = _player->GetGuild();
    if (guild == nullptr)
    {
        SendPacket(SmsgGuildCommandResult(GC_TYPE_VIEW_TAB, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

#if VERSION_STRING < Cata
    guild->sendBankList(this, 0, false, false);
#else
    guild->sendBankList(this, 0, true, true);
#endif
}

void WorldSession::handleGuildBankMoneyWithdrawn(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->GetGuild())
        guild->sendMoneyInfo(this);
}

void WorldSession::handleGuildSetRank(WorldPacket& recvPacket)
{
    CmsgGuildSetRank srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Guild* guild = _player->GetGuild())
        guild->handleSetRankInfo(this, static_cast<uint8_t>(srlPacket.newRankId), srlPacket.rankName, srlPacket.newRights, srlPacket.moneyPerDay, srlPacket._rightsAndSlots);
}


void WorldSession::handleCharterShowSignatures(WorldPacket& recvPacket)
{
    CmsgPetitionShowSignatures srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Charter* charter = objmgr.GetCharterByItemGuid(srlPacket.itemGuid))
        _player->GetSession()->SendPacket(SmsgPetitionShowSignatures(srlPacket.itemGuid, charter->GetLeader(), charter->GetID(), static_cast<uint8_t>(charter->SignatureCount),
            charter->Slots, charter->Signatures).serialise().get());
}

void WorldSession::handleCharterOffer(WorldPacket& recvPacket)
{
    CmsgOfferPetition srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    Player* pTarget = _player->GetMapMgr()->GetPlayer(srlPacket.playerGuid.getGuidLow());
    Charter* pCharter = objmgr.GetCharterByItemGuid(srlPacket.itemGuid);
    if (pCharter != nullptr)
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(76));
        return;
    }

    if (pTarget == nullptr || pTarget->getTeam() != _player->getTeam() || (pTarget == _player && !worldConfig.player.isInterfactionGuildEnabled))
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(77));
        return;
    }

    if (!pTarget->CanSignCharter(pCharter, _player))
    {
        SendNotification(_player->GetSession()->LocalizedWorldSrv(78));
        return;
    }

    pTarget->GetSession()->SendPacket(SmsgPetitionShowSignatures(srlPacket.itemGuid, pCharter->GetLeader(), pCharter->GetID(), static_cast<uint8_t>(pCharter->SignatureCount),
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
    CmsgPetitionSign srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Charter* charter = objmgr.GetCharterByItemGuid(srlPacket.itemGuid))
    {
        for (uint32_t i = 0; i < charter->SignatureCount; ++i)
        {
            if (charter->Signatures[i] == _player->getGuid())
            {
                SendNotification(_player->GetSession()->LocalizedWorldSrv(79));
                SendPacket(SmsgPetitionSignResult(srlPacket.itemGuid, _player->getGuid(), PetitionSignResult::AlreadySigned).serialise().get());
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

        player->SendPacket(SmsgPetitionSignResult(srlPacket.itemGuid, _player->getGuid(), PetitionSignResult::OK).serialise().get());
        SendPacket(SmsgPetitionSignResult(srlPacket.itemGuid, uint64_t(charter->GetLeader()), PetitionSignResult::OK).serialise().get());
    }
}

void WorldSession::handleCharterDecline(WorldPacket& recvPacket)
{
    MsgPetitionDecline srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    Charter* charter = objmgr.GetCharterByItemGuid(srlPacket.itemGuid);
    if (charter == nullptr)
        return;

    Player* player = objmgr.GetPlayer(charter->GetLeader());
    if (player)
        player->GetSession()->SendPacket(MsgPetitionDecline(_player->getGuid()).serialise().get());
}

void WorldSession::handleCharterRename(WorldPacket& recvPacket)
{
    MsgPetitionRename srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    Charter* charter1 = objmgr.GetCharterByItemGuid(srlPacket.itemGuid);
    if (charter1 == nullptr)
        return;

    Guild* guild = sGuildMgr.getGuildByName(srlPacket.name);
    Charter* charter = objmgr.GetCharterByName(srlPacket.name, static_cast<CharterTypes>(charter1->CharterType));
    if (charter || guild)
    {
        SendNotification("That name is in use by another guild.");
        return;
    }

    charter = charter1;
    charter->GuildName = srlPacket.name;
    charter->SaveToDB();

    SendPacket(MsgPetitionRename(srlPacket.itemGuid, srlPacket.name).serialise().get());
}

void WorldSession::handleCharterTurnInCharter(WorldPacket& recvPacket)
{
    CmsgTurnInPetition srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto charter = objmgr.GetCharterByItemGuid(srlPacket.itemGuid);
    if (charter == nullptr)
        return;

    if (charter->CharterType == CHARTER_TYPE_GUILD)
    {
        const auto playerCharter = _player->m_charters[CHARTER_TYPE_GUILD];
        if (playerCharter == nullptr)
            return;

        if (playerCharter->SignatureCount < playerCharter->GetNumberOfSlotsByType() && worldConfig.guild.requireAllSignatures && !_player->GetSession()->HasGMPermissions())
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

        _player->getItemInterface()->RemoveItemAmt(CharterEntry::Guild, 1);
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

        if (charter->SignatureCount < charter->GetNumberOfSlotsByType() && !_player->GetSession()->HasGMPermissions())
        {
            ///\ todo: missing correct error message for arena charters
            Guild::sendTurnInPetitionResult(this, PETITION_ERROR_NEED_MORE_SIGNATURES);
            return;
        }

        const auto arenaTeam = new ArenaTeam(type, objmgr.GenerateArenaTeamId());
        arenaTeam->m_name = charter->GuildName;
        arenaTeam->m_emblemColour = srlPacket.iconColor;
        arenaTeam->m_emblemStyle = srlPacket.icon;
        arenaTeam->m_borderColour = srlPacket.borderColor;
        arenaTeam->m_borderStyle = srlPacket.border;
        arenaTeam->m_backgroundColour = srlPacket.background;
        arenaTeam->m_leader = _player->getGuidLow();
        arenaTeam->m_stat_rating = 1500;

        objmgr.AddArenaTeam(arenaTeam);
        objmgr.UpdateArenaTeamRankings();
        arenaTeam->AddMember(_player->m_playerInfo);

        for (uint32_t i = 0; i < charter->SignatureCount; ++i)
            if (PlayerInfo* info = objmgr.GetPlayerInfo(charter->Signatures[i]))
                arenaTeam->AddMember(info);

        _player->getItemInterface()->SafeFullRemoveItemByGuid(srlPacket.itemGuid);
        _player->m_charters[charter->CharterType] = nullptr;
        charter->Destroy();
    }

    Guild::sendTurnInPetitionResult(this, PETITION_ERROR_OK);
}

void WorldSession::handleCharterQuery(WorldPacket& recvPacket)
{
    CmsgPetitionQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Charter* charter = objmgr.GetCharterByItemGuid(srlPacket.itemGuid))
        SendPacket(SmsgPetitionQueryResponse(srlPacket.charterId, static_cast<uint64>(charter->LeaderGuid),
            charter->GuildName, charter->CharterType, charter->Slots).serialise().get());
}

void WorldSession::handleCharterBuy(WorldPacket& recvPacket)
{
    CmsgPetitionBuy srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    Creature* creature = _player->GetMapMgr()->GetCreature(srlPacket.creatureGuid.getGuidLow());
    if (!creature)
    {
        Disconnect();
        return;
    }

    if (!creature->isTabardDesigner())
    {
        const uint32_t arena_type = srlPacket.arenaIndex - 1;
        if (arena_type > 2)
            return;

        if (_player->m_arenaTeams[arena_type])
        {
            SendNotification(_player->GetSession()->LocalizedWorldSrv(SS_ALREADY_ARENA_TEAM));
            return;
        }

        ArenaTeam* arenaTeam = objmgr.GetArenaTeamByName(srlPacket.name, arena_type);
        if (arenaTeam != nullptr)
        {
            sChatHandler.SystemMessage(this, _player->GetSession()->LocalizedWorldSrv(SS_PETITION_NAME_ALREADY_USED));
            return;
        }

        if (objmgr.GetCharterByName(srlPacket.name, static_cast<CharterTypes>(srlPacket.arenaIndex)))
        {
            sChatHandler.SystemMessage(this, _player->GetSession()->LocalizedWorldSrv(SS_PETITION_NAME_ALREADY_USED));
            return;
        }

        if (_player->m_charters[srlPacket.arenaIndex])
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
        static uint32_t costs[] = { worldConfig.arena.charterCost2v2, worldConfig.arena.charterCost3v3, worldConfig.arena.charterCost5v5 };

        if (!_player->hasEnoughCoinage(costs[arena_type]))
        {
            SendNotification("You do not have enough gold to purchase this charter");
            return;
        }

        ItemProperties const* itemProperties = sMySQLStore.getItemProperties(item_ids[arena_type]);
        if (itemProperties == nullptr)
            return;

        const SlotResult slotResult = _player->getItemInterface()->FindFreeInventorySlot(itemProperties);
        if (slotResult.Result == 0)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
            return;
        }

        const uint8_t error = _player->getItemInterface()->CanReceiveItem(itemProperties, 1);
        if (error)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        }
        else
        {
            Item* item = objmgr.CreateItem(item_ids[arena_type], _player);

            Charter* charter = objmgr.CreateCharter(_player->getGuidLow(), static_cast<CharterTypes>(srlPacket.arenaIndex));
            if (item == nullptr || charter == nullptr)
                return;

            charter->GuildName = srlPacket.name;
            charter->ItemGuid = item->getGuid();

            charter->PetitionSignerCount = srlPacket.signerCount;

            item->setStackCount(1);
            item->addFlags(ITEM_FLAG_SOULBOUND);
            item->setEnchantmentId(0, charter->GetID());
            item->setPropertySeed(57813883);
            if (!_player->getItemInterface()->AddItemToFreeSlot(item))
            {
                charter->Destroy();
                item->DeleteMe();
                return;
            }

            charter->SaveToDB();

            _player->sendItemPushResultPacket(false, true, false, _player->getItemInterface()->LastSearchItemBagSlot(),
                _player->getItemInterface()->LastSearchItemSlot(), 1, item->getEntry(), item->getPropertySeed(), item->getRandomPropertiesId(), item->getStackCount());

            _player->modCoinage(-static_cast<int32_t>(costs[arena_type]));
            _player->m_charters[srlPacket.arenaIndex] = charter;
            _player->SaveToDB(false);
        }
    }
    else
    {
        if (!_player->hasEnoughCoinage(worldConfig.guild.charterCost))
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_NOT_ENOUGH_MONEY);
            return;
        }

        Guild* guild = sGuildMgr.getGuildByName(srlPacket.name);
        Charter* charter = objmgr.GetCharterByName(srlPacket.name, CHARTER_TYPE_GUILD);
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

        const SlotResult slotResult = _player->getItemInterface()->FindFreeInventorySlot(itemProperties);
        if (slotResult.Result == 0)
        {
            _player->getItemInterface()->BuildInventoryChangeError(0, 0, INV_ERR_INVENTORY_FULL);
            return;
        }

        const uint8_t error = _player->getItemInterface()->CanReceiveItem(sMySQLStore.getItemProperties(CharterEntry::Guild), 1);
        if (error)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        }
        else
        {
            _player->sendPlayObjectSoundPacket(srlPacket.creatureGuid, 6594);

            Item* item = objmgr.CreateItem(CharterEntry::Guild, _player);

            Charter* guildCharter = objmgr.CreateCharter(_player->getGuidLow(), CHARTER_TYPE_GUILD);
            if (item == nullptr || guildCharter == nullptr)
                return;

            guildCharter->GuildName = srlPacket.name;
            guildCharter->ItemGuid = item->getGuid();

            guildCharter->PetitionSignerCount = srlPacket.signerCount;

            item->setStackCount(1);
            item->addFlags(ITEM_FLAG_SOULBOUND);
            item->setEnchantmentId(0, guildCharter->GetID());
            item->setPropertySeed(57813883);
            if (!_player->getItemInterface()->AddItemToFreeSlot(item))
            {
                guildCharter->Destroy();
                item->DeleteMe();
                return;
            }

            guildCharter->SaveToDB();

            _player->sendItemPushResultPacket(false, true, false, _player->getItemInterface()->LastSearchItemBagSlot(),
                _player->getItemInterface()->LastSearchItemSlot(), 1, item->getEntry(), item->getPropertySeed(), item->getRandomPropertiesId(), item->getStackCount());

            _player->m_charters[CHARTER_TYPE_GUILD] = guildCharter;
            _player->modCoinage(-1000);
            _player->SaveToDB(false);
        }
    }
}

#if VERSION_STRING >= Cata
void WorldSession::handleGuildAssignRankOpcode(WorldPacket& recvPacket)
{
    ObjectGuid targetGuid;
    ObjectGuid setterGuid;

    uint32_t rankId;
    recvPacket >> rankId;

    targetGuid[1] = recvPacket.readBit();
    targetGuid[7] = recvPacket.readBit();

    setterGuid[4] = recvPacket.readBit();
    setterGuid[2] = recvPacket.readBit();

    targetGuid[4] = recvPacket.readBit();
    targetGuid[5] = recvPacket.readBit();
    targetGuid[6] = recvPacket.readBit();

    setterGuid[1] = recvPacket.readBit();
    setterGuid[7] = recvPacket.readBit();

    targetGuid[2] = recvPacket.readBit();
    targetGuid[3] = recvPacket.readBit();
    targetGuid[0] = recvPacket.readBit();

    setterGuid[6] = recvPacket.readBit();
    setterGuid[3] = recvPacket.readBit();
    setterGuid[0] = recvPacket.readBit();
    setterGuid[5] = recvPacket.readBit();

    recvPacket.ReadByteSeq(targetGuid[0]);

    recvPacket.ReadByteSeq(setterGuid[1]);
    recvPacket.ReadByteSeq(setterGuid[3]);
    recvPacket.ReadByteSeq(setterGuid[5]);

    recvPacket.ReadByteSeq(targetGuid[7]);
    recvPacket.ReadByteSeq(targetGuid[3]);

    recvPacket.ReadByteSeq(setterGuid[0]);

    recvPacket.ReadByteSeq(targetGuid[1]);

    recvPacket.ReadByteSeq(setterGuid[6]);

    recvPacket.ReadByteSeq(targetGuid[2]);
    recvPacket.ReadByteSeq(targetGuid[5]);
    recvPacket.ReadByteSeq(targetGuid[4]);

    recvPacket.ReadByteSeq(setterGuid[2]);
    recvPacket.ReadByteSeq(setterGuid[4]);

    recvPacket.ReadByteSeq(targetGuid[6]);

    recvPacket.ReadByteSeq(setterGuid[7]);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_ASSIGN_MEMBER_RANK %s: Target: %u Rank: %u, Issuer: %u",
        _player->getName().c_str(), WoWGuid::getGuidLowPartFromUInt64(targetGuid), rankId, WoWGuid::getGuidLowPartFromUInt64(setterGuid));

    if (Guild* guild = _player->GetGuild())
        guild->handleSetMemberRank(this, targetGuid, setterGuid, rankId);
}

void WorldSession::handleGuildQueryRanksOpcode(WorldPacket& recvPacket)
{
    ObjectGuid guildGuid;

    guildGuid[2] = recvPacket.readBit();
    guildGuid[3] = recvPacket.readBit();
    guildGuid[0] = recvPacket.readBit();
    guildGuid[6] = recvPacket.readBit();
    guildGuid[4] = recvPacket.readBit();
    guildGuid[7] = recvPacket.readBit();
    guildGuid[5] = recvPacket.readBit();
    guildGuid[1] = recvPacket.readBit();

    recvPacket.ReadByteSeq(guildGuid[3]);
    recvPacket.ReadByteSeq(guildGuid[4]);
    recvPacket.ReadByteSeq(guildGuid[5]);
    recvPacket.ReadByteSeq(guildGuid[7]);
    recvPacket.ReadByteSeq(guildGuid[1]);
    recvPacket.ReadByteSeq(guildGuid[0]);
    recvPacket.ReadByteSeq(guildGuid[6]);
    recvPacket.ReadByteSeq(guildGuid[2]);

    LogDebugFlag(LF_OPCODE, "CMSG_GUILD_QUERY_RANKS %s: Guild: %u", _player->getName().c_str(), WoWGuid::getGuidLowPartFromUInt64(guildGuid));

    if (Guild* guild = sGuildMgr.getGuildById(WoWGuid::getGuidLowPartFromUInt64(guildGuid)))
    {
        if (guild->isMember(_player->getGuid()))
            guild->sendGuildRankInfo(this);
    }
}

void WorldSession::handleGuildRequestChallengeUpdate(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->GetGuild())
        guild->handleGuildRequestChallengeUpdate(this);
}

void WorldSession::handleGuildQueryXPOpcode(WorldPacket& recvPacket)
{
    ObjectGuid guildGuid;

    guildGuid[2] = recvPacket.readBit();
    guildGuid[1] = recvPacket.readBit();
    guildGuid[0] = recvPacket.readBit();
    guildGuid[5] = recvPacket.readBit();
    guildGuid[4] = recvPacket.readBit();
    guildGuid[7] = recvPacket.readBit();
    guildGuid[6] = recvPacket.readBit();
    guildGuid[3] = recvPacket.readBit();

    recvPacket.ReadByteSeq(guildGuid[7]);
    recvPacket.ReadByteSeq(guildGuid[2]);
    recvPacket.ReadByteSeq(guildGuid[3]);
    recvPacket.ReadByteSeq(guildGuid[6]);
    recvPacket.ReadByteSeq(guildGuid[1]);
    recvPacket.ReadByteSeq(guildGuid[5]);
    recvPacket.ReadByteSeq(guildGuid[0]);
    recvPacket.ReadByteSeq(guildGuid[4]);

    uint32_t guildId = WoWGuid::getGuidLowPartFromUInt64(guildGuid);

    LogDebugFlag(LF_OPCODE, "CMSG_QUERY_GUILD_XP %s: guildId: %u", _player->getName().c_str(), guildId);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        if (guild->isMember(_player->getGuid()))
            guild->sendGuildXP(this);
    }
}

void WorldSession::handleGuildRequestPartyState(WorldPacket& recvPacket)
{
    ObjectGuid guildGuid;

    guildGuid[0] = recvPacket.readBit();
    guildGuid[6] = recvPacket.readBit();
    guildGuid[7] = recvPacket.readBit();
    guildGuid[3] = recvPacket.readBit();
    guildGuid[5] = recvPacket.readBit();
    guildGuid[1] = recvPacket.readBit();
    guildGuid[2] = recvPacket.readBit();
    guildGuid[4] = recvPacket.readBit();

    recvPacket.ReadByteSeq(guildGuid[6]);
    recvPacket.ReadByteSeq(guildGuid[3]);
    recvPacket.ReadByteSeq(guildGuid[2]);
    recvPacket.ReadByteSeq(guildGuid[1]);
    recvPacket.ReadByteSeq(guildGuid[5]);
    recvPacket.ReadByteSeq(guildGuid[0]);
    recvPacket.ReadByteSeq(guildGuid[7]);
    recvPacket.ReadByteSeq(guildGuid[4]);

    uint32_t guildId = WoWGuid::getGuidLowPartFromUInt64(guildGuid);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
        guild->handleGuildPartyRequest(this);
}

void WorldSession::handleGuildRequestMaxDailyXP(WorldPacket& recvPacket)
{
    ObjectGuid guid;

    guid[0] = recvPacket.readBit();
    guid[3] = recvPacket.readBit();
    guid[5] = recvPacket.readBit();
    guid[1] = recvPacket.readBit();
    guid[4] = recvPacket.readBit();
    guid[6] = recvPacket.readBit();
    guid[7] = recvPacket.readBit();
    guid[2] = recvPacket.readBit();

    recvPacket.ReadByteSeq(guid[7]);
    recvPacket.ReadByteSeq(guid[4]);
    recvPacket.ReadByteSeq(guid[3]);
    recvPacket.ReadByteSeq(guid[5]);
    recvPacket.ReadByteSeq(guid[1]);
    recvPacket.ReadByteSeq(guid[2]);
    recvPacket.ReadByteSeq(guid[6]);
    recvPacket.ReadByteSeq(guid[0]);

    uint32_t guildId = WoWGuid::getGuidLowPartFromUInt64(guid);

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

void WorldSession::handleAutoDeclineGuildInvites(WorldPacket& recvPacket)
{
    uint8_t enable;
    recvPacket >> enable;

    bool enabled = enable > 0 ? true : false;

    _player->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_AUTO_DECLINE_GUILD, enabled);
}

void WorldSession::handleGuildRewardsQueryOpcode(WorldPacket& recvPacket)
{
    recvPacket.read_skip<uint32_t>();

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

void WorldSession::handleGuildQueryNewsOpcode(WorldPacket& recvPacket)
{
    recvPacket.read_skip<uint32_t>();

    if (Guild* guild = _player->GetGuild())
        guild->sendNewsUpdate(this);
}

void WorldSession::handleGuildNewsUpdateStickyOpcode(WorldPacket& recvPacket)
{
    uint32_t newsId;
    recvPacket >> newsId;

    ObjectGuid guid;
    guid[2] = recvPacket.readBit();
    guid[4] = recvPacket.readBit();
    guid[3] = recvPacket.readBit();
    guid[0] = recvPacket.readBit();

    bool isSticky = recvPacket.readBit();

    guid[6] = recvPacket.readBit();
    guid[7] = recvPacket.readBit();
    guid[1] = recvPacket.readBit();
    guid[5] = recvPacket.readBit();

    recvPacket.ReadByteSeq(guid[6]);
    recvPacket.ReadByteSeq(guid[2]);
    recvPacket.ReadByteSeq(guid[1]);
    recvPacket.ReadByteSeq(guid[0]);
    recvPacket.ReadByteSeq(guid[5]);
    recvPacket.ReadByteSeq(guid[3]);
    recvPacket.ReadByteSeq(guid[7]);
    recvPacket.ReadByteSeq(guid[4]);

    if (Guild* guild = _player->GetGuild())
        guild->handleNewsSetSticky(this, newsId, isSticky);
}

void WorldSession::handleGuildSetGuildMaster(WorldPacket& recvPacket)
{
    const auto nameLength = static_cast<uint8_t>(recvPacket.readBits(7));

    recvPacket.readBit();

    const auto playerName = recvPacket.ReadString(nameLength);

    if (Guild* guild = _player->GetGuild())
        guild->handleSetNewGuildMaster(this, playerName);
}

//////////////////////////////////////////////////////////////////////////////////////////
// GuildFinder
void WorldSession::handleGuildFinderAddRecruit(WorldPacket& recvPacket)
{
    if (sGuildFinderMgr.getAllMembershipRequestsForPlayer(_player->getGuidLow()).size() == 10)
        return;

    uint32_t classRoles = 0;
    uint32_t availability = 0;
    uint32_t guildInterests = 0;

    recvPacket >> classRoles;
    recvPacket >> guildInterests;
    recvPacket >> availability;

    ObjectGuid guid;

    guid[3] = recvPacket.readBit();
    guid[0] = recvPacket.readBit();
    guid[6] = recvPacket.readBit();
    guid[1] = recvPacket.readBit();

    uint16_t commentLength = static_cast<uint16_t>(recvPacket.readBits(11));

    guid[5] = recvPacket.readBit();
    guid[4] = recvPacket.readBit();
    guid[7] = recvPacket.readBit();

    uint8_t nameLength = static_cast<uint8_t>(recvPacket.readBits(7));

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

    uint32_t guildLowGuid = WoWGuid::getGuidLowPartFromUInt64(uint64_t(guid));

    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
        return;

    if (!(availability & AVAILABILITY_ALWAYS) || availability > AVAILABILITY_ALWAYS)
        return;

    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
        return;

    MembershipRequest request = MembershipRequest(_player->getGuidLow(), guildLowGuid, availability, classRoles, guildInterests, comment, time(nullptr));
    sGuildFinderMgr.addMembershipRequest(guildLowGuid, request);
}

void WorldSession::handleGuildFinderBrowse(WorldPacket& recvPacket)
{
    uint32_t classRoles = 0;
    uint32_t availability = 0;
    uint32_t guildInterests = 0;
    uint32_t playerLevel = 0;

    recvPacket >> classRoles;
    recvPacket >> availability;
    recvPacket >> guildInterests;
    recvPacket >> playerLevel;

    if (!(classRoles & GUILDFINDER_ALL_ROLES) || classRoles > GUILDFINDER_ALL_ROLES)
        return;

    if (!(availability & AVAILABILITY_ALWAYS) || availability > AVAILABILITY_ALWAYS)
        return;

    if (!(guildInterests & ALL_INTERESTS) || guildInterests > ALL_INTERESTS)
        return;

    if (playerLevel > worldConfig.player.playerLevelCap || playerLevel < 1)
        return;

    Player* player = _player;

    LFGuildPlayer settings(player->getGuidLow(), static_cast<uint8_t>(classRoles), static_cast<uint8_t>(availability), static_cast<uint8_t>(guildInterests), ANY_FINDER_LEVEL);
    LFGuildStore guildList = sGuildFinderMgr.getGuildsMatchingSetting(settings, player->getTeam());
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

void WorldSession::handleGuildFinderDeclineRecruit(WorldPacket& recvPacket)
{
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

    WoWGuid wowGuid;
    wowGuid.Init(playerGuid);

    if (!wowGuid.isPlayer())
        return;

    sGuildFinderMgr.removeMembershipRequest(wowGuid.getGuidLowPart(), _player->getGuildId());
}

void WorldSession::handleGuildFinderGetApplications(WorldPacket& /*recvPacket*/)
{
    std::list<MembershipRequest> applicatedGuilds = sGuildFinderMgr.getAllMembershipRequestsForPlayer(_player->getGuidLow());
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
    data << uint32_t(10 - sGuildFinderMgr.countRequestsFromPlayer(_player->getGuidLow()));

    _player->SendPacket(&data);
}

void WorldSession::handleGuildFinderGetRecruits(WorldPacket& recvPacket)
{
    uint32_t unkTime = 0;
    recvPacket >> unkTime;

    Player* player = _player;
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

void WorldSession::handleGuildFinderPostRequest(WorldPacket& /*recvPacket*/)
{
    Player* player = _player;
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

void WorldSession::handleGuildFinderRemoveRecruit(WorldPacket& recvPacket)
{
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

    sGuildFinderMgr.removeMembershipRequest(WoWGuid::getGuidLowPartFromUInt64(_player->getGuid()), WoWGuid::getGuidLowPartFromUInt64(guildGuid));
}

void WorldSession::handleGuildFinderSetGuildPost(WorldPacket& recvPacket)
{
    uint32_t classRoles = 0;
    uint32_t availability = 0;
    uint32_t guildInterests = 0;
    uint32_t level = 0;

    recvPacket >> level;
    recvPacket >> availability;
    recvPacket >> guildInterests;
    recvPacket >> classRoles;

    if (level == 0)
        level = ANY_FINDER_LEVEL;

    uint32_t length = recvPacket.readBits(11);
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

    Player* player = _player;
    if (!player->getGuildId())
        return;

    if (Guild* guild = sGuildMgr.getGuildById(player->getGuildId()))
    {
        if (guild->getLeaderGUID() != player->getGuid())
            return;
    }

    LFGuildSettings settings(listed, player->getTeam(), player->getGuildId(), static_cast<uint8_t>(classRoles), static_cast<uint8_t>(availability), static_cast<uint8_t>(guildInterests), static_cast<uint8_t>(level), comment);
    sGuildFinderMgr.setGuildSettings(player->getGuildId(), settings);
}

#endif
