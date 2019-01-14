/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgSwapItem.h"
#include "Server/WorldSession.h"
#include "Units/Players/Player.h"
#include "Management/ItemInterface.h"
#include "Server/Packets/CmsgItemrefundinfo.h"
#include "Server/Packets/CmsgItemrefundrequest.h"
#include "Server/Packets/CmsgSplitItem.h"
#include "Server/Packets/SmsgInventoryChangeFailure.h"
#include "Server/Packets/CmsgAutoequipItem.h"
#include "Server/Packets/CmsgAutostoreBagItem.h"
#include "Server/Packets/CmsgReadItem.h"
#include "Server/Packets/SmsgReadItemOk.h"
#include "Server/Packets/SmsgReadItemFailed.h"
#include "Server/Packets/CmsgRepairItem.h"
#include "Server/Packets/CmsgAutostoreBankItem.h"
#include "Server/Packets/CmsgSocketGems.h"
#include "Server/Packets/CmsgWrapItem.h"
#include "Server/Packets/CmsgCancelTempEnchantment.h"
#include "Server/Packets/CmsgAutobankItem.h"
#include "Server/Packets/CmsgBuyItemInSlot.h"
#include "Server/Packets/CmsgBuybackItem.h"
#include "Server/Packets/CmsgAutoequipItemSlot.h"
#include "Server/Packets/CmsgDestroyItem.h"
#include "Server/Packets/CmsgSwapInvItem.h"
#include "Server/Packets/CmsgUseItem.h"
#include "Management/Battleground/Battleground.h"
#include "Spell/SpellMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Creatures/Pet.h"
#include "Management/Container.h"
#include "Map/MapMgr.h"
#include "Server/Packets/CmsgListInventory.h"
#include "Server/Packets/SmsgBuyItem.h"
#include "Server/Packets/CmsgBuyItem.h"
#include "Server/Packets/SmsgSellItem.h"
#include "Server/Packets/CmsgSellItem.h"
#include "Server/Packets/CmsgItemQuerySingle.h"
#include "Spell/Definitions/AuraInterruptFlags.h"
#include "Server/Packets/SmsgBuyFailed.h"

using namespace AscEmu::Packets;

void WorldSession::handleUseItemOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgUseItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    Item* tmpItem = _player->getItemInterface()->GetInventoryItem(srlPacket.containerIndex, srlPacket.inventorySlot);
    if (tmpItem == nullptr)
    {
        tmpItem = _player->getItemInterface()->GetInventoryItem(srlPacket.inventorySlot);
    }

    if (tmpItem == nullptr)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (tmpItem->getGuid() != srlPacket.itemGuid)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    ItemProperties const* itemProto = tmpItem->getItemProperties();
    if (!itemProto)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    // Equipable items needs to be equipped before use
    if (itemProto->InventoryType != INVTYPE_NON_EQUIP && !_player->getItemInterface()->IsEquipped(itemProto->ItemId))
    {
        // todo: is this correct error msg?
        _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (!_player->isAlive())
    {
        _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_YOU_ARE_DEAD);
        return;
    }

    if (tmpItem->isSoulbound() && tmpItem->getOwnerGuid() != _player->getGuid() && !tmpItem->isAccountbound())
    {
        _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_DONT_OWN_THAT_ITEM);
        return;
    }

    if ((itemProto->Flags2 & ITEM_FLAG2_HORDE_ONLY) && _player->getTeam() != TEAM_HORDE ||
        (itemProto->Flags2 & ITEM_FLAG2_ALLIANCE_ONLY) && _player->getTeam() != TEAM_ALLIANCE)
    {
        _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM);
        return;
    }

    if ((itemProto->AllowableClass & _player->getClassMask()) == 0 || (itemProto->AllowableRace & _player->getRaceMask()) == 0)
    {
        _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM);
        return;
    }

    if (itemProto->RequiredSkill)
    {
        if (!_player->_HasSkillLine(itemProto->RequiredSkill))
        {
            _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_NO_REQUIRED_PROFICIENCY);
            return;
        }
        else if (_player->_GetSkillLineCurrent(itemProto->RequiredSkill) < itemProto->RequiredSkillRank)
        {
            _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_SKILL_ISNT_HIGH_ENOUGH);
            return;
        }
    }

    if (itemProto->RequiredSkillSubRank != 0 && !_player->HasSpell(itemProto->RequiredSkillSubRank))
    {
        _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_NO_REQUIRED_PROFICIENCY);
        return;
    }

    if (_player->getLevel() < itemProto->RequiredLevel)
    {
        _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_YOU_MUST_REACH_LEVEL_N);
        return;
    }

    // Learning spells (mounts, companion pets etc)
    if (itemProto->Spells[0].Id == 483 || itemProto->Spells[0].Id == 55884)
    {
        if (_player->HasSpell(itemProto->Spells[1].Id))
            // No error message, handled elsewhere
            return;
    }

    if (itemProto->RequiredFaction && uint32_t(_player->GetStanding(itemProto->RequiredFaction)) < itemProto->RequiredFactionStanding)
    {
        _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_ITEM_REPUTATION_NOT_ENOUGH);
        return;
    }

    // Arena cases
    if (_player->m_bg != nullptr && isArena(_player->m_bg->GetType()))
    {
        // Not all consumables are usable in arena
        if (itemProto->Class == ITEM_CLASS_CONSUMABLE && !itemProto->HasFlag(ITEM_FLAG_USEABLE_IN_ARENA))
        {
            _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_NOT_DURING_ARENA_MATCH);
            return;
        }

        // Not all items are usable in arena
        if (itemProto->HasFlag(ITEM_FLAG_NOT_USEABLE_IN_ARENA))
        {
            _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_NOT_DURING_ARENA_MATCH);
            return;
        }
    }

    if (itemProto->Bonding == ITEM_BIND_ON_USE || itemProto->Bonding == ITEM_BIND_ON_PICKUP || itemProto->Bonding == ITEM_BIND_QUEST)
    {
        if (!tmpItem->isSoulbound())
            tmpItem->addFlags(ITEM_FLAG_SOULBOUND);
    }

    // Combat check
    if (_player->getcombatstatus()->IsInCombat())
    {
        for (uint8_t i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (SpellInfo const* spellInfo = sSpellMgr.getSpellInfo(itemProto->Spells[i].Id))
            {
                if (spellInfo->getAttributes() & ATTRIBUTES_REQ_OOC)
                {
                    _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_CANT_DO_IN_COMBAT);
                    return;
                }
            }
        }
    }

    // Start quest
    if (itemProto->QuestId)
    {
        QuestProperties const* quest = sMySQLStore.getQuestProperties(itemProto->QuestId);
        if (!quest)
            return;

        // Create packet
        WorldPacket data;
        sQuestMgr.BuildQuestDetails(&data, quest, tmpItem, 0, language, _player);
        SendPacket(&data);
    }

    // Anticheat to prevent WDB editing
    bool found = false;
    for (uint8_t i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (itemProto->Spells[i].Trigger == USE && itemProto->Spells[i].Id == srlPacket.spellId)
        {
            found = true;
            break;
        }
    }

    if (tmpItem->HasOnUseSpellID(srlPacket.spellId))
        found = true;

    // Item doesn't have this spell, either player is cheating or player's itemcache.wdb doesn't match with database
    if (!found)
    {
        Disconnect();
        Anticheat_Log->writefromsession(this, "Player tried to use an item with a spell that didn't match the spell in the database.");
        Anticheat_Log->writefromsession(this, "Possibly corrupted or intentionally altered itemcache.wdb");
        Anticheat_Log->writefromsession(this, "Itemid: %lu", itemProto->ItemId);
        Anticheat_Log->writefromsession(this, "Spellid: %lu", srlPacket.spellId);
        Anticheat_Log->writefromsession(this, "Player was disconnected.");
        return;
    }

    // Call item scripts
    if (sScriptMgr.CallScriptedItem(tmpItem, _player))
        return;

    SpellCastTargets targets(recvPacket, _player->getGuid());
    const auto spellInfo = sSpellMgr.getSpellInfo(srlPacket.spellId);
    if (spellInfo == nullptr)
    {
        LogError("WORLD: Unknown spell id %i in ::handleUseItemOpcode() from item id %i", srlPacket.spellId, itemProto->ItemId);
        return;
    }

    // TODO: remove this and get rid of 'ForcedPetId' hackfix
    // move the spells from MySQLDataStore.cpp to SpellScript
    if (itemProto->ForcedPetId >= 0)
    {
        if (itemProto->ForcedPetId == 0)
        {
            if (targets.m_unitTarget != _player->getGuid())
            {
                _player->sendCastFailedPacket(spellInfo->getId(), SPELL_FAILED_BAD_TARGETS, srlPacket.castCount, 0);
                return;
            }
        }
        else
        {
            if (!_player->GetSummon() || _player->GetSummon()->getEntry() != static_cast<uint32_t>(itemProto->ForcedPetId))
            {
                _player->sendCastFailedPacket(spellInfo->getId(), SPELL_FAILED_BAD_TARGETS, srlPacket.castCount, 0);
                return;
            }
        }
    }

    // TODO: move this to rewritten canCast() and castMe()
    if (spellInfo->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP && !_player->isSitting())
    {
        if (_player->CombatStatus.IsInCombat() || _player->IsMounted())
        {
            _player->getItemInterface()->BuildInventoryChangeError(tmpItem, nullptr, INV_ERR_CANT_DO_IN_COMBAT);
            return;
        }
        else
        {
            _player->setStandState(STANDSTATE_SIT);
        }
    }

    Spell* spell = sSpellMgr.newSpell(_player, spellInfo, false, nullptr);
    spell->extra_cast_number = srlPacket.castCount;
    spell->i_caster = tmpItem;
    spell->m_glyphslot = srlPacket.glyphIndex;

    // Some spell cast packets include more data
    if (srlPacket.castFlags & 0x02)
    {
        float projectilePitch, projectileSpeed;
        uint8_t hasMovementData; // 1 or 0
        recvPacket >> projectilePitch >> projectileSpeed >> hasMovementData;

        LocationVector const spellDestination = targets.destination();
        LocationVector const spellSource = targets.source();
        float const deltaX = spellDestination.x - spellSource.y; // Calculate change of x position
        float const deltaY = spellDestination.y - spellSource.y; // Calculate change of y position

        uint32_t travelTime = 0;
        if ((projectilePitch != M_PI / 4) && (projectilePitch != -M_PI / 4)) // No division by zero
        {
            // Calculate projectile's travel time by using Pythagorean theorem to get distance from delta X and delta Y, and divide that with the projectile's velocity
            travelTime = static_cast<uint32_t>((sqrtf(deltaX * deltaX + deltaY * deltaY) / (cosf(projectilePitch) * projectileSpeed)) * 1000);
        }

        if (hasMovementData)
        {
            recvPacket.SetOpcode(recvPacket.read<uint16_t>()); // MSG_MOVE_STOP
            handleMovementOpcodes(recvPacket);
        }

        spell->m_missilePitch = projectilePitch;
        spell->m_missileTravelTime = travelTime;
    }

    spell->prepare(&targets);

#if VERSION_STRING > TBC
    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM, itemProto->ItemId, 0, 0);
#endif
}

void WorldSession::handleSwapItemOpcode(WorldPacket& recvPacket)
{
    CmsgSwapItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_SWAP_ITEM: destInventorySlot %i destSlot %i srcInventorySlot %i srcInventorySlot %i",
        srlPacket.destInventorySlot, srlPacket.destSlot, srlPacket.srcInventorySlot, srlPacket.srcSlot);

    _player->getItemInterface()->SwapItems(srlPacket.destInventorySlot,
        srlPacket.destSlot, srlPacket.srcInventorySlot, srlPacket.srcSlot);
}

