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
#include "Server/Packets/SmsgStandstateUpdate.h"
#include "Server/Packets/CmsgZoneupdate.h"
#include "Server/Packets/CmsgResurrectResponse.h"
#include "Server/Packets/CmsgBug.h"
#include "Server/Packets/CmsgUpdateAccountData.h"
#include "Server/Packets/SmsgUpdateAccountDataComplete.h"
#include "Server/Packets/CmsgSummonResponse.h"
#include "Server/Packets/SmsgResurrectFailed.h"
#include "Server/Packets/CmsgReclaimCorpse.h"

using namespace AscEmu::Packets;

void WorldSession::handleStandStateChangeOpcode(WorldPacket& recvPacket)
{
    CmsgStandStateChange srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->setStandState(srlPacket.state);
}

void WorldSession::handleWhoOpcode(WorldPacket& recvPacket)
{
    CmsgWho srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    bool cname = false;
    bool gname = false;

    if (srlPacket.player_name.length() > 0)
        cname = true;

    if (srlPacket.guild_name.length() > 0)
        gname = true;

    LogDebugFlag(LF_OPCODE, "Received CMSG_WHO with %u zones and %u names", srlPacket.zone_count, srlPacket.name_count);

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
        if (cname && srlPacket.player_name.compare(plr->getName()) != 0)
            continue;

        // Guild name
        if (gname)
        {
            if (!plr->GetGuild() || srlPacket.guild_name.compare(plr->GetGuild()->getName()) != 0)
                continue;
        }

        // Level check
        if (srlPacket.min_level && srlPacket.max_level)
        {
            // skip players outside of level range
            if (plr->getLevel() < srlPacket.min_level || plr->getLevel() > srlPacket.max_level)
                continue;
        }

        // Zone id compare
        if (srlPacket.zone_count)
        {
            // people that fail the zone check don't get added
            add = false;
            for (uint32_t i = 0; i < srlPacket.zone_count; ++i)
            {
                if (srlPacket.zones[i] == plr->GetZoneId())
                {
                    add = true;
                    break;
                }
            }
        }

        if (!((srlPacket.class_mask >> 1) & plr->getClassMask()) || !((srlPacket.race_mask >> 1) & plr->getRaceMask()))
            add = false;

        // skip players that fail zone check
        if (!add)
            continue;

        if (srlPacket.name_count)
        {
            // people that fail name check don't get added
            add = false;
            for (uint32_t i = 0; i < srlPacket.name_count; ++i)
            {
                if (!strnicmp(srlPacket.names[i].c_str(), plr->getName().c_str(), srlPacket.names[i].length()))
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

        if (plr->m_playerInfo->m_guild)
            data << sGuildMgr.getGuildById(plr->m_playerInfo->m_guild)->getName().c_str();
        else
            data << uint8_t(0);

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
    CmsgSetSelection srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->SetSelection(srlPacket.guid);

    if (_player->m_comboPoints)
        _player->UpdateComboPoints();

    _player->setTargetGuid(srlPacket.guid);
    if (srlPacket.guid == 0)
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
    CmsgTutorialFlag srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const uint32_t tutorial_index = (srlPacket.flag / 32);
    const uint32_t tutorial_status = (srlPacket.flag % 32);

    if (tutorial_index >= 7)
    {
        Disconnect();
        return;
    }

    uint32_t tutorial_flag = _player->GetTutorialInt(tutorial_index);
    tutorial_flag |= (1 << tutorial_status);
    _player->SetTutorialInt(tutorial_index, tutorial_flag);

    LogDebugFlag(LF_OPCODE, "Received Tutorial flag: (%u).", srlPacket.flag);
}

void WorldSession::handleTutorialClear(WorldPacket& /*recvPacket*/)
{
    for (uint32_t id = 0; id < 8; ++id)
        _player->SetTutorialInt(id, 0xFFFFFFFF);
}

void WorldSession::handleTutorialReset(WorldPacket& /*recvPacket*/)
{
    for (uint32_t id = 0; id < 8; ++id)
        _player->SetTutorialInt(id, 0x00000000);
}

void WorldSession::handleLogoutRequestOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    auto player = _player;
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

        player->SendPacket(SmsgStandstateUpdate(STANDSTATE_SIT).serialise().get());

        SetLogoutTimer(PLAYER_LOGOUT_DELAY);
    }
}

