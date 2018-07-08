/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/WeatherMgr.h"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.h"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "zlib.h"
#include "Map/MapMgr.h"
#include "Spell/SpellMgr.h"
#include "Server/Packets/SmsgLogoutResponse.h"
#include "Server/Packets/CmsgStandStateChange.h"
#include "Server/Packets/CmsgWho.h"
#include "Server/Packets/CmsgSetSelection.h"
#include "Server/Packets/CmsgTutorialFlag.h"
#include "Server/Packets/CmsgSetSheathed.h"
#include "Server/Packets/CmsgPlayedTime.h"
#include "Server/Packets/SmsgPlayedTime.h"
#include "Server/Packets/CmsgSetActionButton.h"
#include "Server/Packets/CmsgSetWatchedFaction.h"
#include "Server/Packets/MsgRandomRoll.h"
#include "Server/Packets/CmsgRealmSplit.h"
#include "Server/Packets/SmsgRealmSplit.h"
#include "Server/Packets/CmsgSetTaxiBenchmarkMode.h"
#include "Server/Packets/SmsgWorldStateUiTimerUpdate.h"
#include "Server/Packets/CmsgGameobjReportUse.h"
#include "Server/Packets/MsgSetDungeonDifficulty.h"
#include "Server/Packets/MsgSetRaidDifficulty.h"
#include "Server/Packets/CmsgOptOutOfLoot.h"
#include "Server/Packets/CmsgSetActionbarToggles.h"
#include "Server/Packets/CmsgLootRoll.h"
#include "Server/Packets/CmsgOpenItem.h"
#include "Server/Packets/CmsgSetTitle.h"
#include "Management/GuildMgr.h"

using namespace AscEmu::Packets;

void WorldSession::handleStandStateChangeOpcode(WorldPacket& recvPacket)
{
    CmsgStandStateChange recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    _player->setStandState(recv_packet.state);
}

void WorldSession::handleWhoOpcode(WorldPacket& recvPacket)
{
    CmsgWho recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    bool cname = false;
    bool gname = false;

    if (recv_packet.player_name.length() > 0)
        cname = true;

    if (recv_packet.guild_name.length() > 0)
        gname = true;

    LOG_DEBUG("WORLD: Recvd CMSG_WHO Message with %u zones and %u names", recv_packet.zone_count, recv_packet.name_count);

    uint32_t team = _player->GetTeam();

    uint32_t sent_count = 0;
    uint32_t total_count = 0;

    WorldPacket data;
    data.SetOpcode(SMSG_WHO);
    data << uint64_t(0);

    objmgr._playerslock.AcquireReadLock();
    PlayerStorageMap::const_iterator iend = objmgr._players.end();
    PlayerStorageMap::const_iterator itr = objmgr._players.begin();
    while (itr != iend && sent_count < 49)
    {
        Player* plr = itr->second;
        ++itr;

        if (!plr->GetSession() || !plr->IsInWorld())
            continue;

        if (!worldConfig.server.showGmInWhoList && !HasGMPermissions())
        {
            if (plr->GetSession()->HasGMPermissions())
                continue;
        }

        // Team check
        if (!HasGMPermissions() && plr->GetTeam() != team && !plr->GetSession()->HasGMPermissions() && !worldConfig.player.isInterfactionMiscEnabled)
            continue;

        ++total_count;

        // Add by default, if we don't have any checks
        bool add = true;

        // Chat name
        if (cname && recv_packet.player_name.compare(plr->getName()) != 0)
            continue;

        // Guild name
        if (gname)
        {
#if VERSION_STRING != Cata
            if (!plr->GetGuild() || recv_packet.guild_name != plr->GetGuild()->getGuildName())
                continue;
#else
            if (!plr->GetGuild() || recv_packet.guild_name.compare(plr->GetGuild()->getName()) != 0)
                continue;
#endif
        }

        // Level check
        if (recv_packet.min_level && recv_packet.max_level)
        {
            // skip players outside of level range
            if (plr->getLevel() < recv_packet.min_level || plr->getLevel() > recv_packet.max_level)
                continue;
        }

        // Zone id compare
        if (recv_packet.zone_count)
        {
            // people that fail the zone check don't get added
            add = false;
            for (uint32_t i = 0; i < recv_packet.zone_count; ++i)
            {
                if (recv_packet.zones[i] == plr->GetZoneId())
                {
                    add = true;
                    break;
                }
            }
        }

        if (!((recv_packet.class_mask >> 1) & plr->getClassMask()) || !((recv_packet.race_mask >> 1) & plr->getRaceMask()))
            add = false;

        // skip players that fail zone check
        if (!add)
            continue;

        if (recv_packet.name_count)
        {
            // people that fail name check don't get added
            add = false;
            for (uint32_t i = 0; i < recv_packet.name_count; ++i)
            {
                if (!strnicmp(recv_packet.names[i].c_str(), plr->getName().c_str(), recv_packet.names[i].length()))
                {
                    add = true;
                    break;
                }
            }
        }

        if (!add)
            continue;

        // if we're here, it means we've passed all tests
        data << plr->getName().c_str();

#if VERSION_STRING != Cata
        if (plr->m_playerInfo->guild)
            data << plr->m_playerInfo->guild->getGuildName();
        else
            data << uint8_t(0);
#else
        if (plr->m_playerInfo->m_guild)
            data << sGuildMgr.getGuildById(plr->m_playerInfo->m_guild)->getName().c_str();
        else
            data << uint8_t(0);
#endif

        data << plr->getLevel();
        data << uint32_t(plr->getClass());
        data << uint32_t(plr->getRace());
        data << plr->getGender();
        data << uint32_t(plr->GetZoneId());
        ++sent_count;
    }
    objmgr._playerslock.ReleaseReadLock();
    data.wpos(0);
    data << sent_count;
    data << sent_count;

    SendPacket(&data);
}

