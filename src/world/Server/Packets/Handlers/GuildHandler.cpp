/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Management/ArenaTeam.hpp"
#include "Management/Charter.hpp"
#include "Management/ItemInterface.h"
#include "Server/Packets/CmsgGuildQuery.h"
#include "Server/Packets/CmsgQueryGuildXp.h"
#include "Server/Packets/SmsgGuildCommandResult.h"
#include "Server/Packets/CmsgGuildInvite.h"
#include "Management/Guild/GuildMgr.hpp"
#include "Management/ObjectMgr.hpp"
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
#include "Server/Packets/CmsgGuildBankWithdrawMoney.h"
#include "Server/Packets/CmsgGuildBankDepositMoney.h"
#include "Server/Packets/CmsgGuildBankUpdateTab.h"
#include "Server/Packets/CmsgGuildBankSwapItems.h"
#include "Server/Packets/CmsgGuildBankQueryTab.h"
#include "Server/Packets/CmsgGuildBankerActivate.h"
#include "Server/Packets/CmsgGuildSetRank.h"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Item.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
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
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Packets/CmsgGuildBankQueryText.h"
#include "Server/Script/HookInterface.hpp"
#include "Storage/WorldStrings.h"

#if VERSION_STRING < Cata
#include "Server/Packets/SmsgGuildInfo.h"
#include "Server/Packets/CmsgGuildSetPublicNote.h"
#include "Server/Packets/CmsgGuildSetOfficerNote.h"
#include "Server/Packets/MsgQueryGuildBankText.h"
#else
#include "Server/Packets/CmsgGuildBankQueryText.h"
#include "Server/Packets/CmsgGuildDelRank.h"
#include "Server/Packets/CmsgGuildSetNote.h"
#endif

#if VERSION_STRING >= Cata
#include "Management/Guild/GuildFinderMgr.hpp"
#include "Server/Packets/SmsgGuildMaxDailyXp.h"
#include "Server/Packets/SmsgGuildRewardsList.h"
#include "Server/Packets/SmsgLfGuildBrowseUpdated.h"
#include "Server/Packets/SmsgLfGuildMembershipListUpdated.h"
#include "Server/Packets/SmsgLfGuildRecruitListUpdated.h"
#include "Server/Packets/SmsgLfGuildPostUpdated.h"
#include "Server/Packets/CmsgGuildAssignMemberRank.h"
#include "Server/Packets/CmsgGuildQueryRanks.h"
#include "Server/Packets/CmsgGuildRequestPartyState.h"
#include "Server/Packets/CmsgGuildRequestMaxDailyXp.h"
#include "Server/Packets/CmsgQueryGuildRewards.h"
#include "Server/Packets/CmsgGuildQueryNews.h"
#include "Server/Packets/CmsgGuildNewsUpdateSticky.h"
#include "Server/Packets/CmsgReplaceGuildMaster.h"
#include "Server/Packets/CmsgAutoDeclineGuildInvites.h"
#include "Server/Packets/CmsgLfGuildAddRecruit.h"
#include "Server/Packets/CmsgLfGuildBrowse.h"
#include "Server/Packets/CmsgLfGuildDeclineRecruit.h"
#include "Server/Packets/CmsgLfGuildGetRecruits.h"
#include "Server/Packets/CmsgLfGuildRemoveRecruit.h"
#include "Server/Packets/CmsgLfGuildSetGuildPost.h"
#endif

using namespace AscEmu::Packets;

void WorldSession::handleGuildQuery(WorldPacket& recvPacket)
{
    CmsgGuildQuery srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
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
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->sendGuildInvitePacket(_player->getSession(), srlPacket.name);
}

void WorldSession::handleGuildInfo(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING < Cata
    if (const auto guild = _player->getGuild())
    {
        SmsgGuildInfo managedPacket(guild->getName(), guild->getCreatedDate(), guild->getMembersCount(), guild->getAccountCount());
        sendManagedPacket(managedPacket);
    }
#endif
}

void WorldSession::handleSaveGuildEmblem(WorldPacket& recvPacket)
{
    MsgSaveGuildEmblem srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debug("MSG_SAVE_GUILD_EMBLEM {}: vendorGuid: {} style: {}, color: {}, borderStyle: {}, borderColor: {}, backgroundColor: {}",
        _player->getName(), srlPacket.guid.getGuidLow(), srlPacket.emblemInfo.getStyle(), srlPacket.emblemInfo.getColor(),
        srlPacket.emblemInfo.getBorderStyle(), srlPacket.emblemInfo.getBorderColor(), srlPacket.emblemInfo.getBackgroundColor());

    Guild* guild = _player->getGuild();
    if (guild == nullptr)
    {
        MsgSaveGuildEmblem managedPacket(GEM_ERROR_NOGUILD);
        sendManagedPacket(managedPacket);
        return;
    }

    if (guild->getLeaderGUID() != _player->getGuid())
    {
        MsgSaveGuildEmblem managedPacket(GEM_ERROR_NOTGUILDMASTER);
        sendManagedPacket(managedPacket);
        return;
    }

    guild->handleSetEmblem(this, srlPacket.emblemInfo);
}

