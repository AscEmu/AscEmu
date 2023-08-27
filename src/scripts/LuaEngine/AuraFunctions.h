/*
 * Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

#ifndef LUA_AURA_H
#define LUA_AURA_H

#include "SpellFunctions.h"
#include "Spell/SpellAuras.h"

namespace LuaAura
{
    int GetObjectType(lua_State* L, Aura* aura)
    {
        if (!aura)
        {
            lua_pushnil(L);
            return 1;
        }
        lua_pushstring(L, "Aura");
        return 1;
    }

    int GetSpellId(lua_State* L, Aura* aura)
    {
        if (!aura)
        {
            lua_pushnil(L);
            return 1;
        }
        lua_pushnumber(L, aura->getSpellId());
        return 1;
    }

    int GetCaster(lua_State* L, Aura* aura)
    {
        if (!aura)
        {
            lua_pushnil(L);
            return 1;
        }
        Object* caster = aura->getCaster();
        if (caster->isCreatureOrPlayer())  //unit caster
        {
            PUSH_UNIT(L, caster);
            return 1;
        }
        
        if (caster->isGameObject())  //gameobject
        {
            PUSH_GO(L, caster);
            return 1;
        }
        
        if (caster->getObjectTypeId() == TYPEID_ITEM)  //item
        {
            PUSH_ITEM(L, caster);
            return 1;
        }

        lua_pushnil(L);
        return 1;
    }

    int GetTarget(lua_State* L, Aura* aura)
    {
        if (!aura)
        {
            lua_pushnil(L);
            return 1;
        }

        PUSH_UNIT(L, aura->getOwner());
        return 1;
    }

    int GetDuration(lua_State* L, Aura* aura)
    {
        if (!aura)
        {
            lua_pushnil(L);
            return 1;
        }
        RET_NUMBER(aura->getMaxDuration()); //in milliseconds
    }

    int SetDuration(lua_State* L, Aura* aura)
    {
        if (!aura)
            return 0;

        uint32_t duration = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        aura->setTimeLeft(duration);
        aura->getOwner()->sendAuraUpdate(aura, false);
        return 0;
    }

    int GetTimeLeft(lua_State* L, Aura* aura)
    {
        if (!aura)
        {
            lua_pushnil(L);
            return 1;
        }
        RET_NUMBER(aura->getTimeLeft()); //in milliseconds
    }

    int Remove(lua_State* /*L*/, Aura* aura)
    {
        if (!aura)
            return 0;
        aura->removeAura();
        return 0;
    }

    int SetVar(lua_State* L, Aura* aura)
    {
        const char* var = luaL_checkstring(L, 1);
        int subindex = 0;
        if (lua_gettop(L) == 3)
        {
            subindex = static_cast<int>(luaL_optinteger(L, 2, 0));
        }
        if (!aura || !var || subindex < 0)
        {
            lua_pushboolean(L, 0);
            return 1;
        }
        int valindex = 2;
        if (subindex)
            valindex++;
        SpellInfo const* proto = aura->getSpellInfo();
        LuaSpellEntry l = GetLuaSpellEntryByName(var);
        if (!l.name)
            RET_BOOL(false);
        switch (l.typeId)  //0: int, 1: char*, 2: bool, 3: float
        {
            case 0:
                GET_SPELLVAR_INT(proto, l.offset, subindex) = static_cast<int>(luaL_checkinteger(L, valindex));
                lua_pushboolean(L, 1);
                break;
            case 1:
                strcpy(GET_SPELLVAR_CHAR(proto, l.offset, subindex), luaL_checkstring(L, valindex));
                lua_pushboolean(L, 1);
                break;
            case 2:
                GET_SPELLVAR_BOOL(proto, l.offset, subindex) = CHECK_BOOL(L, valindex);
                lua_pushboolean(L, 1);
                break;
            case 3:
                GET_SPELLVAR_FLOAT(proto, l.offset, subindex) = (float)luaL_checknumber(L, valindex);
                lua_pushboolean(L, 1);
                break;
        }
        return 1;
    }

    int GetVar(lua_State* L, Aura* aura)
    {
        const char* var = luaL_checkstring(L, 1);
        int subindex = static_cast<int>(luaL_optinteger(L, 2, 0));
        if (!aura || !var || subindex < 0)
        {
            lua_pushnil(L);
            return 1;
        }
        SpellInfo const* proto = aura->getSpellInfo();
        LuaSpellEntry l = GetLuaSpellEntryByName(var);
        if (!l.name)
        {
            lua_pushnil(L);
            return 1;
        }
        switch (l.typeId)  //0: int, 1: char*, 2: bool, 3: float
        {
            case 0:
                lua_pushinteger(L, GET_SPELLVAR_INT(proto, l.offset, subindex));
                break;
            case 1:
                lua_pushstring(L, GET_SPELLVAR_CHAR(proto, l.offset, subindex));
                break;
            case 2:
                lua_pushboolean(L, (GET_SPELLVAR_BOOL(proto, l.offset, subindex)) ? 1 : 0);
                break;
            case 3:
                lua_pushnumber(L, GET_SPELLVAR_FLOAT(proto, l.offset, subindex));
                break;
        }
        return 1;
    }

    int GetAuraSlot(lua_State* L, Aura* aura)
    {
        if (!aura)
        {
            lua_pushnil(L);
            return 1;
        }
        RET_INT(aura->getAuraSlot());
    }

    int SetAuraSlot(lua_State* L, Aura* aura)
    {
        if (!aura) return 0;
        uint16_t slot = CHECK_USHORT(L, 1);
        aura->setAuraSlot(slot);
        return 0;
    }
};

#endif      // LUA_AURA_H
