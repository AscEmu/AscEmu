/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Storage/WDB/WDBStores.hpp"
#include "Objects/Item.hpp"
#include "Management/WeatherMgr.hpp"
#include "Management/ItemInterface.h"
#include "Management/Loot/LootMgr.hpp"
#include "Management/Loot/LootItem.hpp"
#include "Macros/CorpseMacros.hpp"
#include "Management/Battleground/Battleground.hpp"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "zlib.h"
#include "Chat/ChatDefines.hpp"
#include "Management/AddonMgr.h"
#include "Management/Group.h"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestMgr.h"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Spell/SpellMgr.hpp"
#include "Server/Packets/SmsgLogoutResponse.h"
#include "Server/Packets/CmsgStandStateChange.h"
#include "Server/Packets/CmsgWho.h"
#include "Server/Packets/CmsgSetSelection.h"
#include "Server/Packets/CmsgTutorialFlag.h"
#include "Server/Packets/CmsgSetSheathed.h"
#include "Server/Packets/CmsgRequestPlayedTime.h"
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
#include "Management/Guild/GuildMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Server/Packets/CmsgZoneupdate.h"
#include "Server/Packets/CmsgResurrectResponse.h"
#include "Server/Packets/CmsgBug.h"
#include "Server/Packets/CmsgUpdateAccountData.h"
#include "Server/Packets/SmsgUpdateAccountDataComplete.h"
#include "Server/Packets/CmsgSummonResponse.h"
#include "Server/Packets/SmsgResurrectFailed.h"
#include "Server/Packets/CmsgReclaimCorpse.h"
#include "Server/Packets/CmsgRemoveGlyph.h"
#include "Server/Packets/CmsgWhoIs.h"
#include "Server/Packets/SmsgBarberShopResult.h"
#include "Server/Packets/CmsgAlterAppearance.h"
#include "Server/Packets/CmsgGameobjUse.h"
#include "Server/Packets/CmsgInspect.h"
#include "Server/Packets/SmsgAccountDataTimes.h"
#include "Server/Packets/SmsgLogoutCancelAck.h"
#include "Server/Packets/SmsgMotd.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Objects/Transporter.hpp"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/Script/HookInterface.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/Spell.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Strings.hpp"

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
    {
        return;
    }

    bool const hasCharName = !srlPacket.player_name.empty();
    bool const hasGuildName = !srlPacket.guild_name.empty();

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_WHO with {} zones and {} names", srlPacket.zone_count, srlPacket.name_count);

    PlayerTeam const team = _player->getTeam();

    uint32_t sent_count = 0;
    uint32_t total_count = 0;

    WorldPacket data;
    data.SetOpcode(SMSG_WHO);
    data << uint64_t(0);

    sObjectMgr.m_playerLock.lock();

    for (auto const& itr : sObjectMgr.getPlayerStorage())
    {
        Player* player = itr.second;
        if (player == nullptr || player->getSession() == nullptr || !player->IsInWorld())
        {
            continue;
        }

        if (!worldConfig.gm.showGmInWhoList && !HasGMPermissions() && player->getSession()->HasGMPermissions())
        {
            continue;
        }

        // Team check
        if (!HasGMPermissions() && player->getTeam() != team && !player->getSession()->HasGMPermissions() && !worldConfig.player.isInterfactionMiscEnabled)
        {
            continue;
        }

        ++total_count;

        // Chat name
        if (hasCharName && srlPacket.player_name.compare(player->getName()) != 0)
        {
            continue;
        }

        // Guild name
        if (hasGuildName && (!player->getGuild() || srlPacket.guild_name.compare(player->getGuild()->getName()) != 0))
        {
            continue;
        }

        // Level check
        // skip players outside of level range
        if (srlPacket.min_level > 0 && srlPacket.max_level > 0 && player->getLevel() < srlPacket.min_level || player->getLevel() > srlPacket.max_level)
        {
            continue;
        }

        // Zone id compare
        if (srlPacket.zone_count > 0)
        {
            // people that fail the zone check don't get added
            bool skip = true;
            for (uint32_t i = 0; i < srlPacket.zone_count; ++i)
            {
                if (srlPacket.zones[i] == player->getZoneId())
                {
                    skip = false;
                    break;
                }
            }

            if (skip)
            {
                continue;
            }
        }

        if (((srlPacket.class_mask >> 1) & player->getClassMask()) == 0 || ((srlPacket.race_mask >> 1) & player->getRaceMask()) == 0)
        {
            continue;
        }

        if (srlPacket.name_count > 0)
        {
            // people that fail name check don't get added
            bool skip = true;
            for (uint32_t i = 0; i < srlPacket.name_count; ++i)
            {
                if (AscEmu::Util::Strings::isEqual(srlPacket.names[i].c_str(), player->getName().c_str()))
                {
                    skip = false;
                    break;
                }
            }

            if (skip)
            {
                continue;
            }
        }

        // if we're here, it means we've passed all tests
        data << player->getName().c_str();

        if (player->m_playerInfo->m_guild > 0)
        {
            data << sGuildMgr.getGuildById(player->m_playerInfo->m_guild)->getName().c_str();
        }
        else
        {
            data << uint8_t(0);
        }

        data << player->getLevel();
        data << uint32_t(player->getClass());
        data << uint32_t(player->getRace());
        data << player->getGender();
        data << uint32_t(player->getZoneId());

        ++sent_count;
        if (sent_count >= 49)
        {
            break;
        }
    }

    sObjectMgr.m_playerLock.unlock();
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

    _player->setTargetGuid(srlPacket.guid);

    if (_player->m_comboPoints)
        _player->updateComboPoints();
}

void WorldSession::handleTogglePVPOpcode(WorldPacket& /*recvPacket*/)
{
    _player->togglePvP();
}

void WorldSession::handleTutorialFlag(WorldPacket& recvPacket)
{
    CmsgTutorialFlag srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const uint32_t packet_index = (srlPacket.flag / 32);
    const uint32_t tutorial_status = (srlPacket.flag % 32);

    if (packet_index >= 7)
    {
        Disconnect();
        return;
    }

    const auto tutorial_index = static_cast<uint8_t>(packet_index);

    uint32_t tutorial_flag = _player->getTutorialValueById(tutorial_index);
    tutorial_flag |= (1 << tutorial_status);
    _player->setTutorialValueForId(tutorial_index, tutorial_flag);

    sLogger.debug("Received Tutorial flag: ({}).", srlPacket.flag);
}

void WorldSession::handleTutorialClear(WorldPacket& /*recvPacket*/)
{
    for (uint8_t id = 0; id < 8; ++id)
        _player->setTutorialValueForId(id, 0xFFFFFFFF);
}

void WorldSession::handleTutorialReset(WorldPacket& /*recvPacket*/)
{
    for (uint8_t id = 0; id < 8; ++id)
        _player->setTutorialValueForId(id, 0x00000000);
}

void WorldSession::handleLogoutRequestOpcode(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= TBC // support classic
    if (!sHookInterface.OnLogoutRequest(_player))
    {
        SendPacket(SmsgLogoutResponse(true).serialise().get());
        return;
    }

    if (!hasPermissions())
    {
        if (_player->getCombatHandler().isInCombat() || _player->m_duelPlayer != nullptr)
        {
            SendPacket(SmsgLogoutResponse(true).serialise().get());
            return;
        }

        if (_player->m_isResting || _player->isOnTaxi() || worldConfig.player.enableInstantLogoutForAccessType == 2)
        {
            SetLogoutTimer(1);
            return;
        }
    }

    if (hasPermissions())
    {
        if (_player->m_isResting || _player->isOnTaxi() || worldConfig.player.enableInstantLogoutForAccessType > 0)
        {
            SetLogoutTimer(1);
            return;
        }
    }

    SendPacket(SmsgLogoutResponse(false).serialise().get());

    _player->setMoveRoot(true);
    LoggingOut = true;

    _player->addUnitFlags(UNIT_FLAG_LOCK_PLAYER);

    _player->setStandState(STANDSTATE_SIT);

    SetLogoutTimer(PLAYER_LOGOUT_DELAY);
#endif
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
    CmsgRequestPlayedTime srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_REQUEST_PLAYED_TIME: displayInChatFrame: {}", srlPacket.displayInChatFrame);

    const uint32_t playedTime = static_cast<uint32_t>(UNIXTIME) - _player->m_playedTime[2];
    if (playedTime > 0)
    {
        _player->m_playedTime[0] += playedTime;
        _player->m_playedTime[1] += playedTime;
        _player->m_playedTime[2] += playedTime;
    }

    SendPacket(SmsgPlayedTime(_player->m_playedTime[1], _player->m_playedTime[0], srlPacket.displayInChatFrame).serialise().get());

    sLogger.debug("Sent SMSG_PLAYED_TIME total: {} level: {}", _player->m_playedTime[1], _player->m_playedTime[0]);
}