void WorldSession::handleSetSheathedOpcode(WorldPacket& recvPacket)
{
    CmsgSetSheathed srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->setSheathType(static_cast<uint8_t>(srlPacket.type));
}

void WorldSession::handlePlayedTimeOpcode(WorldPacket& recvPacket)
{
    CmsgPlayedTime srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_PLAYED_TIME: displayinui: %u", srlPacket.displayInUi);

    const uint32_t playedTime = static_cast<uint32_t>(UNIXTIME) - _player->m_playedtime[2];
    if (playedTime > 0)
    {
        _player->m_playedtime[0] += playedTime;
        _player->m_playedtime[1] += playedTime;
        _player->m_playedtime[2] += playedTime;
    }

    SendPacket(SmsgPlayedTime(_player->m_playedtime[1], _player->m_playedtime[0], srlPacket.displayInUi).serialise().get());

    LogDebugFlag(LF_OPCODE, "Sent SMSG_PLAYED_TIME total: %u level: %u", _player->m_playedtime[1], _player->m_playedtime[0]);
}

void WorldSession::handleSetActionButtonOpcode(WorldPacket& recvPacket)
{
    CmsgSetActionButton srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "BUTTON: %u ACTION: %u TYPE: %u MISC: %u", srlPacket.button, srlPacket.action, srlPacket.type, srlPacket.misc);

    if (srlPacket.action == 0)
    {
        LogDebugFlag(LF_OPCODE, "MISC: Remove action from button %u", srlPacket.button);
        _player->setAction(srlPacket.button, 0, 0, 0);
    }
    else
    {
#if VERSION_STRING > TBC
        if (srlPacket.button >= PLAYER_ACTION_BUTTON_COUNT)
            return;
#else
        if (srlPacket.button >= 120)
            return;
#endif

        if (srlPacket.type == 64 || srlPacket.type == 65)
        {
            LogDebugFlag(LF_OPCODE, "MISC: Added Macro %u into button %u", srlPacket.action, srlPacket.button);
            _player->setAction(srlPacket.button, srlPacket.action, srlPacket.type, srlPacket.misc);
        }
        else if (srlPacket.type == 128)
        {
            LogDebugFlag(LF_OPCODE, "MISC: Added Item %u into button %u", srlPacket.action, srlPacket.button);
            _player->setAction(srlPacket.button, srlPacket.action, srlPacket.type, srlPacket.misc);
        }
        else if (srlPacket.type == 0)
        {
            LogDebugFlag(LF_OPCODE, "MISC: Added Spell %u into button %u", srlPacket.action, srlPacket.button);
            _player->setAction(srlPacket.button, srlPacket.action, srlPacket.type, srlPacket.misc);
        }
    }
}

void WorldSession::handleSetWatchedFactionIndexOpcode(WorldPacket& recvPacket)
{
    CmsgSetWatchedFaction srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->setWatchedFaction(srlPacket.factionId);
}

void WorldSession::handleRandomRollOpcode(WorldPacket& recvPacket)
{
    MsgRandomRoll srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_RANDOM_ROLL: %u (min), %u (max)", srlPacket.min, srlPacket.max);

    uint32_t maxValue = srlPacket.max;
    uint32_t minValue = srlPacket.min;

    if (maxValue > RAND_MAX)
        maxValue = RAND_MAX;

    if (minValue > maxValue)
        minValue = maxValue;

    uint32_t randomRoll = Util::getRandomUInt(maxValue - minValue) + minValue;

    if (_player->InGroup())
        _player->GetGroup()->SendPacketToAll(MsgRandomRoll(minValue, maxValue, randomRoll, _player->getGuid()).serialise().get());
    else
        SendPacket(MsgRandomRoll(minValue, maxValue, randomRoll, _player->getGuid()).serialise().get());
}