void WorldSession::handleGuildAccept(WorldPacket& /*recvPacket*/)
{
    if (!_player->getGuildId())
        if (Guild* guild = sGuildMgr.getGuildById(_player->getInvitedByGuildId()))
            guild->handleAcceptMember(this);
}

void WorldSession::handleGuildDecline(WorldPacket& /*recvPacket*/)
{
    _player->setInvitedByGuildId(0);
    _player->setGuildId(0);
}

void WorldSession::handleGuildRoster(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->getGuild())
    {
        guild->handleRoster(this);
    }
    else
    {
        SmsgGuildCommandResult managedPacket(GC_TYPE_ROSTER, "", GC_ERROR_PLAYER_NOT_IN_GUILD);
        sendManagedPacket(managedPacket);
    }
}

void WorldSession::handleGuildLeave(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->getGuild())
        guild->handleLeaveMember(this);
}

void WorldSession::handleGuildDisband(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->getGuild())
        guild->handleDisband(this);
}

void WorldSession::handleGuildLog(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->getGuild())
        guild->sendEventLog(this);
}

void WorldSession::handleGuildPermissions(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->getGuild())
        guild->sendPermissions(this);
}

void WorldSession::handleGuildBankBuyTab(WorldPacket& recvPacket)
{
    CmsgGuildBankBuyTab srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleBuyBankTab(this, srlPacket.tabId);
}

void WorldSession::handleGuildBankLogQuery(WorldPacket& recvPacket)
{
    MsgGuildBankLogQuery srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->sendBankLog(this, srlPacket.tabId);
}

void WorldSession::handleSetGuildBankText(WorldPacket& recvPacket)
{
    CmsgSetGuildBankText srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->setBankTabText(static_cast<uint8_t>(srlPacket.tabId), srlPacket.text);
}

void WorldSession::handleGuildLeader(WorldPacket& recvPacket)
{
    CmsgGuildLeader srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const auto targetPlayerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.name);
    if (targetPlayerInfo == nullptr)
    {
        SmsgGuildCommandResult managedPacket(GC_TYPE_CREATE, srlPacket.name, GC_ERROR_PLAYER_NOT_FOUND_S);
        sendManagedPacket(managedPacket);
        return;
    }

    if (Guild* guild = _player->getGuild())
        guild->handleSetNewGuildMaster(this, targetPlayerInfo->name);
}

void WorldSession::handleGuildMotd(WorldPacket& recvPacket)
{
    CmsgGuildMotd srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleSetMOTD(this, srlPacket.message);
}

void WorldSession::handleGuildAddRank(WorldPacket& recvPacket)
{
    CmsgGuildAddRank srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleAddNewRank(this, srlPacket.name);
}

void WorldSession::handleSetGuildInfo(WorldPacket& recvPacket)
{
    CmsgGuildInfoText srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleSetInfo(this, srlPacket.text);
}

void WorldSession::handleGuildRemove(WorldPacket& recvPacket)
{
    CmsgGuildRemove srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

#if VERSION_STRING < Cata
    const auto targetPlayerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.name);
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleRemoveMember(this, targetPlayerInfo->guid);
#else
    if (Guild* guild = _player->getGuild())
        guild->handleRemoveMember(this, srlPacket.guid);

#endif
}

void WorldSession::handleGuildPromote(WorldPacket& recvPacket)
{
    CmsgGuildPromote srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

#if VERSION_STRING < Cata
    const auto targetPlayerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.name);
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleUpdateMemberRank(this, targetPlayerInfo->guid, false);
#else
    if (Guild* guild = _player->getGuild())
        guild->handleUpdateMemberRank(this, srlPacket.guid, false);

#endif
}

void WorldSession::handleGuildDemote(WorldPacket& recvPacket)
{
    CmsgGuildDemote srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

#if VERSION_STRING < Cata
    const auto targetPlayerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.name);
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleUpdateMemberRank(this, targetPlayerInfo->guid, true);
#else
    if (Guild* guild = _player->getGuild())
        guild->handleUpdateMemberRank(this, srlPacket.guid, true);
#endif
}


void WorldSession::handleGuildSetPublicNote([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
    CmsgGuildSetPublicNote srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const auto targetPlayerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.targetName);
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleSetMemberNote(this, srlPacket.note, targetPlayerInfo->guid, true);
#endif
}

void WorldSession::handleGuildSetOfficerNote([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
    CmsgGuildSetOfficerNote srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const auto targetPlayerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.targetName);
    if (targetPlayerInfo == nullptr)
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleSetMemberNote(this, srlPacket.note, targetPlayerInfo->guid, false);
#endif
}

void WorldSession::handleGuildSetNoteOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgGuildSetNote srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleSetMemberNote(this, srlPacket.note, srlPacket.guid, srlPacket.isPublic);
#endif
}

