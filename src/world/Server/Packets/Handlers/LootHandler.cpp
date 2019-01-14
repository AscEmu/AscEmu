/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgLoot.h"
#include "Server/Packets/SmsgLootMoneyNotify.h"
#include "Server/Packets/SmsgLootMasterList.h"
#include "Server/Packets/CmsgLootRelease.h"
#include "Server/Packets/SmsgLootReleaseResponse.h"
#include "Server/Packets/CmsgAutostoreLootItem.h"
#include "Server/Packets/CmsgLootMasterGive.h"
#include "Server/WorldSession.h"
#include "Objects/GameObject.h"
#include "Map/MapMgr.h"
#include "WoWGuid.h"
#include "Units/Creatures/Creature.h"
#include "Management/ItemInterface.h"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/SmsgLootRemoved.h"
#include "Spell/Definitions/LockTypes.h"
#include "Spell/Spell.Legacy.h"

using namespace AscEmu::Packets;

Loot* WorldSession::getItemLootFromHighGuidType(WoWGuid wowGuid)
{
    switch(wowGuid.getHigh())
    {
        case HighGuid::Unit:
        {
            if (auto creature = _player->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart()))
                return &creature->loot;
        }
        case HighGuid::GameObject:
        {
            if (auto gameObject = _player->GetMapMgr()->GetGameObject(wowGuid.getGuidLowPart()))
            {
                if (gameObject->IsLootable())
                    return &dynamic_cast<GameObject_Lootable*>(gameObject)->loot;
            }
        }
        case HighGuid::Item:
        {
            if (const auto item = _player->getItemInterface()->GetItemByGUID(wowGuid.GetOldGuid()))
                return item->loot;
        }
        case HighGuid::Player:
        {
            if (auto player = _player->GetMapMgr()->GetPlayer(wowGuid.getGuidLowPart()))
                return &player->loot;
        }
        default:
        {
            return nullptr;
        }
    }
}