void WorldSession::handleRealmSplitOpcode(WorldPacket& recvPacket)
{
    CmsgRealmSplit srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_REALM_SPLIT: %u (unk)", srlPacket.unknown);

    const std::string dateFormat = "01/01/01";

    SendPacket(SmsgRealmSplit(srlPacket.unknown, 0, dateFormat).serialise().get());
}

void WorldSession::handleSetTaxiBenchmarkOpcode(WorldPacket& recvPacket)
{
    CmsgSetTaxiBenchmarkMode srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_SET_TAXI_BENCHMARK_MODE: %d (mode)", srlPacket.mode);
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
    CmsgGameobjReportUse srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_GAMEOBJ_REPORT_USE: %u (guid.low)", srlPacket.guid.getGuidLow());

    const auto gameobject = _player->GetMapMgr()->GetGameObject(srlPacket.guid.getGuidLow());
    if (gameobject == nullptr)
        return;

    sQuestMgr.OnGameObjectActivate(_player, gameobject);
    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT, gameobject->getEntry(), 0, 0);

#endif
}

void WorldSession::handleDungeonDifficultyOpcode(WorldPacket& recvPacket)
{
    MsgSetDungeonDifficulty srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_SET_DUNGEON_DIFFICULTY: %d (difficulty)", srlPacket.difficulty);

    _player->SetDungeonDifficulty(srlPacket.difficulty);
    sInstanceMgr.ResetSavedInstances(_player);

    const auto group = _player->GetGroup();
    if (group && _player->IsGroupLeader())
        group->SetDungeonDifficulty(srlPacket.difficulty);
}

void WorldSession::handleRaidDifficultyOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    MsgSetRaidDifficulty srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_SET_RAID_DIFFICULTY: %d (difficulty)", srlPacket.difficulty);

    _player->SetRaidDifficulty(srlPacket.difficulty);
    sInstanceMgr.ResetSavedInstances(_player);

    const auto group = _player->GetGroup();
    if (group && _player->IsGroupLeader())
        group->SetRaidDifficulty(srlPacket.difficulty);
#endif
}

void WorldSession::handleSetAutoLootPassOpcode(WorldPacket& recvPacket)
{
    CmsgOptOutOfLoot srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_OPT_OUT_OF_LOOT: %u (turnedOn)", srlPacket.turnedOn);

    _player->m_passOnLoot = srlPacket.turnedOn > 0 ? true : false;
}

void WorldSession::handleSetActionBarTogglesOpcode(WorldPacket& recvPacket)
{
    CmsgSetActionbarToggles srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_SET_ACTIONBAR_TOGGLES: %d (actionbarId)", srlPacket.actionbarId);

    _player->setByteValue(PLAYER_FIELD_BYTES, 2, srlPacket.actionbarId);
}

void WorldSession::handleLootRollOpcode(WorldPacket& recvPacket)
{
    CmsgLootRoll srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_LOOT_ROLL: %u (objectGuid) %u (slot) %d (choice)", srlPacket.objectGuid.getGuidLow(), srlPacket.slot, srlPacket.choice);

    LootRoll* lootRoll = nullptr;

    const HighGuid guidType = srlPacket.objectGuid.getHigh();

    switch (guidType)
    {
        case HighGuid::GameObject:
        {
            auto gameObject = _player->GetMapMgr()->GetGameObject(srlPacket.objectGuid.getGuidLow());
            if (gameObject == nullptr)
                return;

            if (!gameObject->IsLootable())
                return;

            auto gameObjectLootable = static_cast<GameObject_Lootable*>(gameObject);
            if (srlPacket.slot >= gameObjectLootable->loot.items.size() || gameObjectLootable->loot.items.empty())
                return;

            if (gameObject->getGoType() == GAMEOBJECT_TYPE_CHEST)
                lootRoll = gameObjectLootable->loot.items[srlPacket.slot].roll;
        } break;
        case HighGuid::Unit:
        {
            auto creature = _player->GetMapMgr()->GetCreature(srlPacket.objectGuid.getGuidLow());
            if (creature == nullptr)
                return;

            if (srlPacket.slot >= creature->loot.items.size() || creature->loot.items.empty())
                return;

            lootRoll = creature->loot.items[srlPacket.slot].roll;
        } break;
        default:
            return;
    }

    if (lootRoll == nullptr)
        return;

    lootRoll->PlayerRolled(_player, srlPacket.choice);
}

