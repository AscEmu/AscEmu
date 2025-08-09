/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Packets/CmsgLoot.h"
#include "Server/Packets/SmsgLootMoneyNotify.h"
#include "Server/Packets/SmsgLootMasterList.h"
#include "Server/Packets/CmsgLootRelease.h"
#include "Server/Packets/SmsgLootReleaseResponse.h"
#include "Server/Packets/CmsgAutostoreLootItem.h"
#include "Server/Packets/CmsgLootMasterGive.h"
#include "Server/WorldSession.h"
#include "Objects/GameObject.h"
#include "Map/Management/MapMgr.hpp"
#include "WoWGuid.h"
#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Management/ItemInterface.h"
#include "Management/Loot/LootMgr.hpp"
#include "Management/Loot/LootRoll.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Item.hpp"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/Script/HookInterface.hpp"
#include "Spell/Spell.hpp"
#include "Storage/MySQLDataStore.hpp"

using namespace AscEmu::Packets;

Loot* WorldSession::getItemLootFromHighGuidType(WoWGuid wowGuid)
{
    switch(wowGuid.getHigh())
    {
        case HighGuid::Unit:
        {
            if (auto creature = _player->getWorldMap()->getCreature(wowGuid.getGuidLowPart()))
                return &creature->loot;

            return nullptr;
        }
        case HighGuid::GameObject:
        {
            if (auto gameObject = _player->getWorldMap()->getGameObject(wowGuid.getGuidLowPart()))
            {
                if (gameObject->IsLootable())
                    return &dynamic_cast<GameObject_Lootable*>(gameObject)->loot;
            }

            return nullptr;
        }
        case HighGuid::Item:
        {
            if (const auto item = _player->getItemInterface()->GetItemByGUID(wowGuid.getRawGuid()))
                return item->m_loot.get();

            return nullptr;
        }
        case HighGuid::Player:
        {
            if (auto player = _player->getWorldMap()->getPlayer(wowGuid.getGuidLowPart()))
                return &player->loot;

            return nullptr;
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
    wowGuid.Init(_player->getLootGuid());

    auto loot = getItemLootFromHighGuidType(wowGuid);
    if (loot == nullptr)
        return;

    if (wowGuid.isUnit())
    {
        lootCreature = _player->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
        if (lootCreature == nullptr)
            return;
    }
    else if (wowGuid.isGameObject())
    {
        lootGameObject = _player->getWorldMap()->getGameObject(wowGuid.getGuidLowPart());
        if (lootGameObject == nullptr)
            return;
    }
    else if (wowGuid.isItem())
    {
        lootItem = _player->getItemInterface()->GetItemByGUID(wowGuid.getRawGuid());
        if (lootItem == nullptr)
            return;
    }
    else if (wowGuid.isPlayer())
    {
        const auto player = _player->getWorldMap()->getPlayer(wowGuid.getGuidLowPart());
        if (player == nullptr)
            return;
    }

    // Add item
    auto item = _player->storeNewLootItem(srlPacket.slot, loot);

    if (!item)
        return;

    ItemProperties const* proto = sMySQLStore.getItemProperties(item->getEntry());

    if (lootGameObject)
    {
        if (lootGameObject->GetScript())
            lootGameObject->GetScript()->OnLootTaken(_player, proto);
    }
    else if (lootCreature)
    {
        if (lootCreature->IsInWorld() && lootCreature->isCreature() && lootCreature->GetScript())
            lootCreature->GetScript()->OnLootTaken(_player, proto);
    }

    sHookInterface.OnLoot(_player, lootCreature, 0, item->getEntry());

    if (lootGameObject && lootGameObject->getEntry() == GO_FISHING_BOBBER)
    {
        int count = 0;
        for (const auto& itemFromLoot : loot->items)
            count += itemFromLoot.count;

        if (!count)
            lootGameObject->expireAndDelete();
    }

    if (loot->isLooted() && wowGuid.isItem())
        _player->getSession()->doLootRelease(wowGuid);
}

Loot* WorldSession::getMoneyLootFromHighGuidType(WoWGuid wowGuid)
{
    switch (wowGuid.getHigh())
    {
        case HighGuid::Unit:
        {
            if (auto creature = _player->getWorldMap()->getCreature(wowGuid.getGuidLowPart()))
                return &creature->loot;

            return nullptr;
        }
        case HighGuid::GameObject:
        {
            if (auto gameObject = _player->getWorldMap()->getGameObject(wowGuid.getGuidLowPart()))
            {
                if (gameObject->IsLootable())
                    return &dynamic_cast<GameObject_Lootable*>(gameObject)->loot;
            }

            return nullptr;
        }
        case HighGuid::Item:
        {
            if (const auto item = _player->getItemInterface()->GetItemByGUID(wowGuid.getRawGuid()))
                return item->m_loot.get();

            return nullptr;
        }
        case HighGuid::Player:
        {
            if (auto player = _player->getWorldMap()->getPlayer(wowGuid.getGuidLowPart()))
                return &player->loot;

            return nullptr;
        }
        case HighGuid::Corpse:
        {
            if (auto corpse = sObjectMgr.getCorpseByGuid(wowGuid.getGuidLowPart()))
                return &corpse->loot;

            return nullptr;
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
    wowGuid.Init(_player->getLootGuid());

    auto loot = getItemLootFromHighGuidType(wowGuid);
    if (loot == nullptr)
        return;

    if (wowGuid.isUnit())
    {
        Creature* pCreature = _player->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
        if (!pCreature)
            return;
        pt = pCreature;
    }
    else if (wowGuid.isPlayer())
    {
        Player* pPlayer = _player->getWorldMap()->getPlayer(wowGuid.getGuidLowPart());
        if (!pPlayer)
            return;

        pPlayer->m_lootableOnCorpse = false;
        pt = pPlayer;
    }

    const uint32_t money = loot->gold;

    // Notify Looters
    loot->moneyRemoved();

    // Clear Money
    loot->gold = 0;

    // Delete container if empty
    if (loot->isLooted() && wowGuid.isItem())
        _player->getSession()->doLootRelease(wowGuid);

    if (!_player->isInGroup())
    {
        if (money)
        {
            if (worldConfig.player.isGoldCapEnabled && (_player->getCoinage() + money) > worldConfig.player.limitGoldAmount)
            {
                _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            }
            else
            {
                _player->modCoinage(money);
                _player->getSession()->SendPacket(SmsgLootMoneyNotify(money, 1).serialise().get());
#if VERSION_STRING > TBC
                _player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY, money);
#endif
            }
            sHookInterface.OnLoot(_player, pt, money, 0);
        }
    }
    else
    {
        if (const auto group = _player->getGroup())
        {
            std::vector<Player*> groupMembers;

            groupMembers.reserve(group->MemberCount());

            group->getLock().lock();
            for (uint32_t i = 0; i < group->GetSubGroupCount(); i++)
            {
                auto subGroup = group->GetSubGroup(i);
                for (auto groupMemberPlayerInfo : subGroup->getGroupMembers())
                {
                    if (Player* loggedInPlayer = sObjectMgr.getPlayer(groupMemberPlayerInfo->guid))
                        if (loggedInPlayer->getZoneId() == _player->getZoneId() && _player->GetInstanceID() == loggedInPlayer->GetInstanceID())
                            groupMembers.push_back(loggedInPlayer);
                }
            }
            group->getLock().unlock();

            if (groupMembers.empty())
                return;

            const uint32_t sharedMoney = money / uint32_t(groupMembers.size());

            // TODO: money is given to group members even if they are not near the looter
            for (auto& player : groupMembers)
            {
                if (worldConfig.player.isGoldCapEnabled && (player->getCoinage() + sharedMoney) > worldConfig.player.limitGoldAmount)
                {
                    player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
                }
                else
                {
                    player->modCoinage(sharedMoney);
                    player->getSession()->SendPacket(SmsgLootMoneyNotify(sharedMoney, groupMembers.size() <= 1).serialise().get());

#if VERSION_STRING > TBC
                    player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY, sharedMoney);
#endif
                }
            }
        }
    }
}

void WorldSession::handleLootOpcode(WorldPacket& recvPacket)
{
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

    if (_player->isInGroup() && !_player->m_bg)
    {
        if (auto group = _player->getGroup())
        {
            if (group->GetMethod() == PARTY_LOOT_MASTER_LOOTER)
            {
                group->Lock();
                for (uint32_t i = 0; i < group->GetSubGroupCount(); ++i)
                {
                    if (auto subGroup = group->GetSubGroup(i))
                    {
                        for (auto groupMemberPlayerInfo : subGroup->getGroupMembers())
                        {
                            if (Player* loggedInPlayer = sObjectMgr.getPlayer(groupMemberPlayerInfo->guid))
                                if (_player->getZoneId() == loggedInPlayer->getZoneId())
                                    onlineGroupMembers.push_back(loggedInPlayer->getGuid());
                        }
                    }
                }
                group->Unlock();

                group->SendPacketToAll(SmsgLootMasterList(onlineGroupMembers).serialise().get());
            }
        }
    }
    _player->sendLoot(srlPacket.guid, LOOT_CORPSE, _player->GetMapId());
}

void WorldSession::handleLootReleaseOpcode(WorldPacket& recvPacket)
{
    CmsgLootRelease srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (uint64_t lguid = GetPlayer()->getLootGuid())
        if (lguid == srlPacket.guid.getRawGuid())
            doLootRelease(srlPacket.guid);
}

void WorldSession::doLootRelease(WoWGuid lguid)
{
    Player* player = GetPlayer();
    Loot* loot = nullptr;

    SendPacket(SmsgLootReleaseResponse(lguid.getRawGuid(), 1).serialise().get());

    _player->setLootGuid(0);
    _player->removeUnitFlags(UNIT_FLAG_LOOTING);
    _player->m_currentLoot = 0;

    if (!player->IsInWorld())
        return;

    if (lguid.isGameObject())
    {
        GameObject* go = GetPlayer()->getWorldMap()->getGameObject(lguid.getGuidLowPart());
        if (auto gameObjectLootable = dynamic_cast<GameObject_Lootable*>(go))
        {
            // Remove our Guid
            gameObjectLootable->loot.removeLooter(_player->getGuidLow());

            // Remove roundrobin and make Lootable for evryone in our group
            gameObjectLootable->loot.roundRobinPlayer = 0;
            // not check distance for GO in case owned GO (fishing bobber case, for example) or Fishing hole GO
            if (!go || ((go->getCreatedByGuid() != _player->getGuid() && go->getGoType() != GAMEOBJECT_TYPE_FISHINGHOLE) && !go->IsWithinDistInMap(_player, 30.0f)))
                return;

            loot = &gameObjectLootable->loot;

            if (go->getGoType() == GAMEOBJECT_TYPE_DOOR)
            {
                // locked doors are opened with spelleffect openlock, prevent remove its as looted
                go->useDoorOrButton();
            }
            else if (loot->isLooted() || go->getGoType() == GAMEOBJECT_TYPE_FISHINGNODE)
            {
                if (go->getGoType() == GAMEOBJECT_TYPE_FISHINGHOLE)
                {                                               // The fishing hole used once more
                    go->addUse();                               // if the max usage is reached, will be despawned in next tick
                    if (go->getUseCount() >= dynamic_cast<GameObject_FishingHole*>(go)->getMaxOpen())
                        go->setLootState(GO_JUST_DEACTIVATED);
                    else
                        go->setLootState(GO_READY);
                }
                else
                {
                    go->setLootState(GO_JUST_DEACTIVATED);
                }

                loot->clear();
            }
            else
            {
                // not fully looted object
                go->setLootState(GO_ACTIVATED, player);

                // if the round robin player release, reset it.
                if (player->getGuid() == loot->roundRobinPlayer)
                    loot->roundRobinPlayer = 0;
            }
        }
    }
    else if (lguid.isCorpse())        // ONLY remove insignia at BG
    {
        auto* corpse = sObjectMgr.getCorpseByGuid(lguid.getGuidLow());
        if (!corpse || !corpse->IsWithinDistInMap(_player, 5.0f))
            return;

        loot = &corpse->loot;

        if (loot->isLooted())
        {
            loot->clear();
            corpse->setDynamicFlags(0);
        }
    }
    else if (lguid.isItem())
    {
        if (auto item = _player->getItemInterface()->GetItemByGUID(lguid.getRawGuid()))
        {
            if (item->m_loot != nullptr)
            {
                if (item->m_loot->isLooted())
                {
                    item->m_loot = nullptr;
                }
            }

            if (item->m_loot == nullptr)
                _player->getItemInterface()->RemoveItemAmtByGuid(lguid.getRawGuid(), 1);
        }
        return;                                             // item can be looted only single player
    }
    else if (lguid.isPlayer())
    {
        if (auto lootablePlayer = sObjectMgr.getPlayer(lguid.getGuidLow()))
        {
            lootablePlayer->m_lootableOnCorpse = false;
            lootablePlayer->loot.items.clear();
            lootablePlayer->removeDynamicFlags(U_DYN_FLAG_LOOTABLE);
        }
    }
    else
    {
        if (Creature* creature = GetPlayer()->getWorldMap()->getCreature(lguid.getGuidLowPart()))
        {
            // Remove roundrobin and make Lootable for evryone in our group
            creature->loot.roundRobinPlayer = 0;

            loot = &creature->loot;
            if (creature->loot.isLooted())
            {
                // Make creature no Longer Lootable we have no more loot left
                for (auto players : creature->getInRangePlayersSet())
                {
                    Player* plr = players->ToPlayer();
                    if (creature->isTaggedByPlayerOrItsGroup(plr))
                    {
#if VERSION_STRING < Mop
                        creature->BuildFieldUpdatePacket(plr, getOffsetForStructuredField(WoWUnit, dynamic_flags), 0);
#else
                        creature->BuildFieldUpdatePacket(plr, getOffsetForStructuredField(WoWObject, dynamic_field), 0);
#endif
                    }
                }

                // Make our Creature Skinnable when possible
                if (!creature->Skinned && sLootMgr.isSkinnable(creature->getEntry()))
                    creature->BuildFieldUpdatePacket(_player, getOffsetForStructuredField(WoWUnit, unit_flags), UNIT_FLAG_SKINNABLE);
            }
            else
            {
                // When Loot is left make Lootable for our mates
                // Send Loot Update to our GroupMembers
                for (auto players : _player->getInRangePlayersSet())
                {
                    Player* plr = players->ToPlayer();
                    if (creature->isTaggedByPlayerOrItsGroup(plr))
                    {
                        plr->sendLootUpdate(creature);
                    }
                }
            }
        }
    }

    //Player is not looking at loot list, he doesn't need to see updates on the loot list
    if (loot)
        loot->removeLooter(_player->getGuidLow());
}

void WorldSession::handleLootMasterGiveOpcode(WorldPacket& recvPacket)
{
    CmsgLootMasterGive srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (_player->getGroup() == nullptr || _player->getGroup()->GetLooter() != _player->getPlayerInfo())
        return;

    auto player = _player->getWorldMap()->getPlayer(srlPacket.playerGuid.getGuidLow());
    if (player == nullptr)
        return;

    if (_player->getLootGuid() != srlPacket.creatureGuid.getRawGuid())
        return;

    Creature* creature = nullptr;
    Loot* loot = nullptr;

    WoWGuid lootGuid;
    lootGuid.Init(_player->getLootGuid());

    if (lootGuid.isUnit())
    {
        creature = _player->getWorldMap()->getCreature(srlPacket.creatureGuid.getGuidLowPart());
        if (creature == nullptr)
            return;

        loot = &creature->loot;
    }
    else if (lootGuid.isGameObject())
    {
        auto gameObject = _player->getWorldMap()->getGameObject(srlPacket.creatureGuid.getGuidLowPart());
        if (gameObject == nullptr)
            return;

        if (!gameObject->IsLootable())
            return;

        auto gameObjectLootable = dynamic_cast<GameObject_Lootable*>(gameObject);
        gameObject->setState(GO_STATE_OPEN);
        loot = &gameObjectLootable->loot;
    }

    if (loot && srlPacket.slot >= loot->items.size())
    {
        sLogger.debug("AutoLootItem: Player {} might be using a hack! (slot {}, size {})", _player->getName(), srlPacket.slot, static_cast<uint32_t>(loot->items.size()));
        return;
    }

    LootItem& item = srlPacket.slot >= loot->items.size() ? loot->quest_items[srlPacket.slot - loot->items.size()] : loot->items[srlPacket.slot];

    // Add Item to Player
    Item* newItem = player->storeItem(&item);
    if (!newItem)
        return;

    if (creature)
    {
        if (creature->IsInWorld() && creature->isCreature() && creature->GetScript())
            creature->GetScript()->OnLootTaken(player, item.itemproto);
    }

    // mark as looted
    item.count = 0;
    item.is_looted = true;

    loot->itemRemoved(srlPacket.slot);
    --loot->unlootedCount;
}
