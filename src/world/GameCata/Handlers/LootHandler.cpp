/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "zlib.h"
#include "Map/MapMgr.h"
#include "Spell/SpellMgr.h"
#include "Map/WorldCreator.h"
#include "Spell/Definitions/LockTypes.h"

Loot* WorldSession::getLootFromHighGuidType(uint32_t highGuid)
{
    GameObject* pLootableGameObject = nullptr;
    Creature* pLootableCreature = nullptr;
    Item* pLootableSourceItem = nullptr;

    switch (highGuid)
    {
        case HIGHGUID_TYPE_UNIT:
        {
            pLootableCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(GetPlayer()->GetLootGUID()));
            if (pLootableCreature == nullptr)
                return nullptr;

            return &pLootableCreature->loot;
        }
        case HIGHGUID_TYPE_GAMEOBJECT:
        {
            pLootableGameObject = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(GetPlayer()->GetLootGUID()));
            if (pLootableGameObject == nullptr || !pLootableGameObject->IsLootable())
                return nullptr;

            GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(pLootableGameObject);
            return &pLGO->loot;
        }
        case HIGHGUID_TYPE_ITEM:
        {
            Item* pItem = _player->GetItemInterface()->GetItemByGUID(_player->GetLootGUID());
            if (pItem == nullptr)
                return nullptr;

            pLootableSourceItem = pItem;
            return pItem->loot;
        }
        case HIGHGUID_TYPE_PLAYER:
        {
            Player* pl = _player->GetMapMgr()->GetPlayer((uint32_t)GetPlayer()->GetLootGUID());
            if (pl == nullptr)
                return nullptr;

            return &pl->loot;
        }
        default:
            return nullptr;
    }
}

