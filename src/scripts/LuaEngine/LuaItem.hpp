/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

extern "C"
{
#include <lua/lua.h>
}

class Item;

class LuaItem
{
public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // GOSSIP
    static int GossipCreateMenu(lua_State* L, Item* ptr);
    static int GossipMenuAddItem(lua_State* L, Item* /*ptr*/);
    static int GossipSendMenu(lua_State* L, Item* /*ptr*/);
    static int GossipComplete(lua_State* L, Item* /*ptr*/);
    static int GossipSendPOI(lua_State* L, Item* /*ptr*/);
    static int GossipSendQuickMenu(lua_State *L, Item *ptr);
    static int GetOwner(lua_State* L, Item* ptr);
    static int AddEnchantment(lua_State* L, Item* ptr);
    static int GetGUID(lua_State* L, Item* ptr);
    static int RemoveEnchantment(lua_State* L, Item* ptr);
    static int GetEntryId(lua_State* L, Item* ptr);
    static int GetName(lua_State* L, Item* ptr);
    static int GetSpellId(lua_State* L, Item* ptr);
    static int GetSpellTrigger(lua_State* L, Item* ptr);
    static int AddLoot(lua_State* L, Item* ptr);
    static int GetItemLink(lua_State* L, Item* ptr);
    static int SetByteValue(lua_State* /*L*/, Item* /*ptr*/);
    static int GetByteValue(lua_State* /*L*/, Item* /*ptr*/);
    static int GetItemLevel(lua_State* L, Item* ptr);
    static int GetRequiredLevel(lua_State* L, Item* ptr);
    static int GetBuyPrice(lua_State* L, Item* ptr);
    static int GetSellPrice(lua_State* L, Item* ptr);
    static int RepairItem(lua_State* /*L*/, Item* ptr);
    static int GetMaxDurability(lua_State* L, Item* ptr);
    static int GetDurability(lua_State* L, Item* ptr);
    static int HasEnchantment(lua_State* L, Item* ptr);
    static int ModifyEnchantmentTime(lua_State* L, Item* ptr);
    static int SetStackCount(lua_State* L, Item* ptr);
    static int HasFlag(lua_State* /*L*/, Item* ptr);
    static int IsSoulbound(lua_State* L, Item* ptr);
    static int IsAccountbound(lua_State* L, Item* ptr);
    static int IsContainer(lua_State* L, Item* ptr);
    static int GetContainerItemCount(lua_State* L, Item* ptr);
    static int GetEquippedSlot(lua_State* L, Item* ptr);
    static int GetObjectType(lua_State* L, Item* ptr);
    static int Remove(lua_State* /*L*/, Item* ptr);
    static int Create(lua_State* L, Item* /*ptr*/);
    static int ModUInt32Value(lua_State* /*L*/, Item* /*ptr*/);
    static int ModFloatValue(lua_State* /*L*/, Item* /*ptr*/);
    static int SetUInt32Value(lua_State* /*L*/, Item* /*ptr*/);
    static int SetUInt64Value(lua_State* /*L*/, Item* /*ptr*/);
    static int RemoveFlag(lua_State* /*L*/, Item* /*ptr*/);
    static int SetFlag(lua_State* /*L*/, Item* /*ptr*/);
    static int SetFloatValue(lua_State* /*L*/, Item* /*ptr*/);
    static int GetUInt32Value(lua_State* /*L*/, Item* /*ptr*/);
    static int GetUInt64Value(lua_State* /*L*/, Item* /*ptr*/);
    static int GetFloatValue(lua_State* /*L*/, Item* /*ptr*/);
};