#if VERSION_STRING == WotLK
void WorldSession::sendRefundInfo(uint64_t GUID)
{
    if (!_player || !_player->IsInWorld())
        return;

    auto item = _player->getItemInterface()->GetItemByGUID(GUID);
    if (item == nullptr)
        return;

    if (item->IsEligibleForRefund())
    {
        std::pair<time_t, uint32_t> RefundEntry = _player->getItemInterface()->LookupRefundable(GUID);

        if (RefundEntry.first == 0 || RefundEntry.second == 0)
            return;

        auto item_extended_cost = sItemExtendedCostStore.LookupEntry(RefundEntry.second);
        if (item_extended_cost == nullptr)
            return;

        ItemProperties const* proto = item->getItemProperties();

        item->setFlags(ITEM_FLAG_REFUNDABLE);
        
        WorldPacket packet(SMSG_ITEMREFUNDINFO, 68);
        packet << uint64_t(GUID);
        packet << uint32_t(proto->BuyPrice);
        packet << uint32_t(item_extended_cost->honor_points);
        packet << uint32_t(item_extended_cost->arena_points);

        for (uint8_t i = 0; i < 5; ++i)
        {
            packet << uint32_t(item_extended_cost->item[i]);
            packet << uint32_t(item_extended_cost->count[i]);
        }

        packet << uint32_t(0);

        uint32_t* played = _player->GetPlayedtime();

        if (played[1] > RefundEntry.first + 60 * 60 * 2)
            packet << uint32_t(0);
        else
            packet << uint32_t(RefundEntry.first);

        this->SendPacket(&packet);
    }
}
#elif VERSION_STRING >= Cata
void WorldSession::sendRefundInfo(uint64_t guid)
{
    if (!_player || !_player->IsInWorld())
        return;

    Item* item = _player->getItemInterface()->GetItemByGUID(guid);
    if (item == nullptr)
        return;

    if (item->IsEligibleForRefund())
    {
        std::pair<time_t, uint32_t> refundEntryPair = _player->getItemInterface()->LookupRefundable(guid);

        if (refundEntryPair.first == 0 || refundEntryPair.second == 0)
            return;

        auto itemExtendedCostEntry = sItemExtendedCostStore.LookupEntry(refundEntryPair.second);
        if (itemExtendedCostEntry == nullptr)
            return;

        ItemProperties const* item_properties = item->getItemProperties();
        item->addFlags(ITEM_FLAG_REFUNDABLE);

        ObjectGuid objectGuid = item->getGuid();
        WorldPacket data(SMSG_ITEMREFUNDINFO, 68);
        data.writeBit(objectGuid[3]);
        data.writeBit(objectGuid[5]);
        data.writeBit(objectGuid[7]);
        data.writeBit(objectGuid[6]);
        data.writeBit(objectGuid[2]);
        data.writeBit(objectGuid[4]);
        data.writeBit(objectGuid[0]);
        data.writeBit(objectGuid[1]);
        data.flushBits();
        data.WriteByteSeq(objectGuid[7]);

        uint32_t* played = _player->GetPlayedtime();

        if (played[1] > (refundEntryPair.first + 60 * 60 * 2))
            data << uint32_t(0);
        else
            data << uint32_t(refundEntryPair.first);

        for (uint8_t i = 0; i < 5; ++i)
        {
            data << uint32_t(itemExtendedCostEntry->item[i]);
            data << uint32_t(itemExtendedCostEntry->count[i]);
        }

        data.WriteByteSeq(objectGuid[6]);
        data.WriteByteSeq(objectGuid[4]);
        data.WriteByteSeq(objectGuid[3]);
        data.WriteByteSeq(objectGuid[2]);
        for (uint8_t i = 0; i < 5; ++i)
        {
            data << uint32_t(itemExtendedCostEntry->reqcurrcount[i]);
            data << uint32_t(itemExtendedCostEntry->reqcur[i]);
        }

        data.WriteByteSeq(objectGuid[1]);
        data.WriteByteSeq(objectGuid[5]);
        data << uint32_t(0);
        data.WriteByteSeq(objectGuid[0]);
        data << uint32_t(item_properties->BuyPrice);

        SendPacket(&data);
    }
}
#endif

#if VERSION_STRING >= WotLK
void WorldSession::handleItemRefundInfoOpcode(WorldPacket& recvPacket)
{
    CmsgItemrefundinfo srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_ITEMREFUNDINFO.");

    this->sendRefundInfo(srlPacket.itemGuid);
}

void WorldSession::handleItemRefundRequestOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN
    CmsgItemrefundrequest srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_ITEMREFUNDREQUEST.");

    uint32_t error = 1;

    
#if VERSION_STRING < Cata
    DBC::Structures::ItemExtendedCostEntry const* itemExtendedCostEntry = nullptr;
#else
    DB2::Structures::ItemExtendedCostEntry const* itemExtendedCostEntry = nullptr;
#endif
    ItemProperties const* itemProperties = nullptr;

    auto item = _player->getItemInterface()->GetItemByGUID(srlPacket.itemGuid);

    if (item != nullptr)
    {
        if (item->IsEligibleForRefund())
        {
            const auto refundEntry = _player->getItemInterface()->LookupRefundable(srlPacket.itemGuid);

            if (refundEntry.first != 0 && refundEntry.second != 0)
            {
                uint32_t* played = _player->GetPlayedtime();
                if (played[1] < refundEntry.first + 60 * 60 * 2)
                    itemExtendedCostEntry = sItemExtendedCostStore.LookupEntry(refundEntry.second);
            }

            if (itemExtendedCostEntry != nullptr)
            {
                itemProperties = item->getItemProperties();

                for (uint8_t i = 0; i < 5; ++i)
                    _player->getItemInterface()->AddItemById(itemExtendedCostEntry->item[i], itemExtendedCostEntry->count[i], 0);

                _player->getItemInterface()->AddItemById(43308, itemExtendedCostEntry->honor_points, 0);
                _player->getItemInterface()->AddItemById(43307, itemExtendedCostEntry->arena_points, 0);
                _player->modCoinage(itemProperties->BuyPrice);

                _player->getItemInterface()->RemoveItemAmtByGuid(srlPacket.itemGuid, 1);

                _player->getItemInterface()->RemoveRefundable(srlPacket.itemGuid);

                error = 0;
            }
        }
    }

    WorldPacket packet(SMSG_ITEMREFUNDREQUEST, 60);
    packet << uint64_t(srlPacket.itemGuid);
    packet << uint32_t(error);

    if (error == 0)
    {
        packet << uint32_t(itemProperties->BuyPrice);
        packet << uint32_t(itemExtendedCostEntry->honor_points);
        packet << uint32_t(itemExtendedCostEntry->arena_points);

        for (uint8_t i = 0; i < 5; ++i)
        {
            packet << uint32_t(itemExtendedCostEntry->item[i]);
            packet << uint32_t(itemExtendedCostEntry->count[i]);
        }
    }

    SendPacket(&packet);

    LogDebugFlag(LF_OPCODE, "Sent SMSG_ITEMREFUNDREQUEST.");
}
#endif

bool VerifyBagSlots(int8_t containerSlot, int8_t slot)
{
    if (containerSlot < -1)
        return false;

    if (containerSlot > 0 && (containerSlot < INVENTORY_SLOT_BAG_START || containerSlot >= INVENTORY_SLOT_BAG_END))
        return false;

    if (containerSlot == -1 && slot != -1 && (slot >= INVENTORY_SLOT_ITEM_END || slot <= EQUIPMENT_SLOT_END))
        return false;

    return true;
}

void WorldSession::handleSplitOpcode(WorldPacket& recvPacket)
{
    CmsgSplitItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.itemCount <= 0 || srlPacket.srcInventorySlot <= 0 && srlPacket.srcSlot < INVENTORY_SLOT_ITEM_START)
    {
        sCheatLog.writefromsession(this,
            "tried to split item: srcInventorySlot %d, srcSlot %d, destInventorySlot %d, destSlot %d, itemCount %l",
            srlPacket.srcInventorySlot, srlPacket.srcSlot, srlPacket.destInventorySlot, srlPacket.destSlot, srlPacket.itemCount);
        return;
    }

    if (!VerifyBagSlots(srlPacket.srcInventorySlot, srlPacket.srcSlot))
        return;

    if (!VerifyBagSlots(srlPacket.destInventorySlot, srlPacket.destSlot))
        return;

    uint32_t count = srlPacket.itemCount;

    auto inventoryItem1 = _player->getItemInterface()->GetInventoryItem(srlPacket.srcInventorySlot, srlPacket.srcSlot);
    if (!inventoryItem1)
        return;

    auto inventoryItem2 = _player->getItemInterface()->GetInventoryItem(srlPacket.destInventorySlot, srlPacket.destSlot);

    const uint32_t itemMaxStack1 = inventoryItem1->getOwner()->m_cheats.ItemStackCheat ? 0x7fffffff : inventoryItem1->getItemProperties()->MaxCount;
    const uint32_t itemMaxStack2 = inventoryItem2 ? (inventoryItem2->getOwner()->m_cheats.ItemStackCheat
        ? 0x7fffffff : inventoryItem2->getItemProperties()->MaxCount) : 0;
    if (inventoryItem1->wrapped_item_id || inventoryItem2 && inventoryItem2->wrapped_item_id || count > itemMaxStack1)
    {
        _player->getItemInterface()->BuildInventoryChangeError(inventoryItem1, inventoryItem2, INV_ERR_ITEM_CANT_STACK);
        return;
    }

    // something already in this slot
    if (inventoryItem2)
    {
        if (inventoryItem1->getEntry() == inventoryItem2->getEntry())
        {
            //check if player has the required stacks to avoid exploiting.
            //safe exploit check
            if (count < inventoryItem1->getStackCount())
            {
                //check if there is room on the other item.
                if (count + inventoryItem2->getStackCount() <= itemMaxStack2)
                {
                    inventoryItem1->modStackCount(-srlPacket.itemCount);
                    inventoryItem2->modStackCount(count);
                    inventoryItem1->m_isDirty = true;
                    inventoryItem2->m_isDirty = true;
                }
                else
                {
                    _player->getItemInterface()->BuildInventoryChangeError(inventoryItem1, inventoryItem2, INV_ERR_ITEM_CANT_STACK);
                }
            }
            else
            {
                _player->getItemInterface()->BuildInventoryChangeError(inventoryItem1, inventoryItem2, INV_ERR_COULDNT_SPLIT_ITEMS);
            }
        }
        else
        {
            _player->getItemInterface()->BuildInventoryChangeError(inventoryItem1, inventoryItem2, INV_ERR_ITEM_CANT_STACK);
        }
    }
    else
    {
        if (count < inventoryItem1->getStackCount())
        {
            inventoryItem1->modStackCount(-srlPacket.itemCount);

            inventoryItem2 = objmgr.CreateItem(inventoryItem1->getEntry(), _player);
            if (inventoryItem2 == nullptr)
                return;

            inventoryItem2->setStackCount(count);
            inventoryItem1->m_isDirty = true;
            inventoryItem2->m_isDirty = true;

            int8_t DstSlot = srlPacket.destSlot;
            int8_t DstInvSlot = srlPacket.destInventorySlot;

            if (srlPacket.destSlot == ITEM_NO_SLOT_AVAILABLE)
            {
                if (srlPacket.destInventorySlot != ITEM_NO_SLOT_AVAILABLE)
                {
                    Container* container = _player->getItemInterface()->GetContainer(srlPacket.destInventorySlot);
                    if (container != nullptr)
                        DstSlot = container->FindFreeSlot();
                }
                else
                {
                    // Find a free slot
                    const auto slotResult = _player->getItemInterface()->FindFreeInventorySlot(inventoryItem2->getItemProperties());
                    if (slotResult.Result)
                    {
                        DstSlot = slotResult.Slot;
                        DstInvSlot = slotResult.ContainerSlot;
                    }
                }

                if (DstSlot == ITEM_NO_SLOT_AVAILABLE)
                {
                    _player->getItemInterface()->BuildInventoryChangeError(inventoryItem1, inventoryItem2, INV_ERR_COULDNT_SPLIT_ITEMS);
                    inventoryItem2->DeleteFromDB();
                    inventoryItem2->DeleteMe();
                    inventoryItem2 = nullptr;
                }
            }

            const auto addItemResult = _player->getItemInterface()->SafeAddItem(inventoryItem2, DstInvSlot, DstSlot);
            if (addItemResult == ADD_ITEM_RESULT_ERROR)
            {
                LOG_ERROR("Error while adding item to dstslot");
                if (inventoryItem2 != nullptr)
                {
                    inventoryItem2->DeleteFromDB();
                    inventoryItem2->DeleteMe();
                    inventoryItem2 = nullptr;
                };
            }
        }
        else
        {
            _player->getItemInterface()->BuildInventoryChangeError(inventoryItem1, inventoryItem2, INV_ERR_COULDNT_SPLIT_ITEMS);
        }
    }
}