void WorldSession::handleSetActionButtonOpcode(WorldPacket& recvPacket)
{
    CmsgSetActionButton srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("BUTTON: {} ACTION: {} TYPE: {} MISC: {}", srlPacket.button, srlPacket.action, srlPacket.type, srlPacket.misc);

    if (srlPacket.action == 0)
    {
        sLogger.debug("MISC: Remove action from button {}", srlPacket.button);
        _player->setActionButton(srlPacket.button, 0, 0, 0);
    }
    else
    {
        if (srlPacket.button >= PLAYER_ACTION_BUTTON_COUNT)
            return;

        if (srlPacket.type == 64 || srlPacket.type == 65)
        {
            sLogger.debug("MISC: Added Macro {} into button {}", srlPacket.action, srlPacket.button);
            _player->setActionButton(srlPacket.button, srlPacket.action, srlPacket.type, srlPacket.misc);
        }
        else if (srlPacket.type == 128)
        {
            sLogger.debug("MISC: Added Item {} into button {}", srlPacket.action, srlPacket.button);
            _player->setActionButton(srlPacket.button, srlPacket.action, srlPacket.type, srlPacket.misc);
        }
        else if (srlPacket.type == 0)
        {
            sLogger.debug("MISC: Added Spell {} into button {}", srlPacket.action, srlPacket.button);
            _player->setActionButton(srlPacket.button, srlPacket.action, srlPacket.type, srlPacket.misc);
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

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_RANDOM_ROLL: {} (min), {} (max)", srlPacket.min, srlPacket.max);

    uint32_t maxValue = srlPacket.max;
    uint32_t minValue = srlPacket.min;

    if (maxValue > RAND_MAX)
        maxValue = RAND_MAX;

    if (minValue > maxValue)
        minValue = maxValue;

    uint32_t randomRoll = Util::getRandomUInt(maxValue - minValue) + minValue;

    if (_player->isInGroup())
        _player->getGroup()->SendPacketToAll(MsgRandomRoll(minValue, maxValue, randomRoll, _player->getGuid()).serialise().get());
    else
        SendPacket(MsgRandomRoll(minValue, maxValue, randomRoll, _player->getGuid()).serialise().get());
}

void WorldSession::handleRealmSplitOpcode(WorldPacket& recvPacket)
{
    CmsgRealmSplit srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_REALM_SPLIT: {} (unk)", srlPacket.unknown);

    const std::string dateFormat = "01/01/01";

    SendPacket(SmsgRealmSplit(srlPacket.unknown, 0, dateFormat).serialise().get());
}

void WorldSession::handleSetTaxiBenchmarkOpcode(WorldPacket& recvPacket)
{
    CmsgSetTaxiBenchmarkMode srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_SET_TAXI_BENCHMARK_MODE: {} (mode)", srlPacket.mode);
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

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GAMEOBJ_REPORT_USE: {} (guid.low)", srlPacket.guid.getGuidLow());

    const auto gameobject = _player->getWorldMap()->getGameObject(srlPacket.guid.getGuidLow());
    if (gameobject == nullptr)
        return;

    sQuestMgr.OnGameObjectActivate(_player, gameobject);
    _player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT, gameobject->getEntry(), 0, 0);

#endif
}

void WorldSession::handleDungeonDifficultyOpcode(WorldPacket& recvPacket)
{
    MsgSetDungeonDifficulty srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.difficulty >= InstanceDifficulty::MAX_DUNGEON_DIFFICULTY)
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_SET_DUNGEON_DIFFICULTY: {} (difficulty)", srlPacket.difficulty);

    if (InstanceDifficulty::Difficulties(srlPacket.difficulty) == _player->getDungeonDifficulty())
        return;

    // cannot reset while in an instance
    WorldMap* map = _player->getWorldMap();
    if (map && map->getBaseMap()->isDungeon())
        return;

    if (const auto group = _player->getGroup())
    {
        if (_player->isGroupLeader())
        {
            group->resetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, false, _player);
            group->SetDungeonDifficulty(InstanceDifficulty::Difficulties(srlPacket.difficulty));
        }
    }
    else
    {
        _player->resetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, false);
        _player->setDungeonDifficulty(InstanceDifficulty::Difficulties(srlPacket.difficulty));
    }
}

void WorldSession::handleRaidDifficultyOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    MsgSetRaidDifficulty srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.difficulty >= InstanceDifficulty::MAX_RAID_DIFFICULTY)
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_SET_RAID_DIFFICULTY: {} (difficulty)", srlPacket.difficulty);

    // cannot reset while in an instance
    WorldMap* map = _player->getWorldMap();
    if (map && map->getBaseMap()->isDungeon())
        return;

    if (InstanceDifficulty::Difficulties(srlPacket.difficulty) == _player->getRaidDifficulty())
        return;

    if (const auto group = _player->getGroup())
    {
        if (_player->isGroupLeader())
        {
            group->resetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, true, _player);
            group->SetRaidDifficulty(InstanceDifficulty::Difficulties(srlPacket.difficulty));
        }
    }
    else
    {
        _player->resetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, true);
        _player->setRaidDifficulty(InstanceDifficulty::Difficulties(srlPacket.difficulty));
    }
#endif
}

void WorldSession::handleSetAutoLootPassOpcode(WorldPacket& recvPacket)
{
    CmsgOptOutOfLoot srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_OPT_OUT_OF_LOOT: {} (turnedOn)", srlPacket.turnedOn);

    _player->m_passOnLoot = srlPacket.turnedOn > 0 ? true : false;
}

void WorldSession::handleSetActionBarTogglesOpcode(WorldPacket& recvPacket)
{
    CmsgSetActionbarToggles srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_SET_ACTIONBAR_TOGGLES: {} (actionbarId)", srlPacket.actionbarId);

    _player->setEnabledActionBars(srlPacket.actionbarId);
}

void WorldSession::handleLootRollOpcode(WorldPacket& recvPacket)
{
    CmsgLootRoll srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_LOOT_ROLL: {} (objectGuid) {} (slot) {} (choice)", srlPacket.objectGuid.getGuidLow(), srlPacket.slot, srlPacket.choice);

    LootItem* lootItem = nullptr;

    const HighGuid guidType = srlPacket.objectGuid.getHigh();

    switch (guidType)
    {
        case HighGuid::GameObject:
        {
            auto gameObject = _player->getWorldMap()->getGameObject(srlPacket.objectGuid.getGuidLowPart());
            if (gameObject == nullptr)
                return;

            if (!gameObject->IsLootable())
                return;

            auto gameObjectLootable = static_cast<GameObject_Lootable*>(gameObject);
            if (srlPacket.slot >= gameObjectLootable->loot.items.size() || gameObjectLootable->loot.items.empty())
                return;

            if (gameObject->getGoType() == GAMEOBJECT_TYPE_CHEST)
                lootItem = &gameObjectLootable->loot.items[srlPacket.slot];
        } break;
        case HighGuid::Unit:
        {
            auto creature = _player->getWorldMap()->getCreature(srlPacket.objectGuid.getGuidLowPart());
            if (creature == nullptr)
                return;

            if (srlPacket.slot >= creature->loot.items.size() || creature->loot.items.empty())
                return;

            lootItem = &creature->loot.items[srlPacket.slot];
        } break;
        default:
            return;
    }

    if (lootItem == nullptr)
        return;

    lootItem->playerRolled(_player, srlPacket.choice);
}