void WorldSession::handleOpenItemOpcode(WorldPacket& recvPacket)
{
    CmsgOpenItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_OPEN_ITEM: %u (containerSlot), %u (slot)", srlPacket.containerSlot, srlPacket.slot);

    auto item = _player->GetItemInterface()->GetInventoryItem(srlPacket.containerSlot, srlPacket.slot);
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
        item->SaveToDB(srlPacket.containerSlot, srlPacket.slot, false, nullptr);
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
                const int16_t slot2 = _player->GetItemInterface()->GetInventorySlotById(lockEntry->lockmisc[lockCase]);
                if (slot2 != ITEM_NO_SLOT_AVAILABLE && slot2 >= INVENTORY_SLOT_ITEM_START && slot2 < INVENTORY_SLOT_ITEM_END)
                {
                    removeLockItems[lockCase] = lockEntry->lockmisc[lockCase];
                }
                else
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(item, nullptr, INV_ERR_ITEM_LOCKED);
                    return;
                }
            }
            else if (lockEntry->locktype[lockCase] == 2 && item->locked)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(item, nullptr, INV_ERR_ITEM_LOCKED);
                return;
            }
        }

        for (uint8_t lockCase = 0; lockCase < LOCK_NUM_CASES; ++lockCase)
        {
            if (removeLockItems[lockCase])
                _player->GetItemInterface()->RemoveItemAmt(removeLockItems[lockCase], 1);
        }
    }

    _player->SetLootGUID(item->getGuid());
    if (item->loot == nullptr)
    {
        item->loot = new Loot; //eeeeeek
        lootmgr.FillItemLoot(item->loot, item->getEntry());
    }
    _player->SendLoot(item->getGuid(), LOOT_DISENCHANTING, _player->GetMapId());
}

void WorldSession::handleDismountOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->isOnTaxi())
        return;

    _player->Dismount();
}

void WorldSession::handleToggleHelmOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->hasPlayerFlags(PLAYER_FLAG_NOHELM))
        _player->removePlayerFlags(PLAYER_FLAG_NOHELM);
    else
        _player->addPlayerFlags(PLAYER_FLAG_NOHELM);
}

void WorldSession::handleToggleCloakOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->hasPlayerFlags(PLAYER_FLAG_NOCLOAK))
        _player->removePlayerFlags(PLAYER_FLAG_NOCLOAK);
    else
        _player->addPlayerFlags(PLAYER_FLAG_NOCLOAK);
}

void WorldSession::handleResetInstanceOpcode(WorldPacket& /*recvPacket*/)
{
    sInstanceMgr.ResetSavedInstances(_player);
}

void WorldSession::handleSetTitle(WorldPacket& recvPacket)
{
    CmsgSetTitle srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

#if VERSION_STRING > Classic
    if (srlPacket.titleId == 0xFFFFFFFF)
    {
        _player->setChosenTitle(0);
        return;
    }

    if (_player->HasTitle(static_cast<RankTitles>(srlPacket.titleId)))
        _player->setChosenTitle(srlPacket.titleId);
#endif
}

void WorldSession::handleZoneupdate(WorldPacket& recvPacket)
{
    CmsgZoneupdate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (_player->GetZoneId() == srlPacket.zoneId)
        return;

    sWeatherMgr.SendWeather(_player);
    _player->ZoneUpdate(srlPacket.zoneId);
    _player->GetItemInterface()->EmptyBuyBack();
}

