/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2007 Moon++ <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ITEMFUNCTIONS_H
#define ITEMFUNCTIONS_H

#include "Management/Item.h"
#include "Management/Container.h"
#include "Management/ItemInterface.h"
#include "Server/MainServerDefines.h"

namespace luaItem
{
    ///////GOSSIP///////
    int GossipCreateMenu(lua_State* L, Item* ptr)
    {
        uint32 text_id = static_cast<uint32>(luaL_checkinteger(L, 1));
        auto player = CHECK_PLAYER(L, 2);
        int autosend = static_cast<int>(luaL_checkinteger(L, 3));

        if (player == nullptr)
            return 0;

        if (LuaGlobal::instance()->m_menu != nullptr)
            delete LuaGlobal::instance()->m_menu;

        LuaGlobal::instance()->m_menu = new Arcemu::Gossip::Menu(ptr->GetGUID(), text_id);

        if (autosend != 0)
            LuaGlobal::instance()->m_menu->Send(player);

        return 1;
    }

    int GossipMenuAddItem(lua_State* L, Item* /*ptr*/)
    {
        uint8 icon = static_cast<uint8>(luaL_checkinteger(L, 1));
        const char* menu_text = luaL_checkstring(L, 2);
        uint32 IntId = static_cast<uint32>(luaL_checkinteger(L, 3));
        bool coded = (luaL_checkinteger(L, 4)) ? true : false;
        const char* boxmessage = luaL_optstring(L, 5, "");
        uint32 boxmoney = static_cast<uint32>(luaL_optinteger(L, 6, 0));

        if (LuaGlobal::instance()->m_menu == NULL)
        {
            LOG_ERROR("There is no menu to add items to!");
            return 0;
        }

        LuaGlobal::instance()->m_menu->AddItem(icon, menu_text, IntId, boxmoney, boxmessage, coded);

        return 0;
    }

    int GossipSendMenu(lua_State* L, Item* /*ptr*/)
    {
        Player* plr = CHECK_PLAYER(L, 1);

        if (LuaGlobal::instance()->m_menu == NULL)
        {
            LOG_ERROR("There is no menu to send!");
            return 0;
        }

        LuaGlobal::instance()->m_menu->Send(plr);

        return 1;
    }

    int GossipComplete(lua_State* L, Item* /*ptr*/)
    {
        Player* plr = CHECK_PLAYER(L, 1);

        if (LuaGlobal::instance()->m_menu == NULL)
        {
            LOG_ERROR("There is no menu to complete!");
            return 0;
        }

        LuaGlobal::instance()->m_menu->Complete(plr);

        return 1;
    }

    int GossipSendPOI(lua_State* L, Item* /*ptr*/)
    {
        Player* plr = CHECK_PLAYER(L, 1);
        float x = CHECK_FLOAT(L, 2);
        float y = CHECK_FLOAT(L, 3);
        uint32 icon = static_cast<uint32>(luaL_checkinteger(L, 4));
        uint32 flags = static_cast<uint32>(luaL_checkinteger(L, 5));
        uint32 data = static_cast<uint32>(luaL_checkinteger(L, 6));
        const char* name = luaL_checkstring(L, 7);

        plr->Gossip_SendPOI(x, y, icon, flags, data, name);
        return 1;
    }

    static int GossipSendQuickMenu(lua_State *L, Item *ptr)
    {
        if (ptr == NULL)
            return 0;

        uint32 text_id = static_cast<uint32>(luaL_checkinteger(L, 1));
        Player *player = CHECK_PLAYER(L, 2);
        uint32 itemid = static_cast<uint32>(luaL_checkinteger(L, 3));
        uint8 itemicon = CHECK_UINT8(L, 4);
        const char *itemtext = luaL_checkstring(L, 5);
        uint32 requiredmoney = CHECK_ULONG(L, 6);
        const char *moneytext = luaL_checkstring(L, 7);
        uint8 extra = CHECK_UINT8(L, 8);

        if (player == NULL)
            return 0;

        Arcemu::Gossip::Menu::SendQuickMenu(ptr->GetGUID(), text_id, player, itemid, itemicon, itemtext, requiredmoney, moneytext, extra);

        return 0;
    }