void WorldSession::handleSwapInvItemOpcode(WorldPacket& recvPacket)
{
    CmsgSwapInvItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_SWAP_INV_ITEM src slot: %u dst slot: %u",
        static_cast<uint32_t>(srlPacket.srcSlot), static_cast<uint32_t>(srlPacket.destSlot));

    // player trying to add item to the same slot
    if (srlPacket.destSlot == srlPacket.srcSlot)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEMS_CANT_BE_SWAPPED);
        return;
    }

    Item* dstItem = _player->getItemInterface()->GetInventoryItem(srlPacket.destSlot);
    Item* srcItem = _player->getItemInterface()->GetInventoryItem(srlPacket.srcSlot);

    // allow weapon switching in combat
    bool skip_combat = false;
    // We're doing an equip swap.
    if (srlPacket.srcSlot < EQUIPMENT_SLOT_END || srlPacket.destSlot < EQUIPMENT_SLOT_END)
    {
        if (_player->CombatStatus.IsInCombat())
        {
            // These can't be swapped
            if (srlPacket.srcSlot < EQUIPMENT_SLOT_MAINHAND || srlPacket.destSlot < EQUIPMENT_SLOT_MAINHAND)
            {
                _player->getItemInterface()->BuildInventoryChangeError(srcItem, dstItem, INV_ERR_CANT_DO_IN_COMBAT);
                return;
            }
            skip_combat = true;
        }
    }

    if (!srcItem)
    {
        _player->getItemInterface()->BuildInventoryChangeError(srcItem, dstItem, INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM);
        return;
    }

    if (srlPacket.srcSlot == srlPacket.destSlot)
    {
        _player->getItemInterface()->BuildInventoryChangeError(srcItem, dstItem, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
        return;
    }

    int8_t error;

    if ((error = _player->getItemInterface()->CanEquipItemInSlot2(INVENTORY_SLOT_NOT_SET, srlPacket.destSlot, srcItem, skip_combat, false)) != 0)
    {
        if (srlPacket.destSlot < INVENTORY_KEYRING_END)
        {
            _player->getItemInterface()->BuildInventoryChangeError(srcItem, dstItem, error);
            return;
        }
    }

    if (dstItem != nullptr)
    {
        if ((error = _player->getItemInterface()->CanEquipItemInSlot2(INVENTORY_SLOT_NOT_SET, srlPacket.srcSlot, dstItem, skip_combat)) != 0)
        {
            if (srlPacket.srcSlot < INVENTORY_KEYRING_END)
            {
                const uint32_t reqLevel = dstItem->getItemProperties()->RequiredLevel;
                SendPacket(SmsgInventoryChangeFailure(error, srcItem->getGuid(), dstItem->getGuid(), reqLevel, true).serialise().get());
                return;
            }
        }
    }

    if (srcItem->isContainer())
    {
        //source has items and dst is a backpack or bank
        if (dynamic_cast<Container*>(srcItem)->HasItems())
            if (!_player->getItemInterface()->IsBagSlot(srlPacket.destSlot))
            {
                _player->getItemInterface()->BuildInventoryChangeError(srcItem, dstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                return;
            }

        if (dstItem)
        {
            //source is a bag and dst slot is a bag inventory and has items
            if (dstItem->isContainer())
            {
                if (dynamic_cast<Container*>(dstItem)->HasItems() && !_player->getItemInterface()->IsBagSlot(srlPacket.srcSlot))
                {
                    _player->getItemInterface()->BuildInventoryChangeError(srcItem, dstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                    return;
                }
            }
            else
            {
                //dst item is not a bag, swap impossible
                _player->getItemInterface()->BuildInventoryChangeError(srcItem, dstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                return;
            }
        }

        //dst is bag inventory
        if (srlPacket.destSlot < INVENTORY_SLOT_BAG_END)
        {
            if (srcItem->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
                srcItem->addFlags(ITEM_FLAG_SOULBOUND);
        }
    }

    // swap items
    if (_player->isDead())
    {
        _player->getItemInterface()->BuildInventoryChangeError(srcItem, nullptr, INV_ERR_YOU_ARE_DEAD);
        return;
    }

#if VERSION_STRING > TBC
    if (dstItem && srlPacket.srcSlot < INVENTORY_SLOT_BAG_END)
    {
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, dstItem->getItemProperties()->ItemId, 0, 0);
        if (srlPacket.srcSlot < INVENTORY_SLOT_BAG_START) // check Superior/Epic achievement
        {
            // Achievement ID:556 description Equip an epic item in every slot with a minimum item level of 213.
            // "213" value not found in achievement or criteria entries, have to hard-code it here?
            // Achievement ID:557 description Equip a superior item in every slot with a minimum item level of 187.
            // "187" value not found in achievement or criteria entries, have to hard-code it here?
            if (dstItem->getItemProperties()->Quality == ITEM_QUALITY_RARE_BLUE && dstItem->getItemProperties()->ItemLevel >= 187 ||
                dstItem->getItemProperties()->Quality == ITEM_QUALITY_EPIC_PURPLE && dstItem->getItemProperties()->ItemLevel >= 213)
                _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, srlPacket.srcSlot,
                    dstItem->getItemProperties()->Quality, 0);
        }
    }
    if (srcItem && srlPacket.destSlot < INVENTORY_SLOT_BAG_END)
    {
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, srcItem->getItemProperties()->ItemId, 0, 0);
        if (srlPacket.destSlot < INVENTORY_SLOT_BAG_START) // check Superior/Epic achievement
        {
            // Achievement ID:556 description Equip an epic item in every slot with a minimum item level of 213.
            // "213" value not found in achievement or criteria entries, have to hard-code it here?
            // Achievement ID:557 description Equip a superior item in every slot with a minimum item level of 187.
            // "187" value not found in achievement or criteria entries, have to hard-code it here?
            if (srcItem->getItemProperties()->Quality == ITEM_QUALITY_RARE_BLUE && srcItem->getItemProperties()->ItemLevel >= 187 ||
                srcItem->getItemProperties()->Quality == ITEM_QUALITY_EPIC_PURPLE && srcItem->getItemProperties()->ItemLevel >= 213)
                _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, srlPacket.destSlot,
                    srcItem->getItemProperties()->Quality, 0);
        }
    }
#endif

    _player->getItemInterface()->SwapItemSlots(srlPacket.srcSlot, srlPacket.destSlot);
}

void WorldSession::handleDestroyItemOpcode(WorldPacket& recvPacket)
{
    CmsgDestroyItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_DESTROYITEM SrcInv Slot: %i Src slot: %i", srlPacket.srcInventorySlot, srlPacket.srcSlot);

    if (Item* srcItem = _player->getItemInterface()->GetInventoryItem(srlPacket.srcInventorySlot, srlPacket.srcSlot))
    {
        if (srcItem->isContainer())
        {
            if (dynamic_cast<Container*>(srcItem)->HasItems())
            {
                _player->getItemInterface()->BuildInventoryChangeError(srcItem, nullptr, INV_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS);
                return;
            }
        }

        if (srcItem->getItemProperties()->HasFlag(ITEM_FLAG_INDESTRUCTIBLE))
        {
            _player->getItemInterface()->BuildInventoryChangeError(srcItem, nullptr, INV_ERR_CANT_DROP_SOULBOUND);
            return;
        }

        if (srcItem->getItemProperties()->ItemId == CharterEntry::Guild)
        {
            Charter* gc = _player->m_charters[CHARTER_TYPE_GUILD];
            if (gc)
                gc->Destroy();

            _player->m_charters[CHARTER_TYPE_GUILD] = nullptr;
        }

        if (srcItem->getItemProperties()->ItemId == CharterEntry::TwoOnTwo)
        {
            Charter* gc = _player->m_charters[CHARTER_TYPE_ARENA_2V2];
            if (gc)
                gc->Destroy();

            _player->m_charters[CHARTER_TYPE_ARENA_2V2] = nullptr;
        }

        if (srcItem->getItemProperties()->ItemId == CharterEntry::FiveOnFive)
        {
            Charter* gc = _player->m_charters[CHARTER_TYPE_ARENA_5V5];
            if (gc)
                gc->Destroy();

            _player->m_charters[CHARTER_TYPE_ARENA_5V5] = nullptr;
        }

        if (srcItem->getItemProperties()->ItemId == CharterEntry::ThreeOnThree)
        {
            Charter* gc = _player->m_charters[CHARTER_TYPE_ARENA_3V3];
            if (gc)
                gc->Destroy();

            _player->m_charters[CHARTER_TYPE_ARENA_3V3] = nullptr;
        }

        Item* pItem = _player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(srlPacket.srcInventorySlot, srlPacket.srcSlot, false);
        if (!pItem)
            return;

        for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
        {
            if (_player->getCurrentSpell(CurrentSpellType(i)) != nullptr
                && _player->getCurrentSpell(CurrentSpellType(i))->i_caster == pItem)
            {
                _player->getCurrentSpell(CurrentSpellType(i))->i_caster = nullptr;
                _player->interruptSpellWithSpellType(CurrentSpellType(i));
            }
        }

        pItem->DeleteFromDB();
        pItem->DeleteMe();
    }
}