void WorldSession::handleOpenItemOpcode(WorldPacket& recvPacket)
{
    CmsgOpenItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_OPEN_ITEM: {} (containerSlot), {} (slot)", srlPacket.containerSlot, srlPacket.slot);

    auto item = _player->getItemInterface()->GetInventoryItem(srlPacket.containerSlot, srlPacket.slot);
    if (item == nullptr)
        return;

    if (item->getGiftCreatorGuid() && item->m_wrappedItemId)
    {
        const auto wrappedItem = sMySQLStore.getItemProperties(item->m_wrappedItemId);
        if (wrappedItem == nullptr)
            return;

        item->setGiftCreatorGuid(0);
        item->setEntry(item->m_wrappedItemId);
        item->m_wrappedItemId = 0;
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
        item->saveToDB(srlPacket.containerSlot, srlPacket.slot, false, nullptr);
        return;
    }

    uint32_t removeLockItems[LOCK_NUM_CASES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    const auto lockEntry = sLockStore.lookupEntry(item->getItemProperties()->LockId);
    if (lockEntry)
    {
        for (uint8_t lockCase = 0; lockCase < LOCK_NUM_CASES; ++lockCase)
        {
            if (lockEntry->locktype[lockCase] == 1 && lockEntry->lockmisc[lockCase] > 0)
            {
                const int16_t slot2 = _player->getItemInterface()->GetInventorySlotById(lockEntry->lockmisc[lockCase]);
                if (slot2 != ITEM_NO_SLOT_AVAILABLE && slot2 >= INVENTORY_SLOT_ITEM_START && slot2 < INVENTORY_SLOT_ITEM_END)
                {
                    removeLockItems[lockCase] = lockEntry->lockmisc[lockCase];
                }
                else
                {
                    _player->getItemInterface()->buildInventoryChangeError(item, nullptr, INV_ERR_ITEM_LOCKED);
                    return;
                }
            }
            else if (lockEntry->locktype[lockCase] == 2 && item->m_isLocked)
            {
                _player->getItemInterface()->buildInventoryChangeError(item, nullptr, INV_ERR_ITEM_LOCKED);
                return;
            }
        }

        for (uint8_t lockCase = 0; lockCase < LOCK_NUM_CASES; ++lockCase)
        {
            if (removeLockItems[lockCase])
                _player->getItemInterface()->RemoveItemAmt(removeLockItems[lockCase], 1);
        }
    }

    _player->setLootGuid(item->getGuid());
    if (item->m_loot == nullptr)
    {
        item->m_loot = std::make_unique<Loot>();
        sLootMgr.fillItemLoot(_player, item->m_loot.get(), item->getEntry(), 0);
    }
    _player->sendLoot(item->getGuid(), LOOT_DISENCHANTING, _player->GetMapId());
}

void WorldSession::handleDismountOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->isOnTaxi())
        return;

    _player->dismount();
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
    if (const auto group = _player->getGroup())
    {
        if (group->GetLeader()->guid == _player->getGuidLow())
            group->resetInstances(INSTANCE_RESET_ALL, false, _player);
    }
    else
    {
        _player->resetInstances(INSTANCE_RESET_ALL, false);
    }
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

    if (_player->hasPvPTitle(static_cast<RankTitles>(srlPacket.titleId)))
        _player->setChosenTitle(srlPacket.titleId);
#endif
}

void WorldSession::handleZoneupdate(WorldPacket& recvPacket)
{
    CmsgZoneupdate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (_player->getZoneId() == srlPacket.zoneId)
        return;

    sWeatherMgr.sendWeather(_player);
    _player->zoneUpdate(srlPacket.zoneId);
    _player->getItemInterface()->EmptyBuyBack();
}

void WorldSession::handleResurrectResponse(WorldPacket& recvPacket)
{
    CmsgResurrectResponse srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!_player->isAlive())
        return;

    auto player = _player->getWorldMap()->getPlayer(srlPacket.guid.getGuidLow());
    if (player == nullptr)
        player = sObjectMgr.getPlayer(srlPacket.guid.getGuidLow());

    if (player == nullptr)
        return;

    if (srlPacket.status != 1 || _player->m_resurrecter || _player->m_resurrecter != srlPacket.guid.getRawGuid())
    {
        _player->m_resurrectHealth = 0;
        _player->m_resurrectMana = 0;
        _player->m_resurrecter = 0;
        return;
    }

    _player->resurrect();
    _player->setMoveRoot(false);
}

void WorldSession::handleSelfResurrect(WorldPacket& /*recvPacket*/)
{
    if (const auto resurrectSpell = _player->getSelfResurrectSpell())
    {
        const auto spellInfo = sSpellMgr.getSpellInfo(resurrectSpell);
        if (const auto spell = sSpellMgr.newSpell(_player, spellInfo, true, nullptr))
        {
            SpellCastTargets spellCastTargets(_player->getGuid());
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
        sLogger.failure("WARNING: Accountdata > 8 ({}) was requested to be updated by {} of account {}!",
            srlPacket.uiId, _player->getName(), this->GetAccountId());
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
    auto data = std::make_unique<char[]>(srlPacket.uiDecompressedSize + 1);
    memset(data.get(), 0, srlPacket.uiDecompressedSize + 1);

    if (srlPacket.uiDecompressedSize > receivedPackedSize)
    {
        const int32_t ZlibResult = uncompress(reinterpret_cast<uint8_t*>(data.get()), &uid, recvPacket.contents() + 8,
            static_cast<uLong>(receivedPackedSize));

        switch (ZlibResult)
        {
            case Z_OK:                  //0 no error decompression is OK
            {
                SetAccountData(srlPacket.uiId, std::move(data), false, srlPacket.uiDecompressedSize);
                sLogger.debug("Successfully decompressed account data {} for {}, and updated storage array.",
                    srlPacket.uiId, _player->getName());
            } break;
            case Z_ERRNO:               //-1
            case Z_STREAM_ERROR:        //-2
            case Z_DATA_ERROR:          //-3
            case Z_MEM_ERROR:           //-4
            case Z_BUF_ERROR:           //-5
            case Z_VERSION_ERROR:       //-6
            {
                sLogger.failure("Decompression of account data {} for {} FAILED.", srlPacket.uiId, _player->getName());
            } break;

            default:
            {
                sLogger.failure("Decompression gave a unknown error: {:x}, of account data {} for {} FAILED.",
                    ZlibResult, srlPacket.uiId, _player->getName());
            } break;
        }
    }
    else
    {
        memcpy(data.get(), recvPacket.contents() + 8, srlPacket.uiDecompressedSize);
        SetAccountData(srlPacket.uiId, std::move(data), false, srlPacket.uiDecompressedSize);
    }

#if VERSION_STRING > TBC
    SendPacket(SmsgUpdateAccountDataComplete(srlPacket.uiId, 0).serialise().get());
#endif
}

void WorldSession::handleRequestAccountData(WorldPacket& recvPacket)
{
    if (!worldConfig.server.useAccountData)
        return;

    uint32_t accountDataId;
    recvPacket >> accountDataId;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_REQUEST_ACCOUNT_DATA id {}.", accountDataId);

    if (accountDataId > 8)
    {
        sLogger.debug("CMSG_REQUEST_ACCOUNT_DATA: Accountdata > 8 ({}) was requested by {} of account {}!", accountDataId, _player->getName(), this->GetAccountId());
        return;
    }

    AccountDataEntry* accountDataEntry = GetAccountData(accountDataId);
    WorldPacket data;
    data.SetOpcode(SMSG_UPDATE_ACCOUNT_DATA);
    data << accountDataId;

    if (!accountDataEntry || !accountDataEntry->data)
    {
        data << uint32_t(0);
    }
    else
    {
        data << accountDataEntry->sz;

        if (accountDataEntry->sz > 200)
        {
            data.resize(accountDataEntry->sz + 800);

            uLongf destSize;
            if (compress(data.contents() + (sizeof(uint32_t) * 2), &destSize, reinterpret_cast<const uint8_t*>(accountDataEntry->data.get()), accountDataEntry->sz) != Z_OK)
            {
                sLogger.debug("CMSG_REQUEST_ACCOUNT_DATA: Error while compressing data");
                return;
            }

            data.resize(destSize + 8);
        }
        else
        {
            data.append(accountDataEntry->data.get(), accountDataEntry->sz);
        }
    }

    SendPacket(&data);
}

#if VERSION_STRING < Cata
void WorldSession::handleBugOpcode(WorldPacket& recv_data)
{
    CmsgBug srlPacket;
    if (!srlPacket.deserialise(recv_data))
        return;

    if (srlPacket.suggestion == 0)
        sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_BUG [Bug Report]");
    else
        sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_BUG [Suggestion]");

    uint64_t accountId = GetAccountId();
    uint32_t timeStamp = uint32_t(UNIXTIME);
    uint32_t reportId = sObjectMgr.generateReportId();

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

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_BUG [Bug Report] lenght: {} message: {}", lenght, bugMessage);

    uint64_t accountId = GetAccountId();
    uint32_t timeStamp = uint32_t(UNIXTIME);
    uint32_t reportId = sObjectMgr.generateReportId();

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

void WorldSession::handleSuggestionOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    uint8_t unk1;
    uint8_t unk2;

    recvPacket >> unk1;
    recvPacket >> unk2;

    uint32_t lenght = 0;
    lenght = unk1 * 16;
    lenght += unk2 / 16;

    std::string suggestionMessage;
    suggestionMessage = recvPacket.ReadString(lenght);

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_SUGGESTIONS [Suggestion] lenght: {} message: {}", lenght, suggestionMessage);

    uint64_t accountId = GetAccountId();
    uint32_t timeStamp = uint32_t(UNIXTIME);
    uint32_t reportId = sObjectMgr.generateReportId();

    std::stringstream ss;

    ss << "INSERT INTO playerbugreports VALUES('";
    ss << reportId << "','";
    ss << accountId << "','";
    ss << timeStamp << "',";
    ss << "'1',";
    ss << "'1','";
    ss << CharacterDatabase.EscapeString(suggestionMessage) << "')";

    CharacterDatabase.ExecuteNA(ss.str().c_str());
#endif
}

void WorldSession::handleReturnToGraveyardOpcode(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
    if (_player->isAlive())
        return;

    _player->repopAtGraveyard(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId());
#endif
}

void WorldSession::handleLogDisconnectOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    uint32_t disconnectReason;
    recvPacket >> disconnectReason; // 13 - closed window

    sLogger.debug("Player {} disconnected on {} - Reason {}", _player->getName(), Util::GetCurrentDateTimeString(), disconnectReason);
#endif
}

void WorldSession::handleCompleteCinematic(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_COMPLETE_CINEMATIC");

    _player->setStandState(STANDSTATE_STAND);
}

void WorldSession::handleNextCinematic(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_NEXT_CINEMATIC_CAMERA");

    _player->SetPosition(float(_player->GetPositionX() + 0.01), float(_player->GetPositionY() + 0.01),
        float(_player->GetPositionZ() + 0.01), _player->GetOrientation());
}

void WorldSession::handleReadyForAccountDataTimes(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_READY_FOR_ACCOUNT_DATA_TIMES");

    sendAccountDataTimes(GLOBAL_CACHE_MASK);
}

void WorldSession::handleSummonResponseOpcode(WorldPacket& recvPacket)
{
    CmsgSummonResponse srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!srlPacket.isClickOn)
        return;

    if (!_player->m_summonData.summonerId)
    {
        SendNotification("You do not have permission to perform that function.");
        return;
    }

    if (_player->getCombatHandler().isInCombat())
        return;

    _player->safeTeleport(_player->m_summonData.mapId, _player->m_summonData.instanceId, _player->m_summonData.position);

    _player->m_summonData.summonerId = _player->m_summonData.instanceId = _player->m_summonData.mapId = 0;
}

