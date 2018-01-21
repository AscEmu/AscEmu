/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/Container.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"

void WorldSession::HandleItemRefundInfoOpcode(WorldPacket& recvPacket)
{
    uint64_t guid;
    recvPacket >> guid;

    SendRefundInfo(guid);
}

void WorldSession::SendRefundInfo(uint64_t guid)
{
    if (!_player || !_player->IsInWorld())
        return;

    Item* item = _player->GetItemInterface()->GetItemByGUID(guid);
    if (item == nullptr)
        return;

    if (item->IsEligibleForRefund())
    {
        std::pair<time_t, uint32_t> refundEntry = _player->GetItemInterface()->LookupRefundable(guid);

        if (refundEntry.first == 0 || refundEntry.second == 0)
            return;

        auto item_extended_cost = sItemExtendedCostStore.LookupEntry(refundEntry.second);
        if (item_extended_cost == nullptr)
            return;

        ItemProperties const* item_properties = item->GetItemProperties();
        item->SetFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_REFUNDABLE);

        ObjectGuid objectGuid = item->GetGUID();
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

        if (played[1] >(refundEntry.first + 60 * 60 * 2))
            data << uint32_t(0);
        else
            data << uint32_t(refundEntry.first);

        for (uint8_t i = 0; i < 5; ++i)
        {
            data << uint32_t(item_extended_cost->item[i]);
            data << uint32_t(item_extended_cost->count[i]);
        }

        data.WriteByteSeq(objectGuid[6]);
        data.WriteByteSeq(objectGuid[4]);
        data.WriteByteSeq(objectGuid[3]);
        data.WriteByteSeq(objectGuid[2]);
        for (uint8_t i = 0; i < 5; ++i)
        {
            data << uint32_t(item_extended_cost->reqcurrcount[i]);
            data << uint32_t(item_extended_cost->reqcur[i]);
        }

        data.WriteByteSeq(objectGuid[1]);
        data.WriteByteSeq(objectGuid[5]);
        data << uint32_t(0);
        data.WriteByteSeq(objectGuid[0]);
        data << uint32_t(item_properties->BuyPrice);

        SendPacket(&data);
    }
}

//\todo Rewrite for cata - after this all functions are copied from wotlk

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

void WorldSession::HandleSplitOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 8);

    int8 DstInvSlot = 0;
    int8 DstSlot = 0;
    int8 SrcInvSlot = 0;
    int8 SrcSlot = 0;
    int32 count = 0;

    recvData >> SrcInvSlot;
    recvData >> SrcSlot;
    recvData >> DstInvSlot;
    recvData >> DstSlot;
    recvData >> count;

    /* exploit fix */
    if (count <= 0 || (SrcInvSlot <= 0 && SrcSlot < INVENTORY_SLOT_ITEM_START))
    {
        sCheatLog.writefromsession(this, "tried to split item: SrcInvSlot %d, SrcSlot %d, DstInvSlot %d, DstSlot %d, count %l", SrcInvSlot, SrcSlot, DstInvSlot, DstSlot, count);
        return;
    }

    if (!VerifyBagSlots(SrcInvSlot, SrcSlot))
        return;

    if (!VerifyBagSlots(DstInvSlot, DstSlot))
        return;

    uint32 c = count;

    auto i1 = _player->GetItemInterface()->GetInventoryItem(SrcInvSlot, SrcSlot);
    if (!i1)
        return;
    auto i2 = _player->GetItemInterface()->GetInventoryItem(DstInvSlot, DstSlot);

    uint32 itemMaxStack1 = (i1->GetOwner()->ItemStackCheat) ? 0x7fffffff : i1->GetItemProperties()->MaxCount;
    uint32 itemMaxStack2 = (i2) ? ((i2->GetOwner()->ItemStackCheat) ? 0x7fffffff : i2->GetItemProperties()->MaxCount) : 0;
    if ((i1 && i1->wrapped_item_id) || (i2 && i2->wrapped_item_id) || (c > itemMaxStack1))
    {
        GetPlayer()->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_ITEM_CANT_STACK);
        return;
    }

    // something already in this slot
    if (i2)
    {
        if (i1->GetEntry() == i2->GetEntry())
        {
            //check if player has the required stacks to avoid exploiting.
            //safe exploit check
            if (c < i1->GetStackCount())
            {
                //check if there is room on the other item.
                if (((c + i2->GetStackCount()) <= itemMaxStack2))
                {
                    i1->ModStackCount(-count);
                    i2->ModStackCount(c);
                    i1->m_isDirty = true;
                    i2->m_isDirty = true;
                }
                else
                {
                    GetPlayer()->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_ITEM_CANT_STACK);
                }
            }
            else
            {
                //error cant split item
                _player->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_COULDNT_SPLIT_ITEMS);
            }
        }
        else
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_ITEM_CANT_STACK);
        }
    }
    else
    {
        if (c < i1->GetStackCount())
        {
            i1->ModStackCount(-count);

            i2 = objmgr.CreateItem(i1->GetEntry(), _player);
            if (i2 == nullptr)
                return;

            i2->SetStackCount(c);
            i1->m_isDirty = true;
            i2->m_isDirty = true;

            if (DstSlot == ITEM_NO_SLOT_AVAILABLE)
            {
                if (DstInvSlot != ITEM_NO_SLOT_AVAILABLE)
                {
                    Container *container = _player->GetItemInterface()->GetContainer(DstInvSlot);
                    if (container != nullptr)
                        DstSlot = container->FindFreeSlot();
                }
                else
                {
                    // Find a free slot
                    SlotResult res = _player->GetItemInterface()->FindFreeInventorySlot(i2->GetItemProperties());
                    if (res.Result)
                    {
                        DstSlot = res.Slot;
                        DstInvSlot = res.ContainerSlot;
                    }
                }

                if (DstSlot == ITEM_NO_SLOT_AVAILABLE)
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_COULDNT_SPLIT_ITEMS);
                    i2->DeleteFromDB();
                    i2->DeleteMe();
                    i2 = nullptr;
                }
            }

            AddItemResult result = _player->GetItemInterface()->SafeAddItem(i2, DstInvSlot, DstSlot);
            if (result == ADD_ITEM_RESULT_ERROR)
            {
                LOG_ERROR("HandleSplit: Error while adding item to dstslot");
                if (i2 != nullptr)
                {
                    i2->DeleteFromDB();
                    i2->DeleteMe();
                    i2 = nullptr;
                };
            }
        }
        else
        {
            _player->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_COULDNT_SPLIT_ITEMS);
        }
    }
}

void WorldSession::HandleSwapItemOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 4);

    int8 DstInvSlot = 0;
    int8 DstSlot = 0;
    int8 SrcInvSlot = 0;
    int8 SrcSlot = 0;

    recvData >> DstInvSlot;
    recvData >> DstSlot;
    recvData >> SrcInvSlot;
    recvData >> SrcSlot;

    LOG_DETAIL("ITEM: swap, DstInvSlot %i DstSlot %i SrcInvSlot %i SrcSlot %i", DstInvSlot, DstSlot, SrcInvSlot, SrcSlot);

    _player->GetItemInterface()->SwapItems(DstInvSlot, DstSlot, SrcInvSlot, SrcSlot);
}