void WorldSession::handleResurrectResponse(WorldPacket& recvPacket)
{
    CmsgResurrectResponse srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!_player->isAlive())
        return;

    auto player = _player->GetMapMgr()->GetPlayer(srlPacket.guid.getGuidLow());
    if (player == nullptr)
        player = objmgr.GetPlayer(srlPacket.guid.getGuidLow());

    if (player == nullptr)
        return;

    if (srlPacket.status != 1 || _player->m_resurrecter || _player->m_resurrecter != srlPacket.guid.GetOldGuid())
    {
        _player->m_resurrectHealth = 0;
        _player->m_resurrectMana = 0;
        _player->m_resurrecter = 0;
        return;
    }

    _player->ResurrectPlayer();
    _player->setMoveRoot(false);
}

void WorldSession::handleSelfResurrect(WorldPacket& /*recvPacket*/)
{
    if (const auto resurrectSpell = _player->getSelfResurrectSpell())
    {
        const auto spellInfo = sSpellCustomizations.GetSpellInfo(resurrectSpell);
        if (const auto spell = sSpellFactoryMgr.NewSpell(_player, spellInfo, true, nullptr))
        {
            SpellCastTargets spellCastTargets;
            spellCastTargets.m_unitTarget = _player->getGuid();
            spell->prepare(&spellCastTargets);
        }
    }
}

void WorldSession::handleUpdateAccountData(WorldPacket& recvPacket)
{
    if (!worldConfig.server.useAccountData)
        return;

    CmsgUpdateAccountData srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.uiId > 8)
    {
        LOG_ERROR("WARNING: Accountdata > 8 (%u) was requested to be updated by %s of account %u!",
            srlPacket.uiId, _player->getName().c_str(), this->GetAccountId());
        return;
    }

    uLongf uid = srlPacket.uiDecompressedSize;

    if (srlPacket.uiDecompressedSize == 0)
    {
        SetAccountData(srlPacket.uiId, NULL, false, 0);
#if VERSION_STRING > TBC
        SendPacket(SmsgUpdateAccountDataComplete(srlPacket.uiId, 0).serialise().get());
#endif
        return;
    }

    size_t receivedPackedSize = recvPacket.size() - 8;
    auto data = new char[srlPacket.uiDecompressedSize + 1];
    memset(data, 0, srlPacket.uiDecompressedSize + 1);

    if (srlPacket.uiDecompressedSize > receivedPackedSize)
    {
        const int32_t ZlibResult = uncompress(reinterpret_cast<uint8_t*>(data), &uid, recvPacket.contents() + 8, 
            static_cast<uLong>(receivedPackedSize));

        switch (ZlibResult)
        {
            case Z_OK:                  //0 no error decompression is OK
            {
                SetAccountData(srlPacket.uiId, data, false, srlPacket.uiDecompressedSize);
                LogDebugFlag(LF_OPCODE, "Successfully decompressed account data %d for %s, and updated storage array.",
                    srlPacket.uiId, _player->getName().c_str());
            } break;
            case Z_ERRNO:               //-1
            case Z_STREAM_ERROR:        //-2
            case Z_DATA_ERROR:          //-3
            case Z_MEM_ERROR:           //-4
            case Z_BUF_ERROR:           //-5
            case Z_VERSION_ERROR:       //-6
            {
                delete[] data;
                LOG_ERROR("Decompression of account data %u for %s FAILED.", srlPacket.uiId, _player->getName().c_str());
            } break;

            default:
            {
                delete[] data;
                LOG_ERROR("Decompression gave a unknown error: %x, of account data %u for %s FAILED.",
                    ZlibResult, srlPacket.uiId, _player->getName().c_str());
            } break;
        }
    }
    else
    {
        memcpy(data, recvPacket.contents() + 8, srlPacket.uiDecompressedSize);
        SetAccountData(srlPacket.uiId, data, false, srlPacket.uiDecompressedSize);
    }

#if VERSION_STRING > TBC
    SendPacket(SmsgUpdateAccountDataComplete(srlPacket.uiId, 0).serialise().get());
#endif
}