    int GetOwner(lua_State* L, Item* ptr)
    {
        Player* owner = ptr->GetOwner();
        if (owner != NULL)
            PUSH_UNIT(L, owner);
        else
            lua_pushnil(L);
        return 1;
    }

    int AddEnchantment(lua_State* L, Item* ptr)
    {
        uint32 entry = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 duration = static_cast<uint32>(luaL_checkinteger(L, 2));
        bool permanent = (duration == 0) ? true : false;
        bool temp = (luaL_checkinteger(L, 3) == 1) ? true : false;

        auto spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(entry);
        if (spell_item_enchant == nullptr)
            return 0;

        lua_pushinteger(L, ptr->AddEnchantment(spell_item_enchant, duration, permanent, true, temp)); //Return the enchantment Slot back to LUA
        return 1;
    }

    int GetGUID(lua_State* L, Item* ptr)
    {
        PUSH_GUID(L, ptr->GetGUID());
        return 1;
    }

    int RemoveEnchantment(lua_State* L, Item* ptr)
    {
        int slot = static_cast<int>(luaL_checkinteger(L, 1));
        bool temp = CHECK_BOOL(L, 2);

        if (slot == -1)	ptr->RemoveAllEnchantments(temp);
        else if (slot == -2) ptr->RemoveProfessionEnchant();
        else if (slot == -3) ptr->RemoveSocketBonusEnchant();
        else if (slot >= 0) ptr->RemoveEnchantment(slot);

        return 0;
    }

    int GetEntryId(lua_State* L, Item* ptr)
    {
        if (!ptr)
            return 0;
        ItemProperties const* proto = ptr->GetItemProperties();
        lua_pushnumber(L, proto->ItemId);
        return 1;
    }

    int GetName(lua_State* L, Item* ptr)
    {
        if (!ptr)
            return 0;

        ItemProperties const* proto = ptr->GetItemProperties();
        lua_pushstring(L, proto->Name.c_str());
        return 1;
    }

    int GetSpellId(lua_State* L, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32 index = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (index >= 5)
            return 0;

        ItemProperties const* proto = ptr->GetItemProperties();
        lua_pushnumber(L, proto->Spells[index].Id);
        return 1;
    }

    int GetSpellTrigger(lua_State* L, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32 index = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (index >= 5)
            return 0;

        ItemProperties const* proto = ptr->GetItemProperties();
        lua_pushnumber(L, proto->Spells[index].Trigger);
        /*
            USE				= 0,
            ON_EQUIP		= 1,
            CHANCE_ON_HIT	= 2,
            SOULSTONE		= 4,
            LEARNING		= 6,
            */
        return 1;
    }

    int AddLoot(lua_State* L, Item* ptr)
    {
        //TEST_UNIT()
        if ((lua_gettop(L) != 3) || (lua_gettop(L) != 5))
            return 0;
        uint32 itemid = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 mincount = static_cast<uint32>(luaL_checkinteger(L, 2));
        uint32 maxcount = static_cast<uint32>(luaL_checkinteger(L, 3));
        bool perm = ((luaL_optinteger(L, 4, 0) == 1) ? true : false);
        if (perm)
        {
            float chance = CHECK_FLOAT(L, 5);
            QueryResult* result = WorldDatabase.Query("SELECT * FROM loot_items WHERE entryid = %u, itemid = %u", ptr->GetEntry(), itemid);
            if (!result)
                WorldDatabase.Execute("REPLACE INTO loot_items VALUES (%u, %u, %f, 0, 0, 0, %u, %u )", ptr->GetEntry(), itemid, chance, mincount, maxcount);
            delete result;
        }
        lootmgr.AddLoot(ptr->loot, itemid, mincount, maxcount);
        return 1;
    }