void WorldSession::handleAutostoreLootItemOpcode(WorldPacket& recvPacket)
{
    CmsgAutostoreLootItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->interruptSpell();

    GameObject* lootGameObject = nullptr;
    Creature* lootCreature = nullptr;
    Item* lootItem = nullptr;

    WoWGuid wowGuid;
    wowGuid.Init(_player->GetLootGUID());

    auto loot = getItemLootFromHighGuidType(wowGuid);
    if (loot == nullptr)
        return;

    if (wowGuid.isUnit())
    {
        lootCreature = _player->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
        if (lootCreature == nullptr)
            return;
    }
    else if (wowGuid.isGameObject())
    {
        lootGameObject = _player->GetMapMgr()->GetGameObject(wowGuid.getGuidLowPart());
        if (lootGameObject == nullptr)
            return;
    }
    else if (wowGuid.isItem())
    {
        lootItem = _player->getItemInterface()->GetItemByGUID(wowGuid.GetOldGuid());
        if (lootItem == nullptr)
            return;
    }
    else if (wowGuid.isPlayer())
    {
        const auto player = _player->GetMapMgr()->GetPlayer(wowGuid.getGuidLowPart());
        if (player == nullptr)
            return;
    }

    if (srlPacket.slot >= loot->items.size())
    {
        LogDebugFlag(LF_OPCODE, "Player %s might be using a hack! (slot %d, size %d)", _player->getName().c_str(), srlPacket.slot, loot->items.size());
        return;
    }

    if (loot->items[srlPacket.slot].looted)
    {
        LogDebugFlag(LF_OPCODE, "Player %s GUID %u tried to loot an already looted item.", _player->getName().c_str(), _player->getGuidLow());
        return;
    }

    const uint32_t amt = loot->items.at(srlPacket.slot).iItemsCount;
    if (loot->items.at(srlPacket.slot).roll != nullptr)
        return;

    if (!loot->items.at(srlPacket.slot).ffa_loot)
    {
        if (amt == 0)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ALREADY_LOOTED);
            return;
        }
    }
    else
    {
        const auto itr = loot->items.at(srlPacket.slot).has_looted.find(_player->getGuidLow());

        if (loot->items.at(srlPacket.slot).has_looted.end() != itr)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ALREADY_LOOTED);
            return;
        }
    }

    const uint32_t itemId = loot->items.at(srlPacket.slot).item.itemproto->ItemId;
    const auto itemProperties = loot->items.at(srlPacket.slot).item.itemproto;

    if (const uint8_t error = _player->getItemInterface()->CanReceiveItem(itemProperties, 1))
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    if (lootGameObject)
        CALL_GO_SCRIPT_EVENT(lootGameObject, OnLootTaken)(_player, itemProperties);
    else if (lootCreature)
        CALL_SCRIPT_EVENT(lootCreature, OnLootTaken)(_player, itemProperties);

    auto add = _player->getItemInterface()->FindItemLessMax(itemId, amt, false);
    sHookInterface.OnLoot(_player, lootCreature, 0, itemId);
    if (add == nullptr)
    {
        const auto slotResult = _player->getItemInterface()->FindFreeInventorySlot(itemProperties);
        if (!slotResult.Result)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
            return;
        }

        LogDebugFlag(LF_OPCODE, "AutoLootItem");
        auto item = objmgr.CreateItem(itemId, _player);
        if (item == nullptr)
            return;

        item->setStackCount(amt);
        if (loot->items.at(srlPacket.slot).iRandomProperty != nullptr)
        {
            item->setRandomPropertiesId(loot->items.at(srlPacket.slot).iRandomProperty->ID);
            item->ApplyRandomProperties(false);
        }
        else if (loot->items.at(srlPacket.slot).iRandomSuffix != nullptr)
        {
            item->SetRandomSuffix(loot->items.at(srlPacket.slot).iRandomSuffix->id);
            item->ApplyRandomProperties(false);
        }

        if (_player->getItemInterface()->SafeAddItem(item, slotResult.ContainerSlot, slotResult.Slot))
        {
            sQuestMgr.OnPlayerItemPickup(_player, item);
            _player->sendItemPushResultPacket(
                false,
                true,
                true,
                slotResult.ContainerSlot,
                slotResult.Slot,
                1,
                item->getEntry(),
                item->getPropertySeed(),
                item->getRandomPropertiesId(),
                item->getStackCount()
            );
#if VERSION_STRING > TBC
            _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, item->getEntry(), 1, 0);
#endif
        }
        else
            item->DeleteMe();
    }
    else
    {
        add->setStackCount(add->getStackCount() + amt);
        add->m_isDirty = true;

        sQuestMgr.OnPlayerItemPickup(_player, add);
        _player->sendItemPushResultPacket(
            false,
            false,
            true,
            static_cast<uint8_t>(_player->getItemInterface()->GetBagSlotByGuid(add->getGuid())),
            0,
            amt,
            add->getEntry(),
            add->getPropertySeed(),
            add->getRandomPropertiesId(),
            add->getStackCount()
        );
#if VERSION_STRING > TBC
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, add->getEntry(), 1, 0);
#endif
    }

    if (!loot->items.at(srlPacket.slot).ffa_loot)
    {
        loot->items.at(srlPacket.slot).iItemsCount = 0;

        for (auto looterSet : loot->looters)
        {
            if (const auto plr = _player->GetMapMgr()->GetPlayer(looterSet))
                plr->GetSession()->SendPacket(SmsgLootRemoved(srlPacket.slot).serialise().get());
        }
    }
    else
    {
        loot->items.at(srlPacket.slot).has_looted.insert(_player->getGuidLow());
        _player->GetSession()->SendPacket(SmsgLootRemoved(srlPacket.slot).serialise().get());
    }

    if (lootItem != nullptr)
        loot->items[srlPacket.slot].looted = true;

    if (lootGameObject && lootGameObject->getEntry() == GO_FISHING_BOBBER)
    {
        int count = 0;
        for (const auto& itemFromLoot : loot->items)
            count += itemFromLoot.iItemsCount;

        if (!count)
            lootGameObject->ExpireAndDelete();
    }
}