void WorldSession::handleAutoEquipItemOpcode(WorldPacket& recvPacket)
{
    CmsgAutoequipItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUTOEQUIP_ITEM Inventory slot: %i Source Slot: %i", srlPacket.srcInventorySlot, srlPacket.srcSlot);

    Item* eitem = _player->getItemInterface()->GetInventoryItem(srlPacket.srcInventorySlot, srlPacket.srcSlot);

    if (eitem == nullptr)
    {
        _player->getItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    int8_t Slot = _player->getItemInterface()->GetItemSlotByType(eitem->getItemProperties()->InventoryType);
    if (Slot == ITEM_NO_SLOT_AVAILABLE)
    {
        _player->getItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_ITEM_CANT_BE_EQUIPPED);
        return;
    }

    int8_t error = 0;

    // handle equipping of 2h when we have two items equipped! :) special case.
    if ((Slot == EQUIPMENT_SLOT_MAINHAND || Slot == EQUIPMENT_SLOT_OFFHAND) 
        && !_player->canDualWield2H())
    {
        Item* mainhandweapon = _player->getItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
        if (mainhandweapon != nullptr && mainhandweapon->getItemProperties()->InventoryType == INVTYPE_2HWEAPON)
        {
            if (Slot == EQUIPMENT_SLOT_OFFHAND && (eitem->getItemProperties()->InventoryType == INVTYPE_WEAPON
                || eitem->getItemProperties()->InventoryType == INVTYPE_2HWEAPON))
            {
                Slot = EQUIPMENT_SLOT_MAINHAND;
            }
        }
        else
        {
            if (Slot == EQUIPMENT_SLOT_OFFHAND && eitem->getItemProperties()->InventoryType == INVTYPE_2HWEAPON)
            {
                Slot = EQUIPMENT_SLOT_MAINHAND;
            }
        }

        error = _player->getItemInterface()->CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET,
            Slot, eitem->getItemProperties(), true, true);
        if (error)
        {
            _player->getItemInterface()->BuildInventoryChangeError(eitem, nullptr, error);
            return;
        }

        if (eitem->getItemProperties()->InventoryType == INVTYPE_2HWEAPON)
        {
            // see if we have a weapon equipped in the offhand, if so we need to remove it
            Item* offhandweapon = _player->getItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND);
            if (offhandweapon != nullptr)
            {
                // we need to de-equip this
                SlotResult result = _player->getItemInterface()->FindFreeInventorySlot(offhandweapon->getItemProperties());
                if (!result.Result)
                {
                    // no free slots for this item
                    _player->getItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_BAG_FULL);
                    return;
                }

                offhandweapon = _player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET,
                    EQUIPMENT_SLOT_OFFHAND, false);
                if (offhandweapon == nullptr)
                    return; // should never happen

                // shouldn't happen either.
                if (!_player->getItemInterface()->SafeAddItem(offhandweapon, result.ContainerSlot, result.Slot)
                    && !_player->getItemInterface()->AddItemToFreeSlot(offhandweapon))
                {
                    offhandweapon->DeleteMe();
                    offhandweapon = nullptr;
                }
            }
        }
        else
        {
            // can't equip a non-two-handed weapon with a two-handed weapon
            mainhandweapon = _player->getItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
            if (mainhandweapon != nullptr && mainhandweapon->getItemProperties()->InventoryType == INVTYPE_2HWEAPON)
            {
                // we need to de-equip this
                SlotResult result = _player->getItemInterface()->FindFreeInventorySlot(mainhandweapon->getItemProperties());
                if (!result.Result)
                {
                    // no free slots for this item
                    _player->getItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_BAG_FULL);
                    return;
                }

                mainhandweapon = _player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET,
                    EQUIPMENT_SLOT_MAINHAND, false);
                if (mainhandweapon == nullptr)
                    return; // should never happen

                // shouldn't happen either.
                if (!_player->getItemInterface()->SafeAddItem(mainhandweapon, result.ContainerSlot, result.Slot)
                    && !_player->getItemInterface()->AddItemToFreeSlot(mainhandweapon))
                {
                    mainhandweapon->DeleteMe();
                    mainhandweapon = nullptr;
                }
            }
        }
    }
    else
    {
        error = _player->getItemInterface()->CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET,
            Slot, eitem->getItemProperties(), false, false);
        if (error)
        {
            _player->getItemInterface()->BuildInventoryChangeError(eitem, nullptr, error);
            return;
        }
    }

    if (Slot <= INVENTORY_SLOT_BAG_END)
    {
        error = _player->getItemInterface()->CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET,
            Slot, eitem->getItemProperties(), false, false);
        if (error)
        {
            _player->getItemInterface()->BuildInventoryChangeError(eitem, nullptr, error);
            return;
        }
    }

    if (srlPacket.srcInventorySlot == INVENTORY_SLOT_NOT_SET)
    {
        _player->getItemInterface()->SwapItemSlots(srlPacket.srcSlot, Slot);
    }
    else
    {
        eitem = _player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(srlPacket.srcInventorySlot, srlPacket.srcSlot, false);
        Item* oitem = _player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, Slot, false);
        AddItemResult result;
        if (oitem != nullptr)
        {
            result = _player->getItemInterface()->SafeAddItem(oitem, srlPacket.srcInventorySlot, srlPacket.srcSlot);
            if (!result)
            {
                LOG_ERROR("Error while adding item to SrcSlot");
                oitem->DeleteMe();
                oitem = nullptr;
            }
        }
        if (eitem != nullptr)
        {
            result = _player->getItemInterface()->SafeAddItem(eitem, INVENTORY_SLOT_NOT_SET, Slot);
            if (!result)
            {
                LOG_ERROR("Error while adding item to Slot");
                eitem->DeleteMe();
                eitem = nullptr;
                return;
            }
        }

    }

    if (eitem != nullptr)
    {
        if (eitem->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
            eitem->addFlags(ITEM_FLAG_SOULBOUND);
#if VERSION_STRING > TBC
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM,
            eitem->getItemProperties()->ItemId, 0, 0);
        // Achievement ID:556 description Equip an epic item in every slot with a minimum item level of 213.
        // "213" value not found in achievement or criteria entries, have to hard-code it here? :(
        // Achievement ID:557 description Equip a superior item in every slot with a minimum item level of 187.
        // "187" value not found in achievement or criteria entries, have to hard-code it here? :(
        if (eitem->getItemProperties()->Quality == ITEM_QUALITY_RARE_BLUE && eitem->getItemProperties()->ItemLevel >= 187 ||
            eitem->getItemProperties()->Quality == ITEM_QUALITY_EPIC_PURPLE && eitem->getItemProperties()->ItemLevel >= 213)
            _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, Slot,
                eitem->getItemProperties()->Quality, 0);
#endif
    }
    //Recalculate Expertise (for Weapon specs)
    _player->CalcExpertise();
}

void WorldSession::handleAutoEquipItemSlotOpcode(WorldPacket& recvPacket)
{
    CmsgAutoequipItemSlot srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUTOEQUIP_ITEM_SLOT");

    int8_t srcSlot = static_cast<int8_t>(_player->getItemInterface()->GetInventorySlotByGuid(srlPacket.itemGuid));
    Item* item = _player->getItemInterface()->GetItemByGUID(srlPacket.itemGuid);

    if (item == nullptr)
        return;

    int8_t slotType = _player->getItemInterface()->GetItemSlotByType(item->getItemProperties()->InventoryType);
    bool hasDualWield2H = false;

    LogDebugFlag(LF_OPCODE, "CMSG_AUTOEQUIP_ITEM_SLOT ItemGUID: %u, SrcSlot: %i, DestSlot: %i, SlotType: %i",
        srlPacket.itemGuid, srcSlot, srlPacket.destSlot, slotType);

    if (srcSlot == srlPacket.destSlot)
        return;

    if (_player->canDualWield2H() && (slotType == EQUIPMENT_SLOT_OFFHAND
        || slotType == EQUIPMENT_SLOT_MAINHAND))
        hasDualWield2H = true;

    // Need to check if the item even goes into that slot
    // Item system is a mess too, so it needs rewrite, but hopefully this will do for now
    int8_t error = _player->getItemInterface()->CanEquipItemInSlot2(INVENTORY_SLOT_NOT_SET, srlPacket.destSlot, item);
    if (error)
    {
        _player->getItemInterface()->BuildInventoryChangeError(item, nullptr, error);
        return;
    }

    // Handle destination slot checking.
    if (srlPacket.destSlot == slotType || hasDualWield2H)
    {
        uint32_t invType = item->getItemProperties()->InventoryType;
        if (invType == INVTYPE_WEAPON || invType == INVTYPE_WEAPONMAINHAND ||
            invType == INVTYPE_WEAPONOFFHAND || invType == INVTYPE_2HWEAPON)
        {
            Item* mainHand = _player->getItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
            Item* offHand = _player->getItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND);

            if (mainHand != nullptr && offHand != nullptr && !_player->canDualWield2H())
            {
                // No DualWield2H like Titan's grip. Unequip offhand.
                SlotResult result = _player->getItemInterface()->FindFreeInventorySlot(offHand->getItemProperties());
                if (!result.Result)
                {
                    // No free slots for this item.
                    _player->getItemInterface()->BuildInventoryChangeError(offHand, nullptr, INV_ERR_BAG_FULL);
                    return;
                }
                mainHand = _player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET,
                    EQUIPMENT_SLOT_OFFHAND, false);
                _player->getItemInterface()->AddItemToFreeSlot(offHand);
                _player->getItemInterface()->SwapItemSlots(srcSlot, srlPacket.destSlot);   // Now swap Main hand with 2H weapon.
            }
            else
            {
                // Swap 2H with 2H or 2H with 1H if player has DualWield2H (ex: Titans Grip).
                _player->getItemInterface()->SwapItemSlots(srcSlot, srlPacket.destSlot);
            }
        }
        else if (srlPacket.destSlot == slotType)
        {
            // If item slot types match, swap.
            _player->getItemInterface()->SwapItemSlots(srcSlot, srlPacket.destSlot);
        }
        else
        {
            // Item slots do not match. We get here only for players who have DualWield2H (ex: Titans Grip).
            _player->getItemInterface()->BuildInventoryChangeError(item, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
        }
    }
    else
    {
        // Item slots do not match.
        _player->getItemInterface()->BuildInventoryChangeError(item, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
    }
}

#if VERSION_STRING == TBC
void WorldSession::handleItemQuerySingleOpcode(WorldPacket& recvPacket)
{
    CmsgItemQuerySingle srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    ItemProperties const* itemProto = sMySQLStore.getItemProperties(srlPacket.item_id);
    if (!itemProto)
    {
        LOG_ERROR("Unknown item id %u", srlPacket.item_id);
        return;
    }

    std::string Name;
    std::string Description;

    MySQLStructure::LocalesItem const* li = (language > 0) 
    ? sMySQLStore.getLocalizedItem(srlPacket.item_id, language) : nullptr;
    if (li != nullptr)
    {
        Name = li->name;
        Description = li->description;
    }
    else
    {
        Name = itemProto->Name;
        Description = itemProto->Description;
    }

    WorldPacket data(SMSG_ITEM_QUERY_SINGLE_RESPONSE, 800);
    data << itemProto->ItemId;
    data << itemProto->Class;
    data << uint32_t(itemProto->SubClass);
    data << itemProto->unknown_bc;  // soundOverride
    data << Name;
    data << uint8_t(0);           // name 2?
    data << uint8_t(0);           // name 3?
    data << uint8_t(0);           // name 4?
    data << itemProto->DisplayInfoID;
    data << itemProto->Quality;
    data << itemProto->Flags;
    //data << itemProto->Flags2;
    data << itemProto->BuyPrice;
    data << itemProto->SellPrice;
    data << itemProto->InventoryType;
    data << itemProto->AllowableClass;
    data << itemProto->AllowableRace;
    data << itemProto->ItemLevel;
    data << itemProto->RequiredLevel;
    data << itemProto->RequiredSkill;
    data << itemProto->RequiredSkillRank;
    data << itemProto->RequiredSkillSubRank;
    data << itemProto->RequiredPlayerRank1;
    data << itemProto->RequiredPlayerRank2;
    data << itemProto->RequiredFaction;
    data << itemProto->RequiredFactionStanding;
    data << itemProto->Unique;
    data << itemProto->MaxCount;
    data << itemProto->ContainerSlots;
    for (uint8_t i = 0; i < 10; i++) //itemProto->itemstatscount
    {
        data << itemProto->Stats[i].Type;
        data << itemProto->Stats[i].Value;
    }

    //data << itemProto->ScalingStatsEntry;
    //data << itemProto->ScalingStatsFlag;
    for (uint8_t i = 0; i < 2; i++)
    {
        data << itemProto->Damage[i].Min;
        data << itemProto->Damage[i].Max;
        data << itemProto->Damage[i].Type;
    }

    for (uint8_t i = 0; i < 3; i++)
    {
        data << float(0.0f);
        data << float(0.0f);
        data << uint32_t(0);
    }

    data << itemProto->Armor;
    data << itemProto->HolyRes;
    data << itemProto->FireRes;
    data << itemProto->NatureRes;
    data << itemProto->FrostRes;
    data << itemProto->ShadowRes;
    data << itemProto->ArcaneRes;
    data << itemProto->Delay;
    data << itemProto->AmmoType;
    data << itemProto->Range;
    for (uint8_t i = 0; i < 5; i++)
    {
        data << itemProto->Spells[i].Id;
        data << itemProto->Spells[i].Trigger;
        data << itemProto->Spells[i].Charges;
        data << itemProto->Spells[i].Cooldown;
        data << itemProto->Spells[i].Category;
        data << itemProto->Spells[i].CategoryCooldown;
    }
    data << itemProto->Bonding;

    data << Description;

    data << itemProto->PageId;
    data << itemProto->PageLanguage;
    data << itemProto->PageMaterial;
    data << itemProto->QuestId;
    data << itemProto->LockId;
    data << itemProto->LockMaterial;
    data << itemProto->SheathID;
    data << itemProto->RandomPropId;
    data << itemProto->RandomSuffixId;
    data << itemProto->Block;
    data << sMySQLStore.getItemSetLinkedBonus(itemProto->ItemSet);
    data << itemProto->MaxDurability;
    data << itemProto->ZoneNameID;
    data << itemProto->MapID;
    data << itemProto->BagFamily;
    data << itemProto->TotemCategory;
    data << itemProto->Sockets[0].SocketColor;
    data << itemProto->Sockets[0].Unk;
    data << itemProto->Sockets[1].SocketColor;
    data << itemProto->Sockets[1].Unk;
    data << itemProto->Sockets[2].SocketColor;
    data << itemProto->Sockets[2].Unk;
    data << itemProto->SocketBonus;
    data << itemProto->GemProperties;
    data << itemProto->DisenchantReqSkill;
    data << itemProto->ArmorDamageModifier;
    data << itemProto->ExistingDuration;                    // 2.4.2 Item duration in seconds

    SendPacket(&data);
}
#else
void WorldSession::handleItemQuerySingleOpcode(WorldPacket& recvPacket)
{
    CmsgItemQuerySingle srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto itemProperties = sMySQLStore.getItemProperties(srlPacket.item_id);
    if (!itemProperties)
    {
        LOG_ERROR("Unknown item id %u", srlPacket.item_id);
        return;
    }

    std::string Name;
    std::string Description;

    MySQLStructure::LocalesItem const* li = language > 0
    ? sMySQLStore.getLocalizedItem(srlPacket.item_id, language) : nullptr;
    if (li != nullptr)
    {
        Name = li->name;
        Description = li->description;
    }
    else
    {
        Name = itemProperties->Name;
        Description = itemProperties->Description;
    }

    WorldPacket data(SMSG_ITEM_QUERY_SINGLE_RESPONSE, 800);
    data << itemProperties->ItemId;
    data << itemProperties->Class;
    data << uint32_t(itemProperties->SubClass);
    data << itemProperties->unknown_bc;  // soundOverride
    data << Name;
    data << uint8_t(0);           // name 2?
    data << uint8_t(0);           // name 3?
    data << uint8_t(0);           // name 4?
    data << itemProperties->DisplayInfoID;
    data << itemProperties->Quality;
    data << itemProperties->Flags;
    data << itemProperties->Flags2;
    data << itemProperties->BuyPrice;
    data << itemProperties->SellPrice;
    data << itemProperties->InventoryType;
    data << itemProperties->AllowableClass;
    data << itemProperties->AllowableRace;
    data << itemProperties->ItemLevel;
    data << itemProperties->RequiredLevel;
    data << itemProperties->RequiredSkill;
    data << itemProperties->RequiredSkillRank;
    data << itemProperties->RequiredSkillSubRank;
    data << itemProperties->RequiredPlayerRank1;
    data << itemProperties->RequiredPlayerRank2;
    data << itemProperties->RequiredFaction;
    data << itemProperties->RequiredFactionStanding;
    data << itemProperties->Unique;
    data << itemProperties->MaxCount;
    data << itemProperties->ContainerSlots;
    data << itemProperties->itemstatscount;
    for (uint8_t i = 0; i < itemProperties->itemstatscount; i++)
    {
        data << itemProperties->Stats[i].Type;
        data << itemProperties->Stats[i].Value;
    }
    data << itemProperties->ScalingStatsEntry;
    data << itemProperties->ScalingStatsFlag;
    // originally this went up to 5, now only to 2
    for (uint8_t i = 0; i < 2; i++)
    {
        data << itemProperties->Damage[i].Min;
        data << itemProperties->Damage[i].Max;
        data << itemProperties->Damage[i].Type;
    }
    data << itemProperties->Armor;
    data << itemProperties->HolyRes;
    data << itemProperties->FireRes;
    data << itemProperties->NatureRes;
    data << itemProperties->FrostRes;
    data << itemProperties->ShadowRes;
    data << itemProperties->ArcaneRes;
    data << itemProperties->Delay;
    data << itemProperties->AmmoType;
    data << itemProperties->Range;
    for (uint8_t i = 0; i < 5; i++)
    {
        data << itemProperties->Spells[i].Id;
        data << itemProperties->Spells[i].Trigger;
        data << itemProperties->Spells[i].Charges;
        data << itemProperties->Spells[i].Cooldown;
        data << itemProperties->Spells[i].Category;
        data << itemProperties->Spells[i].CategoryCooldown;
    }
    data << itemProperties->Bonding;

    data << Description;

    data << itemProperties->PageId;
    data << itemProperties->PageLanguage;
    data << itemProperties->PageMaterial;
    data << itemProperties->QuestId;
    data << itemProperties->LockId;
    data << itemProperties->LockMaterial;
    data << itemProperties->SheathID;
    data << itemProperties->RandomPropId;
    data << itemProperties->RandomSuffixId;
    data << itemProperties->Block;
    data << sMySQLStore.getItemSetLinkedBonus(itemProperties->ItemSet);
    data << itemProperties->MaxDurability;
    data << itemProperties->ZoneNameID;
    data << itemProperties->MapID;
    data << itemProperties->BagFamily;
    data << itemProperties->TotemCategory;
    data << itemProperties->Sockets[0].SocketColor;
    data << itemProperties->Sockets[0].Unk;
    data << itemProperties->Sockets[1].SocketColor;
    data << itemProperties->Sockets[1].Unk;
    data << itemProperties->Sockets[2].SocketColor;
    data << itemProperties->Sockets[2].Unk;
    data << itemProperties->SocketBonus;
    data << itemProperties->GemProperties;
    data << itemProperties->DisenchantReqSkill;
    data << itemProperties->ArmorDamageModifier;
    data << itemProperties->ExistingDuration;                    // 2.4.2 Item duration in seconds
    data << itemProperties->ItemLimitCategory;
    data << itemProperties->HolidayId;                           // HolidayNames.dbc
    SendPacket(&data);
}
#endif