void WorldSession::handleGuildDelRank([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
    if (Guild* guild = _player->getGuild())
        guild->handleRemoveLowestRank(this);
#else
    CmsgGuildDelRank srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleRemoveRank(this, static_cast<uint8_t>(srlPacket.rankId));
#endif
}

void WorldSession::handleGuildBankWithdrawMoney(WorldPacket& recvPacket)
{
    CmsgGuildBankWithdrawMoney srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleMemberWithdrawMoney(this, srlPacket.money);
}

void WorldSession::handleGuildBankDepositMoney(WorldPacket& recvPacket)
{
    CmsgGuildBankDepositMoney srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    //\todo HasGold requires an uint32_t
    if (srlPacket.money && _player->hasEnoughCoinage(srlPacket.money))
        if (Guild* guild = _player->getGuild())
            guild->handleMemberDepositMoney(this, srlPacket.money);
}

void WorldSession::handleGuildBankUpdateTab(WorldPacket& recvPacket)
{
    CmsgGuildBankUpdateTab srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (!srlPacket.tabName.empty() && !srlPacket.tabIcon.empty())
        if (Guild* guild = _player->getGuild())
            guild->handleSetBankTabInfo(this, srlPacket.slot, srlPacket.tabName, srlPacket.tabIcon);
}

void WorldSession::handleGuildBankSwapItems(WorldPacket& recvPacket)
{
    Guild* guild = _player->getGuild();
    if (guild == nullptr)
    {
        recvPacket.rfinish();
        return;
    }

    CmsgGuildBankSwapItems srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (srlPacket.bankToBank)
        guild->swapItems(_player, srlPacket.tabId, srlPacket.slotId, srlPacket.destTabId, srlPacket.destSlotId, srlPacket.splitedAmount);
    else
        guild->swapItemsWithInventory(_player, srlPacket.toChar, srlPacket.tabId, srlPacket.slotId, srlPacket.playerBag, srlPacket.playerSlotId, srlPacket.splitedAmount);
}


void WorldSession::handleGuildBankQueryText([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
    MsgQueryGuildBankText srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->sendBankTabText(this, srlPacket.tabId);
#endif
}

void WorldSession::handleQueryGuildBankTabText([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgGuildBankQueryText srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->sendBankTabText(this, srlPacket.tabId);
#endif
}

void WorldSession::handleGuildBankQueryTab(WorldPacket& recvPacket)
{
    CmsgGuildBankQueryTab srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    Guild* guild = _player->getGuild();
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
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const auto gameObject = _player->getWorldMap()->getGameObject(srlPacket.guid.getGuidLow());
    if (gameObject == nullptr)
        return;

    Guild* guild = _player->getGuild();
    if (guild == nullptr)
    {
        SmsgGuildCommandResult managedPacket(GC_TYPE_VIEW_TAB, "", GC_ERROR_PLAYER_NOT_IN_GUILD);
        sendManagedPacket(managedPacket);
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
    if (Guild* guild = _player->getGuild())
        guild->sendMoneyInfo(this);
}

void WorldSession::handleGuildSetRank(WorldPacket& recvPacket)
{
    CmsgGuildSetRank srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleSetRankInfo(this, static_cast<uint8_t>(srlPacket.newRankId), srlPacket.rankName, srlPacket.newRights, srlPacket.moneyPerDay, srlPacket._rightsAndSlots);
}


void WorldSession::handleCharterShowSignatures(WorldPacket& recvPacket)
{
    CmsgPetitionShowSignatures srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (const auto charter = sObjectMgr.getCharterByItemGuid(srlPacket.itemGuid))
    {
        SmsgPetitionShowSignatures managedPacket(srlPacket.itemGuid, charter->getLeaderGuid(), charter->getId(),
            charter->getSignatureCount(), charter->getAvailableSlots(), charter->getSignatures());
        _player->getSession()->sendManagedPacket(managedPacket);
    }
}

void WorldSession::handleCharterOffer(WorldPacket& recvPacket)
{
    CmsgOfferPetition srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    Player* pTarget = _player->getWorldMap()->getPlayer(srlPacket.playerGuid.getGuidLow());
    const auto pCharter = sObjectMgr.getCharterByItemGuid(srlPacket.itemGuid);
    if (pCharter == nullptr)
    {
        sendNotification(_player->getSession()->localizedWorldSrv(ServerString::SS_ITEM_NOT_FOUND));
        return;
    }

    if (pTarget == nullptr || pTarget->getTeam() != _player->getTeam() || (pTarget == _player && !worldConfig.player.isInterfactionGuildEnabled))
    {
        sendNotification(_player->getSession()->localizedWorldSrv(ServerString::SS_TARGET_WRONG_FACTION));
        return;
    }

    if (!pTarget->canSignCharter(pCharter, _player))
    {
        sendNotification(_player->getSession()->localizedWorldSrv(ServerString::SS_CANNOT_SIGN_MORE_REASONS));
        return;
    }

    SmsgPetitionShowSignatures managedPacket(srlPacket.itemGuid, pCharter->getLeaderGuid(), pCharter->getId(),
        pCharter->getSignatureCount(), pCharter->getAvailableSlots(), pCharter->getSignatures());

    pTarget->getSession()->sendManagedPacket(managedPacket);
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
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (const auto charter = sObjectMgr.getCharterByItemGuid(srlPacket.itemGuid))
    {
        for (const uint32_t playerGuid : charter->getSignatures())
        {
            if (playerGuid == _player->getGuid())
            {
                sendNotification(_player->getSession()->localizedWorldSrv(ServerString::SS_ALREADY_SIGNED_CHARTER));
                SmsgPetitionSignResult managedPacket(srlPacket.itemGuid, _player->getGuid(), PetitionSignResult::AlreadySigned);
                sendManagedPacket(managedPacket);
                return;
            }
        }

        if (charter->isFull())
            return;

        charter->addSignature(_player->getGuidLow());
        charter->saveToDB();
        _player->m_charters[charter->getCharterType()] = charter;
        _player->saveToDB(false);

        Player* player = _player->getWorldMap()->getPlayer(charter->getLeaderGuid());
        if (player == nullptr)
            return;

        SmsgPetitionSignResult managedSigneePacket(srlPacket.itemGuid, _player->getGuid(), PetitionSignResult::OK);
        player->getSession()->sendManagedPacket(managedSigneePacket);

        SmsgPetitionSignResult managedPacket(srlPacket.itemGuid, uint64_t(charter->getLeaderGuid()), PetitionSignResult::OK);
        sendManagedPacket(managedPacket);
    }
}

void WorldSession::handleCharterDecline(WorldPacket& recvPacket)
{
    MsgPetitionDecline srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    auto const charter = sObjectMgr.getCharterByItemGuid(srlPacket.itemGuid);
    if (charter == nullptr)
        return;

    Player* player = sObjectMgr.getPlayer(charter->getLeaderGuid());
    if (player && player->getSession())
    {
        MsgPetitionDecline managedPacket(_player->getGuid());
        player->getSession()->sendManagedPacket(managedPacket);
    }
}

void WorldSession::handleCharterRename(WorldPacket& recvPacket)
{
    MsgPetitionRename srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    auto const charter1 = sObjectMgr.getCharterByItemGuid(srlPacket.itemGuid);
    if (charter1 == nullptr)
        return;

    Guild* guild = sGuildMgr.getGuildByName(srlPacket.name);
    auto charter = sObjectMgr.getCharterByName(srlPacket.name, static_cast<CharterTypes>(charter1->getCharterType()));
    if (charter || guild)
    {
        sendNotification("That name is in use by another guild.");
        return;
    }

    charter = charter1;
    charter->setGuildName(srlPacket.name);
    charter->saveToDB();

    MsgPetitionRename managedPacket(srlPacket.itemGuid, srlPacket.name);
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCharterTurnInCharter(WorldPacket& recvPacket)
{
    CmsgTurnInPetition srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const auto charter = sObjectMgr.getCharterByItemGuid(srlPacket.itemGuid);
    if (charter == nullptr)
        return;

    if (charter->getCharterType() == CHARTER_TYPE_GUILD)
    {
        const auto playerCharter = _player->m_charters[CHARTER_TYPE_GUILD];
        if (playerCharter == nullptr)
            return;

        if (playerCharter->getSignatureCount() < playerCharter->getNumberOfAvailableSlots() && worldConfig.guild.requireAllSignatures && !_player->getSession()->HasGMPermissions())
        {
            Guild::sendTurnInPetitionResult(this, PETITION_ERROR_NEED_MORE_SIGNATURES);
            return;
        }

        auto* guild = sGuildMgr.createGuild(_player, playerCharter->getGuildName());
        if (guild == nullptr)
        {
            return;
        }

        _player->m_charters[CHARTER_TYPE_GUILD] = nullptr;
        playerCharter->destroy();

        _player->getItemInterface()->RemoveItemAmt(CharterEntry::Guild, 1);
        sHookInterface.OnGuildCreate(_player, guild);
    }
    else
    {
        uint8_t type;

        switch (charter->getCharterType())
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
                sendNotification("Chartertype not allowed for Arena");
                return;
        }

        if (_player->getArenaTeam(charter->getCharterType() - 1U) != nullptr)
        {
            systemMessage(localizedWorldSrv(ServerString::SS_ALREADY_ARENA_TEAM));
            return;
        }

        if (charter->getSignatureCount() < charter->getNumberOfAvailableSlots() && !_player->getSession()->HasGMPermissions())
        {
            ///\ todo: missing correct error message for arena charters
            Guild::sendTurnInPetitionResult(this, PETITION_ERROR_NEED_MORE_SIGNATURES);
            return;
        }

        ArenaTeamEmblem emblem{ .emblemStyle = srlPacket.icon, .emblemColour = srlPacket.iconColor,
            .borderStyle = srlPacket.border, .borderColour = srlPacket.borderColor, .backgroundColour = srlPacket.background };

        if (auto* const arenaTeam = sObjectMgr.createArenaTeam(type, _player, charter->getGuildName(), 1500, emblem))
        {
            // set up the leader
            _player->setArenaTeam(arenaTeam->m_type, arenaTeam);

            sObjectMgr.updateArenaTeamRankings();

            // set up the members
            for (const uint32_t playerGuid : charter->getSignatures())
            {
                if (const auto info = sObjectMgr.getCachedCharacterInfo(playerGuid))
                {
                    if (arenaTeam->addMember(info))
                    {
                        if (const auto arenaMember = sObjectMgr.getPlayer(playerGuid))
                            arenaMember->setArenaTeam(arenaTeam->m_type, arenaTeam);
                    }
                }
            }

            _player->getItemInterface()->SafeFullRemoveItemByGuid(srlPacket.itemGuid);
            _player->m_charters[charter->getCharterType()] = nullptr;
            charter->destroy();
        }
        
    }

    Guild::sendTurnInPetitionResult(this, PETITION_ERROR_OK);
}

void WorldSession::handleCharterQuery(WorldPacket& recvPacket)
{
    CmsgPetitionQuery srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (auto const charter = sObjectMgr.getCharterByItemGuid(srlPacket.itemGuid))
    {
        SmsgPetitionQueryResponse managedPacket(srlPacket.charterId, charter->getLeaderGuid(),
            charter->getGuildName(), charter->getCharterType(), charter->getAvailableSlots());
        sendManagedPacket(managedPacket);
    }
}

void WorldSession::handleCharterBuy(WorldPacket& recvPacket)
{
    CmsgPetitionBuy srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    Creature* creature = _player->getWorldMap()->getCreature(srlPacket.creatureGuid.getGuidLowPart());
    if (!creature)
    {
        Disconnect();
        return;
    }

    if (!creature->isTabardDesigner())
    {
        if ((srlPacket.arenaIndex - 1) > 2)
            return;

        const auto arena_type = static_cast<uint8_t>(srlPacket.arenaIndex - 1);
        if (_player->getArenaTeam(arena_type))
        {
            sendNotification(_player->getSession()->localizedWorldSrv(ServerString::SS_ALREADY_ARENA_TEAM));
            return;
        }

        const auto arenaTeam = sObjectMgr.getArenaTeamByName(srlPacket.name, arena_type);
        if (arenaTeam != nullptr)
        {
           systemMessage(_player->getSession()->localizedWorldSrv(ServerString::SS_PETITION_NAME_ALREADY_USED));
            return;
        }

        if (sObjectMgr.getCharterByName(srlPacket.name, static_cast<CharterTypes>(srlPacket.arenaIndex)))
        {
            systemMessage(_player->getSession()->localizedWorldSrv(ServerString::SS_PETITION_NAME_ALREADY_USED));
            return;
        }

        if (_player->m_charters[srlPacket.arenaIndex])
        {
            sendNotification(_player->getSession()->localizedWorldSrv(ServerString::SS_ALREADY_ARENA_CHARTER));
            return;
        }

        if (_player->getLevel() < PLAYER_ARENA_MIN_LEVEL)
        {
            sendNotification("You must be at least level {} to buy Arena charter", PLAYER_ARENA_MIN_LEVEL);
            return;
        }

        static uint32_t item_ids[] = { CharterEntry::TwoOnTwo, CharterEntry::ThreeOnThree, CharterEntry::FiveOnFive };
        static uint32_t costs[] = { worldConfig.arena.charterCost2v2, worldConfig.arena.charterCost3v3, worldConfig.arena.charterCost5v5 };

        if (!_player->hasEnoughCoinage(costs[arena_type]))
        {
            sendNotification("You do not have enough gold to purchase this charter");
            return;
        }

        ItemProperties const* itemProperties = sMySQLStore.getItemProperties(item_ids[arena_type]);
        if (itemProperties == nullptr)
            return;

        const SlotResult slotResult = _player->getItemInterface()->FindFreeInventorySlot(itemProperties);
        if (slotResult.Result == 0)
        {
            _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
            return;
        }

        const uint8_t error = _player->getItemInterface()->CanReceiveItem(itemProperties, 1);
        if (error)
        {
            _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, error, item_ids[arena_type]);
        }
        else
        {
            auto item = sObjectMgr.createItem(item_ids[arena_type], _player);

            auto const charter = sObjectMgr.createCharter(_player->getGuidLow(), static_cast<CharterTypes>(srlPacket.arenaIndex));
            if (item == nullptr || charter == nullptr)
                return;

            charter->setGuildName(srlPacket.name);
            charter->setItemGuid(item->getGuid());

            charter->m_petitionSignerCount = srlPacket.signerCount;

            item->setStackCount(1);
            item->addFlags(ITEM_FLAG_SOULBOUND);
            item->setEnchantmentId(0, charter->getId());
            item->setPropertySeed(57813883);
            auto* itemRawPtr = item.get();
            const auto [addResult, _] = _player->getItemInterface()->AddItemToFreeSlot(std::move(item));
            if (!addResult)
            {
                charter->destroy();
                return;
            }

            charter->saveToDB();

            _player->sendItemPushResultPacket(false, true, false, _player->getItemInterface()->LastSearchItemBagSlot(),
                _player->getItemInterface()->LastSearchItemSlot(), 1, itemRawPtr->getEntry(), itemRawPtr->getPropertySeed(), itemRawPtr->getRandomPropertiesId(), itemRawPtr->getStackCount());

            _player->modCoinage(-static_cast<int32_t>(costs[arena_type]));
            _player->m_charters[srlPacket.arenaIndex] = charter;
            _player->saveToDB(false);
        }
    }
    else
    {
        if (!_player->hasEnoughCoinage(worldConfig.guild.charterCost))
        {
            _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_NOT_ENOUGH_MONEY);
            return;
        }

        Guild* guild = sGuildMgr.getGuildByName(srlPacket.name);
        auto const charter = sObjectMgr.getCharterByName(srlPacket.name, CHARTER_TYPE_GUILD);
        if (guild != nullptr || charter != nullptr)
        {
            sendNotification(_player->getSession()->localizedWorldSrv(ServerString::SS_GUILD_NAME_ALREADY_IN_USE));
            return;
        }

        if (_player->m_charters[CHARTER_TYPE_GUILD])
        {
            sendNotification(_player->getSession()->localizedWorldSrv(ServerString::SS_ALREADY_GUILD_CHARTER));
            return;
        }

        ItemProperties const* itemProperties = sMySQLStore.getItemProperties(CharterEntry::Guild);
        if (itemProperties == nullptr)
            return;

        const SlotResult slotResult = _player->getItemInterface()->FindFreeInventorySlot(itemProperties);
        if (slotResult.Result == 0)
        {
            _player->getItemInterface()->buildInventoryChangeError(0, 0, INV_ERR_INVENTORY_FULL);
            return;
        }

        const uint8_t error = _player->getItemInterface()->CanReceiveItem(sMySQLStore.getItemProperties(CharterEntry::Guild), 1);
        if (error)
        {
            _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, error, CharterEntry::Guild);
        }
        else
        {
            _player->sendPlayObjectSoundPacket(srlPacket.creatureGuid, 6594);

            auto item = sObjectMgr.createItem(CharterEntry::Guild, _player);

            auto const guildCharter = sObjectMgr.createCharter(_player->getGuidLow(), CHARTER_TYPE_GUILD);
            if (item == nullptr || guildCharter == nullptr)
                return;

            guildCharter->setGuildName(srlPacket.name);
            guildCharter->setItemGuid(item->getGuid());

            guildCharter->m_petitionSignerCount = srlPacket.signerCount;

            item->setStackCount(1);
            item->addFlags(ITEM_FLAG_SOULBOUND);
            item->setEnchantmentId(0, guildCharter->getId());
            item->setPropertySeed(57813883);
            auto* itemRawPtr = item.get();
            const auto [addResult, _] = _player->getItemInterface()->AddItemToFreeSlot(std::move(item));
            if (!addResult)
            {
                guildCharter->destroy();
                return;
            }

            guildCharter->saveToDB();

            _player->sendItemPushResultPacket(false, true, false, _player->getItemInterface()->LastSearchItemBagSlot(),
                _player->getItemInterface()->LastSearchItemSlot(), 1, itemRawPtr->getEntry(), itemRawPtr->getPropertySeed(), itemRawPtr->getRandomPropertiesId(), itemRawPtr->getStackCount());

            _player->m_charters[CHARTER_TYPE_GUILD] = guildCharter;
            _player->modCoinage(-1000);
            _player->saveToDB(false);
        }
    }
}

void WorldSession::handleGuildAssignRankOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgGuildAssignMemberRank srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    // Mop no longer sends the setter guid on the wire - the client always means "me".
    const WoWGuid setterGuid = _socket->getClientProtocol().isMop() ? _player->GetNewGUID() : srlPacket.setterGuid;

    sLogger.debug("CMSG_GUILD_ASSIGN_MEMBER_RANK {}: Target: {} Rank: {}, Issuer: {}",
        _player->getName(), WoWGuid::getGuidLowPartFromUInt64(srlPacket.targetGuid), srlPacket.rankId, WoWGuid::getGuidLowPartFromUInt64(setterGuid));

    if (Guild* guild = _player->getGuild())
        guild->handleSetMemberRank(this, srlPacket.targetGuid, setterGuid, srlPacket.rankId);
#endif
}