#if VERSION_STRING != Cata
void WorldSession::handleBugOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CmsgBug srlPacket;
    if (!srlPacket.deserialise(recv_data))
        return;

    if (srlPacket.suggestion == 0)
        LogDebugFlag(LF_OPCODE, "Received CMSG_BUG [Bug Report]");
    else
        LogDebugFlag(LF_OPCODE, "Received CMSG_BUG [Suggestion]");

    uint64_t accountId = GetAccountId();
    uint32_t timeStamp = uint32(UNIXTIME);
    uint32_t reportId = objmgr.GenerateReportID();

    std::stringstream ss;

    ss << "INSERT INTO playerbugreports VALUES('";
    ss << reportId << "','";
    ss << accountId << "','";
    ss << timeStamp << "','";
    ss << srlPacket.suggestion << "','";
    ss << CharacterDatabase.EscapeString(srlPacket.type) << "','";
    ss << CharacterDatabase.EscapeString(srlPacket.content) << "')";

    CharacterDatabase.ExecuteNA(ss.str().c_str());
}
#else
void WorldSession::handleBugOpcode(WorldPacket& recv_data)
{
    uint8_t unk1;
    uint8_t unk2;

    recv_data >> unk1;
    recv_data >> unk2;

    uint32_t lenght = 0;
    lenght = unk1 * 16;
    lenght += unk2 / 16;

    std::string bugMessage;
    bugMessage = recv_data.ReadString(lenght);

    LogDebugFlag(LF_OPCODE, "Received CMSG_BUG [Bug Report] lenght: %u message: %s", lenght, bugMessage.c_str());

    uint64_t accountId = GetAccountId();
    uint32_t timeStamp = uint32_t(UNIXTIME);
    uint32_t reportId = objmgr.GenerateReportID();

    std::stringstream ss;

    ss << "INSERT INTO playerbugreports VALUES('";
    ss << reportId << "','";
    ss << accountId << "','";
    ss << timeStamp << "',";
    ss << "'0',";
    ss << "'0','";
    ss << CharacterDatabase.EscapeString(bugMessage) << "')";

    CharacterDatabase.ExecuteNA(ss.str().c_str());
}
#endif

#if VERSION_STRING == Cata
void WorldSession::handleSuggestionOpcode(WorldPacket& recvPacket)
{
    uint8_t unk1;
    uint8_t unk2;

    recvPacket >> unk1;
    recvPacket >> unk2;

    uint32_t lenght = 0;
    lenght = unk1 * 16;
    lenght += unk2 / 16;

    std::string suggestionMessage;
    suggestionMessage = recvPacket.ReadString(lenght);

    LogDebugFlag(LF_OPCODE, "Received CMSG_SUGGESTIONS [Suggestion] lenght: %u message: %s", lenght, suggestionMessage.c_str());

    uint64_t accountId = GetAccountId();
    uint32_t timeStamp = uint32_t(UNIXTIME);
    uint32_t reportId = objmgr.GenerateReportID();

    std::stringstream ss;

    ss << "INSERT INTO playerbugreports VALUES('";
    ss << reportId << "','";
    ss << accountId << "','";
    ss << timeStamp << "',";
    ss << "'1',";
    ss << "'1','";
    ss << CharacterDatabase.EscapeString(suggestionMessage) << "')";

    CharacterDatabase.ExecuteNA(ss.str().c_str());
}
#endif

#if VERSION_STRING == Cata
void WorldSession::handleReturnToGraveyardOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->isAlive())
        return;

    _player->RepopAtGraveyard(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId());
}
#endif

#if VERSION_STRING == Cata
void WorldSession::handleLogDisconnectOpcode(WorldPacket& recvPacket)
{
    uint32_t disconnectReason;
    recvPacket >> disconnectReason;     // 13 - closed window

    LogDebugFlag(LF_OPCODE, "Player %s disconnected on %s - Reason %u", _player->getName().c_str(),
        Util::GetCurrentDateTimeString().c_str(), disconnectReason);
}
#endif