void WorldSession::HandleAutostoreLootItemOpcode(WorldPacket& recv_data)
{
    uint8_t lootSlot;
    recv_data >> lootSlot;

    if (_player->isCastingNonMeleeSpell())
        _player->interruptSpell();

    GameObject* pLootableGameObject = nullptr;
    Creature* pLootableCreature = nullptr;
    Item* pLootableSourceItem = nullptr;

    uint32_t guidtype = GET_TYPE_FROM_GUID(_player->GetLootGUID());
    Loot* pLoot = getLootFromHighGuidType(guidtype);

    if (guidtype == HIGHGUID_TYPE_UNIT)
    {
        pLootableCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(GetPlayer()->GetLootGUID()));
        if (pLootableCreature == nullptr)
            return;

    }
    else if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        pLootableGameObject = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(GetPlayer()->GetLootGUID()));
        if (pLootableGameObject == nullptr || !pLootableGameObject->IsLootable())
            return;
    }
    else if (guidtype == HIGHGUID_TYPE_ITEM)
    {
        Item* pItem = _player->GetItemInterface()->GetItemByGUID(_player->GetLootGUID());
        if (pItem == nullptr)
            return;

        pLootableSourceItem = pItem;
    }

    if (pLoot == nullptr)
        return;

    if (lootSlot >= pLoot->items.size())
    {
        LOG_DEBUG("Player %s might be using a hack! (slot %d, size %d)", GetPlayer()->GetName(), lootSlot, pLoot->items.size());
        return;
    }

    if (pLoot->items[lootSlot].looted)
    {
        LOG_DEBUG("Player %s GUID %u tried to loot an already looted item.", _player->GetName(), _player->GetLowGUID());
        return;
    }

    uint32_t lootItemAmount = pLoot->items.at(lootSlot).iItemsCount;
    if (pLoot->items.at(lootSlot).roll != NULL)
        return;

    if (!pLoot->items.at(lootSlot).ffa_loot)
    {
        if (!lootItemAmount)
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ALREADY_LOOTED);
            return;
        }
    }
    else
    {
        LooterSet::iterator itr = pLoot->items.at(lootSlot).has_looted.find(_player->GetLowGUID());
        if (pLoot->items.at(lootSlot).has_looted.end() != itr)
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ALREADY_LOOTED);
            return;
        }
    }

    uint32_t itemid = pLoot->items.at(lootSlot).item.itemproto->ItemId;
    ItemProperties const* it = pLoot->items.at(lootSlot).item.itemproto;

    uint8_t error = 0;
    if ((error = _player->GetItemInterface()->CanReceiveItem(it, 1)) != 0)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    Item* add = GetPlayer()->GetItemInterface()->FindItemLessMax(itemid, lootItemAmount, false);
    if (add == nullptr)
    {
        SlotResult slotresult = GetPlayer()->GetItemInterface()->FindFreeInventorySlot(it);
        if (!slotresult.Result)
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
            return;
        }

        LOG_DEBUG("AutoLootItem MISC");
        Item* item = objmgr.CreateItem(itemid, GetPlayer());
        if (item == nullptr)
            return;

        item->SetStackCount(lootItemAmount);
        if (pLoot->items.at(lootSlot).iRandomProperty != nullptr)
        {
            item->SetItemRandomPropertyId(pLoot->items.at(lootSlot).iRandomProperty->ID);
            item->ApplyRandomProperties(false);
        }
        else if (pLoot->items.at(lootSlot).iRandomSuffix != nullptr)
        {
            item->SetRandomSuffix(pLoot->items.at(lootSlot).iRandomSuffix->id);
            item->ApplyRandomProperties(false);
        }

        if (GetPlayer()->GetItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot))
        {
            sQuestMgr.OnPlayerItemPickup(GetPlayer(), item);
            _player->SendItemPushResult(false, true, true, true, slotresult.ContainerSlot, slotresult.Slot, 1, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());
            _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, item->GetEntry(), 1, 0);
        }
        else
            item->DeleteMe();
    }
    else
    {
        add->SetStackCount(add->GetStackCount() + lootItemAmount);
        add->m_isDirty = true;

        sQuestMgr.OnPlayerItemPickup(GetPlayer(), add);
        _player->SendItemPushResult(false, false, true, false, (uint8_t)_player->GetItemInterface()->GetBagSlotByGuid(add->GetGUID()), 0xFFFFFFFF, lootItemAmount, add->GetEntry(), add->GetItemRandomSuffixFactor(), add->GetItemRandomPropertyId(), add->GetStackCount());
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, add->GetEntry(), 1, 0);
    }

    if (!pLoot->items.at(lootSlot).ffa_loot)
    {
        pLoot->items.at(lootSlot).iItemsCount = 0;

        WorldPacket data(SMSG_LOOT_REMOVED, 1);
        data << uint8_t(lootSlot);
        for (LooterSet::iterator itr = pLoot->looters.begin(); itr != pLoot->looters.end(); ++itr)
        {
            if (Player* plr = _player->GetMapMgr()->GetPlayer(*itr))
                plr->GetSession()->SendPacket(&data);
        }
    }
    else
    {
        pLoot->items.at(lootSlot).has_looted.insert(_player->GetLowGUID());
        WorldPacket data(SMSG_LOOT_REMOVED, 1);
        data << uint8_t(lootSlot);
        _player->GetSession()->SendPacket(&data);
    }

        if (pLootableGameObject)
        {
            CALL_GO_SCRIPT_EVENT(pLootableGameObject, OnLootTaken)(_player, it);

            if (pLootableGameObject->GetEntry() == GO_FISHING_BOBBER)
            {
                uint32_t count = 0;
                for (std::vector<__LootItem>::iterator itr = pLoot->items.begin(); itr != pLoot->items.end(); ++itr)
                    count += (*itr).iItemsCount;

                if (count == 0)
                    pLootableGameObject->ExpireAndDelete();
            }
        }

        if (pLootableCreature)
        {
            CALL_SCRIPT_EVENT(pLootableCreature, OnLootTaken)(_player, it);
            sHookInterface.OnLoot(_player, pLootableCreature, 0, itemid);
        }

        if (pLootableSourceItem)
            pLoot->items[lootSlot].looted = true;

}