void WorldSession::handleGuildQueryRanksOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgGuildQueryRanks srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debug("CMSG_GUILD_QUERY_RANKS {}: Guild: {}", _player->getName(), WoWGuid::getGuidLowPartFromUInt64(srlPacket.guildGuid));

    if (Guild* guild = sGuildMgr.getGuildById(WoWGuid::getGuidLowPartFromUInt64(srlPacket.guildGuid)))
    {
        if (guild->isMember(_player->getGuid()))
            guild->sendGuildRankInfo(this);
    }
#endif
}

void WorldSession::handleGuildRequestChallengeUpdate(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
    if (Guild* guild = _player->getGuild())
        guild->handleGuildRequestChallengeUpdate(this);
#endif
}

void WorldSession::handleGuildQueryXPOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgQueryGuildXp srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    uint32_t guildId = srlPacket.guildGuid.getGuidLowPart();

    sLogger.debug("CMSG_QUERY_GUILD_XP {}: guildId: {}", _player->getName(), guildId);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        if (guild->isMember(_player->getGuid()))
            guild->sendGuildXP(this);
    }
#endif
}

void WorldSession::handleGuildRequestPartyState([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgGuildRequestPartyState srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const uint32_t guildId = WoWGuid::getGuidLowPartFromUInt64(srlPacket.guildGuid);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
        guild->handleGuildPartyRequest(this);
#endif
}

void WorldSession::handleGuildRequestMaxDailyXP([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgGuildRequestMaxDailyXp srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const uint32_t guildId = WoWGuid::getGuidLowPartFromUInt64(srlPacket.guildGuid);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        if (guild->isMember(_player->getGuid()))
        {
            SmsgGuildMaxDailyXp managedPacket(worldConfig.guild.maxXpPerDay);
            sendManagedPacket(managedPacket);
        }
    }
#endif
}

void WorldSession::handleAutoDeclineGuildInvites([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgAutoDeclineGuildInvites srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (srlPacket.enable)
        _player->addPlayerFlags(PLAYER_FLAG_DECLINE_GUILD_INVITES);
    else
        _player->removePlayerFlags(PLAYER_FLAG_DECLINE_GUILD_INVITES);
#endif
}

void WorldSession::handleGuildRewardsQueryOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgQueryGuildRewards srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (sGuildMgr.getGuildById(_player->getGuildId()))
    {
        std::vector<GuildReward> const& rewards = sGuildMgr.getGuildRewards();

        SmsgGuildRewardsList managedPacket(rewards);
        sendManagedPacket(managedPacket);
    }
#endif
}

void WorldSession::handleGuildQueryNewsOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgGuildQueryNews srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->sendNewsUpdate(this);
#endif
}