void WorldSession::handleLogoutCancelOpcode(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_LOGOUT_CANCEL");

    if (!LoggingOut)
        return;

    LoggingOut = false;

    SetLogoutTimer(0);

    SendPacket(SmsgLogoutCancelAck().serialise().get());

    _player->setMoveRoot(false);

    _player->setStandState(STANDSTATE_STAND);

    _player->removeUnitFlags(UNIT_FLAG_LOCK_PLAYER);

    sLogger.debug("Sent SMSG_LOGOUT_CANCEL_ACK");
}

void WorldSession::handlePlayerLogoutOpcode(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_PLAYER_LOGOUT");
    if (!HasGMPermissions())
        SendNotification("You do not have permission to perform that function.");
    else
        LogoutPlayer(true);
}

void WorldSession::handleCorpseReclaimOpcode(WorldPacket& recvPacket)
{
    CmsgReclaimCorpse srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_RECLAIM_CORPSE");

    if (srlPacket.guid.getRawGuid() == 0)
        return;

    auto corpse = sObjectMgr.getCorpseByGuid(srlPacket.guid.getGuidLow());
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

    if (time(nullptr) < corpse->getDeathClock() + CORPSE_RECLAIM_TIME)
    {
        SendPacket(SmsgResurrectFailed(1).serialise().get());
        return;
    }

    _player->resurrect();
    _player->setHealth(_player->getMaxHealth() / 2);
}


void WorldSession::handleLoadScreenOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    uint32_t mapId;

    recvPacket >> mapId;
    recvPacket.readBit();
#endif
}

void WorldSession::handleUITimeRequestOpcode(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
    WorldPacket data(SMSG_UI_TIME, 4);
    data << uint32_t(time(nullptr));
    SendPacket(&data);
#endif
}

void WorldSession::handleTimeSyncRespOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    uint32_t counter;
    uint32_t clientTicks;
    recvPacket >> counter;
    recvPacket >> clientTicks;
#endif
}

void WorldSession::handleObjectUpdateFailedOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    ObjectGuid guid;

#if VERSION_STRING == Cata
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
#elif VERSION_STRING == Mop
    guid[3] = recvPacket.readBit();
    guid[5] = recvPacket.readBit();
    guid[6] = recvPacket.readBit();
    guid[0] = recvPacket.readBit();
    guid[1] = recvPacket.readBit();
    guid[2] = recvPacket.readBit();
    guid[7] = recvPacket.readBit();
    guid[4] = recvPacket.readBit();

    recvPacket.ReadByteSeq(guid[0]);
    recvPacket.ReadByteSeq(guid[6]);
    recvPacket.ReadByteSeq(guid[5]);
    recvPacket.ReadByteSeq(guid[7]);
    recvPacket.ReadByteSeq(guid[2]);
    recvPacket.ReadByteSeq(guid[1]);
    recvPacket.ReadByteSeq(guid[3]);
    recvPacket.ReadByteSeq(guid[4]);
#endif

    sLogger.failure("handleObjectUpdateFailedOpcode : Object update failed for playerguid {}", WoWGuid::getGuidLowPartFromUInt64(guid));

    if (_player == nullptr)
        return;

    if (_player->getGuid() == guid)
    {
        LogoutPlayer(true);
        return;
    }

    //_player->updateVisibility();
#endif
}



#define DB2_REPLY_SPARSE 2442913102
#define DB2_REPLY_ITEM   1344507586

void WorldSession::sendItemDb2Reply(uint32_t entry)
{
#if VERSION_STRING >= Cata
#if VERSION_STRING < Mop
    WorldPacket data(SMSG_DB_REPLY, 44);
    ItemProperties const* proto = sMySQLStore.getItemProperties(entry);
    if (!proto)
    {
        data << uint32_t(-1);                                   // entry
        data << uint32_t(DB2_REPLY_ITEM);
        data << uint32_t(1322512289);                           // hotfix date
        data << uint32_t(0);                                    // size of next block
        return;
    }

    data << uint32_t(entry);
    data << uint32_t(DB2_REPLY_ITEM);
    data << uint32_t(1322512290);                               // hotfix date

    ByteBuffer buff;
    buff << uint32_t(entry);
    buff << uint32_t(proto->Class);
    buff << uint32_t(proto->SubClass);
    buff << int32_t(0);                                         // unk?
    buff << uint32_t(proto->LockMaterial);
    buff << uint32_t(proto->DisplayInfoID);
    buff << uint32_t(proto->InventoryType);
    buff << uint32_t(proto->SheathID);

    data << uint32_t(buff.size());
    data.append(buff);

    SendPacket(&data);
#endif
#endif
}

void WorldSession::sendItemSparseDb2Reply(uint32_t entry)
{
#if VERSION_STRING >= Cata
#if VERSION_STRING < Mop
    WorldPacket data(SMSG_DB_REPLY, 526);
    ItemProperties const* proto = sMySQLStore.getItemProperties(entry);
    if (!proto)
    {
        data << uint32_t(-1);                                   // entry
        data << uint32_t(DB2_REPLY_SPARSE);
        data << uint32_t(1322512289);                           // hotfix date
        data << uint32_t(0);                                    // size of next block
        return;
    }

    data << uint32_t(entry);
    data << uint32_t(DB2_REPLY_SPARSE);
    data << uint32_t(1322512290);                               // hotfix date

    ByteBuffer buff;
    buff << uint32_t(entry);
    buff << uint32_t(proto->Quality);
    buff << uint32_t(proto->Flags);
    buff << uint32_t(proto->Flags2);
    buff << float(1.0f);
    buff << float(1.0f);
    buff << uint32_t(proto->MaxCount);
    buff << int32_t(proto->BuyPrice);
    buff << uint32_t(proto->SellPrice);
    buff << uint32_t(proto->InventoryType);
    buff << int32_t(proto->AllowableClass);
    buff << int32_t(proto->AllowableRace);
    buff << uint32_t(proto->ItemLevel);
    buff << uint32_t(proto->RequiredLevel);
    buff << uint32_t(proto->RequiredSkill);
    buff << uint32_t(proto->RequiredSkillRank);
    buff << uint32_t(0);                                        // req spell
    buff << uint32_t(proto->RequiredPlayerRank1);
    buff << uint32_t(proto->RequiredPlayerRank2);
    buff << uint32_t(proto->RequiredFactionStanding);
    buff << uint32_t(proto->RequiredFaction);
    buff << int32_t(proto->MaxCount);
    buff << int32_t(0);                                         // stackable
    buff << uint32_t(proto->ContainerSlots);

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_STATS; ++x)
        buff << uint32_t(proto->Stats[x].Type);

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_STATS; ++x)
        buff << int32_t(proto->Stats[x].Value);

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_STATS; ++x)
        buff << int32_t(0);                                     // unk

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_STATS; ++x)
        buff << int32_t(0);                                     // unk

    buff << uint32_t(proto->ScalingStatsEntry);
    buff << uint32_t(0);                                        // damage type
    buff << uint32_t(proto->Delay);
    buff << float(40);                                          // ranged range

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
        buff << int32_t(0);

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
        buff << uint32_t(0);

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
        buff << int32_t(0);

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
        buff << int32_t(0);

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
        buff << uint32_t(0);

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
        buff << int32_t(0);

    buff << uint32_t(proto->Bonding);

    // item name
    utf8_string name = proto->Name;
    buff << uint16_t(name.length());
    if (name.length())
        buff << name;

    for (uint32_t i = 0; i < 3; ++i)                            // other 3 names
        buff << uint16_t(0);

    std::string desc = proto->Description;
    buff << uint16_t(desc.length());
    if (desc.length())
        buff << desc;

    buff << uint32_t(proto->PageId);
    buff << uint32_t(proto->PageLanguage);
    buff << uint32_t(proto->PageMaterial);
    buff << uint32_t(proto->QuestId);
    buff << uint32_t(proto->LockId);
    buff << int32_t(proto->LockMaterial);
    buff << uint32_t(proto->SheathID);
    buff << int32_t(proto->RandomPropId);
    buff << int32_t(proto->RandomSuffixId);
    buff << uint32_t(proto->ItemSet);

    buff << uint32_t(0);// area
    buff << uint32_t(proto->MapID);
    buff << uint32_t(proto->BagFamily);
    buff << uint32_t(proto->TotemCategory);

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_SOCKETS; ++x)
        buff << uint32_t(proto->Sockets[x].SocketColor);

    for (uint32_t x = 0; x < MAX_ITEM_PROTO_SOCKETS; ++x)
        buff << uint32_t(proto->Sockets[x].Unk);

    buff << uint32_t(proto->SocketBonus);
    buff << uint32_t(proto->GemProperties);
    buff << float(proto->ArmorDamageModifier);
    buff << int32_t(proto->ExistingDuration);
    buff << uint32_t(proto->ItemLimitCategory);
    buff << uint32_t(proto->HolidayId);
    buff << float(proto->ScalingStatsFlag);                     // StatScalingFactor
    buff << uint32_t(0);                                        // archaeology unk
    buff << uint32_t(0);                                        // archaeology findinds count

    data << uint32_t(buff.size());
    data.append(buff);

    SendPacket(&data);