void WorldSession::HandleLootMoneyOpcode(WorldPacket& /*recvData*/)
{
    uint64_t lootguid = GetPlayer()->GetLootGUID();
    if (lootguid == 0)
        return;

    if (_player->isCastingNonMeleeSpell())
        _player->interruptSpell();

    Unit* pt = nullptr;
    uint32_t guidtype = GET_TYPE_FROM_GUID(lootguid);
    Loot* pLoot = getLootFromHighGuidType(guidtype);

    if (guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(lootguid));
        if (pCreature == nullptr)
            return;

        pt = pCreature;
    }

    if (pLoot == nullptr)
        return;

    uint32_t money = pLoot->gold;

    pLoot->gold = 0;
    WorldPacket data(SMSG_LOOT_CLEAR_MONEY, 1);

    for (LooterSet::iterator itr = pLoot->looters.begin(); itr != pLoot->looters.end(); ++itr)
    {
        Player* plr;
        if ((plr = _player->GetMapMgr()->GetPlayer(*itr)) != nullptr)
            plr->GetSession()->SendPacket(&data);
    }

    if (!_player->InGroup())
    {
        if (money)
        {
            if (sWorld.settings.player.isGoldCapEnabled && (GetPlayer()->GetGold() + money) > sWorld.settings.player.limitGoldAmount)
            {
                GetPlayer()->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            }
            else
            {
                GetPlayer()->ModGold(money);
                GetPlayer()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY, money, 0, 0);
            }
            sHookInterface.OnLoot(_player, pt, money, 0);
        }
    }
    else
    {
        Group* party = _player->GetGroup();
        if (party)
        {
            std::vector<Player*> targets;
            targets.reserve(party->MemberCount());

            GroupMembersSet::iterator itr;
            SubGroup* sgrp;
            party->getLock().Acquire();
            for (uint32_t i = 0; i < party->GetSubGroupCount(); i++)
            {
                sgrp = party->GetSubGroup(i);
                for (itr = sgrp->GetGroupMembersBegin(); itr != sgrp->GetGroupMembersEnd(); ++itr)
                {
                    if ((*itr)->m_loggedInPlayer && (*itr)->m_loggedInPlayer->GetZoneId() == _player->GetZoneId() && _player->GetInstanceID() == (*itr)->m_loggedInPlayer->GetInstanceID())
                        targets.push_back((*itr)->m_loggedInPlayer);
                }
            }
            party->getLock().Release();

            if (!targets.size())
                return;

            uint32_t share = money / uint32_t(targets.size());

            WorldPacket pkt(SMSG_LOOT_MONEY_NOTIFY);
            pkt << share;

            for (std::vector<Player*>::iterator itr2 = targets.begin(); itr2 != targets.end(); ++itr2)
            {
                if (sWorld.settings.player.isGoldCapEnabled && ((*itr2)->GetGold() + share) > sWorld.settings.player.limitGoldAmount)
                {
                    (*itr2)->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
                }
                else
                {
                    (*itr2)->ModGold(share);
                    (*itr2)->GetSession()->SendPacket(&pkt);
                    (*itr2)->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY, share, 0, 0);
                }
            }
        }
    }
}

void WorldSession::HandleLootOpcode(WorldPacket& recv_data)
{
    uint64_t guid;
    recv_data >> guid;

    if (guid == 0)
        return;

    if (_player->IsDead())
        return;

    if (_player->IsStealth())
        _player->RemoveStealth();

    if (_player->isCastingNonMeleeSpell())
        _player->interruptSpell();

    if (_player->IsInvisible())
        _player->RemoveInvisibility();


    if (_player->InGroup() && !_player->m_bg)
    {
        Group* party = _player->GetGroup();
        if (party)
        {
            if (party->GetMethod() == PARTY_LOOT_MASTER)
            {
                WorldPacket data(SMSG_LOOT_MASTER_LIST, 330);
                data << uint8_t(party->MemberCount());
                uint32_t real_count = 0;
                party->Lock();
                for (uint32_t i = 0; i < party->GetSubGroupCount(); ++i)
                {
                    SubGroup* s = party->GetSubGroup(i);
                    if (s != nullptr)
                    {
                        for (GroupMembersSet::iterator itr = s->GetGroupMembersBegin(); itr != s->GetGroupMembersEnd(); ++itr)
                        {
                            if ((*itr)->m_loggedInPlayer && _player->GetZoneId() == (*itr)->m_loggedInPlayer->GetZoneId())
                            {
                                data << (*itr)->m_loggedInPlayer->GetGUID();
                                ++real_count;
                            }
                        }
                    }
                }
                party->Unlock();
                *(uint8_t*)&data.contents()[0] = static_cast<uint8_t>(real_count);

                party->SendPacketToAll(&data);
            }
        }
    }
    _player->SendLoot(guid, LOOT_CORPSE, _player->GetMapId());
}