void WorldSession::HandleSwapInvItemOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 2);
    WorldPacket data;
    int8 srcslot = 0;
    int8 dstslot = 0;
    int8 error = 0;

    recvData >> dstslot;
    recvData >> srcslot;

    LOG_DETAIL("ITEM: swap, src slot: %u dst slot: %u", (uint32)srcslot, (uint32)dstslot);

    if (dstslot == srcslot) // player trying to add item to the same slot
    {
        GetPlayer()->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEMS_CANT_BE_SWAPPED);
        return;
    }

    Item* dstitem = _player->GetItemInterface()->GetInventoryItem(dstslot);
    Item* srcitem = _player->GetItemInterface()->GetInventoryItem(srcslot);

    // allow weapon switching in combat
    bool skip_combat = false;
    if (srcslot < EQUIPMENT_SLOT_END || dstslot < EQUIPMENT_SLOT_END)        // We're doing an equip swap.
    {
        if (_player->CombatStatus.IsInCombat())
        {
            if (srcslot < EQUIPMENT_SLOT_MAINHAND || dstslot < EQUIPMENT_SLOT_MAINHAND)    // These can't be swapped
            {
                _player->GetItemInterface()->BuildInventoryChangeError(srcitem, dstitem, INV_ERR_CANT_DO_IN_COMBAT);
                return;
            }
            skip_combat = true;
        }
    }

    if (!srcitem)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(srcitem, dstitem, INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM);
        return;
    }

    if (srcslot == dstslot)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(srcitem, dstitem, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
        return;
    }

    if ((error = _player->GetItemInterface()->CanEquipItemInSlot2(INVENTORY_SLOT_NOT_SET, dstslot, srcitem, skip_combat, false)) != 0)
    {
        if (dstslot < INVENTORY_KEYRING_END)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(srcitem, dstitem, error);
            return;
        }
    }

    if (dstitem != nullptr)
    {
        if ((error = _player->GetItemInterface()->CanEquipItemInSlot2(INVENTORY_SLOT_NOT_SET, srcslot, dstitem, skip_combat)) != 0)
        {
            if (srcslot < INVENTORY_KEYRING_END)
            {
                data.Initialize(SMSG_INVENTORY_CHANGE_FAILURE);
                data << error;
                data << srcitem->GetGUID();
                data << dstitem->GetGUID();
                data << uint8(0);

                if (error == INV_ERR_YOU_MUST_REACH_LEVEL_N)
                {
                    data << dstitem->GetItemProperties()->RequiredLevel;
                }

                SendPacket(&data);
                return;
            }
        }
    }

    if (srcitem->IsContainer())
    {
        //source has items and dst is a backpack or bank
        if (static_cast< Container* >(srcitem)->HasItems())
            if (!_player->GetItemInterface()->IsBagSlot(dstslot))
            {
                _player->GetItemInterface()->BuildInventoryChangeError(srcitem, dstitem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                return;
            }

        if (dstitem)
        {
            //source is a bag and dst slot is a bag inventory and has items
            if (dstitem->IsContainer())
            {
                if (static_cast< Container* >(dstitem)->HasItems() && !_player->GetItemInterface()->IsBagSlot(srcslot))
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(srcitem, dstitem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                    return;
                }
            }
            else
            {
                //dst item is not a bag, swap impossible
                _player->GetItemInterface()->BuildInventoryChangeError(srcitem, dstitem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                return;
            }
        }

        //dst is bag inventory
        if (dstslot < INVENTORY_SLOT_BAG_END)
        {
            if (srcitem->GetItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
                srcitem->SoulBind();
        }
    }

    // swap items
    if (_player->IsDead())
    {
        _player->GetItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_YOU_ARE_DEAD);
        return;
    }

#if VERSION_STRING > TBC
    if (dstitem && srcslot < INVENTORY_SLOT_BAG_END)
    {
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, dstitem->GetItemProperties()->ItemId, 0, 0);
        if (srcslot < INVENTORY_SLOT_BAG_START) // check Superior/Epic achievement
        {
            // Achievement ID:556 description Equip an epic item in every slot with a minimum item level of 213.
            // "213" value not found in achievement or criteria entries, have to hard-code it here? :(
            // Achievement ID:557 description Equip a superior item in every slot with a minimum item level of 187.
            // "187" value not found in achievement or criteria entries, have to hard-code it here? :(
            if ((dstitem->GetItemProperties()->Quality == ITEM_QUALITY_RARE_BLUE && dstitem->GetItemProperties()->ItemLevel >= 187) ||
                (dstitem->GetItemProperties()->Quality == ITEM_QUALITY_EPIC_PURPLE && dstitem->GetItemProperties()->ItemLevel >= 213))
                _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, srcslot, dstitem->GetItemProperties()->Quality, 0);
        }
    }
    if (srcitem && dstslot < INVENTORY_SLOT_BAG_END)
    {
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, srcitem->GetItemProperties()->ItemId, 0, 0);
        if (dstslot < INVENTORY_SLOT_BAG_START) // check Superior/Epic achievement
        {
            // Achievement ID:556 description Equip an epic item in every slot with a minimum item level of 213.
            // "213" value not found in achievement or criteria entries, have to hard-code it here? :(
            // Achievement ID:557 description Equip a superior item in every slot with a minimum item level of 187.
            // "187" value not found in achievement or criteria entries, have to hard-code it here? :(
            if ((srcitem->GetItemProperties()->Quality == ITEM_QUALITY_RARE_BLUE && srcitem->GetItemProperties()->ItemLevel >= 187) ||
                (srcitem->GetItemProperties()->Quality == ITEM_QUALITY_EPIC_PURPLE && srcitem->GetItemProperties()->ItemLevel >= 213))
                _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, dstslot, srcitem->GetItemProperties()->Quality, 0);
        }
    }
#endif

    _player->GetItemInterface()->SwapItemSlots(srcslot, dstslot);
}

void WorldSession::HandleDestroyItemOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 2);

    int8 SrcInvSlot;
    int8 SrcSlot;

    recvData >> SrcInvSlot;
    recvData >> SrcSlot;

    LOG_DETAIL("ITEM: destroy, SrcInv Slot: %i Src slot: %i", SrcInvSlot, SrcSlot);
    Item* it = _player->GetItemInterface()->GetInventoryItem(SrcInvSlot, SrcSlot);

    if (it)
    {
        if (it->IsContainer())
        {
            if (static_cast< Container* >(it)->HasItems())
            {
                _player->GetItemInterface()->BuildInventoryChangeError(
                    it, nullptr, INV_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS);
                return;
            }
        }

        if (it->GetItemProperties()->HasFlag(ITEM_FLAG_INDESTRUCTIBLE))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(it, nullptr, INV_ERR_CANT_DROP_SOULBOUND);
            return;
        }

        if (it->GetItemProperties()->ItemId == ITEM_ENTRY_GUILD_CHARTER)
        {
            Charter* gc = _player->m_charters[CHARTER_TYPE_GUILD];
            if (gc)
                gc->Destroy();

            _player->m_charters[CHARTER_TYPE_GUILD] = nullptr;
        }

        if (it->GetItemProperties()->ItemId == ARENA_TEAM_CHARTER_2v2)
        {
            Charter* gc = _player->m_charters[CHARTER_TYPE_ARENA_2V2];
            if (gc)
                gc->Destroy();

            _player->m_charters[CHARTER_TYPE_ARENA_2V2] = nullptr;
        }

        if (it->GetItemProperties()->ItemId == ARENA_TEAM_CHARTER_5v5)
        {
            Charter* gc = _player->m_charters[CHARTER_TYPE_ARENA_5V5];
            if (gc)
                gc->Destroy();

            _player->m_charters[CHARTER_TYPE_ARENA_5V5] = nullptr;
        }

        if (it->GetItemProperties()->ItemId == ARENA_TEAM_CHARTER_3v3)
        {
            Charter* gc = _player->m_charters[CHARTER_TYPE_ARENA_3V3];
            if (gc)
                gc->Destroy();

            _player->m_charters[CHARTER_TYPE_ARENA_3V3] = nullptr;
        }

        Item* pItem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot, SrcSlot, false);
        if (!pItem)
            return;

        for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
        {
            if (_player->getCurrentSpell(CurrentSpellType(i)) != nullptr && _player->getCurrentSpell(CurrentSpellType(i))->i_caster == pItem)
            {
                _player->getCurrentSpell(CurrentSpellType(i))->i_caster = nullptr;
                _player->interruptSpellWithSpellType(CurrentSpellType(i));
            }
        }

        pItem->DeleteFromDB();
        pItem->DeleteMe();
    }
}

