/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "LuaSpell.hpp"

#include "LUAEngine.hpp"
#include "LuaGlobal.hpp"
#include "LuaMacros.h"
#include "Management/ItemInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Item.hpp"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/Spell.hpp"

int LuaSpell::GetCaster(lua_State* L, Spell* sp)
{
    if (!sp)
        return 0;

    if (sp->getUnitCaster())  //unit caster
    {
        PUSH_UNIT(L, sp->getUnitCaster());
        return 1;
    }

    if (sp->getGameObjectCaster())  //gameobject
    {
        PUSH_GO(L, sp->getGameObjectCaster());
        return 1;
    }

    if (sp->getItemCaster())  //item
    {
        PUSH_ITEM(L, sp->getItemCaster());
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

int LuaSpell::GetEntry(lua_State* L, Spell* sp)
{
    if (!sp)
        return 0;
    lua_pushinteger(L, sp->getSpellInfo()->getId());
    return 1;
}

int LuaSpell::IsDuelSpell(lua_State* L, Spell* sp)
{
    if (!sp)
        return 0;
    lua_pushboolean(L, sp->wasCastedinDuel() ? 1 : 0);
    return 1;
}

int LuaSpell::GetSpellType(lua_State* L, Spell* sp)
{
    if (!sp)
        return 0;
    lua_pushinteger(L, sp->GetType());
    return 1;
}

int LuaSpell::GetSpellState(lua_State* L, Spell* sp)
{
    /*
    SPELL_STATE_NULL      = 0,
    SPELL_STATE_PREPARING = 1,
    SPELL_STATE_CHANNELING   = 2,
    SPELL_STATE_FINISHED  = 3,
    SPELL_STATE_IDLE      = 4
    */
    if (!sp)
        return 0;
    lua_pushinteger(L, sp->getState());
    return 1;
}

int LuaSpell::Cancel(lua_State* /*L*/, Spell* sp)
{
    if (!sp || !sp->getCaster()->IsInWorld())
        return 0;
    sp->getCaster()->interruptSpell(sp->getSpellInfo()->getId());
    return 0;
}

int LuaSpell::Cast(lua_State* L, Spell* sp)
{
    if (!sp)
        return 0;
    bool check = CHECK_BOOL(L, 1);
    sp->castMe(check);
    return 0;
}

int LuaSpell::CanCast(lua_State* L, Spell* sp)
{
    if (!sp)
        return 0;
    lua_pushinteger(L, sp->canCast(false, 0, 0));
    return 1;
}

int LuaSpell::Finish(lua_State* /*L*/, Spell* sp)
{
    if (!sp)
        return 0;
    sp->finish();
    return 0;
}

int LuaSpell::GetTarget(lua_State* L, Spell* sp)
{
    if (!sp || !sp->getCaster()->IsInWorld())
    {
        lua_pushnil(L);
        return 1;
    }

    if (sp->m_targets.getUnitTargetGuid())
    {
        PUSH_UNIT(L, sp->getCaster()->getWorldMap()->getUnit(sp->m_targets.getUnitTargetGuid()));
        return 1;
    }

    if (sp->m_targets.getItemTargetGuid())
    {
        if (!sp->getPlayerCaster())
        {
            lua_pushnil(L);
            PUSH_ITEM(L, sp->getPlayerCaster()->getItemInterface()->GetItemByGUID(sp->m_targets.getItemTargetGuid()));
            return 1;
        }
    }

    lua_pushnil(L);
    return 1;
}

int LuaSpell::IsStealthSpell(lua_State* L, Spell* sp)
{
    if (!sp) return 0;
    lua_pushboolean(L, sp->IsStealthSpell() ? 1 : 0);
    return 1;
}

int LuaSpell::IsInvisibilitySpell(lua_State* L, Spell* sp)
{
    if (!sp) return 0;
    lua_pushboolean(L, sp->IsInvisibilitySpell() ? 1 : 0);
    return 1;
}

int LuaSpell::GetPossibleEnemy(lua_State* L, Spell* sp)
{
    float range = (float)luaL_optnumber(L, 1, 0.0f);
    if (!sp || range < 0) return 0;
    PUSH_GUID(L, sp->GetSinglePossibleEnemy(0, range));
    return 1;
}

int LuaSpell::GetPossibleFriend(lua_State* L, Spell* sp)
{
    float range = (float)luaL_optnumber(L, 1, 0.0f);
    if (!sp || range < 0) return 0;
    PUSH_GUID(L, sp->GetSinglePossibleFriend(0, range));
    return 1;
}

int LuaSpell::HasPower(lua_State* L, Spell* sp)
{
    if (!sp) return 0;
    lua_pushboolean(L, sp->checkPower() ? 1 : 0);
    return 1;
}

int LuaSpell::IsAspect(lua_State* L, Spell* sp)
{
    if (!sp) return 0;
    lua_pushboolean(L, sp->IsAspect() ? 1 : 0);
    return 1;
}

int LuaSpell::IsSeal(lua_State* L, Spell* sp)
{
    if (!sp) return 0;
    lua_pushboolean(L, sp->IsSeal() ? 1 : 0);
    return 1;
}

int LuaSpell::GetObjectType(lua_State* L, Spell* sp)
{
    if (!sp)
    {
        lua_pushnil(L);
        return 1;
    }
    lua_pushstring(L, "Spell");
    return 1;
}

int LuaSpell::SetVar(lua_State* L, Spell* sp)
{
    const char* var = luaL_checkstring(L, 1);
    int subindex = 0;
    int valindex = 2;
    if (lua_gettop(L) == 3)
    {
        subindex = static_cast<int>(luaL_optinteger(L, 2, 0));
        valindex++;
    }
    if (!sp || !var || subindex < 0)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    sp->InitProtoOverride();
    SpellInfo const* proto = sp->getSpellInfo();
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

int LuaSpell::GetVar(lua_State* L, Spell* sp)
{
    const char* var = luaL_checkstring(L, 1);
    int subindex = static_cast<int>(luaL_optinteger(L, 2, 0));
    if (!sp || !var || subindex < 0)
    {
        lua_pushnil(L);
        return 1;
    }
    SpellInfo const* proto = sp->getSpellInfo();
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

int LuaSpell::ResetVar(lua_State* L, Spell* sp)
{
    const char* var = luaL_checkstring(L, 1);
    int subindex = static_cast<int>(luaL_optinteger(L, 2, 0));
    if (!sp || !var || subindex < 0)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    LuaSpellEntry l = GetLuaSpellEntryByName(var);
    if (!l.name)
        RET_BOOL(false);
    switch (l.typeId)  //0: int, 1: char*, 2: bool, 3: float
    {
        case 0:
            GET_SPELLVAR_INT(sp->getSpellInfo(), l.offset, subindex) = GET_SPELLVAR_INT(sp->getSpellInfo(), l.offset, subindex);
            lua_pushboolean(L, 1);
            break;
        case 1:
            GET_SPELLVAR_CHAR(sp->getSpellInfo(), l.offset, subindex) = GET_SPELLVAR_CHAR(sp->getSpellInfo(), l.offset, subindex);
            lua_pushboolean(L, 1);
            break;
        case 2:
            GET_SPELLVAR_BOOL(sp->getSpellInfo(), l.offset, subindex) = GET_SPELLVAR_BOOL(sp->getSpellInfo(), l.offset, subindex);
            lua_pushboolean(L, 1);
            break;
        case 3:
            GET_SPELLVAR_FLOAT(sp->getSpellInfo(), l.offset, subindex) = GET_SPELLVAR_FLOAT(sp->getSpellInfo(), l.offset, subindex);
            lua_pushboolean(L, 1);
            break;
    }
    return 1;
}

int LuaSpell::ResetAllVars(lua_State* /*L*/, Spell* sp)
{
    if (!sp)
        return 0;
    sp->resetSpellInfoOverride();
    return 0;
}

int LuaSpell::GetCastedItemId(lua_State* L, Spell* sp)
{
    if (!sp)
    {
        lua_pushnil(L);
        return 1;
    }
    lua_pushnumber(L, sp->castedItemId);
    return 1;
}