Loot* WorldSession::getMoneyLootFromHighGuidType(WoWGuid wowGuid)
{
    switch (wowGuid.getHigh())
    {
        case HighGuid::Unit:
        {
            if (auto creature = _player->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart()))
                return &creature->loot;
        }
        case HighGuid::GameObject:
        {
            if (auto gameObject = _player->GetMapMgr()->GetGameObject(wowGuid.getGuidLowPart()))
            {
                if (gameObject->IsLootable())
                    return &dynamic_cast<GameObject_Lootable*>(gameObject)->loot;
            }
        }
        case HighGuid::Item:
        {
            if (const auto item = _player->getItemInterface()->GetItemByGUID(wowGuid.GetOldGuid()))
                return item->loot;
        }
        case HighGuid::Player:
        {
            if (auto player = _player->GetMapMgr()->GetPlayer(wowGuid.getGuidLowPart()))
                return &player->loot;
        }
        case HighGuid::Corpse:
        {
            if (auto corpse = objmgr.GetCorpse(wowGuid.getGuidLowPart()))
                return &corpse->loot;
        }
        default:
        {
            return nullptr;
        }
    }
}

void WorldSession::handleLootMoneyOpcode(WorldPacket& /*recvPacket*/)
{
    _player->interruptSpell();

    Unit* pt = nullptr;

    WoWGuid wowGuid;
    wowGuid.Init(_player->GetLootGUID());

    auto loot = getItemLootFromHighGuidType(wowGuid);
    if (loot == nullptr)
        return;

    if (wowGuid.isUnit())
    {
        Creature* pCreature = _player->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
        if (!pCreature)
            return;
        pt = pCreature;
    }
    else if (wowGuid.isPlayer())
    {
        Player* pPlayer = _player->GetMapMgr()->GetPlayer(wowGuid.getGuidLowPart());
        if (!pPlayer)
            return;

        pPlayer->bShouldHaveLootableOnCorpse = false;
        pt = pPlayer;
    }

    const uint32_t money = loot->gold;
    loot->gold = 0;

    // send clear money packet
    {
        WorldPacket data(1);
        data.SetOpcode(SMSG_LOOT_CLEAR_MONEY);

        for (auto looters : loot->looters)
        {
            if (const auto player = _player->GetMapMgr()->GetPlayer(looters))
                player->GetSession()->SendPacket(&data);
        }
    }

    if (!_player->InGroup())
    {
        if (money)
        {
            if (worldConfig.player.isGoldCapEnabled && (_player->getCoinage() + money) > worldConfig.player.limitGoldAmount)
            {
                _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            }
            else
            {
                _player->modCoinage(money);
#if VERSION_STRING > TBC
                _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY, money, 0, 0);
#endif
            }
            sHookInterface.OnLoot(_player, pt, money, 0);
        }
    }
    else
    {
        if (Group* party = _player->GetGroup())
        {
            std::vector<Player*> groupMembers;

            groupMembers.reserve(party->MemberCount());

            party->getLock().Acquire();
            for (uint32_t i = 0; i < party->GetSubGroupCount(); i++)
            {
                auto subGroup = party->GetSubGroup(i);
                for (auto groupMemberPlayerInfo : subGroup->getGroupMembers())
                {
                    if (groupMemberPlayerInfo->m_loggedInPlayer
                        && groupMemberPlayerInfo->m_loggedInPlayer->GetZoneId() == _player->GetZoneId()
                        && _player->GetInstanceID() == groupMemberPlayerInfo->m_loggedInPlayer->GetInstanceID())
                    {
                        groupMembers.push_back(groupMemberPlayerInfo->m_loggedInPlayer);
                    }
                }
            }
            party->getLock().Release();

            if (groupMembers.empty())
                return;

            const uint32_t sharedMoney = money / uint32_t(groupMembers.size());

            for (auto& player : groupMembers)
            {
                if (worldConfig.player.isGoldCapEnabled && (player->getCoinage() + sharedMoney) > worldConfig.player.limitGoldAmount)
                {
                    player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
                }
                else
                {
                    player->modCoinage(sharedMoney);
                    player->GetSession()->SendPacket(SmsgLootMoneyNotify(sharedMoney).serialise().get());

#if VERSION_STRING > TBC
                    player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY, sharedMoney, 0, 0);
#endif
                }
            }
        }
    }
}