void WorldSession::handleBuyBackOpcode(WorldPacket& recvPacket)
{
    CmsgBuybackItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_BUYBACK_ITEM");

    srlPacket.buybackSlot -= 74;

    if (Item* it = _player->getItemInterface()->GetBuyBack(srlPacket.buybackSlot))
    {
        // Find free slot and break if inv full
        uint32_t amount = it->getStackCount();
        uint32_t itemid = it->getEntry();

        Item* add = _player->getItemInterface()->FindItemLessMax(itemid, amount, false);

        uint32_t FreeSlots = _player->getItemInterface()->CalculateFreeSlots(it->getItemProperties());
        if (FreeSlots == 0 && !add)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
            return;
        }

        // Check for gold
        uint32_t cost = _player->getUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_PRICE_1 + srlPacket.buybackSlot));
        if (!_player->hasEnoughCoinage(cost))
        {
            sendBuyFailed(srlPacket.buybackSlot, itemid, 2);
            return;
        }

        uint8_t error;

        // Check for item uniqueness
        if ((error = _player->getItemInterface()->CanReceiveItem(it->getItemProperties(), amount)) != 0)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
            return;
        }

        int32_t coins = cost * -1;
        _player->modCoinage(coins);
        _player->getItemInterface()->RemoveBuyBackItem(srlPacket.buybackSlot);

        if (!add)
        {
            it->m_isDirty = true;            // save the item again on logout
            AddItemResult result = _player->getItemInterface()->AddItemToFreeSlot(it);
            if (!result)
            {
                LOG_ERROR("Error while adding item to free slot");
                it->DeleteMe();
            }
        }
        else
        {
            add->setStackCount(add->getStackCount() + amount);
            add->m_isDirty = true;

            // delete the item
            it->DeleteFromDB();
            it->DeleteMe();
        }

#if VERSION_STRING < Cata
        WorldPacket data(16);
        data.Initialize(SMSG_BUY_ITEM);
        data << uint64_t(srlPacket.itemGuid);
        data << Util::getMSTime(); //VLack: seen is Aspire code
        data << uint32_t(itemid);
        data << uint32_t(amount);
#else
        WorldPacket data(SMSG_BUY_ITEM, 8 + 4 + 4 + 4);
        data << uint64_t(srlPacket.itemGuid);
        data << uint32_t(srlPacket.buybackSlot + 1);// numbered from 1 at client
        data << int32_t(amount);
        data << uint32_t(amount);
        data << uint32_t(amount);
#endif
        SendPacket(&data);
    }
}

void WorldSession::handleSellItemOpcode(WorldPacket& recvPacket)
{
    CmsgSellItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_SELL_ITEM");

    _player->interruptSpell();

    // Check if item exists
    if (!srlPacket.itemGuid)
    {
        sendSellItem(srlPacket.vendorGuid.GetOldGuid(), srlPacket.itemGuid, 1);
        return;
    }

    Creature* unit = _player->GetMapMgr()->GetCreature(srlPacket.vendorGuid.getGuidLowPart());
    // Check if Vendor exists
    if (unit == nullptr)
    {
        sendSellItem(srlPacket.vendorGuid.GetOldGuid(), srlPacket.itemGuid, 3);
        return;
    }

    Item* item = _player->getItemInterface()->GetItemByGUID(srlPacket.itemGuid);
    if (!item)
    {
        sendSellItem(srlPacket.vendorGuid.GetOldGuid(), srlPacket.itemGuid, 1);
        return; //our player doesn't have this item
    }

    ItemProperties const* it = item->getItemProperties();

    if (item->isContainer() && dynamic_cast<Container*>(item)->HasItems())
    {
        sendSellItem(srlPacket.vendorGuid.GetOldGuid(), srlPacket.itemGuid, 6);
        return;
    }

    // Check if item can be sold
    if (it->SellPrice == 0 || item->wrapped_item_id != 0)
    {
        sendSellItem(srlPacket.vendorGuid.GetOldGuid(), srlPacket.itemGuid, 2);
        return;
    }

    uint32_t stackcount = item->getStackCount();
    uint32_t quantity = 0;

    if (srlPacket.amount != 0)
        quantity = srlPacket.amount;
    else
        quantity = stackcount; //allitems

    if (quantity > stackcount)
        quantity = stackcount; //make sure we don't over do it

    uint32_t price = GetSellPriceForItem(it, quantity);

    // Check they don't have more than the max gold
    if (worldConfig.player.isGoldCapEnabled)
    {
        if (_player->getCoinage() + price > worldConfig.player.limitGoldAmount)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            return;
        }
    }

    _player->modCoinage(price);

    if (quantity < stackcount)
    {
        item->setStackCount(stackcount - quantity);
        item->m_isDirty = true;
    }
    else
    {
        //removing the item from the char's inventory
        //again to remove item from slot
        item = _player->getItemInterface()->SafeRemoveAndRetreiveItemByGuid(srlPacket.itemGuid, false);
        if (item)
        {
            _player->getItemInterface()->AddBuyBackItem(item, it->SellPrice * quantity);
            item->DeleteFromDB();
        }
    }

    SendPacket(SmsgSellItem(srlPacket.vendorGuid.GetOldGuid(), srlPacket.itemGuid).serialise().get());
}

void WorldSession::handleBuyItemInSlotOpcode(WorldPacket& recvPacket)
{
    CmsgBuyItemInSlot srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_BUY_ITEM_IN_SLOT");

    int8_t slot = srlPacket.slot;
    uint8_t amount = srlPacket.amount;

    uint8_t error;
    int8_t bagslot = INVENTORY_SLOT_NOT_SET;

    if (amount < 1)
        amount = 1;

    _player->interruptSpell();

    Creature* unit = _player->GetMapMgr()->GetCreature(srlPacket.srcGuid.getGuidLowPart());
    if (unit == nullptr || !unit->HasItems())
        return;

    CreatureItem ci{};
    unit->GetSellItemByItemId(srlPacket.itemId, ci);

    if (ci.itemid == 0)
        return;

    if (ci.max_amount > 0 && ci.available_amount < amount)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_IS_CURRENTLY_SOLD_OUT);
        return;
    }

    ItemProperties const* it = sMySQLStore.getItemProperties(srlPacket.itemId);
    if (it == nullptr)
        return;

    uint32_t itemMaxStack = _player->m_cheats.ItemStackCheat ? 0x7fffffff : it->MaxCount;
    if (itemMaxStack > 0 && ci.amount * amount > itemMaxStack)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_CANT_CARRY_MORE_OF_THIS);
        return;
    }

    uint32_t count_per_stack = ci.amount * amount;

    // if slot is different than -1, check for validation, else continue for auto storing.
    if (slot != INVENTORY_SLOT_NOT_SET)
    {
        if (!(srlPacket.bagGuid >> 32))//buy to backpack
        {
            if (slot > INVENTORY_SLOT_ITEM_END || slot < INVENTORY_SLOT_ITEM_START)
            {
                //hackers!
                _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
                return;
            }
        }
        else
        {
            Container* c = dynamic_cast<Container*>(_player->getItemInterface()->GetItemByGUID(srlPacket.bagGuid));
            if (!c)
                return;
            bagslot = static_cast<int8_t>(_player->getItemInterface()->GetBagSlotByGuid(srlPacket.bagGuid));

            if (bagslot == INVENTORY_SLOT_NOT_SET || static_cast<uint32_t>(slot) > c->getItemProperties()->ContainerSlots)
            {
                _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
                return;
            }
        }
    }
    else
    {
        if (srlPacket.bagGuid >> 32)
        {
            Container* c = dynamic_cast<Container*>(_player->getItemInterface()->GetItemByGUID(srlPacket.bagGuid));
            if (!c)
            {
                _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_NOT_FOUND);
                return;//non empty
            }

            bagslot = static_cast<int8_t>(_player->getItemInterface()->GetBagSlotByGuid(srlPacket.bagGuid));
            slot = c->FindFreeSlot();
        }
        else
            slot = _player->getItemInterface()->FindFreeBackPackSlot();
    }

    if ((error = _player->getItemInterface()->CanReceiveItem(it, amount)) != 0)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    if ((error = _player->getItemInterface()->CanAffordItem(it, amount, unit)) != 0)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    if (slot == INVENTORY_SLOT_NOT_SET)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_BAG_FULL);
        return;
    }

    // ok our z and slot are set.
    Item* oldItem = nullptr;
    Item* pItem;
    if (slot != INVENTORY_SLOT_NOT_SET)
        oldItem = _player->getItemInterface()->GetInventoryItem(bagslot, slot);

    if (oldItem != nullptr)
    {
        // try to add to the existing items stack
        if (oldItem->getItemProperties() != it)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
            return;
        }

        if (oldItem->getStackCount() + count_per_stack > itemMaxStack)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_CANT_CARRY_MORE_OF_THIS);
            return;
        }

        oldItem->modStackCount(count_per_stack);
        oldItem->m_isDirty = true;
        pItem = oldItem;
    }
    else
    {
        if (slot == ITEM_NO_SLOT_AVAILABLE)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_BAG_FULL);
            return;
        }

        pItem = objmgr.CreateItem(it->ItemId, _player);
        if (pItem)
        {
            pItem->setStackCount(count_per_stack);
            pItem->m_isDirty = true;
            if (!_player->getItemInterface()->SafeAddItem(pItem, bagslot, slot))
            {
                pItem->DeleteMe();
                return;
            }
        }
        else
            return;
    }

    _player->sendItemPushResultPacket(false, true, false, bagslot, pItem != oldItem ? slot : 0,
        amount * ci.amount, pItem->getEntry(), pItem->getPropertySeed(),
        pItem->getRandomPropertiesId(), pItem->getStackCount());

    WorldPacket data(SMSG_BUY_ITEM, 22);
    data << uint64_t(srlPacket.srcGuid.GetOldGuid());