void WorldSession::handleGuildNewsUpdateStickyOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgGuildNewsUpdateSticky srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleNewsSetSticky(this, srlPacket.newsId, srlPacket.isSticky);
#endif
}

void WorldSession::handleGuildSetGuildMaster([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgReplaceGuildMaster srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (Guild* guild = _player->getGuild())
        guild->handleReplaceGuildMaster(this);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// GuildFinder
void WorldSession::handleGuildFinderAddRecruit([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    if (sGuildFinderMgr.getAllMembershipRequestsForPlayer(_player->getGuidLow()).size() == 10)
        return;

    CmsgLfGuildAddRecruit srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const uint32_t guildLowGuid = WoWGuid::getGuidLowPartFromUInt64(uint64_t(srlPacket.guid));

    if (!(srlPacket.classRoles & GUILDFINDER_ALL_ROLES) || srlPacket.classRoles > GUILDFINDER_ALL_ROLES)
        return;

    if (!(srlPacket.availability & AVAILABILITY_ALWAYS) || srlPacket.availability > AVAILABILITY_ALWAYS)
        return;

    if (!(srlPacket.guildInterests & ALL_INTERESTS) || srlPacket.guildInterests > ALL_INTERESTS)
        return;

    MembershipRequest request = MembershipRequest(_player->getGuidLow(), guildLowGuid, srlPacket.availability, srlPacket.classRoles, srlPacket.guildInterests, srlPacket.comment, time(nullptr));
    sGuildFinderMgr.addMembershipRequest(guildLowGuid, request);
#endif
}

void WorldSession::handleGuildFinderBrowse([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgLfGuildBrowse srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const uint32_t classRoles = srlPacket.classRoles;
    const uint32_t availability = srlPacket.availability;
    const uint32_t guildInterests = srlPacket.guildInterests;
    const uint32_t playerLevel = srlPacket.playerLevel;

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

    std::vector<SmsgLfGuildBrowseEntry> guildEntries;
    guildEntries.reserve(guildList.size());

    for (LFGuildStore::const_iterator itr = guildList.begin(); itr != guildList.end(); ++itr)
    {
        LFGuildSettings guildSettings = itr->second;
        Guild* guild = sGuildMgr.getGuildById(itr->first);

        SmsgLfGuildBrowseEntry entry;
        entry.guildGuid = guild->getGUID();
        entry.name = guild->getName();
        entry.comment = guildSettings.getComment();
        entry.emblemColor = guild->getEmblemInfo().getColor();
        entry.emblemBorderStyle = guild->getEmblemInfo().getBorderStyle();
        entry.emblemStyle = guild->getEmblemInfo().getStyle();
        entry.emblemBackgroundColor = guild->getEmblemInfo().getBackgroundColor();
        entry.emblemBorderColor = guild->getEmblemInfo().getBorderColor();
        entry.level = guild->getLevel();
        entry.interests = guildSettings.getInterests();
        entry.availability = guildSettings.getAvailability();
        entry.classRoles = guildSettings.getClassRoles();
        entry.membersCount = guild->getMembersCount();
        entry.hasRequest = sGuildFinderMgr.hasRequest(player->getGuidLow(), guild->getId());

        guildEntries.push_back(entry);
    }

    SmsgLfGuildBrowseUpdated managedPacket(guildEntries);
    player->getSession()->sendManagedPacket(managedPacket);
#endif
}

void WorldSession::handleGuildFinderDeclineRecruit([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgLfGuildDeclineRecruit srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    WoWGuid wowGuid;
    wowGuid.init(srlPacket.playerGuid);

    if (!wowGuid.isPlayer())
        return;

    sGuildFinderMgr.removeMembershipRequest(wowGuid.getGuidLowPart(), _player->getGuildId());
#endif
}

void WorldSession::handleGuildFinderGetApplications(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
    std::list<MembershipRequest> applicatedGuilds = sGuildFinderMgr.getAllMembershipRequestsForPlayer(_player->getGuidLow());

    std::vector<SmsgLfGuildMembershipEntry> applications;
    applications.reserve(applicatedGuilds.size());

    for (std::list<MembershipRequest>::const_iterator itr = applicatedGuilds.begin(); itr != applicatedGuilds.end(); ++itr)
    {
        Guild* guild = sGuildMgr.getGuildById(itr->getGuildId());
        LFGuildSettings guildSettings = sGuildFinderMgr.getGuildSettings(itr->getGuildId());
        MembershipRequest request = *itr;

        SmsgLfGuildMembershipEntry entry;
        entry.guildGuid = guild->getGUID();
        entry.name = guild->getName();
        entry.comment = request.getComment();
        entry.availability = guildSettings.getAvailability();
        entry.classRoles = guildSettings.getClassRoles();
        entry.interests = guildSettings.getInterests();
        entry.submitTime = request.getSubmitTime();
        entry.expiryTime = request.getExpiryTime();

        applications.push_back(entry);
    }

    SmsgLfGuildMembershipListUpdated managedPacket(applications, 10 - sGuildFinderMgr.countRequestsFromPlayer(_player->getGuidLow()));
    _player->getSession()->sendManagedPacket(managedPacket);
#endif
}

void WorldSession::handleGuildFinderGetRecruits([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgLfGuildGetRecruits srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    Player* player = _player;
    if (!player->getGuildId())
        return;

    std::vector<MembershipRequest> recruitsList = sGuildFinderMgr.getAllMembershipRequestsForGuild(player->getGuildId());

    std::vector<SmsgLfGuildRecruitEntry> recruitEntries;
    recruitEntries.reserve(recruitsList.size());

    for (std::vector<MembershipRequest>::const_iterator itr = recruitsList.begin(); itr != recruitsList.end(); ++itr)
    {
        MembershipRequest request = *itr;
        WoWGuid playerGuid(request.getPlayerGUID(), 0, HIGHGUID_TYPE_PLAYER);

        const auto* info = sObjectMgr.getCachedCharacterInfo(request.getPlayerGUID());

        SmsgLfGuildRecruitEntry entry;
        entry.playerGuid = playerGuid;
        entry.name = info->name;
        entry.comment = request.getComment();
        entry.level = static_cast<int32_t>(info->lastLevel);
        entry.availability = request.getAvailability();
        entry.classRoles = request.getClassRoles();
        entry.interests = request.getInterests();
        entry.playerClass = info->cl;
        entry.submitTime = request.getSubmitTime();
        entry.expiryTime = request.getExpiryTime();

        recruitEntries.push_back(entry);
    }

    SmsgLfGuildRecruitListUpdated managedPacket(recruitEntries);
    player->getSession()->sendManagedPacket(managedPacket);
#endif
}

void WorldSession::handleGuildFinderPostRequest(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
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

    SmsgLfGuildPostUpdated managedPacket(isGuildMaster, settings.isListed(), settings.getLevel(), settings.getComment(),
        settings.getAvailability(), settings.getClassRoles(), settings.getInterests());
    player->getSession()->sendManagedPacket(managedPacket);
#endif
}

void WorldSession::handleGuildFinderRemoveRecruit([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgLfGuildRemoveRecruit srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sGuildFinderMgr.removeMembershipRequest(WoWGuid::getGuidLowPartFromUInt64(_player->getGuid()), WoWGuid::getGuidLowPartFromUInt64(srlPacket.guildGuid));
#endif
}

void WorldSession::handleGuildFinderSetGuildPost([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgLfGuildSetGuildPost srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    uint32_t classRoles = srlPacket.classRoles;
    uint32_t availability = srlPacket.availability;
    uint32_t guildInterests = srlPacket.guildInterests;
    uint32_t level = srlPacket.level;
    bool listed = srlPacket.listed;
    std::string& comment = srlPacket.comment;

    if (level == 0)
        level = ANY_FINDER_LEVEL;

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
#endif
}