void WorldSession::HandleAutoEquipItemOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 2);
    WorldPacket data;

    int8 SrcInvSlot;
    int8 SrcSlot;
    int8 error = 0;

    recvData >> SrcInvSlot;
    recvData >> SrcSlot;

    LOG_DETAIL("ITEM: autoequip, Inventory slot: %i Source Slot: %i", SrcInvSlot, SrcSlot);

    Item* eitem = _player->GetItemInterface()->GetInventoryItem(SrcInvSlot, SrcSlot);

    if (eitem == nullptr)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    int8 Slot = _player->GetItemInterface()->GetItemSlotByType(eitem->GetItemProperties()->InventoryType);
    if (Slot == ITEM_NO_SLOT_AVAILABLE)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_ITEM_CANT_BE_EQUIPPED);
        return;
    }

    // handle equipping of 2h when we have two items equipped! :) special case.
    if ((Slot == EQUIPMENT_SLOT_MAINHAND || Slot == EQUIPMENT_SLOT_OFFHAND) && !_player->DualWield2H)
    {
        Item* mainhandweapon = _player->GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
        if (mainhandweapon != nullptr && mainhandweapon->GetItemProperties()->InventoryType == INVTYPE_2HWEAPON)
        {
            if (Slot == EQUIPMENT_SLOT_OFFHAND && (eitem->GetItemProperties()->InventoryType == INVTYPE_WEAPON || eitem->GetItemProperties()->InventoryType == INVTYPE_2HWEAPON))
            {
                Slot = EQUIPMENT_SLOT_MAINHAND;
            }
        }
        else
        {
            if (Slot == EQUIPMENT_SLOT_OFFHAND && eitem->GetItemProperties()->InventoryType == INVTYPE_2HWEAPON)
            {
                Slot = EQUIPMENT_SLOT_MAINHAND;
            }
        }

        error = _player->GetItemInterface()->CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET, Slot, eitem->GetItemProperties(), true, true);
        if (error)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(eitem, nullptr, error);
            return;
        }

        if (eitem->GetItemProperties()->InventoryType == INVTYPE_2HWEAPON)
        {
            // see if we have a weapon equipped in the offhand, if so we need to remove it
            Item* offhandweapon = _player->GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND);
            if (offhandweapon != nullptr)
            {
                // we need to de-equip this
                SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(offhandweapon->GetItemProperties());
                if (!result.Result)
                {
                    // no free slots for this item
                    _player->GetItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_BAG_FULL);
                    return;
                }

                offhandweapon = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND, false);
                if (offhandweapon == nullptr)
                    return; // should never happen

                if (!_player->GetItemInterface()->SafeAddItem(offhandweapon, result.ContainerSlot, result.Slot) && !_player->GetItemInterface()->AddItemToFreeSlot(offhandweapon))       // shouldn't happen either.
                {
                    offhandweapon->DeleteMe();
                    offhandweapon = nullptr;
                }
            }
        }
        else
        {
            // can't equip a non-two-handed weapon with a two-handed weapon
            mainhandweapon = _player->GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
            if (mainhandweapon != nullptr && mainhandweapon->GetItemProperties()->InventoryType == INVTYPE_2HWEAPON)
            {
                // we need to de-equip this
                SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(mainhandweapon->GetItemProperties());
                if (!result.Result)
                {
                    // no free slots for this item
                    _player->GetItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_BAG_FULL);
                    return;
                }

                mainhandweapon = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND, false);
                if (mainhandweapon == nullptr)
                    return; // should never happen

                if (!_player->GetItemInterface()->SafeAddItem(mainhandweapon, result.ContainerSlot, result.Slot) && !_player->GetItemInterface()->AddItemToFreeSlot(mainhandweapon))          // shouldn't happen either.
                {
                    mainhandweapon->DeleteMe();
                    mainhandweapon = nullptr;
                }
            }
        }
    }
    else
    {
        error = _player->GetItemInterface()->CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET, Slot, eitem->GetItemProperties(), false, false);
        if (error)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(eitem, nullptr, error);
            return;
        }
    }

    if (Slot <= INVENTORY_SLOT_BAG_END)
    {
        error = _player->GetItemInterface()->CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET, Slot, eitem->GetItemProperties(), false, false);
        if (error)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(eitem, nullptr, error);
            return;
        }
    }

    Item* oitem = nullptr;

    if (SrcInvSlot == INVENTORY_SLOT_NOT_SET)
    {
        _player->GetItemInterface()->SwapItemSlots(SrcSlot, Slot);
    }
    else
    {
        eitem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot, SrcSlot, false);
        oitem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, Slot, false);
        AddItemResult result;
        if (oitem != nullptr)
        {
            result = _player->GetItemInterface()->SafeAddItem(oitem, SrcInvSlot, SrcSlot);
            if (!result)
            {
                LOG_ERROR("HandleAutoEquip: Error while adding item to SrcSlot");
                oitem->DeleteMe();
                oitem = nullptr;
            }
        }
        if (eitem != nullptr)
        {
            result = _player->GetItemInterface()->SafeAddItem(eitem, INVENTORY_SLOT_NOT_SET, Slot);
            if (!result)
            {
                LOG_ERROR("HandleAutoEquip: Error while adding item to Slot");
                eitem->DeleteMe();
                eitem = nullptr;
                return;
            }
        }

    }

    if (eitem != nullptr)
    {
        if (eitem->GetItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
            eitem->SoulBind();
#if VERSION_STRING > TBC
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, eitem->GetItemProperties()->ItemId, 0, 0);
        // Achievement ID:556 description Equip an epic item in every slot with a minimum item level of 213.
        // "213" value not found in achievement or criteria entries, have to hard-code it here? :(
        // Achievement ID:557 description Equip a superior item in every slot with a minimum item level of 187.
        // "187" value not found in achievement or criteria entries, have to hard-code it here? :(
        if ((eitem->GetItemProperties()->Quality == ITEM_QUALITY_RARE_BLUE && eitem->GetItemProperties()->ItemLevel >= 187) ||
            (eitem->GetItemProperties()->Quality == ITEM_QUALITY_EPIC_PURPLE && eitem->GetItemProperties()->ItemLevel >= 213))
            _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, Slot, eitem->GetItemProperties()->Quality, 0);
#endif
    }
    //Recalculate Expertise (for Weapon specs)
    _player->CalcExpertise();
}

void WorldSession::HandleAutoEquipItemSlotOpcode(WorldPacket & recvData)
{
    LOG_DETAIL("WORLD: Received CMSG_AUTOEQUIP_ITEM_SLOT");
    CHECK_PACKET_SIZE(recvData, 8 + 1);

    uint64 itemguid;
    int8 destSlot;
    //int8 error = 0; // useless?

    recvData >> itemguid;
    recvData >> destSlot;

    int8 srcSlot = (int8)_player->GetItemInterface()->GetInventorySlotByGuid(itemguid);
    Item* item = _player->GetItemInterface()->GetItemByGUID(itemguid);

    if (item == nullptr)
        return;

    int8 slotType = _player->GetItemInterface()->GetItemSlotByType(item->GetItemProperties()->InventoryType);
    bool hasDualWield2H = false;

    LOG_DEBUG("ITEM: AutoEquipItemSlot, ItemGUID: %u, SrcSlot: %i, DestSlot: %i, SlotType: %i", itemguid, srcSlot, destSlot, slotType);

    if (srcSlot == destSlot)
        return;

    if (_player->DualWield2H && (slotType == EQUIPMENT_SLOT_OFFHAND || slotType == EQUIPMENT_SLOT_MAINHAND))
        hasDualWield2H = true;

    // Need to check if the item even goes into that slot
    // Item system is a mess too, so it needs rewrite, but hopefully this will do for now
    int8 error = _player->GetItemInterface()->CanEquipItemInSlot2(INVENTORY_SLOT_NOT_SET, destSlot, item);
    if (error)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(item, nullptr, error);
        return;
    }

    // Handle destination slot checking.
    if (destSlot == slotType || hasDualWield2H)
    {
        uint32 invType = item->GetItemProperties()->InventoryType;
        if (invType == INVTYPE_WEAPON || invType == INVTYPE_WEAPONMAINHAND ||
            invType == INVTYPE_WEAPONOFFHAND || invType == INVTYPE_2HWEAPON)
        {
            Item* mainHand = _player->GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
            Item* offHand = _player->GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND);

            if (mainHand != nullptr && offHand != nullptr && !_player->DualWield2H)
            {
                // No DualWield2H like Titan's grip. Unequip offhand.
                SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(offHand->GetItemProperties());
                if (!result.Result)
                {
                    // No free slots for this item.
                    _player->GetItemInterface()->BuildInventoryChangeError(offHand, nullptr, INV_ERR_BAG_FULL);
                    return;
                }
                mainHand = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND, false);
                _player->GetItemInterface()->AddItemToFreeSlot(offHand);
                _player->GetItemInterface()->SwapItemSlots(srcSlot, destSlot);   // Now swap Main hand with 2H weapon.
            }
            else
            {
                // Swap 2H with 2H or 2H with 1H if player has DualWield2H (ex: Titans Grip).
                _player->GetItemInterface()->SwapItemSlots(srcSlot, destSlot);
            }
        }
        else if (destSlot == slotType)
        {
            // If item slot types match, swap.
            _player->GetItemInterface()->SwapItemSlots(srcSlot, destSlot);
        }
        else
        {
            // Item slots do not match. We get here only for players who have DualWield2H (ex: Titans Grip).
            _player->GetItemInterface()->BuildInventoryChangeError(item, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
        }
        return;
    }
    else
    {
        // Item slots do not match.
        _player->GetItemInterface()->BuildInventoryChangeError(item, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
    }
}