void WorldSession::handleSetSelectionOpcode(WorldPacket& recvPacket)
{
    CmsgSetSelection recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    _player->SetSelection(recv_packet.guid);

    if (_player->m_comboPoints)
        _player->UpdateComboPoints();

    _player->setTargetGuid(recv_packet.guid);
    if (recv_packet.guid == 0)
    {
        if (_player->IsInWorld())
            _player->CombatStatusHandler_ResetPvPTimeout();
    }
}

void WorldSession::handleTogglePVPOpcode(WorldPacket& /*recvPacket*/)
{
    _player->PvPToggle();
}

void WorldSession::handleTutorialFlag(WorldPacket& recvPacket)
{
    CmsgTutorialFlag recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const uint32_t tutorial_index = (recv_packet.flag / 32);
    const uint32_t tutorial_status = (recv_packet.flag % 32);

    if (tutorial_index >= 7)
    {
        Disconnect();
        return;
    }

    uint32_t tutorial_flag = GetPlayer()->GetTutorialInt(tutorial_index);
    tutorial_flag |= (1 << tutorial_status);
    GetPlayer()->SetTutorialInt(tutorial_index, tutorial_flag);

    LOG_DEBUG("Received Tutorial flag: (%u).", recv_packet.flag);
}

void WorldSession::handleTutorialClear(WorldPacket& /*recvPacket*/)
{
    for (uint32_t id = 0; id < 8; ++id)
        GetPlayer()->SetTutorialInt(id, 0xFFFFFFFF);
}

void WorldSession::handleTutorialReset(WorldPacket& /*recvPacket*/)
{
    for (uint32_t id = 0; id < 8; ++id)
        GetPlayer()->SetTutorialInt(id, 0x00000000);
}

void WorldSession::handleLogoutRequestOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    auto player = GetPlayer();
    if (player)
    {
        if (!sHookInterface.OnLogoutRequest(player))
        {
            SendPacket(SmsgLogoutResponse(true).serialise().get());
            return;
        }

        if (GetPermissionCount() == 0)
        {
            if (player->CombatStatus.IsInCombat() || player->DuelingWith != nullptr)
            {
                SendPacket(SmsgLogoutResponse(true).serialise().get());
                return;
            }

            if (player->m_isResting || player->isOnTaxi() || worldConfig.player.enableInstantLogoutForAccessType == 2)
            {
                SetLogoutTimer(1);
                return;
            }
        }

        if (GetPermissionCount() > 0)
        {
            if (player->m_isResting || player->isOnTaxi() || worldConfig.player.enableInstantLogoutForAccessType > 0)
            {
                SetLogoutTimer(1);
                return;
            }
        }

        SendPacket(SmsgLogoutResponse(false).serialise().get());

        player->setMoveRoot(true);
        LoggingOut = true;

        player->addUnitFlags(UNIT_FLAG_LOCK_PLAYER);

        player->setStandState(STANDSTATE_SIT);
        SetLogoutTimer(20000);
    }
}

void WorldSession::handleSetSheathedOpcode(WorldPacket& recvPacket)
{
    CmsgSetSheathed recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    GetPlayer()->setSheathType(static_cast<uint8_t>(recv_packet.type));
}