#if VERSION_STRING < Cata
    data << Util::getMSTime();
    data << uint32_t(srlPacket.itemId);
#else
    data << uint32_t(slot + 1);       // numbered from 1 at client
    data << int32_t(amount);
#endif
    data << uint32_t(amount);

    SendPacket(&data);

    LogDebugFlag(LF_OPCODE, "Sent SMSG_BUY_ITEM");

    _player->getItemInterface()->BuyItem(it, amount, unit);
    if (ci.max_amount)
    {
        unit->ModAvItemAmount(ci.itemid, ci.amount * amount);

        // there is probably a proper opcode for this. - burlex
        sendInventoryList(unit);
    }
}

void WorldSession::handleBuyItemOpcode(WorldPacket& recvPacket)
{
    CmsgBuyItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_BUY_ITEM");

    uint8_t error = 0;
    SlotResult slotResult;

    auto creature = _player->GetMapMgr()->GetCreature(srlPacket.sourceGuid.getGuidLowPart());
    if (creature == nullptr || !creature->HasItems())
        return;

    auto item_extended_cost = creature->GetItemExtendedCostByItemId(srlPacket.itemEntry);

    if (srlPacket.amount < 1)
        srlPacket.amount = 1;

    CreatureItem creature_item{};
    creature->GetSellItemByItemId(srlPacket.itemEntry, creature_item);

    if (creature_item.itemid == 0)
    {
        // vendor does not sell this item.. bitch about cheaters?
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_DONT_OWN_THAT_ITEM);
        return;
    }

    if (creature_item.max_amount > 0 && creature_item.available_amount < srlPacket.amount)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_IS_CURRENTLY_SOLD_OUT);
        return;
    }

    ItemProperties const* it = sMySQLStore.getItemProperties(srlPacket.itemEntry);
    if (!it)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_DONT_OWN_THAT_ITEM);
        return;
    }

    uint32_t itemMaxStack = _player->m_cheats.ItemStackCheat ? 0x7fffffff : it->MaxCount;
    if (itemMaxStack > 0 && srlPacket.amount * creature_item.amount > itemMaxStack)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_CANT_STACK);
        return;
    }

    if ((error = _player->getItemInterface()->CanReceiveItem(it, srlPacket.amount * creature_item.amount)) != 0)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    if ((error = _player->getItemInterface()->CanAffordItem(it, srlPacket.amount, creature)) != 0)
    {
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    // Find free slot and break if inv full
    auto addItem = _player->getItemInterface()->FindItemLessMax(srlPacket.itemEntry,
        srlPacket.amount * creature_item.amount, false);

    if (!addItem)
    {
        slotResult = _player->getItemInterface()->FindFreeInventorySlot(it);
    }

    if (!slotResult.Result && !addItem)
    {
        //Player doesn't have a free slot in his/her bag(s)
        _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
        return;
    }

    if (!addItem)
    {
        Item* item = objmgr.CreateItem(creature_item.itemid, _player);
        if (!item)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_DONT_OWN_THAT_ITEM);
            return;
        }

        item->m_isDirty = true;
        item->setStackCount(srlPacket.amount * creature_item.amount);

        if (slotResult.ContainerSlot == ITEM_NO_SLOT_AVAILABLE)
        {
            const auto addItemResult = _player->getItemInterface()->SafeAddItem(item, INVENTORY_SLOT_NOT_SET, slotResult.Slot);
            if (!addItemResult)
            {
                item->DeleteMe();
            }
            else
            {
                if (item->IsEligibleForRefund() && item_extended_cost != nullptr)
                {
                    item->getOwner()->getItemInterface()->AddRefundable(item->getGuid(), item_extended_cost->costid);
                }
                _player->sendItemPushResultPacket(false, true, false, static_cast<uint8_t>(INVENTORY_SLOT_NOT_SET),
                    slotResult.Result, srlPacket.amount * creature_item.amount, item->getEntry(), item->getPropertySeed(),
                    item->getRandomPropertiesId(), item->getStackCount());
            }
        }
        else
        {
            if (Item* bag = _player->getItemInterface()->GetInventoryItem(slotResult.ContainerSlot))
            {
                if (!dynamic_cast<Container*>(bag)->AddItem(slotResult.Slot, item))
                {
                    item->DeleteMe();
                }
                else
                {
                    if (item->IsEligibleForRefund() && item_extended_cost != nullptr)
                    {
                        item->getOwner()->getItemInterface()->AddRefundable(item->getGuid(), item_extended_cost->costid);
                    }
                    _player->sendItemPushResultPacket(false, true, false, slotResult.ContainerSlot,
                        slotResult.Result, 1, item->getEntry(), item->getPropertySeed(),
                        item->getRandomPropertiesId(), item->getStackCount());
                }
            }
        }
    }
    else
    {
        addItem->modStackCount(srlPacket.amount * creature_item.amount);
        addItem->m_isDirty = true;
        _player->sendItemPushResultPacket(false, true, false, 
            static_cast<uint8_t>(_player->getItemInterface()->GetBagSlotByGuid(addItem->getGuid())), 0,
            srlPacket.amount * creature_item.amount, addItem->getEntry(), addItem->getPropertySeed(),
            addItem->getRandomPropertiesId(), addItem->getStackCount());
    }

    _player->getItemInterface()->BuyItem(it, srlPacket.amount, creature);

    SendPacket(SmsgBuyItem(srlPacket.sourceGuid.GetOldGuid(), Util::getMSTime(), srlPacket.itemEntry,
        srlPacket.amount * creature_item.amount).serialise().get());

    if (creature_item.max_amount)
    {
        creature->ModAvItemAmount(creature_item.itemid, creature_item.amount * srlPacket.amount);

        sendInventoryList(creature);
    }
}

void WorldSession::handleListInventoryOpcode(WorldPacket& recvPacket)
{
    CmsgListInventory srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    WoWGuid wowGuid;
    wowGuid.Init(srlPacket.guid);

    Creature* unit = _player->GetMapMgr()->GetCreature(wowGuid.getGuidLowPart());
    if (unit == nullptr)
        return;

    MySQLStructure::VendorRestrictions const* vendor = sMySQLStore.getVendorRestriction(unit->GetCreatureProperties()->Id);

    //this is a blizzlike check
#if VERSION_STRING < Cata
    if (!_player->obj_movement_info.isOnTransport())
#else
    if (_player->obj_movement_info.getTransportGuid().IsEmpty())
#endif
    {
        //avoid talking to anyone by guid hacking. Like sell farmed items anytime ? Low chance hack
        if (_player->getDistanceSq(unit) > 100)
            return;
    }

    if (unit->GetAIInterface())
        unit->GetAIInterface()->StopMovement(180000);

    _player->Reputation_OnTalk(unit->m_factionEntry);

    if (_player->CanBuyAt(vendor))
        sendInventoryList(unit);
    else
        Arcemu::Gossip::Menu::SendSimpleMenu(unit->getGuid(), vendor->cannotbuyattextid, _player);
}

void WorldSession::sendInventoryList(Creature* unit)
{
    if (!unit->HasItems())
    {
        sChatHandler.BlueSystemMessage(_player->GetSession(),
            "No sell template found. Report this to database's devs: %d (%s)",
            unit->getEntry(), unit->GetCreatureProperties()->Name.c_str());
        LOG_ERROR("'%s' discovered that a creature with entry %u (%s) has no sell template.",
            _player->getName().c_str(), unit->getEntry(), unit->GetCreatureProperties()->Name.c_str());
        Arcemu::Gossip::Menu::Complete(_player);
        return;
    }

#if VERSION_STRING < Cata
    WorldPacket data((unit->GetSellItemCount() * 28 + 9));       // allocate
    data.SetOpcode(SMSG_LIST_INVENTORY);
    data << unit->getGuid();
    data << uint8_t(0);   // placeholder for item count
#else
    WorldPacket data(((unit->GetSellItemCount()) + 12));       // allocate
    ByteBuffer itemsData(32 * unit->GetSellItemCount());
    std::vector<bool> enablers;
    enablers.reserve(2 * unit->GetSellItemCount());

#endif

    uint32_t counter = 0;
    for (auto sellItem : *unit->getSellItems())
    {
        if (sellItem.itemid && (sellItem.max_amount == 0 || sellItem.max_amount > 0 && sellItem.available_amount > 0))
        {
            if (const auto curItem = sMySQLStore.getItemProperties(sellItem.itemid))
            {
                // looking up everything for active gms
                if (!_player->isGMFlagSet() && !worldConfig.player.showAllVendorItems)
                {
                    if (curItem->AllowableClass && !(_player->getClassMask() & curItem->AllowableClass))
                        continue;

                    if (curItem->AllowableRace && !(_player->getRaceMask() & curItem->AllowableRace))
                        continue;

                    if (curItem->HasFlag2(ITEM_FLAG2_HORDE_ONLY) && !_player->isTeamHorde())
                        continue;

                    if (curItem->HasFlag2(ITEM_FLAG2_ALLIANCE_ONLY) && !_player->isTeamAlliance())
                        continue;
                }

                uint32_t av_am = sellItem.max_amount > 0 ? sellItem.available_amount : 0xFFFFFFFF;
                uint32_t price = 0;
                if (sellItem.extended_cost == nullptr || curItem->HasFlag2(ITEM_FLAG2_EXT_COST_REQUIRES_GOLD))
                    price = GetBuyPriceForItem(curItem, 1, _player, unit);

#if VERSION_STRING < Cata
                data << uint32_t(counter + 1);    // we start from 0 but client starts from 1
                data << uint32_t(curItem->ItemId);
                data << uint32_t(curItem->DisplayInfoID);
                data << uint32_t(av_am);
                data << uint32_t(price);
                data << uint32_t(curItem->MaxDurability);
                data << uint32_t(sellItem.amount);


                if (sellItem.extended_cost != nullptr)
                    data << uint32_t(sellItem.extended_cost->costid);
                else
                    data << uint32_t(0);
#else
                itemsData << uint32_t(counter + 1);        // client expects counting to start at 1
                itemsData << uint32_t(curItem->MaxDurability);
                if (sellItem.extended_cost != nullptr)
                {
                    enablers.push_back(0);
                    itemsData << uint32_t(sellItem.extended_cost->costid);
                }
                else
                {
                    enablers.push_back(1);
                }

                enablers.push_back(1);                 // unk bit

                itemsData << uint32_t(curItem->ItemId);
                itemsData << uint32_t(1);     // 1 is items, 2 is currency
                itemsData << uint32_t(price);
                itemsData << uint32_t(curItem->DisplayInfoID);
                itemsData << int32_t(av_am);
                itemsData << uint32_t(sellItem.amount);
#endif

                ++counter;
                if (counter >= creatureMaxInventoryItems)
                    break;
            }
        }
    }

#if VERSION_STRING < Cata
    const_cast<uint8_t*>(data.contents())[8] = static_cast<uint8_t>(counter);
#else
    ObjectGuid guid = unit->getGuid();

    data.SetOpcode(SMSG_LIST_INVENTORY);
    data.writeBit(guid[1]);
    data.writeBit(guid[0]);

    data.writeBits(counter, 21); // item count

    data.writeBit(guid[3]);
    data.writeBit(guid[6]);
    data.writeBit(guid[5]);
    data.writeBit(guid[2]);
    data.writeBit(guid[7]);

    for (std::vector<bool>::const_iterator itr = enablers.begin(); itr != enablers.end(); ++itr)
        data.writeBit(*itr);

    data.writeBit(guid[4]);

    data.flushBits();
    data.append(itemsData);

    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[6]);

    data << uint8_t(counter == 0); // unk byte, item count 0: 1, item count != 0: 0 or some "random" value below 300

    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[7]);