#endif
#endif
}

void WorldSession::handleRequestHotfix(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
#if VERSION_STRING == Cata
    uint32_t type;
    recvPacket >> type;

    uint32_t count = recvPacket.readBits(23);

    auto guids = std::make_unique<ObjectGuid[]>(count);
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

        switch (type)
        {
            case DB2_REPLY_ITEM:
                sendItemDb2Reply(entry);
                break;
            case DB2_REPLY_SPARSE:
                sendItemSparseDb2Reply(entry);
                break;
            default:
                sLogger.debug("Received unknown hotfix type {}", type);
                recvPacket.clear();
                break;
        }
    }
#elif VERSION_STRING == Mop
    uint32_t type;
    recvPacket >> type;

    if (type != DB2_REPLY_ITEM && type != DB2_REPLY_SPARSE)
        return;

    uint32_t count = recvPacket.readBits(21);

    auto guids = std::make_unique<ObjectGuid[]>(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        guids[i][6] = recvPacket.readBit();
        guids[i][3] = recvPacket.readBit();
        guids[i][0] = recvPacket.readBit();
        guids[i][1] = recvPacket.readBit();
        guids[i][4] = recvPacket.readBit();
        guids[i][5] = recvPacket.readBit();
        guids[i][7] = recvPacket.readBit();
        guids[i][2] = recvPacket.readBit();
    }

    uint32_t entry;
    for (uint32_t i = 0; i < count; ++i)
    {
        recvPacket.ReadByteSeq(guids[i][1]);
        recvPacket >> entry;
        recvPacket.ReadByteSeq(guids[i][0]);
        recvPacket.ReadByteSeq(guids[i][5]);
        recvPacket.ReadByteSeq(guids[i][6]);
        recvPacket.ReadByteSeq(guids[i][4]);
        recvPacket.ReadByteSeq(guids[i][7]);
        recvPacket.ReadByteSeq(guids[i][2]);
        recvPacket.ReadByteSeq(guids[i][3]);

        /*switch (type)
        {
            case DB2_REPLY_ITEM:
                SendItemDb2Reply(entry);
                break;
            case DB2_REPLY_SPARSE:
                SendItemSparseDb2Reply(entry);
                break;
            default:
                sLogger.debug("Received unknown hotfix type {}", type);
                recvPacket.clear();
                break;
        }*/

        WorldPacket data(SMSG_DB_REPLY, 16);
        data << uint32_t(entry);
        data << uint32_t(time(NULL));
        data << uint32_t(type);
        data << uint32_t(0);

        SendPacket(&data);
    }
#endif
#endif
}

void WorldSession::handleRequestCemeteryListOpcode(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_REQUEST_CEMETERY_LIST");

    auto result = WorldDatabase.Query("SELECT id FROM graveyards WHERE faction = %u OR faction = 3;", _player->getTeam());
    if (result)
    {
        WorldPacket data(SMSG_REQUEST_CEMETERY_LIST_RESPONSE, 8 * result->GetRowCount());
        data.writeBit(false); // unk bit
        data.flushBits();
        data.writeBits(result->GetRowCount(), 24);
        data.flushBits();

        do
        {
            Field* field = result->Fetch();
            data << uint32_t(field[0].asUint32());
        } while (result->NextRow());

        SendPacket(&data);
    }
#endif
}



void WorldSession::handleRemoveGlyph(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgRemoveGlyph srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.glyphNumber > 5)
        return;

    const uint32_t glyphId = _player->getGlyph(srlPacket.glyphNumber);
    if (glyphId == 0)
        return;

    const auto glyphPropertiesEntry = sGlyphPropertiesStore.lookupEntry(glyphId);
    if (!glyphPropertiesEntry)
        return;

    _player->setGlyph(srlPacket.glyphNumber, 0);
    _player->removeAllAurasById(glyphPropertiesEntry->SpellID);
    _player->m_specs[_player->m_talentActiveSpec].setGlyph(0, srlPacket.glyphNumber);
    _player->smsg_TalentsInfo(false);
#endif
}

#if VERSION_STRING > TBC
namespace BarberShopResult
{
    enum
    {
        Ok = 0,
        NoMoney = 1
    };
}
#endif

void WorldSession::handleBarberShopResult(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    // todo: Here was SMSG_BARBER_SHOP:RESULT... maybe itr is MSG or it was just wrong. Check it!
    CmsgAlterAppearance srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_ALTER_APPEARANCE");

    const uint32_t oldHair = _player->getHairStyle();
    const uint32_t oldHairColor = _player->getHairColor();
    const uint32_t oldFacial = _player->getFacialFeatures();

    uint32_t cost = 0;

    const auto barberShopHair = sBarberShopStyleStore.lookupEntry(srlPacket.hair);
    if (!barberShopHair)
        return;

    const auto newHair = barberShopHair->hair_id;

    const auto newHairColor = srlPacket.hairColor;

    const auto barberShopFacial = sBarberShopStyleStore.lookupEntry(srlPacket.facialHairOrPiercing);
    if (!barberShopFacial)
        return;

    const auto newFacial = barberShopFacial->hair_id;

    const auto barberShopSkinColor = sBarberShopStyleStore.lookupEntry(srlPacket.skinColor);
    if (barberShopSkinColor && barberShopSkinColor->race != _player->getRace())
        return;

    auto level = _player->getLevel();
    if (level >= 100)
        level = 100;

    const auto gtBarberShopCostBaseEntry = sBarberShopCostBaseStore.lookupEntry(level - 1);
    if (!gtBarberShopCostBaseEntry)
        return;

    if (newHair != oldHair)
        cost += static_cast<uint32_t>(gtBarberShopCostBaseEntry->cost);
    else if (newHairColor != oldHairColor)
        cost += static_cast<uint32_t>(gtBarberShopCostBaseEntry->cost) >> 1;

    if (newFacial != oldFacial)
        cost += static_cast<uint32_t>(gtBarberShopCostBaseEntry->cost * 0.75f);

    if (!_player->hasEnoughCoinage(cost))
    {
        SendPacket(SmsgBarberShopResult(BarberShopResult::NoMoney).serialise().get());
        return;
    }

    SendPacket(SmsgBarberShopResult(BarberShopResult::Ok).serialise().get());

    _player->setHairStyle(static_cast<uint8_t>(newHair));
    _player->setHairColor(static_cast<uint8_t>(newHairColor));
    _player->setFacialFeatures(static_cast<uint8_t>(newFacial));
    if (barberShopSkinColor)
        _player->setSkinColor(static_cast<uint8_t>(barberShopSkinColor->hair_id));

    _player->modCoinage(-static_cast<int32_t>(cost));

    _player->setStandState(STANDSTATE_STAND);
    _player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP, 1, 0, 0);
    _player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER, cost, 0, 0);
#endif
}