void WorldSession::handlePlayedTimeOpcode(WorldPacket& recvPacket)
{
    CmsgPlayedTime recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_PLAYED_TIME: displayinui: %u", recv_packet.displayInUi);

    const uint32_t playedTime = static_cast<uint32_t>(UNIXTIME) - _player->m_playedtime[2];
    if (playedTime > 0)
    {
        _player->m_playedtime[0] += playedTime;
        _player->m_playedtime[1] += playedTime;
        _player->m_playedtime[2] += playedTime;
    }

    SendPacket(SmsgPlayedTime(_player->m_playedtime[1], _player->m_playedtime[0], recv_packet.displayInUi).serialise().get());

    LOG_DEBUG("Sent SMSG_PLAYED_TIME total: %u level: %u", _player->m_playedtime[1], _player->m_playedtime[0]);
}

void WorldSession::handleSetActionButtonOpcode(WorldPacket& recvPacket)
{
    CmsgSetActionButton recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("BUTTON: %u ACTION: %u TYPE: %u MISC: %u", recv_packet.button, recv_packet.action, recv_packet.type, recv_packet.misc);

    if (recv_packet.action == 0)
    {
        LOG_DEBUG("MISC: Remove action from button %u", recv_packet.button);
        GetPlayer()->setAction(recv_packet.button, 0, 0, 0);
    }
    else
    {
#if VERSION_STRING > TBC
        if (recv_packet.button >= PLAYER_ACTION_BUTTON_COUNT)
            return;
#else
        if (recv_packet.button >= 120)
            return;
#endif

        if (recv_packet.type == 64 || recv_packet.type == 65)
        {
            LOG_DEBUG("MISC: Added Macro %u into button %u", recv_packet.action, recv_packet.button);
            GetPlayer()->setAction(recv_packet.button, recv_packet.action, recv_packet.type, recv_packet.misc);
        }
        else if (recv_packet.type == 128)
        {
            LOG_DEBUG("MISC: Added Item %u into button %u", recv_packet.action, recv_packet.button);
            GetPlayer()->setAction(recv_packet.button, recv_packet.action, recv_packet.type, recv_packet.misc);
        }
        else if (recv_packet.type == 0)
        {
            LOG_DEBUG("MISC: Added Spell %u into button %u", recv_packet.action, recv_packet.button);
            GetPlayer()->setAction(recv_packet.button, recv_packet.action, recv_packet.type, recv_packet.misc);
        }
    }
}

void WorldSession::handleSetWatchedFactionIndexOpcode(WorldPacket& recvPacket)
{
    CmsgSetWatchedFaction recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    GetPlayer()->setWatchedFaction(recv_packet.factionId);
}

void WorldSession::handleRandomRollOpcode(WorldPacket& recvPacket)
{
    MsgRandomRoll recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("RandomRoll: Received MSG_RANDOM_ROLL: %u (min), %u (max)", recv_packet.min, recv_packet.max);

    uint32_t maxValue = recv_packet.max;
    uint32_t minValue = recv_packet.min;

    if (maxValue > RAND_MAX)
        maxValue = RAND_MAX;

    if (minValue > maxValue)
        minValue = maxValue;

    uint32_t randomRoll = Util::getRandomUInt(maxValue - minValue) + minValue;

    if (GetPlayer()->InGroup())
        GetPlayer()->GetGroup()->SendPacketToAll(MsgRandomRoll(minValue, maxValue, randomRoll, GetPlayer()->getGuid()).serialise().get());
    else
        SendPacket(MsgRandomRoll(minValue, maxValue, randomRoll, GetPlayer()->getGuid()).serialise().get());
}

void WorldSession::handleRealmSplitOpcode(WorldPacket& recvPacket)
{
    CmsgRealmSplit recv_paket;
    if (!recv_paket.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_REALM_SPLIT: %u (unk)", recv_paket.unknown);

    const std::string dateFormat = "01/01/01";

    SendPacket(SmsgRealmSplit(recv_paket.unknown, 0, dateFormat).serialise().get());
}

void WorldSession::handleSetTaxiBenchmarkOpcode(WorldPacket& recvPacket)
{
    CmsgSetTaxiBenchmarkMode recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_SET_TAXI_BENCHMARK_MODE: %d (mode)", recv_packet.mode);
}

void WorldSession::handleWorldStateUITimerUpdate(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING > TBC
    SendPacket(SmsgWorldStateUiTimerUpdate(static_cast<uint32_t>(UNIXTIME)).serialise().get());
#endif
}

void WorldSession::handleGameobjReportUseOpCode(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgGameobjReportUse recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_GAMEOBJ_REPORT_USE: %u (guid.low)", recv_packet.guid.getGuidLow());

    const auto gameobject = GetPlayer()->GetMapMgr()->GetGameObject(recv_packet.guid.getGuidLow());
    if (gameobject == nullptr)
        return;

    sQuestMgr.OnGameObjectActivate(GetPlayer(), gameobject);
    GetPlayer()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT, gameobject->getEntry(), 0, 0);

#endif
}