void WorldSession::HandleLootReleaseOpcode(WorldPacket& recv_data)
{
    uint64_t guid;
    recv_data >> guid;

    WorldPacket data(SMSG_LOOT_RELEASE_RESPONSE, 9);
    data << guid;
    data << uint8_t(1);
    SendPacket(&data);

    _player->SetLootGUID(0);
    _player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);
    _player->m_currentLoot = 0;

    if (GET_TYPE_FROM_GUID(guid) == HIGHGUID_TYPE_UNIT)
    {
        Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
        if (pCreature == nullptr)
            return;

        pCreature->loot.looters.erase(_player->GetLowGUID());
        if (pCreature->loot.gold <= 0)
        {
            for (std::vector<__LootItem>::iterator i = pCreature->loot.items.begin(); i != pCreature->loot.items.end(); ++i)
            {
                if (i->iItemsCount > 0)
                {
                    ItemProperties const* proto = i->item.itemproto;
                    if (proto->Class != 12)
                        return;
                    if (_player->HasQuestForItem(i->item.itemproto->ItemId))
                        return;
                }
            }

            pCreature->BuildFieldUpdatePacket(_player, UNIT_DYNAMIC_FLAGS, 0);

            if (!pCreature->Skinned)
            {
                if (lootmgr.IsSkinnable(pCreature->GetEntry()))
                {
                    pCreature->BuildFieldUpdatePacket(_player, UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
                }
            }
        }
    }
    else if (GET_TYPE_FROM_GUID(guid) == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* pGO = _player->GetMapMgr()->GetGameObject((uint32_t)guid);
        if (pGO == nullptr)
            return;

        switch (pGO->GetType())
        {
            case GAMEOBJECT_TYPE_FISHINGNODE:
            {
                GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(pGO);
                pLGO->loot.looters.erase(_player->GetLowGUID());
                if (pGO->IsInWorld())
                {
                    pGO->RemoveFromWorld(true);
                }
                delete pGO;
            }
            break;
            case GAMEOBJECT_TYPE_CHEST:
            {
                GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(pGO);
                pLGO->loot.looters.erase(_player->GetLowGUID());

                bool despawn = false;
                if (pGO->GetGameObjectProperties()->chest.consumable == 1)
                    despawn = true;

                auto pLock = sLockStore.LookupEntry(pGO->GetGameObjectProperties()->chest.lock_id);
                if (pLock != nullptr)
                {
                    for (uint32_t i = 0; i < LOCK_NUM_CASES; ++i)
                    {
                        if (pLock->locktype[i] != 0)
                        {
                            if (pLock->locktype[i] == 1)
                            {
                                if (despawn)
                                    pGO->Despawn(0, (sQuestMgr.GetGameObjectLootQuest(pGO->GetEntry()) ? 180000 + (Util::getRandomUInt(180000)) : 900000 + (Util::getRandomUInt(600000))));
                                else
                                    pGO->SetState(GO_STATE_CLOSED);

                                return;
                            }
                            else if (pLock->locktype[i] == 2)
                            {
                                if (pLock->lockmisc[i] == LOCKTYPE_MINING || pLock->lockmisc[i] == LOCKTYPE_HERBALISM)
                                {
                                    if (pLGO->HasLoot())
                                    {
                                        pGO->SetState(GO_STATE_CLOSED);
                                        return;
                                    }

                                    pGO->Despawn(0, 900000 + (Util::getRandomUInt(600000)));
                                    return;
                                }
                            }
                            else
                            {
                                if (pLGO->HasLoot())
                                {
                                    pGO->SetState(GO_STATE_CLOSED);
                                    return;
                                }
                                pGO->Despawn(0, sQuestMgr.GetGameObjectLootQuest(pGO->GetEntry()) ? 180000 + (Util::getRandomUInt(180000)) : (IS_INSTANCE(pGO->GetMapId()) ? 0 : 900000 + (Util::getRandomUInt(600000))));
                                return;
                            }
                        }
                        else
                        {
                            if (pLGO->HasLoot())
                            {
                                pGO->SetState(GO_STATE_CLOSED);
                                return;
                            }
                            pGO->Despawn(0, sQuestMgr.GetGameObjectLootQuest(pGO->GetEntry()) ? 180000 + (Util::getRandomUInt(180000)) : (IS_INSTANCE(pGO->GetMapId()) ? 0 : 900000 + (Util::getRandomUInt(600000))));
                            return;
                        }
                    }
                }
                else
                {
                    if (pLGO->HasLoot())
                    {
                        pGO->SetState(GO_STATE_CLOSED);
                        return;
                    }
                    pGO->Despawn(0, sQuestMgr.GetGameObjectLootQuest(pGO->GetEntry()) ? 180000 + (Util::getRandomUInt(180000)) : (IS_INSTANCE(pGO->GetMapId()) ? 0 : 900000 + (Util::getRandomUInt(600000))));

                    return;

                }
            }
            default:
                break;
        }
    }
    else if (GET_TYPE_FROM_GUID(guid) == HIGHGUID_TYPE_CORPSE)
    {
        Corpse* pCorpse = objmgr.GetCorpse((uint32_t)guid);
        if (pCorpse)
            pCorpse->setUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS, 0);
    }
    else if (GET_TYPE_FROM_GUID(guid) == HIGHGUID_TYPE_PLAYER)
    {
        Player* plr = objmgr.GetPlayer((uint32_t)guid);
        if (plr)
        {
            plr->bShouldHaveLootableOnCorpse = false;
            plr->loot.items.clear();
            plr->RemoveFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_LOOTABLE);
        }
    }
    else if (GET_TYPE_FROM_GUID(guid) == HIGHGUID_TYPE_ITEM)
    {
        Item* item = _player->GetItemInterface()->GetItemByGUID(guid);
        if (item == nullptr)
            return;

        if (item->loot != nullptr)
        {
            uint32_t itemsNotLooted =
                static_cast<uint32_t>(std::count_if(item->loot->items.begin(), item->loot->items.end(), ItemIsNotLooted()));

            if ((itemsNotLooted == 0) && (item->loot->gold == 0))
            {
                delete item->loot;
                item->loot = nullptr;
            }
        }

        if (item->loot == nullptr)
            _player->GetItemInterface()->RemoveItemAmtByGuid(guid, 1);
    }
    else
        LOG_DEBUG("Unhandled loot source object type in HandleLootReleaseOpcode");
}