void WorldSession::HandleItemQuerySingleOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 4);

    uint32 itemid;
    recvData >> itemid;

    ItemProperties const* itemProto = sMySQLStore.getItemProperties(itemid);
    if (!itemProto)
    {
        LOG_ERROR("WORLD: Unknown item id 0x%.8X", itemid);
        return;
    }

    std::string Name;
    std::string Description;

    MySQLStructure::LocalesItem const* li = (language > 0) ? sMySQLStore.getLocalizedItem(itemid, language) : nullptr;
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
    data << uint8(0);           // name 2?
    data << uint8(0);           // name 3?
    data << uint8(0);           // name 4?
    data << itemProto->DisplayInfoID;
    data << itemProto->Quality;
    data << itemProto->Flags;
    data << itemProto->Flags2;
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
    data << itemProto->itemstatscount;
    for (uint8 i = 0; i < itemProto->itemstatscount; i++)
    {
        data << itemProto->Stats[i].Type;
        data << itemProto->Stats[i].Value;
    }
    data << itemProto->ScalingStatsEntry;
    data << itemProto->ScalingStatsFlag;
    for (uint8 i = 0; i < 2; i++)           // originally this went up to 5, now only to 2
    {
        data << itemProto->Damage[i].Min;
        data << itemProto->Damage[i].Max;
        data << itemProto->Damage[i].Type;
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
    for (uint8 i = 0; i < 5; i++)
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
    data << itemProto->ItemLimitCategory;
    data << itemProto->HolidayId;                           // HolidayNames.dbc
    SendPacket(&data);
}

void WorldSession::HandleBuyBackOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 8);
    uint64 guid;
    int32 stuff;
    uint8 error;

    LOG_DETAIL("WORLD: Received CMSG_BUYBACK_ITEM");

    recvData >> guid;
    recvData >> stuff;
    stuff -= 74;

    Item* it = _player->GetItemInterface()->GetBuyBack(stuff);
    if (it)
    {
        // Find free slot and break if inv full
        uint32 amount = it->GetStackCount();
        uint32 itemid = it->GetEntry();

        Item * add = _player->GetItemInterface()->FindItemLessMax(itemid, amount, false);

        uint32 FreeSlots = _player->GetItemInterface()->CalculateFreeSlots(it->GetItemProperties());
        if ((FreeSlots == 0) && (!add))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
            return;
        }

        // Check for gold
        uint32_t cost = _player->getUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_PRICE_1 + stuff));
        if (!_player->HasGold(cost))
        {
            WorldPacket data(SMSG_BUY_FAILED, 12);
            data << uint64(guid);
            data << uint32(itemid);
            data << uint8(2); //not enough money
            SendPacket(&data);
            return;
        }
        // Check for item uniqueness
        if ((error = _player->GetItemInterface()->CanReceiveItem(it->GetItemProperties(), amount)) != 0)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
            return;
        }
        int32_t coins = cost * -1;
        _player->ModGold(coins);
        _player->GetItemInterface()->RemoveBuyBackItem(stuff);

        if (!add)
        {
            it->m_isDirty = true;            // save the item again on logout
            AddItemResult result = _player->GetItemInterface()->AddItemToFreeSlot(it);
            if (!result)
            {
                LOG_ERROR("HandleBuyBack: Error while adding item to free slot");
                it->DeleteMe();
            }
        }
        else
        {
            add->SetStackCount(add->GetStackCount() + amount);
            add->m_isDirty = true;

            // delete the item
            it->DeleteFromDB();
            it->DeleteMe();
        }

        WorldPacket data(SMSG_BUY_ITEM, 8 + 4 + 4 + 4);
        data << uint64(guid);
        data << uint32(stuff + 1);      // numbered from 1 at client
        data << int32(amount);
        data << uint32(amount);
        data << uint32(amount);

        SendPacket(&data);
    }
}

void WorldSession::HandleSellItemOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 17);
    LOG_DETAIL("WORLD: Received CMSG_SELL_ITEM");

    uint64 vendorguid = 0, itemguid = 0;
    int8 amount = 0;
    //uint8 slot = INVENTORY_NO_SLOT_AVAILABLE;
    //uint8 bagslot = INVENTORY_NO_SLOT_AVAILABLE;
    //int check = 0;

    recvData >> vendorguid;
    recvData >> itemguid;
    recvData >> amount;

    _player->interruptSpell();

    // Check if item exists
    if (!itemguid)
    {
        SendSellItem(vendorguid, itemguid, 1);
        return;
    }

    Creature* unit = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(vendorguid));
    // Check if Vendor exists
    if (unit == nullptr)
    {
        SendSellItem(vendorguid, itemguid, 3);
        return;
    }

    Item* item = _player->GetItemInterface()->GetItemByGUID(itemguid);
    if (!item)
    {
        SendSellItem(vendorguid, itemguid, 1);
        return; //our player doesn't have this item
    }

    ItemProperties const* it = item->GetItemProperties();

    if (item->IsContainer() && static_cast< Container* >(item)->HasItems())
    {
        SendSellItem(vendorguid, itemguid, 6);
        return;
    }

    // Check if item can be sold
    if (it->SellPrice == 0 || item->wrapped_item_id != 0)
    {
        SendSellItem(vendorguid, itemguid, 2);
        return;
    }

    uint32 stackcount = item->GetStackCount();
    uint32 quantity = 0;

    if (amount != 0)
    {
        quantity = amount;
    }
    else
    {
        quantity = stackcount; //allitems
    }

    if (quantity > stackcount) quantity = stackcount; //make sure we don't over do it

    uint32 price = GetSellPriceForItem(it, quantity);

    // Check they don't have more than the max gold
    if (worldConfig.player.isGoldCapEnabled)
    {
        if ((_player->GetGold() + price) > worldConfig.player.limitGoldAmount)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            return;
        }
    }

    _player->ModGold(price);

    if (quantity < stackcount)
    {
        item->SetStackCount(stackcount - quantity);
        item->m_isDirty = true;
    }
    else
    {
        //removing the item from the char's inventory
        item = _player->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(itemguid, false); //again to remove item from slot
        if (item)
        {
            _player->GetItemInterface()->AddBuyBackItem(item, (it->SellPrice) * quantity);
            item->DeleteFromDB();
        }
    }

    WorldPacket data(SMSG_SELL_ITEM, 12);
    data << vendorguid;
    data << itemguid;
    data << uint8(0);
    SendPacket(&data);

    LOG_DETAIL("WORLD: Sent SMSG_SELL_ITEM");
}