void WorldSession::handleDungeonDifficultyOpcode(WorldPacket& recvPacket)
{
    MsgSetDungeonDifficulty recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received MSG_SET_DUNGEON_DIFFICULTY: %d (difficulty)", recv_packet.difficulty);

    GetPlayer()->SetDungeonDifficulty(recv_packet.difficulty);
    sInstanceMgr.ResetSavedInstances(GetPlayer());

    const auto group = GetPlayer()->GetGroup();
    if (group && GetPlayer()->IsGroupLeader())
        group->SetDungeonDifficulty(recv_packet.difficulty);
}

void WorldSession::handleRaidDifficultyOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    MsgSetRaidDifficulty recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received MSG_SET_RAID_DIFFICULTY: %d (difficulty)", recv_packet.difficulty);

    GetPlayer()->SetRaidDifficulty(recv_packet.difficulty);
    sInstanceMgr.ResetSavedInstances(GetPlayer());

    const auto group = GetPlayer()->GetGroup();
    if (group && GetPlayer()->IsGroupLeader())
        group->SetRaidDifficulty(recv_packet.difficulty);
#endif
}

void WorldSession::handleSetAutoLootPassOpcode(WorldPacket& recvPacket)
{
    CmsgOptOutOfLoot recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_OPT_OUT_OF_LOOT: %u (turnedOn)", recv_packet.turnedOn);

    GetPlayer()->m_passOnLoot = recv_packet.turnedOn > 0 ? true : false;
}

void WorldSession::handleSetActionBarTogglesOpcode(WorldPacket& recvPacket)
{
    CmsgSetActionbarToggles recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_SET_ACTIONBAR_TOGGLES: %d (actionbarId)", recv_packet.actionbarId);

    GetPlayer()->setByteValue(PLAYER_FIELD_BYTES, 2, recv_packet.actionbarId);
}

void WorldSession::handleLootRollOpcode(WorldPacket& recvPacket)
{
    CmsgLootRoll recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_LOOT_ROLL: %u (objectGuid) %u (slot) %d (choice)", recv_packet.objectGuid.getGuidLow(), recv_packet.slot, recv_packet.choice);

    LootRoll* lootRoll = nullptr;

    const uint32_t guidType = GET_TYPE_FROM_GUID(recv_packet.objectGuid.GetOldGuid());
    switch (guidType)
    {
        case HIGHGUID_TYPE_GAMEOBJECT:
        {
            auto gameObject = GetPlayer()->GetMapMgr()->GetGameObject(recv_packet.objectGuid.getGuidLow());
            if (gameObject == nullptr)
                return;

            if (!gameObject->IsLootable())
                return;

            auto gameObjectLootable = static_cast<GameObject_Lootable*>(gameObject);
            if (recv_packet.slot >= gameObjectLootable->loot.items.size() || gameObjectLootable->loot.items.empty())
                return;

            if (gameObject->getGoType() == GAMEOBJECT_TYPE_CHEST)
                lootRoll = gameObjectLootable->loot.items[recv_packet.slot].roll;
        } break;
        case HIGHGUID_TYPE_UNIT:
        {
            auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.objectGuid.getGuidLow());
            if (creature == nullptr)
                return;

            if (recv_packet.slot >= creature->loot.items.size() || creature->loot.items.empty())
                return;

            lootRoll = creature->loot.items[recv_packet.slot].roll;
        } break;
        default:
            return;
    }

    if (lootRoll == nullptr)
        return;

    lootRoll->PlayerRolled(GetPlayer(), recv_packet.choice);
}