void WorldSession::handleRepopRequestOpcode(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_REPOP_REQUEST");

    if (_player->getDeathState() != JUST_DIED)
        return;

    if (_player->getTransGuid())
    {
        auto transport = _player->GetTransport();
        if (transport != nullptr)
            transport->RemovePassenger(_player);
    }

    _player->repopRequest();
}

void WorldSession::handleWhoIsOpcode(WorldPacket& recvPacket)
{
    CmsgWhoIs srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("Received WHOIS command from player {} for character {}", _player->getName(), srlPacket.characterName);

    if (!_player->getSession()->CanUseCommand('3'))
    {
        SendNotification("You do not have permission to perform that function.");
        return;
    }

    if (srlPacket.characterName.empty())
    {
        SendNotification("You did not enter a character name!");
        return;
    }

    auto resultAcctId = CharacterDatabase.Query("SELECT acct FROM characters WHERE name = '%s'", srlPacket.characterName.c_str());
    if (!resultAcctId)
    {
        SendNotification("%s does not exit!", srlPacket.characterName.c_str());
        return;
    }

    Field* fields_acctID = resultAcctId->Fetch();
    const uint32_t accId = fields_acctID[0].asUint32();

    //todo: this will not work! no table accounts in character_db!!!
    auto accountInfoResult = CharacterDatabase.Query("SELECT acct, login, gm, email, lastip, muted FROM accounts WHERE acct = %u", accId);
    if (!accountInfoResult)
    {
        SendNotification("Account information for %s not found!", srlPacket.characterName.c_str());
        return;
    }

    Field* fields = accountInfoResult->Fetch();
    std::string acctID = fields[0].asCString();
    if (acctID.empty())
        acctID = "Unknown";

    std::string acctName = fields[1].asCString();
    if (acctName.empty())
        acctName = "Unknown";

    std::string acctPerms = fields[2].asCString();
    if (acctPerms.empty())
        acctPerms = "Unknown";

    std::string acctEmail = fields[3].asCString();
    if (acctEmail.empty())
        acctEmail = "Unknown";

    std::string acctIP = fields[4].asCString();
    if (acctIP.empty())
        acctIP = "Unknown";

    std::string acctMuted = fields[5].asCString();
    if (acctMuted.empty())
        acctMuted = "Unknown";

    std::string msg = srlPacket.characterName + "'s " + "account information: acctID: " + acctID + ", Name: "
    + acctName + ", Permissions: " + acctPerms + ", E-Mail: " + acctEmail + ", lastIP: " + acctIP + ", Muted: " + acctMuted;

    WorldPacket data(SMSG_WHOIS, msg.size() + 1);
    data << msg;
    SendPacket(&data);
}

void WorldSession::handleAmmoSetOpcode(WorldPacket& recvPacket)
{
    uint32_t ammoId;
    recvPacket >> ammoId;

    if (!ammoId)
        return;

    const auto itemProperties = sMySQLStore.getItemProperties(ammoId);
    if (!itemProperties)
        return;

    if (itemProperties->Class != ITEM_CLASS_PROJECTILE || _player->getItemInterface()->GetItemCount(ammoId) == 0)
    {
        sCheatLog.writefromsession(_player->getSession(), "Definitely cheating. tried to add %u as ammo.", ammoId);
        _player->getSession()->Disconnect();
        return;
    }

    if (itemProperties->RequiredLevel)
    {
        if (_player->getLevel() < itemProperties->RequiredLevel)
        {
            _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_RANK_NOT_ENOUGH);
#if VERSION_STRING < Cata
            _player->setAmmoId(0);
#endif
            _player->calculateDamage();
            return;
        }
    }
    if (itemProperties->RequiredSkill)
    {
        if (!_player->hasSkillLine(itemProperties->RequiredSkill))
        {
            _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_RANK_NOT_ENOUGH);
#if VERSION_STRING < Cata
            _player->setAmmoId(0);
#endif
            _player->calculateDamage();
            return;
        }

        if (itemProperties->RequiredSkillRank)
        {
            if (_player->getSkillLineCurrent(itemProperties->RequiredSkill, false) < itemProperties->RequiredSkillRank)
            {
                _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_RANK_NOT_ENOUGH);
#if VERSION_STRING < Cata
                _player->setAmmoId(0);
#endif
                _player->calculateDamage();
                return;
            }
        }
    }
    switch (_player->getClass())
    {
        case PRIEST:        // allowing priest, warlock, mage to equip ammo will mess up wand shoot. stop it.
        case WARLOCK:
        case MAGE:
        case SHAMAN:        // these don't get messed up since they don't use wands, but they don't get to use bows/guns/crossbows anyways
        case DRUID:         // we wouldn't want them cheating extra stats from ammo, would we?
        case PALADIN:
#if VERSION_STRING > TBC
        case DEATHKNIGHT:
#endif
            _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM);
#if VERSION_STRING < Cata
            _player->setAmmoId(0);
#endif
            _player->calculateDamage();
            return;
        default:
#if VERSION_STRING < Cata
            _player->setAmmoId(ammoId);
#endif
            _player->calculateDamage();
            break;
    }
}

void WorldSession::handleGameObjectUse(WorldPacket& recvPacket)
{
    CmsgGameobjUse srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GAMEOBJ_USE: {} (gobj guidLow)", srlPacket.guid.getGuidLowPart());

    auto gameObject = _player->getWorldMap()->getGameObject(srlPacket.guid.getGuidLowPart());
    if (!gameObject)
        return;

    const auto gameObjectProperties = gameObject->GetGameObjectProperties();
    if (!gameObjectProperties)
        return;

    //////////////////////////////////////////////////////////////////////////////////////////
    //\brief: the following lines are handled in gobj class

    sObjectMgr.checkForScripts(_player, gameObjectProperties->raw.parameter_9);

    if (gameObject->GetScript())
        gameObject->GetScript()->OnActivate(_player);

    if (_player->getWorldMap() && _player->getWorldMap()->getScript())
        _player->getWorldMap()->getScript()->OnGameObjectActivate(gameObject, _player);

    _player->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);

    switch (gameObject->getGoType())
    {
        case GAMEOBJECT_TYPE_DOOR:
        case GAMEOBJECT_TYPE_BUTTON:
        case GAMEOBJECT_TYPE_QUESTGIVER:
        case GAMEOBJECT_TYPE_CHEST:
        case GAMEOBJECT_TYPE_CHAIR:
        case GAMEOBJECT_TYPE_GOOBER:
        case GAMEOBJECT_TYPE_CAMERA:
        case GAMEOBJECT_TYPE_FISHINGNODE:
        case GAMEOBJECT_TYPE_RITUAL:
        case GAMEOBJECT_TYPE_SPELLCASTER:
        case GAMEOBJECT_TYPE_MEETINGSTONE:
        case GAMEOBJECT_TYPE_FLAGSTAND:
        case GAMEOBJECT_TYPE_FLAGDROP:
        case GAMEOBJECT_TYPE_BARBER_CHAIR:
            gameObject->onUse(_player);
            break;
        default:
            sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GAMEOBJ_USE for unhandled type {}.", gameObject->getGoType());
            break;
    }
}

