/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

extern "C"
{
#include <lua/lua.h>
}

class Aura;

class LuaAura
{
public:
    static int GetObjectType(lua_State* L, Aura* aura);
    static int GetSpellId(lua_State* L, Aura* aura);
    static int GetCaster(lua_State* L, Aura* aura);
    static int GetTarget(lua_State* L, Aura* aura);
    static int GetDuration(lua_State* L, Aura* aura);
    static int SetDuration(lua_State* L, Aura* aura);
    static int GetTimeLeft(lua_State* L, Aura* aura);
    static int Remove(lua_State* /*L*/, Aura* aura);
    static int SetVar(lua_State* L, Aura* aura);
    static int GetVar(lua_State* L, Aura* aura);
    static int GetAuraSlot(lua_State* L, Aura* aura);
    static int SetAuraSlot(lua_State* L, Aura* aura);
};