void WorldSession::HandleBuyItemInSlotOpcode(WorldPacket& recvData)   // drag & drop
{
    CHECK_PACKET_SIZE(recvData, 22);

    LOG_DETAIL("WORLD: Received CMSG_BUY_ITEM_IN_SLOT");

    uint64 srcguid;
    uint64 bagguid;
    uint32 itemid;
    int8 slot;
    uint8 amount = 0;
    uint8 error;
    int8 bagslot = INVENTORY_SLOT_NOT_SET;
    int32 vendorslot; //VLack: 3.1.2

    recvData >> srcguid;
    recvData >> itemid;
    recvData >> vendorslot; //VLack: 3.1.2 This is the slot's number on the vendor's panel, starts from 1
    recvData >> bagguid;
    recvData >> slot; //VLack: 3.1.2 the target slot the player selected - backpack 23-38, other bags 0-15 (Or how big is the biggest bag? 0-127?)
    recvData >> amount;

    if (amount < 1)
        amount = 1;

    _player->interruptSpell();

    Creature* unit = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(srcguid));
    if (unit == nullptr || !unit->HasItems())
        return;

    Container* c = nullptr;

    CreatureItem ci;
    unit->GetSellItemByItemId(itemid, ci);

    if (ci.itemid == 0)
        return;

    if (ci.max_amount > 0 && ci.available_amount < amount)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_IS_CURRENTLY_SOLD_OUT);
        return;
    }

    ItemProperties const* it = sMySQLStore.getItemProperties(itemid);
    if (it == nullptr)
        return;

    uint32 itemMaxStack = (_player->ItemStackCheat) ? 0x7fffffff : it->MaxCount;
    if (itemMaxStack > 0 && ci.amount * amount > itemMaxStack)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_CANT_CARRY_MORE_OF_THIS);
        return;
    }

    uint32 count_per_stack = ci.amount * amount;

    // if slot is different than -1, check for validation, else continue for auto storing.
    if (slot != INVENTORY_SLOT_NOT_SET)
    {
        if (!(bagguid >> 32))//buy to backpack
        {
            if (slot > INVENTORY_SLOT_ITEM_END || slot < INVENTORY_SLOT_ITEM_START)
            {
                //hackers!
                _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
                return;
            }
        }
        else
        {
            c = static_cast< Container* >(_player->GetItemInterface()->GetItemByGUID(bagguid));
            if (!c)
                return;
            bagslot = (int8)_player->GetItemInterface()->GetBagSlotByGuid(bagguid);

            if (bagslot == INVENTORY_SLOT_NOT_SET || ((uint32)slot > c->GetItemProperties()->ContainerSlots))
            {
                _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
                return;
            }
        }
    }
    else
    {
        if ((bagguid >> 32))
        {
            c = static_cast< Container* >(_player->GetItemInterface()->GetItemByGUID(bagguid));
            if (!c)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_NOT_FOUND);
                return;//non empty
            }

            bagslot = (int8)_player->GetItemInterface()->GetBagSlotByGuid(bagguid);
            slot = c->FindFreeSlot();
        }
        else
            slot = _player->GetItemInterface()->FindFreeBackPackSlot();
    }

    if ((error = _player->GetItemInterface()->CanReceiveItem(it, amount)) != 0)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    if ((error = _player->GetItemInterface()->CanAffordItem(it, amount, unit)) != 0)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    if (slot == INVENTORY_SLOT_NOT_SET)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_BAG_FULL);
        return;
    }

    // ok our z and slot are set.
    Item* oldItem = nullptr;
    Item* pItem = nullptr;
    if (slot != INVENTORY_SLOT_NOT_SET)
        oldItem = _player->GetItemInterface()->GetInventoryItem(bagslot, slot);

    if (oldItem != nullptr)
    {
        // try to add to the existing items stack
        if (oldItem->GetItemProperties() != it)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
            return;
        }

        if ((oldItem->GetStackCount() + count_per_stack) > itemMaxStack)
        {
            //            LOG_DEBUG("SUPADBG can't carry #2");
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_CANT_CARRY_MORE_OF_THIS);
            return;
        }

        oldItem->ModStackCount(count_per_stack);
        oldItem->m_isDirty = true;
        pItem = oldItem;
    }
    else
    {
        // create new item
        /*if (slot == INVENTORY_SLOT_NOT_SET) This cannot be true CID 52838
        slot = c->FindFreeSlot();*/

        if (slot == ITEM_NO_SLOT_AVAILABLE)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_BAG_FULL);
            return;
        }

        pItem = objmgr.CreateItem(it->ItemId, _player);
        if (pItem)
        {
            pItem->SetStackCount(count_per_stack);
            pItem->m_isDirty = true;
            //            LOG_DEBUG("SUPADBG bagslot=%u, slot=%u" , bagslot, slot);
            if (!_player->GetItemInterface()->SafeAddItem(pItem, bagslot, slot))
            {
                pItem->DeleteMe();
                return;
            }
        }
        else
            return;
    }

    _player->SendItemPushResult(false, true, false, (pItem == oldItem) ? false : true, bagslot, slot, amount * ci.amount, pItem->GetEntry(), pItem->GetItemRandomSuffixFactor(), pItem->GetItemRandomPropertyId(), pItem->GetStackCount());

    WorldPacket data(SMSG_BUY_ITEM, 22);
    data << uint64(srcguid);
    data << uint32(slot + 1);       // numbered from 1 at client
    data << int32(amount);
    data << uint32(amount);

    SendPacket(&data);

    LOG_DETAIL("WORLD: Sent SMSG_BUY_ITEM");

    _player->GetItemInterface()->BuyItem(it, amount, unit);
    if (ci.max_amount)
    {
        unit->ModAvItemAmount(ci.itemid, ci.amount * amount);

        // there is probably a proper opcode for this. - burlex
        SendInventoryList(unit);
    }
}

void WorldSession::HandleBuyItemOpcode(WorldPacket& recvData)   // right-click on item
{
    CHECK_PACKET_SIZE(recvData, 14);
    LOG_DETAIL("WORLD: Received CMSG_BUY_ITEM");

    uint64 srcguid = 0;
    uint32 itemid = 0;
    int32 slot = 0;
    uint8 amount = 0;
    uint8 error = 0;
    SlotResult slotresult;

    recvData >> srcguid;

    uint8 itemtype;
    recvData >> itemtype;
    recvData >> itemid;
    recvData >> slot;
    recvData >> amount;

    auto creature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(srcguid));
    if (creature == nullptr || !creature->HasItems())
        return;

    auto item_extended_cost = creature->GetItemExtendedCostByItemId(itemid);

    if (amount < 1)
        amount = 1;

    CreatureItem creature_item;
    creature->GetSellItemByItemId(itemid, creature_item);

    if (creature_item.itemid == 0)
    {
        // vendor does not sell this item.. bitch about cheaters?
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_DONT_OWN_THAT_ITEM);
        return;
    }

    if (creature_item.max_amount > 0 && creature_item.available_amount < amount)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_IS_CURRENTLY_SOLD_OUT);
        return;
    }

    ItemProperties const* it = sMySQLStore.getItemProperties(itemid);
    if (!it)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_DONT_OWN_THAT_ITEM);
        return;
    }

    uint32 itemMaxStack = (_player->ItemStackCheat) ? 0x7fffffff : it->MaxCount;
    if (itemMaxStack > 0 && amount * creature_item.amount > itemMaxStack)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEM_CANT_STACK);
        return;
    }

    if ((error = _player->GetItemInterface()->CanReceiveItem(it, amount * creature_item.amount)) != 0)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    if ((error = _player->GetItemInterface()->CanAffordItem(it, amount, creature)) != 0)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, error);
        return;
    }

    // Find free slot and break if inv full
    auto add_item = _player->GetItemInterface()->FindItemLessMax(itemid, amount * creature_item.amount, false);
    if (!add_item)
    {
        slotresult = _player->GetItemInterface()->FindFreeInventorySlot(it);
    }
    if (!slotresult.Result && !add_item)
    {
        //Player doesn't have a free slot in his/her bag(s)
        _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
        return;
    }

    if (!add_item)
    {
        Item* item = objmgr.CreateItem(creature_item.itemid, _player);
        if (!item)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_DONT_OWN_THAT_ITEM);
            return;
        }

        item->m_isDirty = true;
        item->SetStackCount(amount * creature_item.amount);

        if (slotresult.ContainerSlot == ITEM_NO_SLOT_AVAILABLE)
        {
            AddItemResult result = _player->GetItemInterface()->SafeAddItem(item, INVENTORY_SLOT_NOT_SET, slotresult.Slot);
            if (!result)
            {
                item->DeleteMe();
            }
            else
            {
                if (item->IsEligibleForRefund() && item_extended_cost != nullptr)
                {
                    item->GetOwner()->GetItemInterface()->AddRefundable(item->GetGUID(), item_extended_cost->costid);
                }
                _player->SendItemPushResult(false, true, false, true, static_cast<uint8>(INVENTORY_SLOT_NOT_SET), slotresult.Result, amount * creature_item.amount, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());
            }
        }
        else
        {
            if (Item* bag = _player->GetItemInterface()->GetInventoryItem(slotresult.ContainerSlot))
            {
                if (!static_cast<Container*>(bag)->AddItem(slotresult.Slot, item))
                {
                    item->DeleteMe();
                }
                else
                {
                    if (item->IsEligibleForRefund() && item_extended_cost != nullptr)
                    {
                        item->GetOwner()->GetItemInterface()->AddRefundable(item->GetGUID(), item_extended_cost->costid);
                    }
                    _player->SendItemPushResult(false, true, false, true, slotresult.ContainerSlot, slotresult.Result, 1, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());
                }
            }
        }
    }
    else
    {
        add_item->ModStackCount(amount * creature_item.amount);
        add_item->m_isDirty = true;
        _player->SendItemPushResult(false, true, false, false, (uint8)_player->GetItemInterface()->GetBagSlotByGuid(add_item->GetGUID()), 1, amount * creature_item.amount, add_item->GetEntry(), add_item->GetItemRandomSuffixFactor(), add_item->GetItemRandomPropertyId(), add_item->GetStackCount());
    }

    _player->GetItemInterface()->BuyItem(it, amount, creature);

    WorldPacket data(45);
    data.Initialize(SMSG_BUY_ITEM);
    data << uint64(srcguid);
    data << Util::getMSTime();
    data << uint32(itemid);
    data << uint32(amount * creature_item.amount);

    SendPacket(&data);

    if (creature_item.max_amount)
    {
        creature->ModAvItemAmount(creature_item.itemid, creature_item.amount * amount);

        // there is probably a proper opcode for this. - burlex
        SendInventoryList(creature);
    }
}