void WorldSession::handleCompleteCinematic(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "Received CMSG_COMPLETE_CINEMATIC");

    _player->setStandState(STANDSTATE_STAND);
}

void WorldSession::handleNextCinematic(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN
    LogDebugFlag(LF_OPCODE, "Received CMSG_NEXT_CINEMATIC_CAMERA");

    _player->SetPosition(float(_player->GetPositionX() + 0.01), float(_player->GetPositionY() + 0.01),
        float(_player->GetPositionZ() + 0.01), _player->GetOrientation());
}

void WorldSession::handleReadyForAccountDataTimes(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "Received CMSG_READY_FOR_ACCOUNT_DATA_TIMES");

    SendAccountDataTimes(GLOBAL_CACHE_MASK);
}

void WorldSession::handleSummonResponseOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgSummonResponse srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!srlPacket.isClickOn)
        return;

    if (!_player->m_summoner)
    {
        SendNotification("You do not have permission to perform that function.");
        return;
    }

    if (_player->CombatStatus.IsInCombat())
        return;

    _player->SafeTeleport(_player->m_summonMapId, _player->m_summonInstanceId, _player->m_summonPos);

    _player->m_summoner = _player->m_summonInstanceId = _player->m_summonMapId = 0;
}

void WorldSession::handleLogoutCancelOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    LogDebugFlag(LF_OPCODE, "Received CMSG_LOGOUT_CANCEL");

    if (!LoggingOut)
        return;

    LoggingOut = false;

    SetLogoutTimer(0);

    OutPacket(SMSG_LOGOUT_CANCEL_ACK);

    _player->setMoveRoot(false);

    _player->setStandState(STANDSTATE_STAND);
    _player->SendPacket(SmsgStandstateUpdate(STANDSTATE_STAND).serialise().get());

    _player->removeUnitFlags(UNIT_FLAG_LOCK_PLAYER);

    LogDebugFlag(LF_OPCODE, "Sent SMSG_LOGOUT_CANCEL_ACK");
}

void WorldSession::handlePlayerLogoutOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    LogDebugFlag(LF_OPCODE, "Received CMSG_PLAYER_LOGOUT");
    if (!HasGMPermissions())
        SendNotification("You do not have permission to perform that function.");
    else
        LogoutPlayer(true);
}

void WorldSession::handleCorpseReclaimOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgReclaimCorpse srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_RECLAIM_CORPSE");

    if (srlPacket.guid.GetOldGuid() == 0)
        return;

    auto corpse = objmgr.GetCorpse(srlPacket.guid.getGuidLow());
    if (corpse == nullptr)
        return;

    WoWGuid wowGuid;
    wowGuid.Init(corpse->getOwnerGuid());

    if (wowGuid.getGuidLowPart() != _player->getGuidLow() && corpse->getFlags() == 5)
    {
        SendPacket(SmsgResurrectFailed(1).serialise().get());
        return;
    }

    if (corpse->GetDistance2dSq(_player) > CORPSE_MINIMUM_RECLAIM_RADIUS_SQ)
    {
        SendPacket(SmsgResurrectFailed(1).serialise().get());
        return;
    }

    if (time(nullptr) < corpse->GetDeathClock() + CORPSE_RECLAIM_TIME)
    {
        SendPacket(SmsgResurrectFailed(1).serialise().get());
        return;
    }

    _player->ResurrectPlayer();
    _player->setHealth(GetPlayer()->getMaxHealth() / 2);
}

#if VERSION_STRING == Cata
void WorldSession::handleLoadScreenOpcode(WorldPacket& recvPacket)
{
    uint32_t mapId;

    recvPacket >> mapId;
    recvPacket.readBit();
}

void WorldSession::handleReadyForAccountDataTimesOpcode(WorldPacket& /*recvPacket*/)
{
    SendAccountDataTimes(GLOBAL_CACHE_MASK);
}

void WorldSession::handleUITimeRequestOpcode(WorldPacket& /*recvPacket*/)
{
    WorldPacket data(SMSG_UI_TIME, 4);
    data << uint32_t(time(nullptr));
    SendPacket(&data);
}