    int GetItemLink(lua_State* L, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32 lang = static_cast<uint32>(luaL_optinteger(L, 1, LANG_UNIVERSAL));
        lua_pushstring(L, ptr->GetItemLink(lang).c_str());
        return 1;
    }
    int SetByteValue(lua_State* L, Item* ptr)
    {
        uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        uint8_t index1 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
        uint8_t value = static_cast<uint8_t>(luaL_checkinteger(L, 3));
        ptr->setByteValue(index, index1, value);
        return 1;
    }

    int GetByteValue(lua_State* L, Item* ptr)
    {
        uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        uint8_t index1 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
        lua_pushinteger(L, ptr->getByteValue(index, index1));
        return 1;
    }

    int GetItemLevel(lua_State* L, Item* ptr)
    {
        lua_pushnumber(L, ptr->GetItemProperties()->ItemLevel);
        return 1;
    }

    int GetRequiredLevel(lua_State* L, Item* ptr)
    {
        lua_pushnumber(L, ptr->GetItemProperties()->RequiredLevel);
        return 1;
    }

    int GetBuyPrice(lua_State* L, Item* ptr)
    {
        lua_pushnumber(L, ptr->GetItemProperties()->BuyPrice);
        return 1;
    }

    int GetSellPrice(lua_State* L, Item* ptr)
    {
        lua_pushnumber(L, ptr->GetItemProperties()->SellPrice);
        return 1;
    }

    int RepairItem(lua_State* /*L*/, Item* ptr)
    {
        if (!ptr)
            return 0;
        ptr->SetDurabilityToMax();
        return 1;
    }

    int GetMaxDurability(lua_State* L, Item* ptr)
    {
        if (!ptr)
            return 0;
        lua_pushnumber(L, ptr->GetDurabilityMax());
        return 1;
    }

    int GetDurability(lua_State* L, Item* ptr)
    {
        if (!ptr)
            return 0;
        lua_pushnumber(L, ptr->GetDurability());
        return 1;
    }