void WorldSession::handleInspectOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING < Mop
    CmsgInspect srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_INSPECT: {} (player guid)", static_cast<uint32_t>(srlPacket.guid));

    auto inspectedPlayer = _player->getWorldMap()->getPlayer(static_cast<uint32_t>(srlPacket.guid));
    if (inspectedPlayer == nullptr)
    {
        sLogger.debug("Error received CMSG_INSPECT for unknown player!");
        return;
    }

    _player->setTargetGuid(srlPacket.guid);

    if (_player->m_comboPoints)
        _player->updateComboPoints();

    ByteBuffer packedGuid;
    WorldPacket data(SMSG_INSPECT_TALENT, 1000);
    packedGuid.appendPackGUID(inspectedPlayer->getGuid());
    data.append(packedGuid);

    data << uint32_t(inspectedPlayer->getActiveSpec().getTalentPoints());
    data << uint8_t(inspectedPlayer->m_talentSpecsCount);
    data << uint8_t(inspectedPlayer->m_talentActiveSpec);
    for (uint8_t s = 0; s < inspectedPlayer->m_talentSpecsCount; ++s)
    {
#ifdef FT_DUAL_SPEC
        const PlayerSpec playerSpec = inspectedPlayer->m_specs[s];
#else
        const PlayerSpec playerSpec = inspectedPlayer->m_spec;
#endif

        uint8_t talentCount = 0;
        const auto talentCountPos = data.wpos();
        data << uint8_t(talentCount);

        const auto talentTabIds = getTalentTabPages(inspectedPlayer->getClass());
        for (uint8_t i = 0; i < 3; ++i)
        {
            const uint32_t talentTabId = talentTabIds[i];
            for (uint32_t j = 0; j < sTalentStore.getNumRows(); ++j)
            {
                const auto talentInfo = sTalentStore.lookupEntry(j);
                if (talentInfo == nullptr)
                    continue;

                if (talentInfo->TalentTree != talentTabId)
                    continue;

                int32_t talentMaxRank = -1;
                for (int32_t k = 4; k > -1; --k)
                {
                    if (talentInfo->RankID[k] != 0 && inspectedPlayer->hasSpell(talentInfo->RankID[k]))
                    {
                        talentMaxRank = k;
                        break;
                    }
                }

                if (talentMaxRank < 0)
                    continue;

                data << uint32_t(talentInfo->TalentID);
                data << uint8_t(talentMaxRank);

                ++talentCount;
            }
        }
        data.put<uint8_t>(talentCountPos, talentCount);

#ifdef FT_GLYPHS
        data << uint8_t(GLYPHS_COUNT);

        for (const auto& glyph : playerSpec.getGlyphs())
            data << uint16_t(glyph);
#endif
    }

    uint32_t slotMask = 0;
    const auto slotMaskPos = data.wpos();
    data << uint32_t(slotMask);

    auto itemInterface = inspectedPlayer->getItemInterface();
    for (uint32_t i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        const auto inventoryItem = itemInterface->GetInventoryItem(static_cast<uint16_t>(i));
        if (!inventoryItem)
            continue;

        slotMask |= (1 << i);

        data << uint32_t(inventoryItem->getEntry());

        uint16_t enchantMask = 0;
        const auto enchantMaskPos = data.wpos();

        data << uint16_t(enchantMask);

        for (uint8_t slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
        {
            const uint32_t enchantId = inventoryItem->getEnchantmentId(slot);
            if (!enchantId)
                continue;

            enchantMask |= (1 << slot);
            data << uint16_t(enchantId);
        }
        data.put<uint16_t>(enchantMaskPos, enchantMask);

        data << uint16_t(0);
        FastGUIDPack(data, inventoryItem->getCreatorGuid());
        data << uint32_t(0);
    }
    data.put<uint32_t>(slotMaskPos, slotMask);

#if VERSION_STRING >= Cata
    if (Guild* guild = sGuildMgr.getGuildById(inspectedPlayer->getGuildId()))
    {
        data << guild->getGUID();
        data << uint32_t(guild->getLevel());
        data << uint64_t(guild->getExperience());
        data << uint32_t(guild->getMembersCount());
    }
#endif

    SendPacket(&data);
#endif
}


void WorldSession::readAddonInfoPacket(ByteBuffer &recvPacket)
{
#if VERSION_STRING >= Cata
    if (recvPacket.rpos() + 4 > recvPacket.size())
        return;

    uint32_t recvSize;
    recvPacket >> recvSize;

    if (!recvSize)
        return;

    if (recvSize > 0xFFFFF)
    {
        sLogger.debug("recvSize {} too long", recvSize);
        return;
    }

    uLongf uSize = recvSize;

    uint32_t pos = static_cast<uint32_t>(recvPacket.rpos());

    ByteBuffer unpackedInfo;
    unpackedInfo.resize(recvSize);

    if (uncompress(unpackedInfo.contents(), &uSize, recvPacket.contents() + pos, static_cast<uLong>(recvPacket.size() - pos)) == Z_OK)
    {
        uint32_t addonsCount;
        unpackedInfo >> addonsCount;

        for (uint32_t i = 0; i < addonsCount; ++i)
        {
            std::string addonName;
            uint8_t enabledState;
            uint32_t crc;
            uint32_t unknown;

            if (unpackedInfo.rpos() + 1 > unpackedInfo.size())
                return;

            unpackedInfo >> addonName;

            unpackedInfo >> enabledState;
            unpackedInfo >> crc;
            unpackedInfo >> unknown;

            sLogger.debug("AddOn: {} (CRC: 0x{:x}) - enabled: 0x{:x} - Unknown2: 0x{:x}", addonName, crc, enabledState, unknown);
#if VERSION_STRING < Mop
            AddonEntry addon(addonName, enabledState, crc, 2, true);
#else
            AddonEntry addon(addonName, true, crc, 2, enabledState);
#endif

            SavedAddon const* savedAddon = sAddonMgr.getAddonInfoForAddonName(addonName);
            if (savedAddon)
            {
                if (addon.crc != savedAddon->crc)
                    sLogger.debug("Addon: {}: modified (CRC: 0x{:x}) - accountID {})", addon.name, savedAddon->crc, GetAccountId());
                else
                    sLogger.debug("Addon: {}: validated (CRC: 0x{:x}) - accountID {}", addon.name, savedAddon->crc, GetAccountId());
            }
            else
            {
                sAddonMgr.SaveAddon(addon);
                sLogger.debug("Addon: {}: unknown (CRC: 0x{:x}) - accountId {} (storing addon name and checksum to database)", addon.name, addon.crc, GetAccountId());
            }

            m_addonList.push_back(addon);
        }

        uint32_t addonTime;
        unpackedInfo >> addonTime;
    }
    else
    {
        sLogger.failure("Decompression of addon section of CMSG_AUTH_SESSION failed.");
    }
#endif

}

void WorldSession::sendAddonInfo()
{
#if VERSION_STRING >= Cata
#if VERSION_STRING < Mop
    WorldPacket data(SMSG_ADDON_INFO, 4);
    for (auto itr : m_addonList)
    {
        data << uint8_t(itr.state);

        uint8_t crcpub = itr.usePublicKeyOrCRC;
        data << uint8_t(crcpub);
        if (crcpub)
        {
            uint8_t usepk = (itr.crc != STANDARD_ADDON_CRC);    // standard addon CRC
            data << uint8_t(usepk);
            if (usepk)                                          // add public key if crc is wrong
            {
                sLogger.debug("AddOn: {}: CRC checksum mismatch: got 0x{:x} - expected 0x{:x} - sending pubkey to accountID {}",
                    itr.name, itr.crc, STANDARD_ADDON_CRC, GetAccountId());

                data.append(PublicKey, sizeof(PublicKey));
            }

            data << uint32_t(0);
        }

        data << uint8_t(0);
    }

    m_addonList.clear();

    std::list<BannedAddon> const* bannedAddons = sAddonMgr.getBannedAddonsList();
    data << uint32_t(bannedAddons->size());
    for (auto itr = bannedAddons->begin(); itr != bannedAddons->end(); ++itr)
    {
        data << uint32_t(itr->id);
        data.append(itr->nameMD5, sizeof(itr->nameMD5));
        data.append(itr->versionMD5, sizeof(itr->versionMD5));
        data << uint32_t(itr->timestamp);
        data << uint32_t(1); // banned?
    }

    SendPacket(&data);
#else
    WorldPacket data(SMSG_ADDON_INFO, 1000);

    std::list<BannedAddon> const* bannedAddons = sAddonMgr.getBannedAddonsList();

    data.writeBits(static_cast<uint32_t>(bannedAddons->size()), 18);
    data.writeBits(static_cast<uint32_t>(m_addonList.size()), 23);

    for (auto itr : m_addonList)
    {
        data.writeBit(0); // Has URL
        data.writeBit(itr.enabled);
        data.writeBit(!itr.usePublicKeyOrCRC);
    }

    data.flushBits();

    for (auto itr : m_addonList)
    {
        if (!itr.usePublicKeyOrCRC)
        {
            size_t pos = data.wpos();
            for (int i = 0; i < 256; i++)
                data << uint8_t(0);

            for (int i = 0; i < 256; i++)
                data.put(pos + publicKeyOrder[i], PublicKey[i]);
        }

        if (itr.enabled)
        {
            data << uint8_t(itr.enabled);
            data << uint64_t(0); // sch: normal value - uint32_t
        }

        data << uint8_t(itr.state);
    }

    m_addonList.clear();

    for (auto itr = bannedAddons->begin(); itr != bannedAddons->end(); ++itr)
    {
        data << uint32_t(itr->id);
        data << uint32_t(1); // banned?

        for (int32_t i = 0; i < 8; i++)
            data << uint32_t(0);

        data << uint32_t(itr->timestamp);
    }

    SendPacket(&data);
#endif
#endif
}

bool WorldSession::isAddonRegistered(const std::string& addon_name) const
{
#if VERSION_STRING >= Cata
    if (!isAddonMessageFiltered)
        return true;

    if (mRegisteredAddonPrefixesVector.empty())
        return false;

    auto itr = std::find(mRegisteredAddonPrefixesVector.begin(), mRegisteredAddonPrefixesVector.end(), addon_name);
    return itr != mRegisteredAddonPrefixesVector.end();
#else
    return false;
#endif
}

void WorldSession::handleUnregisterAddonPrefixesOpcode(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_UNREGISTER_ALL_ADDON_PREFIXES");

    mRegisteredAddonPrefixesVector.clear();
#endif
}