void WorldSession::handleOpenItemOpcode(WorldPacket& recvPacket)
{
    CmsgOpenItem recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_OPEN_ITEM: %u (containerSlot), %u (slot)", recv_packet.containerSlot, recv_packet.slot);

    auto item = GetPlayer()->GetItemInterface()->GetInventoryItem(recv_packet.containerSlot, recv_packet.slot);
    if (item == nullptr)
        return;

    if (item->getGiftCreatorGuid() && item->wrapped_item_id)
    {
        const auto wrappedItem = sMySQLStore.getItemProperties(item->wrapped_item_id);
        if (wrappedItem == nullptr)
            return;

        item->setGiftCreatorGuid(0);
        item->setEntry(item->wrapped_item_id);
        item->wrapped_item_id = 0;
        item->setItemProperties(wrappedItem);

        if (wrappedItem->Bonding == ITEM_BIND_ON_PICKUP)
            item->addFlags(ITEM_FLAG_SOULBOUND);
        else
            item->setFlags(ITEM_FLAGS_NONE);

        if (wrappedItem->MaxDurability)
        {
            item->setDurability(wrappedItem->MaxDurability);
            item->setMaxDurability(wrappedItem->MaxDurability);
        }

        item->m_isDirty = true;
        item->SaveToDB(recv_packet.containerSlot, recv_packet.slot, false, nullptr);
        return;
    }

    uint32_t removeLockItems[LOCK_NUM_CASES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    const auto lockEntry = sLockStore.LookupEntry(item->getItemProperties()->LockId);
    if (lockEntry)
    {
        for (uint8_t lockCase = 0; lockCase < LOCK_NUM_CASES; ++lockCase)
        {
            if (lockEntry->locktype[lockCase] == 1 && lockEntry->lockmisc[lockCase] > 0)
            {
                const int16_t slot2 = GetPlayer()->GetItemInterface()->GetInventorySlotById(lockEntry->lockmisc[lockCase]);
                if (slot2 != ITEM_NO_SLOT_AVAILABLE && slot2 >= INVENTORY_SLOT_ITEM_START && slot2 < INVENTORY_SLOT_ITEM_END)
                {
                    removeLockItems[lockCase] = lockEntry->lockmisc[lockCase];
                }
                else
                {
                    GetPlayer()->GetItemInterface()->BuildInventoryChangeError(item, nullptr, INV_ERR_ITEM_LOCKED);
                    return;
                }
            }
            else if (lockEntry->locktype[lockCase] == 2 && item->locked)
            {
                GetPlayer()->GetItemInterface()->BuildInventoryChangeError(item, nullptr, INV_ERR_ITEM_LOCKED);
                return;
            }
        }

        for (uint8_t lockCase = 0; lockCase < LOCK_NUM_CASES; ++lockCase)
        {
            if (removeLockItems[lockCase])
                GetPlayer()->GetItemInterface()->RemoveItemAmt(removeLockItems[lockCase], 1);
        }
    }

    GetPlayer()->SetLootGUID(item->getGuid());
    if (item->loot == nullptr)
    {
        item->loot = new Loot; //eeeeeek
        lootmgr.FillItemLoot(item->loot, item->getEntry());
    }
    GetPlayer()->SendLoot(item->getGuid(), LOOT_DISENCHANTING, GetPlayer()->GetMapId());
}

void WorldSession::handleDismountOpcode(WorldPacket& /*recvPacket*/)
{
    if (GetPlayer()->isOnTaxi())
        return;

    GetPlayer()->Dismount();
}

void WorldSession::handleToggleHelmOpcode(WorldPacket& /*recvPacket*/)
{
    if (GetPlayer()->hasPlayerFlags(PLAYER_FLAG_NOHELM))
        GetPlayer()->removePlayerFlags(PLAYER_FLAG_NOHELM);
    else
        GetPlayer()->addPlayerFlags(PLAYER_FLAG_NOHELM);
}

void WorldSession::handleToggleCloakOpcode(WorldPacket& /*recvPacket*/)
{
    if (GetPlayer()->hasPlayerFlags(PLAYER_FLAG_NOCLOAK))
        GetPlayer()->removePlayerFlags(PLAYER_FLAG_NOCLOAK);
    else
        GetPlayer()->addPlayerFlags(PLAYER_FLAG_NOCLOAK);
}

void WorldSession::handleResetInstanceOpcode(WorldPacket& /*recvPacket*/)
{
    sInstanceMgr.ResetSavedInstances(GetPlayer());
}

void WorldSession::handleSetTitle(WorldPacket& recvPacket)
{
    CmsgSetTitle recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

#if VERSION_STRING > Classic
    if (recv_packet.titleId == 0xFFFFFFFF)
    {
        GetPlayer()->setChosenTitle(0);
        return;
    }

    if (GetPlayer()->HasTitle(static_cast<RankTitles>(recv_packet.titleId)))
        GetPlayer()->setChosenTitle(recv_packet.titleId);
#endif
}