void WorldSession::handleLootOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgLoot srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (_player->isDead())
        return;

    if (_player->isStealthed())
        _player->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);

    _player->interruptSpell();

    if (_player->isInvisible())
        _player->removeAllAurasByAuraEffect(SPELL_AURA_MOD_INVISIBILITY);

    std::vector<uint64_t> onlineGroupMembers;

    if (_player->InGroup() && !_player->m_bg)
    {
        if (auto group = _player->GetGroup())
        {
            if (group->GetMethod() == PARTY_LOOT_MASTER)
            {
                group->Lock();
                for (uint32_t i = 0; i < group->GetSubGroupCount(); ++i)
                {
                    if (auto subGroup = group->GetSubGroup(i))
                    {
                        for (auto groupMemberPlayerInfo : subGroup->getGroupMembers())
                        {
                            if (groupMemberPlayerInfo->m_loggedInPlayer && _player->GetZoneId() == groupMemberPlayerInfo->m_loggedInPlayer->GetZoneId())
                                onlineGroupMembers.push_back(groupMemberPlayerInfo->m_loggedInPlayer->getGuid());
                        }
                    }
                }
                group->Unlock();

                group->SendPacketToAll(SmsgLootMasterList(onlineGroupMembers).serialise().get());
            }
        }
    }
    _player->SendLoot(srlPacket.guid, LOOT_CORPSE, _player->GetMapId());
}