void WorldSession::handleAddonRegisteredPrefixesOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    uint32_t addonCount = recvPacket.readBits(25);

    if (addonCount > 64)
    {
        isAddonMessageFiltered = false;
        recvPacket.rfinish();
        return;
    }

    std::vector<uint8_t> nameLengths(addonCount);
    for (uint32_t i = 0; i < addonCount; ++i)
        nameLengths[i] = static_cast<uint8_t>(recvPacket.readBits(5));

    for (uint32_t i = 0; i < addonCount; ++i)
        mRegisteredAddonPrefixesVector.push_back(recvPacket.ReadString(nameLengths[i]));

    if (mRegisteredAddonPrefixesVector.size() > 64)
    {
        isAddonMessageFiltered = false;
        return;
    }

    isAddonMessageFiltered = true;
#endif
}

void WorldSession::handleReportOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_REPORT");

    uint8_t spam_type;                                      // 0 - mail, 1 - chat
    uint64_t spammer_guid;
    uint32_t unk1 = 0;
    uint32_t unk2 = 0;
    uint32_t unk3 = 0;
    uint32_t unk4 = 0;

    std::string description;
    recvPacket >> spam_type;                                // unk 0x01 const, may be spam type (mail/chat)
    recvPacket >> spammer_guid;                             // player guid

    switch (spam_type)
    {
        case 0:
        {
            recvPacket >> unk1;                             // const 0
            recvPacket >> unk2;                             // probably mail id
            recvPacket >> unk3;                             // const 0

            sLogger.debug("Received REPORT SPAM: type {}, guid {}, unk1 {}, unk2 {}, unk3 {}", spam_type, WoWGuid::getGuidLowPartFromUInt64(spammer_guid), unk1, unk2, unk3);

        } break;
        case 1:
        {
            recvPacket >> unk1;                             // probably language
            recvPacket >> unk2;                             // message type?
            recvPacket >> unk3;                             // probably channel id
            recvPacket >> unk4;                             // unk random value
            recvPacket >> description;                      // spam description string (messagetype, channel name, player name, message)

            sLogger.debug("Received REPORT SPAM: type {}, guid {}, unk1 {}, unk2 {}, unk3 {}, unk4 {}, message {}", spam_type, WoWGuid::getGuidLowPartFromUInt64(spammer_guid), unk1, unk2, unk3, unk4, description);

        } break;
    }

    // Complaint Received message
    WorldPacket data(SMSG_REPORT_RESULT, 1);
    data << uint8_t(0);                                     // 1 reset reported player 0 ignore
    data << uint8_t(0);

    SendPacket(&data);
#endif
}

void WorldSession::handleReportPlayerOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_REPORT_PLAYER {}", static_cast<uint32_t>(recvPacket.size()));

    uint8_t unk3 = 0;                                       // type
    uint8_t unk4 = 0;                                       // guid - 1
    uint32_t unk5 = 0;
    uint64_t unk6 = 0;
    uint32_t unk7 = 0;
    uint32_t unk8 = 0;

    std::string message;

    uint32_t length = recvPacket.readBits(9);               // length * 2
    recvPacket >> unk3;                                     // type
    recvPacket >> unk4;                                     // guid - 1?
    message = recvPacket.ReadString(length / 2);            // message
    recvPacket >> unk5;                                     // unk
    recvPacket >> unk6;                                     // unk
    recvPacket >> unk7;                                     // unk
    recvPacket >> unk8;                                     // unk

    switch (unk3)
    {
        case 0:     // chat spamming
            sLogger.debug("Chat spamming report for guid: {} received.", unk4 + 1);
            break;
        case 2:     // cheat
            recvPacket >> message;
            sLogger.debug("Cheat report for guid: {} received. Message {}", unk4 + 1, message);
            break;
        case 6:     // char name
            recvPacket >> message;
            sLogger.debug("char name report for guid: {} received. Message {}", unk4 + 1, message);
            break;
        case 12:     // guild name
            recvPacket >> message;
            sLogger.debug("guild name report for guid: {} received. Message {}", unk4 + 1, message);
            break;
        case 18:     // arena team name
            recvPacket >> message;
            sLogger.debug("arena team name report for guid: {} received. Message {}", unk4 + 1, message);
            break;
        case 20:     // chat language
            sLogger.debug("Chat language report for guid: {} received.", unk4 + 1);
            break;
        default:
            sLogger.debug("type is {}", unk3);
            break;
    }
#endif
}

void WorldSession::HandleMirrorImageOpcode(WorldPacket& recv_data)
{
    if (!_player->IsInWorld())
        return;

    sLogger.debug("Received CMG_GET_MIRRORIMAGE_DATA");

    uint64_t GUID;

    recv_data >> GUID;

    Unit* Image = _player->getWorldMap()->getUnit(GUID);
    if (Image == nullptr)
        return; // ups no unit found with that GUID on the map. Spoofed packet?

    if (Image->getCreatedByGuid() == 0)
        return;

    uint64_t CasterGUID = Image->getCreatedByGuid();
    Unit* Caster = _player->getWorldMap()->getUnit(CasterGUID);

    if (Caster == nullptr)
        return; // apperantly this mirror image mirrors nothing, poor lonely soul :(Maybe it's the Caster's ghost called Casper

    WorldPacket data(SMSG_MIRRORIMAGE_DATA, 68);

    data << uint64_t(GUID);
    data << uint32_t(Caster->getDisplayId());
    data << uint8_t(Caster->getRace());

    if (Caster->isPlayer())
    {
        if (const auto pcaster = dynamic_cast<Player*>(Caster))
        {
            data << uint8_t(pcaster->getGender());
            data << uint8_t(pcaster->getClass());

            // facial features
            data << uint8_t(pcaster->getSkinColor());
            data << uint8_t(pcaster->getFace());
            data << uint8_t(pcaster->getHairStyle());
            data << uint8_t(pcaster->getHairColor());
            data << uint8_t(pcaster->getFacialFeatures());

            if (pcaster->isInGuild())
                data << uint32_t(pcaster->getGuildId());
            else
                data << uint32_t(0);

            static const uint32_t imageitemslots[] =
            {
                EQUIPMENT_SLOT_HEAD,
                EQUIPMENT_SLOT_SHOULDERS,
                EQUIPMENT_SLOT_BODY,
                EQUIPMENT_SLOT_CHEST,
                EQUIPMENT_SLOT_WAIST,
                EQUIPMENT_SLOT_LEGS,
                EQUIPMENT_SLOT_FEET,
                EQUIPMENT_SLOT_WRISTS,
                EQUIPMENT_SLOT_HANDS,
                EQUIPMENT_SLOT_BACK,
                EQUIPMENT_SLOT_TABARD
            };

            for (uint8_t i = 0; i < 11; ++i)
            {
                Item* item = pcaster->getItemInterface()->GetInventoryItem(static_cast <int16_t> (imageitemslots[i]));
                if (item != nullptr)
                    data << uint32_t(item->getItemProperties()->DisplayInfoID);
                else
                    data << uint32_t(0);
            }
        }
    }
    else // do not send player data for creatures
    {
        data << uint8_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
        data << uint32_t(0);
    }

    SendPacket(&data);

    sLogger.debug("Sent SMSG_MIRRORIMAGE_DATA");
}

void WorldSession::sendClientCacheVersion(uint32_t version)
{
#if VERSION_STRING > TBC
    WorldPacket data(SMSG_CLIENTCACHE_VERSION, 4);
    data << uint32_t(version);
    SendPacket(&data);
#endif
}

void WorldSession::sendAccountDataTimes(uint32_t mask)
{
    SendPacket(SmsgAccountDataTimes(static_cast<uint32_t>(UNIXTIME), 1, mask, NUM_ACCOUNT_DATA_TYPES).serialise().get());
}

void WorldSession::sendMOTD()
{
    std::vector<std::string> motdLines;
    std::string str_motd = worldConfig.getMessageOfTheDay();
    std::string::size_type nextpos;

    std::string::size_type pos = 0;
    while ((nextpos = str_motd.find('@', pos)) != std::string::npos)
    {
        if (nextpos != pos)
            motdLines.push_back(str_motd.substr(pos, nextpos - pos));

        pos = nextpos + 1;
    }

    if (pos < str_motd.length())
        motdLines.push_back(str_motd.substr(pos));

#if VERSION_STRING > Classic
    SendPacket(SmsgMotd(motdLines).serialise().get());
#else
    for (const auto& line : motdLines)
        GetPlayer()->sendChatMessage(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, line.c_str());
#endif
}

void WorldSession::handleInstanceLockResponse(WorldPacket& recvPacket)
{
    uint8_t accept;
    recvPacket >> accept;

    if (!_player->hasPendingBind())
        return;

    if (accept)
        _player->bindToInstance();
    else
        _player->repopAtGraveyard(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId());

    _player->setPendingBind(0, 0);
}