void WorldSession::HandleListInventoryOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 8);
    LOG_DETAIL("WORLD: Recvd CMSG_LIST_INVENTORY");
    uint64 guid;

    recvData >> guid;

    Creature* unit = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (unit == nullptr)
        return;

    MySQLStructure::VendorRestrictions const* vendor = sMySQLStore.getVendorRestriction(unit->GetCreatureProperties()->Id);

    //this is a blizzlike check
    if (_player->obj_movement_info.getTransportGuid().IsEmpty())
    {
        if (_player->getDistanceSq(unit) > 100)
            return; //avoid talking to anyone by guid hacking. Like sell farmed items anytime ? Low chance hack
    }

    if (unit->GetAIInterface())
        unit->GetAIInterface()->StopMovement(180000);

    _player->Reputation_OnTalk(unit->m_factionDBC);

    if (_player->CanBuyAt(vendor))
        SendInventoryList(unit);
    else
    {
        Arcemu::Gossip::Menu::SendSimpleMenu(unit->GetGUID(), vendor->cannotbuyattextid, _player);
    }
}

void WorldSession::SendInventoryList(Creature* unit)
{
    if (!unit->HasItems())
    {
        sChatHandler.BlueSystemMessage(_player->GetSession(), "No sell template found. Report this to database's devs: %d (%s)", unit->GetEntry(), unit->GetCreatureProperties()->Name.c_str());
        LOG_ERROR("'%s' discovered that a creature with entry %u (%s) has no sell template.", GetPlayer()->GetName(), unit->GetEntry(), unit->GetCreatureProperties()->Name.c_str());
        Arcemu::Gossip::Menu::Complete(GetPlayer());
        return;
    }

    uint32 counter = 0;

    WorldPacket data(((unit->GetSellItemCount()) + 12));       // allocate

    ByteBuffer itemsData(32 * unit->GetSellItemCount());
    std::vector<bool> enablers;
    enablers.reserve(2 * unit->GetSellItemCount());

    ItemProperties const* curItem = nullptr;

    for (std::vector<CreatureItem>::iterator itr = unit->GetSellItemBegin(); itr != unit->GetSellItemEnd(); ++itr)
    {
        if (itr->itemid && (itr->max_amount == 0 || (itr->max_amount > 0 && itr->available_amount > 0)))
        {
            if ((curItem = sMySQLStore.getItemProperties(itr->itemid)) != nullptr)
            {
                if (!_player->isGMFlagSet() && !worldConfig.player.showAllVendorItems) // looking up everything for active gms
                {
                    if (curItem->AllowableClass && !(_player->getClassMask() & curItem->AllowableClass))
                        continue;

                    if (curItem->AllowableRace && !(_player->getRaceMask() & curItem->AllowableRace))
                        continue;

                    if (curItem->HasFlag2(ITEM_FLAG2_HORDE_ONLY) && !GetPlayer()->IsTeamHorde())
                        continue;

                    if (curItem->HasFlag2(ITEM_FLAG2_ALLIANCE_ONLY) && !GetPlayer()->IsTeamAlliance())
                        continue;
                }

                uint32 av_am = (itr->max_amount > 0) ? itr->available_amount : 0xFFFFFFFF;
                uint32 price = 0;
                if ((itr->extended_cost == nullptr) || curItem->HasFlag2(ITEM_FLAG2_EXT_COST_REQUIRES_GOLD))
                    price = GetBuyPriceForItem(curItem, 1, _player, unit);

                itemsData << uint32(counter + 1);        // client expects counting to start at 1
                itemsData << uint32(curItem->MaxDurability);
                if (itr->extended_cost != nullptr)
                {
                    enablers.push_back(0);
                    itemsData << uint32(itr->extended_cost->costid);
                }
                else
                {
                    enablers.push_back(1);
                }

                enablers.push_back(1);                 // unk bit

                itemsData << uint32(curItem->ItemId);
                itemsData << uint32(1);     // 1 is items, 2 is currency
                itemsData << uint32(price);
                itemsData << uint32(curItem->DisplayInfoID);
                itemsData << int32(av_am);
                itemsData << uint32(itr->amount);

                ++counter;
                if (counter >= creatureMaxInventoryItems) break;  // cebernic: in 2.4.3, client can't take more than 15 pages,it making crash for us:(
            }
        }
    }

    ObjectGuid guid = unit->GetGUID();

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

    data << uint8(counter == 0); // unk byte, item count 0: 1, item count != 0: 0 or some "random" value below 300

    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[7]);

    SendPacket(&data);
    LOG_DETAIL("WORLD: Sent SMSG_LIST_INVENTORY");
}

void WorldSession::HandleAutoStoreBagItemOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 3);
    LOG_DETAIL("WORLD: Recvd CMSG_AUTO_STORE_BAG_ITEM");

    //WorldPacket data;
    WorldPacket packet;
    int8 SrcInv = 0, Slot = 0, DstInv = 0;
    //    Item *item= NULL;
    Item* srcitem = nullptr;
    Item* dstitem = nullptr;
    int8 NewSlot = 0;
    int8 error;
    AddItemResult result;

    recvData >> SrcInv;
    recvData >> Slot;
    recvData >> DstInv;

    srcitem = _player->GetItemInterface()->GetInventoryItem(SrcInv, Slot);

    //source item exists
    if (srcitem)
    {
        //src containers cant be moved if they have items inside
        if (srcitem->IsContainer() && static_cast< Container* >(srcitem)->HasItems())
        {
            _player->GetItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
            return;
        }
        //check for destination now before swaping.
        //destination is backpack
        if (DstInv == INVENTORY_SLOT_NOT_SET)
        {
            //check for space
            NewSlot = _player->GetItemInterface()->FindFreeBackPackSlot();
            if (NewSlot == ITEM_NO_SLOT_AVAILABLE)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_BAG_FULL);
                return;
            }
            else
            {
                //free space found, remove item and add it to the destination
                srcitem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInv, Slot, false);
                if (srcitem)
                {
                    result = _player->GetItemInterface()->SafeAddItem(srcitem, INVENTORY_SLOT_NOT_SET, NewSlot);
                    if (!result)
                    {
                        LOG_ERROR("HandleAutoStoreBagItem: Error while adding item to newslot");
                        srcitem->DeleteMe();
                        return;
                    }
                }
            }
        }
        else
        {
            if ((error = _player->GetItemInterface()->CanEquipItemInSlot2(DstInv, DstInv, srcitem)) != 0)
            {
                if (DstInv < INVENTORY_KEYRING_END)
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(srcitem, dstitem, error);
                    return;
                }
            }

            //destination is a bag
            dstitem = _player->GetItemInterface()->GetInventoryItem(DstInv);
            if (dstitem)
            {
                //dstitem exists, detect if its a container
                if (dstitem->IsContainer())
                {
                    NewSlot = static_cast< Container* >(dstitem)->FindFreeSlot();
                    if (NewSlot == ITEM_NO_SLOT_AVAILABLE)
                    {
                        _player->GetItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_BAG_FULL);
                        return;
                    }
                    else
                    {
                        srcitem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInv, Slot, false);
                        if (srcitem != nullptr)
                        {
                            result = _player->GetItemInterface()->SafeAddItem(srcitem, DstInv, NewSlot);
                            if (!result)
                            {
                                LOG_ERROR("HandleBuyItemInSlot: Error while adding item to newslot");
                                srcitem->DeleteMe();
                                return;
                            }
                        }
                    }
                }
                else
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
                    return;
                }
            }
            else
            {
                _player->GetItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
                return;
            }
        }
    }
    else
    {
        _player->GetItemInterface()->BuildInventoryChangeError(srcitem, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }
}

void WorldSession::HandleReadItemOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 2);
    int8 uslot = 0, slot = 0;
    recvPacket >> uslot >> slot;

    Item* item = _player->GetItemInterface()->GetInventoryItem(uslot, slot);
    LOG_DEBUG("Received CMSG_READ_ITEM %d", slot);

    if (item)
    {
        // Check if it has pagetext

        if (item->GetItemProperties()->PageId)
        {
            WorldPacket data(SMSG_READ_ITEM_OK, 4);
            data << item->GetGUID();
            SendPacket(&data);
            LOG_DEBUG("Sent SMSG_READ_OK %d", item->GetGUID());
        }
        else
        {
            WorldPacket data(SMSG_READ_ITEM_FAILED, 5);
            data << item->GetGUID();
            data << uint8(2);
            SendPacket(&data);
            LOG_DEBUG("Sent SMSG_READ_ITEM_FAILED %d", item->GetGUID());
        }
    }
}