#endif

    SendPacket(&data);

    LogDebugFlag(LF_OPCODE, "Sent SMSG_LIST_INVENTORY");
}

void WorldSession::handleAutoStoreBagItemOpcode(WorldPacket& recvPacket)
{
    CmsgAutostoreBagItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUTO_STORE_BAG_ITEM");

    Item* srcitem = _player->getItemInterface()->GetInventoryItem(srlPacket.srcContainerSlot, srlPacket.srcSlot);

    //source item exists
    int8_t NewSlot = 0;
    AddItemResult result;

    if (srcitem)
    {
        //src containers cant be moved if they have items inside
        if (srcitem->isContainer() && dynamic_cast<Container*>(srcitem)->HasItems())
        {
            _player->getItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
            return;
        }
        //check for destination now before swaping.
        //destination is backpack
        if (srlPacket.dstContainerSlot == INVENTORY_SLOT_NOT_SET)
        {
            //check for space
            NewSlot = _player->getItemInterface()->FindFreeBackPackSlot();
            if (NewSlot == ITEM_NO_SLOT_AVAILABLE)
            {
                _player->getItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_BAG_FULL);
                return;
            }
            else
            {
                //free space found, remove item and add it to the destination
                srcitem = _player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(srlPacket.srcContainerSlot,
                    srlPacket.srcSlot, false);
                if (srcitem)
                {
                    result = _player->getItemInterface()->SafeAddItem(srcitem, INVENTORY_SLOT_NOT_SET, NewSlot);
                    if (!result)
                    {
                        LOG_ERROR("Error while adding item to newslot");
                        srcitem->DeleteMe();
                        return;
                    }
                }
            }
        }
        else
        {
            const int8_t error = _player->getItemInterface()->CanEquipItemInSlot2(srlPacket.dstContainerSlot,
                srlPacket.dstContainerSlot, srcitem);
            if (error != 0)
            {
                if (srlPacket.dstContainerSlot < INVENTORY_KEYRING_END)
                {
                    _player->getItemInterface()->BuildInventoryChangeError(srcitem, nullptr, error);
                    return;
                }
            }

            //destination is a bag
            if (Item* dstitem = _player->getItemInterface()->GetInventoryItem(srlPacket.dstContainerSlot))
            {
                //dstitem exists, detect if its a container
                if (dstitem->isContainer())
                {
                    NewSlot = dynamic_cast<Container*>(dstitem)->FindFreeSlot();
                    if (NewSlot == ITEM_NO_SLOT_AVAILABLE)
                    {
                        _player->getItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_BAG_FULL);
                        return;
                    }
                    else
                    {
                        srcitem = _player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(srlPacket.srcContainerSlot,
                            srlPacket.srcSlot, false);
                        if (srcitem != nullptr)
                        {
                            result = _player->getItemInterface()->SafeAddItem(srcitem, srlPacket.dstContainerSlot, NewSlot);
                            if (!result)
                            {
                                LOG_ERROR("Error while adding item to newslot");
                                srcitem->DeleteMe();
                            }
                        }
                    }
                }
                else
                {
                    _player->getItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
                }
            }
            else
            {
                _player->getItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
            }
        }
    }
    else
    {
        _player->getItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_ITEM_NOT_FOUND);
    }
}

void WorldSession::handleReadItemOpcode(WorldPacket& recvPacket)
{
    CmsgReadItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_READ_ITEM %d", srlPacket.srcSlot);

    Item* item = _player->getItemInterface()->GetInventoryItem(srlPacket.srcContainerSlot, srlPacket.srcSlot);
    if (item)
    {
        // Check if it has pagetext
        if (item->getItemProperties()->PageId)
            SendPacket(SmsgReadItemOk(item->getGuid()).serialise().get());
        else
            SendPacket(SmsgReadItemFailed(item->getGuid(), 2).serialise().get());
    }
}

void WorldSession::handleRepairItemOpcode(WorldPacket& recvPacket)
{
    CmsgRepairItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(srlPacket.creatureGuid.getGuidLowPart());
    if (pCreature == nullptr)
        return;

    if (!pCreature->isArmorer())
        return;

    //this is a blizzlike check
    if (_player->getDistanceSq(pCreature) > 100)
        return; //avoid talking to anyone by guid hacking. Like repair items anytime in raid ? Low chance hack

    if (!srlPacket.itemGuid)
    {
        int32_t totalcost = 0;
        for (uint32_t i = 0; i < MAX_INVENTORY_SLOT; i++)
        {
            Item* pItem = _player->getItemInterface()->GetInventoryItem(static_cast<int16_t>(i));
            if (pItem != nullptr)
            {
                if (pItem->isContainer())
                {
                    Container* pContainer = dynamic_cast<Container*>(pItem);
                    for (uint32_t j = 0; j < pContainer->getItemProperties()->ContainerSlots; ++j)
                    {
                        pItem = pContainer->GetItem(static_cast<int16_t>(j));
                        if (pItem != nullptr)
                            pItem->RepairItem(_player, srlPacket.isInGuild, &totalcost);
                    }
                }
                else
                {
                    if (i < INVENTORY_SLOT_BAG_END)
                    {
                        if (pItem->getDurability() == 0 && pItem->RepairItem(_player, srlPacket.isInGuild, &totalcost))
                            _player->ApplyItemMods(pItem, static_cast<int16_t>(i), true);
                        else
                            pItem->RepairItem(_player, srlPacket.isInGuild, &totalcost);
                    }
                }
            }
        }
    }
    else
    {
        Item* item = _player->getItemInterface()->GetItemByGUID(srlPacket.itemGuid);
        if (item)
        {
            //this never gets null since we get a pointer to the inteface internal var
            SlotResult* searchres = _player->getItemInterface()->LastSearchResult();
            uint32_t dDurability = item->getMaxDurability() - item->getDurability();

            if (dDurability)
            {
                uint32_t cDurability = item->getDurability();
                //only apply item mods if they are on char equipped
                if (item->RepairItem(_player) && cDurability == 0
                    && searchres->ContainerSlot == static_cast<int8_t>(INVALID_BACKPACK_SLOT)
                    && searchres->Slot < static_cast<int8_t>(INVENTORY_SLOT_BAG_END))
                    _player->ApplyItemMods(item, searchres->Slot, true);
            }
        }
    }
    LogDebugFlag(LF_OPCODE, "Received CMSG_REPAIR_ITEM %d", srlPacket.itemGuid);
}

void WorldSession::handleAutoBankItemOpcode(WorldPacket& recvPacket)
{
    CmsgAutobankItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUTO_BANK_ITEM Inventory slot: %u Source Slot: %u",
        static_cast<uint32_t>(srlPacket.srcInventorySlot), static_cast<uint32_t>(srlPacket.srcSlot));

    Item* eitem = _player->getItemInterface()->GetInventoryItem(srlPacket.srcInventorySlot, srlPacket.srcSlot);
    if (!eitem)
    {
        _player->getItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    const SlotResult slotresult = _player->getItemInterface()->FindFreeBankSlot(eitem->getItemProperties());
    if (!slotresult.Result)
    {
        _player->getItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_BANK_FULL);
    }
    else
    {
        eitem = _player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(srlPacket.srcInventorySlot,
            srlPacket.srcSlot, false);
        if (eitem == nullptr)
            return;

        if (!_player->getItemInterface()->SafeAddItem(eitem, slotresult.ContainerSlot, slotresult.Slot))
        {
            LOG_ERROR("Error while adding item to bank bag!");
            if (!_player->getItemInterface()->SafeAddItem(eitem, srlPacket.srcInventorySlot, srlPacket.srcSlot))
                eitem->DeleteMe();
        }
    }
}

void WorldSession::handleAutoStoreBankItemOpcode(WorldPacket& recvPacket)
{
    CmsgAutostoreBankItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUTOSTORE_BANK_ITEM Inventory slot: %u Source Slot: %u",
        static_cast<uint32_t>(srlPacket.srcInventorySlot), static_cast<uint32_t>(srlPacket.srcSlot));

    Item* eitem = _player->getItemInterface()->GetInventoryItem(srlPacket.srcInventorySlot, srlPacket.srcSlot);
    if (!eitem)
    {
        _player->getItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    const SlotResult slotresult = _player->getItemInterface()->FindFreeInventorySlot(eitem->getItemProperties());
    if (!slotresult.Result)
    {
        _player->getItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_INVENTORY_FULL);
    }
    else
    {
        eitem = _player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(srlPacket.srcInventorySlot,
            srlPacket.srcSlot, false);
        if (eitem == nullptr)
            return;

        if (!_player->getItemInterface()->AddItemToFreeSlot(eitem))
        {
            LOG_ERROR("Error while adding item from one of the bank bags to the player bag!");
            if (!_player->getItemInterface()->SafeAddItem(eitem, srlPacket.srcInventorySlot, srlPacket.srcSlot))
                eitem->DeleteMe();
        }
    }
}

void WorldSession::handleCancelTemporaryEnchantmentOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgCancelTempEnchantment srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    Item* item = _player->getItemInterface()->GetInventoryItem(static_cast<int16_t>(srlPacket.inventorySlot));
    if (!item)
        return;

    item->RemoveAllEnchantments(true);
}