void WorldSession::handleLootReleaseOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgLootRelease srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    SendPacket(SmsgLootReleaseResponse(srlPacket.guid.GetOldGuid(), 1).serialise().get());

    _player->SetLootGUID(0);
    _player->removeUnitFlags(UNIT_FLAG_LOOTING);
    _player->m_currentLoot = 0;

    if (srlPacket.guid.isUnit())
    {
        Creature* creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
        if (creature == nullptr)
            return;

        creature->loot.looters.erase(_player->getGuidLow());
        if (creature->loot.gold <= 0)
        {
            for (auto& item : creature->loot.items)
            {
                if (item.iItemsCount > 0)
                {
                    const auto itemProperties = item.item.itemproto;
                    if (itemProperties->Class != 12)
                        return;
                    if (_player->HasQuestForItem(item.item.itemproto->ItemId))
                        return;
                }
            }
            creature->BuildFieldUpdatePacket(_player, UNIT_DYNAMIC_FLAGS, 0);

            if (!creature->Skinned)
            {
                if (lootmgr.IsSkinnable(creature->getEntry()))
                {
                    creature->BuildFieldUpdatePacket(_player, UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
                }
            }
        }
    }
    else if (srlPacket.guid.isGameObject())
    {
        GameObject* gameObject = _player->GetMapMgr()->GetGameObject(srlPacket.guid.getGuidLow());
        if (gameObject == nullptr)
            return;

        switch (gameObject->getGoType())
        {
            case GAMEOBJECT_TYPE_FISHINGNODE:
            {
                auto pLGO = dynamic_cast<GameObject_Lootable*>(gameObject);
                pLGO->loot.looters.erase(_player->getGuidLow());
                if (gameObject->IsInWorld())
                {
                    gameObject->RemoveFromWorld(true);
                }
                delete gameObject;
            }
            break;
            case GAMEOBJECT_TYPE_CHEST:
            {
                auto gameObjectLootable = dynamic_cast<GameObject_Lootable*>(gameObject);
                gameObjectLootable->loot.looters.erase(_player->getGuidLow());

                bool despawn = false;
                if (gameObject->GetGameObjectProperties()->chest.consumable == 1)
                    despawn = true;

                const uint32_t lootQuestId = sQuestMgr.GetGameObjectLootQuest(gameObject->getEntry());
                const uint32_t longDespawnTime = 900000 + Util::getRandomUInt(600000);
                const uint32_t despawnTime = lootQuestId ? 180000 + Util::getRandomUInt(180000) : longDespawnTime;
                const uint32_t despawnTimeInstanceCheck = lootQuestId ? 180000 + Util::getRandomUInt(180000) : IS_INSTANCE(gameObject->GetMapId()) ? 0 : longDespawnTime;

                const auto lockEntry = sLockStore.LookupEntry(gameObject->GetGameObjectProperties()->chest.lock_id);
                if (lockEntry != nullptr)
                {
                    for (uint32_t i = 0; i < LOCK_NUM_CASES; ++i)
                    {
                        if (lockEntry->locktype[i] != 0)
                        {
                            if (lockEntry->locktype[i] == 1)
                            {
                                if (despawn)
                                    gameObject->Despawn(0, despawnTime);
                                else
                                    gameObject->setState(GO_STATE_CLOSED);

                                return;
                            }

                            if (lockEntry->locktype[i] == 2)
                            {
                                if (lockEntry->lockmisc[i] == LOCKTYPE_MINING || lockEntry->lockmisc[i] == LOCKTYPE_HERBALISM)
                                {
                                    if (gameObjectLootable->HasLoot())
                                    {
                                        gameObject->setState(GO_STATE_CLOSED);
                                        return;
                                    }

                                    gameObject->Despawn(0, longDespawnTime);
                                    return;
                                }
                            }
                            else
                            {
                                if (gameObjectLootable->HasLoot())
                                {
                                    gameObject->setState(GO_STATE_CLOSED);
                                    return;
                                }
                                gameObject->Despawn(0, despawnTimeInstanceCheck);
                                return;
                            }
                        }
                        else
                        {
                            if (gameObjectLootable->HasLoot())
                            {
                                gameObject->setState(1);
                                return;
                            }
                            gameObject->Despawn(0, despawnTimeInstanceCheck);
                            return;
                        }
                    }
                }
                else
                {
                    if (gameObjectLootable->HasLoot())
                    {
                        gameObject->setState(GO_STATE_CLOSED);
                        return;
                    }

                    gameObject->Despawn(0, despawnTimeInstanceCheck);
                }
            }
            default:
                break;
        }
    }
    else if (srlPacket.guid.isCorpse())
    {
        if (auto corpse = objmgr.GetCorpse(srlPacket.guid.getGuidLow()))
            corpse->setDynamicFlags(0);
    }
    else if (srlPacket.guid.isPlayer())
    {
        if (auto player = objmgr.GetPlayer(srlPacket.guid.getGuidLow()))
        {
            player->bShouldHaveLootableOnCorpse = false;
            player->loot.items.clear();
            player->removeDynamicFlags(U_DYN_FLAG_LOOTABLE);
        }
    }
    else if (srlPacket.guid.isItem())
    {
        if (auto item = _player->getItemInterface()->GetItemByGUID(srlPacket.guid.GetOldGuid()))
        {
            if (item->loot != nullptr)
            {
                const auto itemsNotLooted = std::count_if(item->loot->items.begin(), item->loot->items.end(), ItemIsNotLooted());

                if (itemsNotLooted == 0 && item->loot->gold == 0)
                {
                    delete item->loot;
                    item->loot = nullptr;
                }
            }

            if (item->loot == nullptr)
                _player->getItemInterface()->RemoveItemAmtByGuid(srlPacket.guid.GetOldGuid(), 1);
        }
    }
    else
    {
        LogDebugFlag(LF_OPCODE, "Unhandled loot source object type in handleLootReleaseOpcode");
    }
}

