/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Loot.hpp"
#include "LootItem.hpp"
#include "LootRoll.hpp"
#include "Management/Group.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/Packets/SmsgLootAllPassed.h"
#include "Server/Packets/SmsgLootRoll.h"
#include "Server/Packets/SmsgLootRollWon.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/WorldSession.h"
#include "Utilities/Random.hpp"
#include "Utilities/TimeTracker.hpp"

using namespace AscEmu::Packets;

LootRoll::LootRoll(uint32_t timer, uint32_t groupcount, uint64_t guid, uint8_t slotid, uint32_t itemid, uint32_t randomsuffixid, uint32_t randompropertyid, WorldMap* mgr) :
    EventableObject(), _mgr(mgr), _groupcount(groupcount), _guid(guid), _slotid(slotid), _itemid(itemid),
    _randomsuffixid(randomsuffixid), _randompropertyid(randompropertyid), _remaining(groupcount)
{
    sEventMgr.AddEvent(this, &LootRoll::finalize, EVENT_LOOT_ROLL_FINALIZE, timer, 1, 0);
}

LootRoll::~LootRoll()
{
    sEventMgr.RemoveEvents(this);
}

void LootRoll::finalize()
{
    sEventMgr.RemoveEvents(this);
    // this we will have to finalize with groups types.. for now
    // we'll just assume need before greed. person with highest roll
    // in need gets the item.
    uint8_t highest = 0;
    int8_t hightype = -1;
    uint32_t playerLowGuid = 0;

    for (const auto& [lowPlrGuid, roll] : m_NeedRolls)
    {
        if (roll > highest)
        {
            highest = roll;
            playerLowGuid = lowPlrGuid;
            hightype = ROLL_NEED;
        }
    }

    if (highest == 0)
    {
        for (const auto& [lowPlrGuid, roll] : m_GreedRolls)
        {
            if (roll > highest)
            {
                highest = roll;
                playerLowGuid = lowPlrGuid;
                hightype = ROLL_GREED;
            }
        }
    }

    Creature* creature = nullptr;
    GameObject* gameObject = nullptr;

    Loot* pLoot = nullptr;
    WoWGuid wowGuid;
    wowGuid.Init(_guid);

    if (wowGuid.isUnit())
    {
        if (creature = _mgr->getCreature(wowGuid.getGuidLowPart()))
            pLoot = &creature->loot;
    }
    else if (wowGuid.isGameObject())
    {
        if (gameObject = _mgr->getGameObject(wowGuid.getGuidLowPart()))
        {
            if (gameObject->IsLootable())
            {
                GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(gameObject);
                pLoot = &pLGO->loot;
            }
        }
    }

    if (pLoot == nullptr)
    {
        return;
    }

    if (_slotid >= pLoot->items.size())
    {
        return;
    }

    const auto amt = pLoot->items.at(_slotid).count;
    if (amt == 0)
    {
        pLoot->items.at(_slotid).roll = nullptr;
        return;
    }

    Player* _player = playerLowGuid != 0 ? _mgr->getPlayer(playerLowGuid) : nullptr;
    if (_player == nullptr)
    {
        /* all passed */
        auto pitr = m_passRolls.cbegin();
        while (_player == nullptr && pitr != m_passRolls.cend())
            _player = _mgr->getPlayer((*(pitr++)));

        if (_player != nullptr)
        {
            if (_player->isInGroup())
                _player->getGroup()->SendPacketToAll(SmsgLootAllPassed(_guid, _groupcount, _itemid, _randomsuffixid, _randompropertyid).serialise().get());
            else
                _player->getSession()->SendPacket(SmsgLootAllPassed(_guid, _groupcount, _itemid, _randomsuffixid, _randompropertyid).serialise().get());
        }

        /* item can now be looted by anyone :) */
        pLoot->items.at(_slotid).is_passed = true;
        pLoot->items.at(_slotid).is_blocked = false;
        pLoot->items.at(_slotid).roll = nullptr;
        return;
    }

    if (_player->isInGroup())
        _player->getGroup()->SendPacketToAll(SmsgLootRollWon(_guid, _slotid, _itemid, _randomsuffixid, _randompropertyid, _player->getGuid(), highest, hightype).serialise().get());
    else
        _player->getSession()->SendPacket(SmsgLootRollWon(_guid, _slotid, _itemid, _randomsuffixid, _randompropertyid, _player->getGuid(), highest, hightype).serialise().get());

    LootItem const& item = _slotid >= pLoot->items.size() ? pLoot->quest_items[_slotid - pLoot->items.size()] : pLoot->items[_slotid];

    // Add Item to Player
    Item* newItem = _player->storeItem(&item);
    if (newItem == nullptr)
        return;

    if (creature != nullptr)
    {
        if (creature->IsInWorld() && creature->GetScript())
            creature->GetScript()->OnLootTaken(_player, item.itemproto);
    }
    else if (gameObject != nullptr)
    {
        if (gameObject->GetScript())
            gameObject->GetScript()->OnLootTaken(_player, item.itemproto);
    }
    // mark as looted
    pLoot->items.at(_slotid).count = 0;
    pLoot->items.at(_slotid).is_looted = true;

    pLoot->itemRemoved(_slotid);
    --pLoot->unlootedCount;
    pLoot->items.at(_slotid).roll = nullptr;
}

bool LootRoll::playerRolled(Player* player, uint8_t choice)
{
    // don't allow cheaters
    if (m_NeedRolls.find(player->getGuidLow()) != m_NeedRolls.cend() || m_GreedRolls.find(player->getGuidLow()) != m_GreedRolls.cend())
        return false;

    auto roll = static_cast<uint8_t>(Util::getRandomUInt(99) + 1);
    switch (choice)
    {
        case ROLL_PASS:
            m_passRolls.insert(player->getGuidLow());
            roll = 128;
            break;
        case ROLL_GREED:
            m_GreedRolls.insert({ player->getGuidLow(), roll });
            break;
        case ROLL_NEED:
            m_NeedRolls.insert({ player->getGuidLow(), roll });
            break;
        case ROLL_DISENCHANT:
            roll = 128;
            break;
    }

    if (player->isInGroup())
        player->getGroup()->SendPacketToAll(SmsgLootRoll(_guid, _slotid, player->getGuid(), _itemid, _randomsuffixid, _randompropertyid, roll, choice).serialise().get());
    else
        player->getSession()->SendPacket(SmsgLootRoll(_guid, _slotid, player->getGuid(), _itemid, _randomsuffixid, _randompropertyid, roll, choice).serialise().get());

    // check for early completion
    --_remaining;
    if (_remaining == 0)
    {
        // kill event early
        finalize();
        return true;
    }

    return false;
}