void WorldSession::handleInsertGemOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgSocketGems srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    ItemInterface* itemi = _player->getItemInterface();
    Item* TargetItem = itemi->GetItemByGUID(srlPacket.itemGuid);
    if (!TargetItem)
        return;

    ItemProperties const* TargetProto = TargetItem->getItemProperties();
    int slot = itemi->GetInventorySlotByGuid(srlPacket.itemGuid);

    bool apply = slot >= 0 && slot < 19;
    uint32_t FilledSlots = 0;

    //cheat -> tried to socket same gem multiple times
    if (srlPacket.gemGuid[0]
        && (srlPacket.gemGuid[0] == srlPacket.gemGuid[1]  || srlPacket.gemGuid[0] == srlPacket.gemGuid[2])
        || srlPacket.gemGuid[1] && srlPacket.gemGuid[1] == srlPacket.gemGuid[2])
    {
        return;
    }

    bool ColorMatch = true;
    DBC::Structures::GemPropertiesEntry const* gem_properties;
    DBC::Structures::SpellItemEnchantmentEntry const* spell_item_enchant;

    for (uint32_t i = 0; i < TargetItem->GetSocketsCount(); ++i)
    {
        EnchantmentInstance* EI = TargetItem->GetEnchantment(SOCK_ENCHANTMENT_SLOT1 + i);
        if (EI)
        {
            FilledSlots++;
            ItemProperties const* ip = sMySQLStore.getItemProperties(EI->Enchantment->GemEntry);
            if (ip == nullptr)
                gem_properties = nullptr;
            else
                gem_properties = sGemPropertiesStore.LookupEntry(ip->GemProperties);

            if (gem_properties && !(gem_properties->SocketMask & TargetProto->Sockets[i].SocketColor))
                ColorMatch = false;
        }

        if (srlPacket.gemGuid[i])  //add or replace gem
        {
            Item* it = nullptr;
            ItemProperties const* ip = nullptr;

            // tried to put gem in socket where no socket exists (take care about prismatic sockets)
            if (!TargetProto->Sockets[i].SocketColor)
            {
                // no prismatic socket
                if (!TargetItem->GetEnchantment(PRISMATIC_ENCHANTMENT_SLOT))
                    return;

                // not first not-colored (not normally used) socket
                if (i != 0 && !TargetProto->Sockets[i - 1].SocketColor && (i + 1 >= 3
                    || TargetProto->Sockets[i + 1].SocketColor))
                    return;

                // ok, this is first not colored socket for item with prismatic socket
            }

            if (apply)
            {
                it = itemi->GetItemByGUID(srlPacket.gemGuid[i]);
                if (!it)
                    continue;

                ip = it->getItemProperties();
                if (ip->Flags & ITEM_FLAG_UNIQUE_EQUIP && itemi->IsEquipped(ip->ItemId))
                {
                    itemi->BuildInventoryChangeError(it, TargetItem, INV_ERR_CANT_CARRY_MORE_OF_THIS);
                    continue;
                }

                // Skill requirement
                if (ip->RequiredSkill)
                {
                    if (ip->RequiredSkillRank > _player->_GetSkillLineCurrent(ip->RequiredSkill, true))
                    {
                        itemi->BuildInventoryChangeError(it, TargetItem, INV_ERR_SKILL_ISNT_HIGH_ENOUGH);
                        continue;
                    }
                }
#if VERSION_STRING > TBC
                if (ip->ItemLimitCategory)
                {
                    auto item_limit_category = sItemLimitCategoryStore.LookupEntry(ip->ItemLimitCategory);
                    if (item_limit_category != nullptr
                        && itemi->GetEquippedCountByItemLimit(ip->ItemLimitCategory) >= item_limit_category->maxAmount)
                    {
                        itemi->BuildInventoryChangeError(it, TargetItem, INV_ERR_ITEM_MAX_COUNT_EQUIPPED_SOCKETED);
                        continue;
                    }
                }
#endif
            }

            it = itemi->SafeRemoveAndRetreiveItemByGuid(srlPacket.gemGuid[i], true);
            if (!it)
                return; //someone sending hacked packets to crash server

            gem_properties = sGemPropertiesStore.LookupEntry(it->getItemProperties()->GemProperties);
            it->DeleteMe();

            if (!gem_properties)
                continue;

            if (!(gem_properties->SocketMask & TargetProto->Sockets[i].SocketColor))
                ColorMatch = false;

            //this is ok in few cases
            if (!gem_properties->EnchantmentID)
                continue;

            //Meta gems only go in meta sockets.
            if (TargetProto->Sockets[i].SocketColor != GEM_META_SOCKET && gem_properties->SocketMask == GEM_META_SOCKET)
                continue;

            //replace gem
            if (EI)
                TargetItem->RemoveEnchantment(2 + i); //remove previous
            else//add gem
                FilledSlots++;

            spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(gem_properties->EnchantmentID);
            if (spell_item_enchant != nullptr)
            {
                if (TargetItem->getItemProperties()->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
                    TargetItem->AddEnchantment(spell_item_enchant, 0, true, apply, false, 2 + i);
            }

        }
    }

    //Add color match bonus
    if (TargetItem->getItemProperties()->SocketBonus)
    {
        if (ColorMatch && FilledSlots == TargetItem->GetSocketsCount())
        {
            if (TargetItem->HasEnchantment(TargetItem->getItemProperties()->SocketBonus) > 0)
                return;

            spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(TargetItem->getItemProperties()->SocketBonus);
            if (spell_item_enchant != nullptr)
            {
                if (TargetItem->getItemProperties()->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
                {
                    int32_t Slot = TargetItem->FindFreeEnchantSlot(spell_item_enchant, 0);
                    TargetItem->AddEnchantment(spell_item_enchant, 0, true, apply, false, Slot);
                }
            }
        }
        else  //remove
        {
            TargetItem->RemoveSocketBonusEnchant();
        }
    }

    TargetItem->m_isDirty = true;
}

void WorldSession::handleWrapItemOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgWrapItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    Item* src = _player->getItemInterface()->GetInventoryItem(srlPacket.srcBagSlot, srlPacket.srcSlot);
    Item* dst = _player->getItemInterface()->GetInventoryItem(srlPacket.destBagSlot, srlPacket.destSlot);

    if (!src || !dst)
        return;

    if (src == dst || !(src->getItemProperties()->Class == 0 && src->getItemProperties()->SubClass == 8))
    {
        _player->getItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_WRAPPED_CANT_BE_WRAPPED);
        return;
    }

    if (dst->getStackCount() > 1)
    {
        _player->getItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_STACKABLE_CANT_BE_WRAPPED);
        return;
    }

    uint32_t dstItemMaxStack = dst->getOwner()->m_cheats.ItemStackCheat ? 0x7fffffff : dst->getItemProperties()->MaxCount;
    if (dstItemMaxStack > 1)
    {
        _player->getItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_STACKABLE_CANT_BE_WRAPPED);
        return;
    }

    if (dst->isSoulbound())
    {
        _player->getItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_BOUND_CANT_BE_WRAPPED);
        return;
    }

    if (dst->wrapped_item_id || src->wrapped_item_id)
    {
        _player->getItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_WRAPPED_CANT_BE_WRAPPED);
        return;
    }

    if (dst->getItemProperties()->Unique)
    {
        _player->getItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_UNIQUE_CANT_BE_WRAPPED);
        return;
    }

    if (dst->isContainer())
    {
        _player->getItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_BAGS_CANT_BE_WRAPPED);
        return;
    }

    if (dst->HasEnchantments())
    {
        _player->getItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_ITEM_LOCKED);
        return;
    }
    if (srlPacket.destBagSlot == -1
        && (srlPacket.destSlot >= int8_t(EQUIPMENT_SLOT_START)
        && srlPacket.destSlot <= int8_t(INVENTORY_SLOT_BAG_END)))
    {
        _player->getItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_EQUIPPED_CANT_BE_WRAPPED);
        return;
    }

    // all checks passed ok
    uint32_t source_entry = src->getEntry();
    uint32_t itemid = source_entry;
    switch (source_entry)
    {
        case 5042:
            itemid = 5043;
            break;
        case 5048:
            itemid = 5044;
            break;
        case 17303:
            itemid = 17302;
            break;
        case 17304:
            itemid = 17305;
            break;
        case 17307:
            itemid = 17308;
            break;
        case 21830:
            itemid = 21831;
            break;
        default:
            _player->getItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_WRAPPED_CANT_BE_WRAPPED);
            return;
    }

    dst->setItemProperties(src->getItemProperties());

    if (src->getStackCount() <= 1)
    {
        // destroy the source item
        _player->getItemInterface()->SafeFullRemoveItemByGuid(src->getGuid());
    }
    else
    {
        // reduce stack count by one
        src->modStackCount(-1);
        src->m_isDirty = true;
    }

    // change the dest item's entry
    dst->wrapped_item_id = dst->getEntry();
    dst->setEntry(itemid);

    // set the giftwrapper fields
    dst->setGiftCreatorGuid(_player->getGuid());
    dst->setDurability(0);
    dst->setMaxDurability(0);
    dst->addFlags(ITEM_FLAG_WRAPPED);

    // save it
    dst->m_isDirty = true;
    dst->SaveToDB(srlPacket.destBagSlot, srlPacket.destSlot, false, nullptr);
}

#if VERSION_STRING > TBC
void WorldSession::handleEquipmentSetUse(WorldPacket& data)
{
    CHECK_INWORLD_RETURN
    
    LogDebugFlag(LF_OPCODE, "Received CMSG_EQUIPMENT_SET_USE");

    WoWGuid guid;
    int8_t SrcBagID;
    uint8_t SrcSlotID;
    uint8_t result = 0;

    for (int8_t i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        guid.Clear();

        data >> guid;
        data >> SrcBagID;
        data >> SrcSlotID;

        const uint64_t ItemGUID = guid.GetOldGuid();

        const auto item = _player->getItemInterface()->GetItemByGUID(ItemGUID);
        if (item == nullptr)
        {
            result = 1;
            continue;
        }

        const int8_t destSlot = i;
        const int8_t destBag = static_cast<int8_t>(INVALID_BACKPACK_SLOT);

        if (SrcBagID == destBag && (SrcSlotID == destSlot))
            continue;

        auto dstslotitem = _player->getItemInterface()->GetInventoryItem(destSlot);
        if (dstslotitem == nullptr)
        {
            const int8_t equipError = _player->getItemInterface()->CanEquipItemInSlot(destBag, destSlot, item->getItemProperties(), false, false);
            if (equipError == INV_ERR_OK)
            {
                dstslotitem = _player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcBagID, SrcSlotID, false);
                const auto itemResult = _player->getItemInterface()->SafeAddItem(item, destBag, destSlot);
                if (itemResult != ADD_ITEM_RESULT_OK)
                {
                    const auto addItemResult = _player->getItemInterface()->SafeAddItem(item, SrcBagID, SrcSlotID);
                    if (!addItemResult)
                    {
                        LOG_ERROR("handleEquipmentSetUse", "Error while adding item %u to player %s twice", item->getEntry(), _player->getName().c_str());
                        result = 0;
                    }
                    else
                        result = 1;
                }
            }
            else
            {
                result = 1;
            }

        }
        else
        {
            // There is something equipped so we need to swap
            if (!_player->getItemInterface()->SwapItems(INVALID_BACKPACK_SLOT, destSlot, SrcBagID, SrcSlotID))
                result = 1;
        }

    }

    _player->sendEquipmentSetUseResultPacket(result);
}

void WorldSession::handleEquipmentSetSave(WorldPacket& data)
{
    CHECK_INWORLD_RETURN
    
    LogDebugFlag(LF_OPCODE, "Received CMSG_EQUIPMENT_SET_SAVE");

    WoWGuid guid;

    data >> guid;

    uint32_t setGUID = guid.getGuidLowPart();

    if (setGUID == 0)
        setGUID = objmgr.GenerateEquipmentSetID();

    auto equipmentSet = new Arcemu::EquipmentSet();

    equipmentSet->SetGUID = setGUID;

    data >> equipmentSet->SetID;
    data >> equipmentSet->SetName;
    data >> equipmentSet->IconName;

    for (uint32_t i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        guid.Clear();
        data >> guid;
        equipmentSet->ItemGUID[i] = guid.getGuidLowPart();
    }

    if (_player->getItemInterface()->m_EquipmentSets.AddEquipmentSet(equipmentSet->SetGUID, equipmentSet))
    {
        LogDebugFlag(LF_OPCODE, "Player %u successfully stored equipment set %u at slot %u ", _player->getGuidLow(), equipmentSet->SetGUID, equipmentSet->SetID);
        _player->SendEquipmentSetSaved(equipmentSet->SetID, equipmentSet->SetGUID);
    }
    else
    {
        LogDebugFlag(LF_OPCODE, "Player %u couldn't store equipment set %u at slot %u ", _player->getGuidLow(), equipmentSet->SetGUID, equipmentSet->SetID);
    }
}

void WorldSession::handleEquipmentSetDelete(WorldPacket& data)
{
    CHECK_INWORLD_RETURN
    
    LogDebugFlag(LF_OPCODE, "Received CMSG_EQUIPMENT_SET_DELETE");

    WoWGuid guid;

    data >> guid;

    if (_player->getItemInterface()->m_EquipmentSets.DeleteEquipmentSet(guid.getGuidLowPart()))
        LOG_DEBUG("Equipmentset with GUID %u was successfully deleted.", guid.getGuidLowPart());
    else
        LOG_DEBUG("Equipmentset with GUID %u couldn't be deleted.", guid.getGuidLowPart());

}
#endif

void WorldSession::sendBuyFailed(uint64_t guid, uint32_t itemid, uint8_t error)
{
    SendPacket(SmsgBuyFailed(guid, itemid, error).serialise().get());
}

void WorldSession::sendSellItem(uint64_t vendorguid, uint64_t itemid, uint8_t error)
{
    WorldPacket data(SMSG_SELL_ITEM, 17);
    data << vendorguid;
    data << itemid;
    data << error;
    SendPacket(&data);
}