void WorldSession::handleLootMasterGiveOpcode(WorldPacket& recvPacket)
{
    CmsgLootMasterGive srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (_player->GetGroup() == nullptr || _player->GetGroup()->GetLooter() != _player->getPlayerInfo())
        return;

    auto player = _player->GetMapMgr()->GetPlayer(srlPacket.playerGuid.getGuidLow());
    if (player == nullptr)
        return;

    if (_player->GetLootGUID() != srlPacket.creatureGuid.GetOldGuid())
        return;

    Creature* creature = nullptr;
    Loot* loot = nullptr;

    WoWGuid lootGuid;
    lootGuid.Init(_player->GetLootGUID());

    if (lootGuid.isUnit())
    {
        creature = _player->GetMapMgr()->GetCreature(srlPacket.creatureGuid.getGuidLow());
        if (creature == nullptr)
            return;

        loot = &creature->loot;
    }
    else if (lootGuid.isGameObject())
    {
        auto gameObject = _player->GetMapMgr()->GetGameObject(srlPacket.creatureGuid.getGuidLow());
        if (gameObject == nullptr)
            return;

        if (!gameObject->IsLootable())
            return;

        auto gameObjectLootable = dynamic_cast<GameObject_Lootable*>(gameObject);
        gameObject->setState(GO_STATE_OPEN);
        loot = &gameObjectLootable->loot;
    }

    if (srlPacket.slot >= loot->items.size())
    {
        LogDebugFlag(LF_OPCODE, "AutoLootItem: Player %s might be using a hack! (slot %u, size %d)", _player->getName().c_str(), srlPacket.slot, loot->items.size());
        return;
    }

    const uint32_t lootAmount = loot->items.at(srlPacket.slot).iItemsCount;

    if (!loot->items.at(srlPacket.slot).ffa_loot)
    {
        if (!lootAmount)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ALREADY_LOOTED);
            return;
        }
    }
    else
    {
        const auto looterFFA = loot->items.at(srlPacket.slot).has_looted.find(player->getGuidLow());
        if (loot->items.at(srlPacket.slot).has_looted.end() != looterFFA)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ALREADY_LOOTED);
            return;
        }
    }

    const uint32_t itemEntry = loot->items.at(srlPacket.slot).item.itemproto->ItemId;
    const auto itemProperties = loot->items.at(srlPacket.slot).item.itemproto;

    if (const uint8_t error = player->getItemInterface()->CanReceiveItem(itemProperties, 1))
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    if (creature)
        CALL_SCRIPT_EVENT(creature, OnLootTaken)(player, itemProperties);

    const auto slotResult = player->getItemInterface()->FindFreeInventorySlot(itemProperties);
    if (!slotResult.Result)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
        return;
    }

    auto item = objmgr.CreateItem(itemEntry, player);
    if (item == nullptr)
        return;

    item->setStackCount(lootAmount);
    if (loot->items.at(srlPacket.slot).iRandomProperty != nullptr)
    {
        item->setRandomPropertiesId(loot->items.at(srlPacket.slot).iRandomProperty->ID);
        item->ApplyRandomProperties(false);
    }
    else if (loot->items.at(srlPacket.slot).iRandomSuffix != nullptr)
    {
        item->SetRandomSuffix(loot->items.at(srlPacket.slot).iRandomSuffix->id);
        item->ApplyRandomProperties(false);
    }

    if (player->getItemInterface()->SafeAddItem(item, slotResult.ContainerSlot, slotResult.Slot))
    {
        player->sendItemPushResultPacket(false, true, true, slotResult.ContainerSlot, slotResult.Slot, 1, item->getEntry(), item->getPropertySeed(), item->getRandomPropertiesId(), item->getStackCount());
        sQuestMgr.OnPlayerItemPickup(player, item);
#if VERSION_STRING > TBC
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, item->getEntry(), 1, 0);
#endif
    }
    else
    {
        item->DeleteMe();
    }

    loot->items.at(srlPacket.slot).iItemsCount = 0;

    if (!loot->items.at(srlPacket.slot).ffa_loot)
    {
        loot->items.at(srlPacket.slot).iItemsCount = 0;

        for (auto looter : loot->looters)
        {
            if (const auto playerGuid = _player->GetMapMgr()->GetPlayer(looter))
                playerGuid->GetSession()->SendPacket(SmsgLootRemoved(srlPacket.slot).serialise().get());
        }
    }
    else
    {
        loot->items.at(srlPacket.slot).has_looted.insert(player->getGuidLow());
    }
}