void WorldSession::HandleRepairItemOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 17);//8+8+1

    uint64 npcguid;
    uint64 itemguid;
    bool guildmoney;

    recvPacket >> npcguid >> itemguid >> guildmoney;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(npcguid));
    if (pCreature == nullptr)
        return;

    if (!pCreature->isArmorer())
        return;

    //this is a blizzlike check
    if (_player->getDistanceSq(pCreature) > 100)
        return; //avoid talking to anyone by guid hacking. Like repair items anytime in raid ? Low chance hack

    if (guildmoney)
    {
    }

    if (!itemguid)
    {
        int32 totalcost = 0;
        for (uint32 i = 0; i < MAX_INVENTORY_SLOT; i++)
        {
            Item * pItem = _player->GetItemInterface()->GetInventoryItem(static_cast<int16>(i));
            if (pItem != nullptr)
            {
                if (pItem->IsContainer())
                {
                    Container * pContainer = static_cast< Container* >(pItem);
                    for (uint32 j = 0; j < pContainer->GetItemProperties()->ContainerSlots; ++j)
                    {
                        pItem = pContainer->GetItem(static_cast<int16>(j));
                        if (pItem != nullptr)
                            pItem->RepairItem(_player, guildmoney, &totalcost);
                    }
                }
                else
                {
                    if (i < INVENTORY_SLOT_BAG_END)
                    {
                        if (pItem->GetDurability() == 0 && pItem->RepairItem(_player, guildmoney, &totalcost))
                            _player->ApplyItemMods(pItem, static_cast<int16>(i), true);
                        else
                            pItem->RepairItem(_player, guildmoney, &totalcost);
                    }
                }
            }
        }
    }
    else
    {
        Item* item = _player->GetItemInterface()->GetItemByGUID(itemguid);
        if (item)
        {
            SlotResult* searchres = _player->GetItemInterface()->LastSearchResult(); //this never gets null since we get a pointer to the inteface internal var
            uint32 dDurability = item->GetDurabilityMax() - item->GetDurability();

            if (dDurability)
            {
                uint32 cDurability = item->GetDurability();
                //only apply item mods if they are on char equipped
                if (item->RepairItem(_player) && cDurability == 0 && searchres->ContainerSlot == static_cast<int8>(INVALID_BACKPACK_SLOT) && searchres->Slot < static_cast<int8>(INVENTORY_SLOT_BAG_END))
                    _player->ApplyItemMods(item, searchres->Slot, true);
            }
        }
    }
    LOG_DEBUG("Received CMSG_REPAIR_ITEM %d", itemguid);
}

void WorldSession::HandleBuyBankSlotOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 8);

    uint64 guid;
    recvPacket >> guid;
    Creature* Banker = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));

    if (Banker == nullptr || !Banker->isBanker())
    {
        WorldPacket data(SMSG_BUY_BANK_SLOT_RESULT, 4);
        data << uint32(2); // E_ERR_BANKSLOT_NOTBANKER
        SendPacket(&data);
        return;
    }

    uint32 slots;

    LOG_DEBUG("WORLD: CMSG_BUY_bytes_SLOT");

    uint32 bytes = GetPlayer()->getUInt32Value(PLAYER_BYTES_2);
    slots = (uint8)(bytes >> 16);

    LOG_DETAIL("PLAYER: Buy bytes bag slot, slot number = %d", slots);
    auto bank_bag_slot_prices = sBankBagSlotPricesStore.LookupEntry(slots + 1);
    if (bank_bag_slot_prices == nullptr)
    {
        WorldPacket data(SMSG_BUY_BANK_SLOT_RESULT, 4);
        data << uint32(0); // E_ERR_BANKSLOT_FAILED_TOO_MANY
        SendPacket(&data);
        return;
    }

    int32 price = bank_bag_slot_prices->Price;
    if (_player->HasGold(price))
    {
        _player->setUInt32Value(PLAYER_BYTES_2, (bytes & 0xff00ffff) | ((slots + 1) << 16));
        _player->ModGold(-price);
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT, 1, 0, 0);
    }
    else
    {
        WorldPacket data(SMSG_BUY_BANK_SLOT_RESULT, 4);
        data << uint32(1); // E_ERR_BANKSLOT_INSUFFICIENT_FUNDS
        SendPacket(&data);
    }
}

void WorldSession::HandleAutoBankItemOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 2);
    LOG_DEBUG("WORLD: CMSG_AUTO_BANK_ITEM");

    //WorldPacket data;

    int8 SrcInvSlot, SrcSlot;//, error= 0;

    recvPacket >> SrcInvSlot >> SrcSlot;

    LOG_DETAIL("ITEM: Auto Bank, Inventory slot: %u Source Slot: %u", (uint32)SrcInvSlot, (uint32)SrcSlot);

    Item* eitem = _player->GetItemInterface()->GetInventoryItem(SrcInvSlot, SrcSlot);

    if (!eitem)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    SlotResult slotresult = _player->GetItemInterface()->FindFreeBankSlot(eitem->GetItemProperties());

    if (!slotresult.Result)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_BANK_FULL);
        return;
    }
    else
    {
        eitem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot, SrcSlot, false);
        if (eitem == nullptr)
            return;

        if (!_player->GetItemInterface()->SafeAddItem(eitem, slotresult.ContainerSlot, slotresult.Slot))
        {
            LOG_DEBUG("[ERROR]AutoBankItem: Error while adding item to bank bag!");
            if (!_player->GetItemInterface()->SafeAddItem(eitem, SrcInvSlot, SrcSlot))
                eitem->DeleteMe();
        }
    }
}

void WorldSession::HandleAutoStoreBankItemOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 2);
    LOG_DEBUG("WORLD: CMSG_AUTOSTORE_BANK_ITEM");

    //WorldPacket data;

    int8 SrcInvSlot, SrcSlot;//, error= 0, slot=-1, specialbagslot=-1;

    recvPacket >> SrcInvSlot >> SrcSlot;

    LOG_DETAIL("ITEM: AutoStore Bank Item, Inventory slot: %u Source Slot: %u", (uint32)SrcInvSlot, (uint32)SrcSlot);

    Item* eitem = _player->GetItemInterface()->GetInventoryItem(SrcInvSlot, SrcSlot);

    if (!eitem)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    SlotResult slotresult = _player->GetItemInterface()->FindFreeInventorySlot(eitem->GetItemProperties());

    if (!slotresult.Result)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem, nullptr, INV_ERR_INVENTORY_FULL);
        return;
    }
    else
    {
        eitem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot, SrcSlot, false);
        if (eitem == nullptr)
            return;
        if (!_player->GetItemInterface()->AddItemToFreeSlot(eitem))
        {
            LOG_DEBUG("[ERROR]AutoStoreBankItem: Error while adding item from one of the bank bags to the player bag!");
            if (!_player->GetItemInterface()->SafeAddItem(eitem, SrcInvSlot, SrcSlot))
                eitem->DeleteMe();
        }
    }
}

void WorldSession::HandleCancelTemporaryEnchantmentOpcode(WorldPacket& recvPacket)
{
    uint32 inventory_slot;
    recvPacket >> inventory_slot;

    Item* item = _player->GetItemInterface()->GetInventoryItem(static_cast<int16>(inventory_slot));
    if (!item) return;

    item->RemoveAllEnchantments(true);
}