    int HasEnchantment(lua_State* L, Item* ptr)
    {
        if (!ptr)
            return 0;
        if (ptr->HasEnchantments())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    int ModifyEnchantmentTime(lua_State* L, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32 slot = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 duration = static_cast<uint32>(luaL_checkinteger(L, 2));

        ptr->ModifyEnchantmentTime(slot, duration);
        return 1;
    }

    int SetStackCount(lua_State* L, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32 count = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (!count || count > 1000)
            return 0;
        ptr->SetStackCount(count);
        return 1;
    }

    int HasFlag(lua_State* L, Item* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint16 index = static_cast<uint16>(luaL_checkinteger(L, 1));
        uint32 flag = static_cast<uint32>(luaL_checkinteger(L, 2));
        lua_pushboolean(L, ptr->HasFlag(index, flag) ? 1 : 0);
        return 1;
    }

    int IsSoulbound(lua_State* L, Item* ptr)
    {
        ptr->IsSoulbound() ? lua_pushboolean(L, 1) : lua_pushboolean(L, 0);
        return 1;
    }

    int IsAccountbound(lua_State* L, Item* ptr)
    {
        ptr->IsAccountbound() ? lua_pushboolean(L, 1) : lua_pushboolean(L, 0);
        return 1;
    }

    int IsContainer(lua_State* L, Item* ptr)
    {
        ptr->IsContainer() ? lua_pushboolean(L, 1) : lua_pushboolean(L, 0);
        return 1;
    }

    int GetContainerItemCount(lua_State* L, Item* ptr)
    {
        uint32 itemid = CHECK_ULONG(L, 1);
        if (!ptr->IsContainer() || !itemid) return 0;
        Container* pCont = static_cast< Container* >(ptr);
        int16 TotalSlots = static_cast<int16>(pCont->GetNumSlots());
        int cnt = 0;
        for (int16 i = 0; i < TotalSlots; i++)
        {
            Item* item = pCont->GetItem(i);
            if (item)
            {
                if (item->GetEntry() == itemid && item->wrapped_item_id == 0)
                {
                    cnt += item->GetStackCount() ? item->GetStackCount() : 1;
                }
            }
        }
        lua_pushinteger(L, cnt);
        return 1;
    }

    int GetEquippedSlot(lua_State* L, Item* ptr)
    {
        if (!ptr) return 0;
        lua_pushinteger(L, ptr->GetOwner()->GetItemInterface()->GetInventorySlotById(ptr->GetEntry()));
        return 1;
    }

    int GetObjectType(lua_State* L, Item* ptr)
    {
        if (!ptr) { lua_pushnil(L); return 1; }
        lua_pushstring(L, "Item");
        return 1;
    }

    int Remove(lua_State* /*L*/, Item* ptr)
    {
        if (ptr == NULL || !ptr->IsInWorld() || !ptr->IsItem())
        {
            return 0;
        }
        ptr->RemoveFromWorld();
        return 0;
    }

    int Create(lua_State* L, Item* /*ptr*/)
    {
        uint32 id = CHECK_ULONG(L, 1);
        uint32 stackcount = CHECK_ULONG(L, 2);
        Item* pItem = objmgr.CreateItem(id, NULL);
        if (!pItem)
            RET_NIL();
        pItem->SetStackCount(stackcount);
        pItem->SaveToDB(0, 0, true, NULL);
        PUSH_ITEM(L, pItem);
        return 1;
    }

    int ModUInt32Value(lua_State* L, Item* ptr)
    {
        uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        int32 value = static_cast<int32>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->modInt32Value(field, value);
        return 0;
    }

    int ModFloatValue(lua_State* L, Item* ptr)
    {
        uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        float value = CHECK_FLOAT(L, 2);
        if (ptr)
            ptr->modFloatValue(field, value);
        return 0;
    }

    int SetUInt32Value(lua_State* L, Item* ptr)
    {
        uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        uint8_t value = static_cast<uint8_t>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->setUInt32Value(field, value);
        return 0;
    }

    int SetUInt64Value(lua_State* L, Item* ptr)
    {
        uint16_t field = static_cast<uint16_t>(CHECK_ULONG(L, 1));
        uint64_t guid = static_cast<uint64_t>(CHECK_GUID(L, 2));
        if (ptr)
            ptr->setUInt64Value(field, guid);
        return 0;
    }

    int RemoveFlag(lua_State* L, Item* ptr)
    {
        uint16 field = static_cast<uint16>(luaL_checkinteger(L, 1));
        uint32 value = static_cast<uint32>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->RemoveFlag(field, value);
        return 0;
    }

    int SetFlag(lua_State* L, Item* ptr)
    {
        uint16 field = static_cast<uint16>(luaL_checkinteger(L, 1));
        uint32 value = static_cast<uint32>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->SetFlag(field, value);
        return 0;
    }

    int SetFloatValue(lua_State* L, Item* ptr)
    {
        uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        float value = CHECK_FLOAT(L, 2);
        if (ptr)
            ptr->setFloatValue(field, value);
        return 0;
    }

    int GetUInt32Value(lua_State* L, Item* ptr)
    {
        uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        if (ptr)
            lua_pushnumber(L, ptr->getUInt32Value(field));
        return 1;
    }

    int GetUInt64Value(lua_State* L, Item* ptr)
    {
        uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        if (ptr)
            PUSH_GUID(L, ptr->getUInt64Value(field));
        return 1;
    }

    int GetFloatValue(lua_State* L, Item* ptr)
    {
        uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        if (ptr)
            lua_pushnumber(L, ptr->getFloatValue(field));
        return 1;
    }
}

#endif      // ITEMFUNCTIONS_H