void WorldSession::handleTimeSyncRespOpcode(WorldPacket& recvPacket)
{
    uint32_t counter;
    uint32_t clientTicks;
    recvPacket >> counter;
    recvPacket >> clientTicks;
}

void WorldSession::handleObjectUpdateFailedOpcode(WorldPacket& recvPacket)
{
    ObjectGuid guid;

    guid[6] = recvPacket.readBit();
    guid[7] = recvPacket.readBit();
    guid[4] = recvPacket.readBit();
    guid[0] = recvPacket.readBit();
    guid[1] = recvPacket.readBit();
    guid[5] = recvPacket.readBit();
    guid[3] = recvPacket.readBit();
    guid[2] = recvPacket.readBit();

    recvPacket.ReadByteSeq(guid[6]);
    recvPacket.ReadByteSeq(guid[7]);
    recvPacket.ReadByteSeq(guid[2]);
    recvPacket.ReadByteSeq(guid[3]);
    recvPacket.ReadByteSeq(guid[1]);
    recvPacket.ReadByteSeq(guid[4]);
    recvPacket.ReadByteSeq(guid[0]);
    recvPacket.ReadByteSeq(guid[5]);

    LogError("handleObjectUpdateFailedOpcode : Object update failed for playerguid %u", Arcemu::Util::GUID_LOPART(guid));

    if (_player == nullptr)
        return;

    if (_player->getGuid() == guid)
    {
        LogoutPlayer(true);
        return;
    }

    //_player->UpdateVisibility();
}

void WorldSession::handleRequestHotfix(WorldPacket& recvPacket)
{
    uint32_t type;
    recvPacket >> type;

    uint32_t count = recvPacket.readBits(23);

    ObjectGuid* guids = new ObjectGuid[count];
    for (uint32_t i = 0; i < count; ++i)
    {
        guids[i][0] = recvPacket.readBit();
        guids[i][4] = recvPacket.readBit();
        guids[i][7] = recvPacket.readBit();
        guids[i][2] = recvPacket.readBit();
        guids[i][5] = recvPacket.readBit();
        guids[i][3] = recvPacket.readBit();
        guids[i][6] = recvPacket.readBit();
        guids[i][1] = recvPacket.readBit();
    }

    uint32_t entry;
    for (uint32_t i = 0; i < count; ++i)
    {
        recvPacket.ReadByteSeq(guids[i][5]);
        recvPacket.ReadByteSeq(guids[i][6]);
        recvPacket.ReadByteSeq(guids[i][7]);
        recvPacket.ReadByteSeq(guids[i][0]);
        recvPacket.ReadByteSeq(guids[i][1]);
        recvPacket.ReadByteSeq(guids[i][3]);
        recvPacket.ReadByteSeq(guids[i][4]);
        recvPacket >> entry;
        recvPacket.ReadByteSeq(guids[i][2]);

        /*switch (type)
        {
            case DB2_REPLY_ITEM:
                SendItemDb2Reply(entry);
                break;
            case DB2_REPLY_SPARSE:
                SendItemSparseDb2Reply(entry);
                break;
            default:
                LogDebugFlag(LF_OPCODE, "Received unknown hotfix type %u", type);
                recvPacket.clear();
                break;
        }*/
    }
}

void WorldSession::handleRequestCemeteryListOpcode(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "Received CMSG_REQUEST_CEMETERY_LIST");

    QueryResult* result = WorldDatabase.Query("SELECT id FROM graveyards WHERE faction = %u OR faction = 3;", _player->GetTeam());
    if (result)
    {
        WorldPacket data(SMSG_REQUEST_CEMETERY_LIST_RESPONSE, 8 * result->GetRowCount());
        data.writeBit(false);               //unk bit
        data.flushBits();
        data.writeBits(result->GetRowCount(), 24);
        data.flushBits();

        do
        {
            Field* field = result->Fetch();
            data << uint32_t(field[0].GetUInt32());
        } while (result->NextRow());
        delete result;

        SendPacket(&data);
    }
}
#endif