void WorldSession::HandleInsertGemOpcode(WorldPacket& recvPacket)
{
    uint64 itemguid;
    uint64 gemguid[3];
    ItemInterface* itemi = _player->GetItemInterface();
    DBC::Structures::GemPropertiesEntry const* gem_properties;
    DBC::Structures::SpellItemEnchantmentEntry const* spell_item_enchant;
    recvPacket >> itemguid;

    Item* TargetItem = itemi->GetItemByGUID(itemguid);
    if (!TargetItem)
        return;

    ItemProperties const* TargetProto = TargetItem->GetItemProperties();
    int slot = itemi->GetInventorySlotByGuid(itemguid);

    bool apply = (slot >= 0 && slot < 19);
    uint32 FilledSlots = 0;

    //cheat -> tried to socket same gem multiple times
    for (uint8 i = 0; i < 3; ++i)
        recvPacket >> gemguid[i];

    if ((gemguid[0] && (gemguid[0] == gemguid[1] || gemguid[0] == gemguid[2])) || (gemguid[1] && (gemguid[1] == gemguid[2])))
    {
        return;
    }

    bool ColorMatch = true;
    for (uint32 i = 0; i < TargetItem->GetSocketsCount(); ++i)
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

        if (gemguid[i])  //add or replace gem
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
                if (i != 0 && !TargetProto->Sockets[i - 1].SocketColor && (i + 1 >= 3 || TargetProto->Sockets[i + 1].SocketColor))
                    return;

                // ok, this is first not colored socket for item with prismatic socket
            }


            if (apply)
            {
                it = itemi->GetItemByGUID(gemguid[i]);
                if (!it)
                    continue;

                ip = it->GetItemProperties();
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
                if (ip->ItemLimitCategory)
                {
                    auto item_limit_category = sItemLimitCategoryStore.LookupEntry(ip->ItemLimitCategory);
                    if (item_limit_category != nullptr && itemi->GetEquippedCountByItemLimit(ip->ItemLimitCategory) >= item_limit_category->maxAmount)
                    {
                        itemi->BuildInventoryChangeError(it, TargetItem, INV_ERR_ITEM_MAX_COUNT_EQUIPPED_SOCKETED);
                        continue;
                    }
                }
            }

            it = itemi->SafeRemoveAndRetreiveItemByGuid(gemguid[i], true);
            if (!it)
                return; //someone sending hacked packets to crash server

            gem_properties = sGemPropertiesStore.LookupEntry(it->GetItemProperties()->GemProperties);
            it->DeleteMe();

            if (!gem_properties)
                continue;

            if (!(gem_properties->SocketMask & TargetProto->Sockets[i].SocketColor))
                ColorMatch = false;

            if (!gem_properties->EnchantmentID)//this is ok in few cases
                continue;
            //Meta gems only go in meta sockets.
            if (TargetProto->Sockets[i].SocketColor != GEM_META_SOCKET && gem_properties->SocketMask == GEM_META_SOCKET)
                continue;
            if (EI)//replace gem
                TargetItem->RemoveEnchantment(2 + i); //remove previous
            else//add gem
                FilledSlots++;

            spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(gem_properties->EnchantmentID);
            if (spell_item_enchant != nullptr)
            {
                if (TargetItem->GetItemProperties()->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
                    TargetItem->AddEnchantment(spell_item_enchant, 0, true, apply, false, 2 + i);
            }

        }
    }

    //Add color match bonus
    if (TargetItem->GetItemProperties()->SocketBonus)
    {
        if (ColorMatch && (FilledSlots == TargetItem->GetSocketsCount()))
        {
            if (TargetItem->HasEnchantment(TargetItem->GetItemProperties()->SocketBonus) > 0)
                return;

            spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(TargetItem->GetItemProperties()->SocketBonus);
            if (spell_item_enchant != nullptr)
            {
                if (TargetItem->GetItemProperties()->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
                {
                    uint32 Slot = TargetItem->FindFreeEnchantSlot(spell_item_enchant, 0);
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

void WorldSession::HandleWrapItemOpcode(WorldPacket& recvData)
{
    int8 sourceitem_bagslot;
    int8 sourceitem_slot;
    int8 destitem_bagslot;
    int8 destitem_slot;

    recvData >> sourceitem_bagslot;
    recvData >> sourceitem_slot;
    recvData >> destitem_bagslot;
    recvData >> destitem_slot;

    Item * src = _player->GetItemInterface()->GetInventoryItem(sourceitem_bagslot, sourceitem_slot);
    Item *dst = _player->GetItemInterface()->GetInventoryItem(destitem_bagslot, destitem_slot);

    if (!src || !dst)
        return;

    if (src == dst || !(src->GetItemProperties()->Class == 0 && src->GetItemProperties()->SubClass == 8))
    {
        _player->GetItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_WRAPPED_CANT_BE_WRAPPED);
        return;
    }

    if (dst->GetStackCount() > 1)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_STACKABLE_CANT_BE_WRAPPED);
        return;
    }

    uint32 dstItemMaxStack = (dst->GetOwner()->ItemStackCheat) ? 0x7fffffff : dst->GetItemProperties()->MaxCount;
    if (dstItemMaxStack > 1)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_STACKABLE_CANT_BE_WRAPPED);
        return;
    }

    if (dst->IsSoulbound())
    {
        _player->GetItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_BOUND_CANT_BE_WRAPPED);
        return;
    }

    if (dst->wrapped_item_id || src->wrapped_item_id)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_WRAPPED_CANT_BE_WRAPPED);
        return;
    }

    if (dst->GetItemProperties()->Unique)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_UNIQUE_CANT_BE_WRAPPED);
        return;
    }

    if (dst->IsContainer())
    {
        _player->GetItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_BAGS_CANT_BE_WRAPPED);
        return;
    }

    if (dst->HasEnchantments())
    {
        _player->GetItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_ITEM_LOCKED);
        return;
    }
    if (destitem_bagslot == -1 && (destitem_slot >= int8(EQUIPMENT_SLOT_START) && destitem_slot <= int8(INVENTORY_SLOT_BAG_END)))
    {
        _player->GetItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_EQUIPPED_CANT_BE_WRAPPED);
        return;
    }

    // all checks passed ok
    uint32 sourceEntry = src->GetEntry();
    uint32 itemid = sourceEntry;
    switch (sourceEntry)
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
        _player->GetItemInterface()->BuildInventoryChangeError(src, dst, INV_ERR_WRAPPED_CANT_BE_WRAPPED);
        return;
        break;
    }

    dst->SetItemProperties(src->GetItemProperties());

    if (src->GetStackCount() <= 1)
    {
        // destroy the source item
        _player->GetItemInterface()->SafeFullRemoveItemByGuid(src->GetGUID());
    }
    else
    {
        // reduce stack count by one
        src->ModStackCount(-1);
        src->m_isDirty = true;
    }

    // change the dest item's entry
    dst->wrapped_item_id = dst->GetEntry();
    dst->SetEntry(itemid);

    // set the giftwrapper fields
    dst->SetGiftCreatorGUID(_player->GetGUID());
    dst->SetDurability(0);
    dst->SetDurabilityMax(0);
    dst->Wrap();

    // save it
    dst->m_isDirty = true;
    dst->SaveToDB(destitem_bagslot, destitem_slot, false, nullptr);
}

void WorldSession::HandleItemRefundRequestOpcode(WorldPacket& recvPacket)
{
    LOG_DEBUG("Recieved CMSG_ITEMREFUNDREQUEST.");

    uint64 GUID;
    uint32 error = 1;

    std::pair<time_t, uint32> RefundEntry;
    DB2::Structures::ItemExtendedCostEntry const* item_extended_cost = nullptr;
    ItemProperties const* item_proto = nullptr;

    recvPacket >> GUID;

    auto item = _player->GetItemInterface()->GetItemByGUID(GUID);

    if (item != nullptr)
    {
        if (item->IsEligibleForRefund())
        {
            RefundEntry.first = 0;
            RefundEntry.second = 0;

            RefundEntry = _player->GetItemInterface()->LookupRefundable(GUID);

            // If the item is refundable we look up the extendedcost
            if (RefundEntry.first != 0 && RefundEntry.second != 0)
            {
                uint32* played = _player->GetPlayedtime();
                if (played[1] < (RefundEntry.first + 60 * 60 * 2))
                    item_extended_cost = sItemExtendedCostStore.LookupEntry(RefundEntry.second);
            }

            if (item_extended_cost != nullptr)
            {
                item_proto = item->GetItemProperties();

                ////////////////////////////////// We remove the refunded item and refund the cost //////////////////////////////////

                for (uint8 i = 0; i < 5; ++i)
                {
                    _player->GetItemInterface()->AddItemById(item_extended_cost->item[i], item_extended_cost->count[i], 0);
                }

                _player->GetItemInterface()->AddItemById(43308, item_extended_cost->honor_points, 0);
                _player->GetItemInterface()->AddItemById(43307, item_extended_cost->arena_points, 0);
                _player->ModGold(item_proto->BuyPrice);

                _player->GetItemInterface()->RemoveItemAmtByGuid(GUID, 1);

                _player->GetItemInterface()->RemoveRefundable(GUID);

                // we were successful!
                error = 0;
                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            }
        }
    }

    WorldPacket packet(SMSG_ITEMREFUNDREQUEST, 60);
    packet << uint64(GUID);
    packet << uint32(error);

    if (error == 0)
    {
        packet << uint32(item_proto->BuyPrice);
        packet << uint32(item_extended_cost->honor_points);
        packet << uint32(item_extended_cost->arena_points);

        for (uint8 i = 0; i < 5; ++i)
        {
            packet << uint32(item_extended_cost->item[i]);
            packet << uint32(item_extended_cost->count[i]);
        }

    }

    SendPacket(&packet);

    LOG_DEBUG("Sent SMSG_ITEMREFUNDREQUEST.");
}