void WorldSession::HandleLootMasterGiveOpcode(WorldPacket& recv_data)
{
    uint32_t itemid = 0;
    uint32_t amt = 1;
    uint8_t error = 0;
    SlotResult slotresult;

    Creature* pCreature = nullptr;
    GameObject* pGameObject = nullptr;
    Object* pObj = nullptr;

    uint64_t creatureguid;
    uint64_t target_playerguid;
    uint8_t slotid;

    recv_data >> creatureguid;
    recv_data >> slotid;
    recv_data >> target_playerguid;

    if (_player->GetGroup() == nullptr || _player->GetGroup()->GetLooter() != _player->m_playerInfo)
        return;

    Player* player = _player->GetMapMgr()->GetPlayer((uint32_t)target_playerguid);
    if (player == nullptr)
        return;

    if (_player->GetLootGUID() != creatureguid)
        return;

    uint32_t guidtype = GET_TYPE_FROM_GUID(GetPlayer()->GetLootGUID());
    Loot* pLoot = getLootFromHighGuidType(guidtype);
    if (GET_TYPE_FROM_GUID(GetPlayer()->GetLootGUID()) == HIGHGUID_TYPE_UNIT)
    {
        pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(creatureguid));
        if (pCreature == nullptr)
            return;

    }
    else if (GET_TYPE_FROM_GUID(GetPlayer()->GetLootGUID()) == HIGHGUID_TYPE_GAMEOBJECT)
    {
        pGameObject = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(creatureguid));
        if (pGameObject == nullptr || !pGameObject->IsLootable())
            return;

        GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(pGameObject);
        pGameObject->SetState(GO_STATE_OPEN);
        pLoot = &pLGO->loot;
    }


    if (pLoot == nullptr)
        return;

    if (pCreature)
        pObj = pCreature;
    else
        pObj = pGameObject;

    if (slotid >= pLoot->items.size())
    {
        LOG_DEBUG("AutoLootItem: Player %s might be using a hack! (slot %d, size %d)", GetPlayer()->GetName(), slotid, pLoot->items.size());
        return;
    }

    amt = pLoot->items.at(slotid).iItemsCount;

    if (!pLoot->items.at(slotid).ffa_loot)
    {
        if (!amt)
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ALREADY_LOOTED);
            return;
        }
    }
    else
    {
        LooterSet::iterator itr = pLoot->items.at(slotid).has_looted.find(player->GetLowGUID());

        if (pLoot->items.at(slotid).has_looted.end() != itr)
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ALREADY_LOOTED);
            return;
        }
    }

    itemid = pLoot->items.at(slotid).item.itemproto->ItemId;
    ItemProperties const* it = pLoot->items.at(slotid).item.itemproto;

    if ((error = player->GetItemInterface()->CanReceiveItem(it, 1)) != 0)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    if (pCreature)
        CALL_SCRIPT_EVENT(pCreature, OnLootTaken)(player, it);


    slotresult = player->GetItemInterface()->FindFreeInventorySlot(it);
    if (!slotresult.Result)
    {
        GetPlayer()->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
        return;
    }

    Item* item = objmgr.CreateItem(itemid, player);
    if (item == nullptr)
        return;

    item->SetStackCount(amt);
    if (pLoot->items.at(slotid).iRandomProperty != nullptr)
    {
        item->SetItemRandomPropertyId(pLoot->items.at(slotid).iRandomProperty->ID);
        item->ApplyRandomProperties(false);
    }
    else if (pLoot->items.at(slotid).iRandomSuffix != nullptr)
    {
        item->SetRandomSuffix(pLoot->items.at(slotid).iRandomSuffix->id);
        item->ApplyRandomProperties(false);
    }

    if (player->GetItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot))
    {
        player->SendItemPushResult(false, true, true, true, slotresult.ContainerSlot, slotresult.Slot, 1, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());
        sQuestMgr.OnPlayerItemPickup(player, item);
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, item->GetEntry(), 1, 0);
    }
    else
        item->DeleteMe();

    pLoot->items.at(slotid).iItemsCount = 0;

    if (!pLoot->items.at(slotid).ffa_loot)
    {
        pLoot->items.at(slotid).iItemsCount = 0;

        WorldPacket data(SMSG_LOOT_REMOVED, 1);
        data << slotid;
        Player* plr;
        for (LooterSet::iterator itr = pLoot->looters.begin(); itr != pLoot->looters.end(); ++itr)
        {
            if ((plr = _player->GetMapMgr()->GetPlayer(*itr)) != nullptr)
                plr->GetSession()->SendPacket(&data);
        }
    }
    else
    {
        pLoot->items.at(slotid).has_looted.insert(player->GetLowGUID());
    }
}

void WorldSession::HandleLootRollOpcode(WorldPacket& recv_data)
{
    uint64_t creatureguid;
    uint32_t slotid;
    uint8_t choice;

    recv_data >> creatureguid;
    recv_data >> slotid;
    recv_data >> choice;

    LootRoll* li = nullptr;

    uint32_t guidtype = GET_TYPE_FROM_GUID(creatureguid);
    if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* pGO = _player->GetMapMgr()->GetGameObject((uint32_t)creatureguid);
        if (pGO == nullptr || pGO->IsLootable())
            return;

        GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(pGO);
        if ((slotid >= pLGO->loot.items.size()) || (pLGO->loot.items.size() == 0))
            return;

        if (pGO->GetType() == GAMEOBJECT_TYPE_CHEST)
            li = pLGO->loot.items[slotid].roll;
    }
    else if (guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(creatureguid));
        if (pCreature == nullptr)
            return;

        if (slotid >= pCreature->loot.items.size() || pCreature->loot.items.size() == 0)
            return;

        li = pCreature->loot.items[slotid].roll;
    }
    else
        return;

    if (li == nullptr)
        return;

    li->PlayerRolled(_player, choice);
}
