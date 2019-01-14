/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

#pragma once

#include "Management/ItemInterface.h"

#define GET_SPELLVAR_INT(proto,offset,subindex) *(int*)((char*)(proto) + (offset) + (subindex))
#define GET_SPELLVAR_CHAR(proto,offset,subindex) *(char**)((char*)(proto) + (offset) + (subindex))
#define GET_SPELLVAR_BOOL(proto,offset,subindex) *(bool*)((char*)(proto) + (offset) + (subindex))
#define GET_SPELLVAR_FLOAT(proto,offset,subindex) *(float*)((char*)(proto) + (offset) + (subindex))

struct LuaSpellEntry
{
    const char* name;
    uint32 typeId; //0: int, 1: char*, 2: bool, 3: float
    size_t offset;
};

LuaSpellEntry luaSpellVars[] =
{
    //{ "Id", 0, offsetof(SpellInfo, Id) },
    //{ "Category", 0, offsetof(SpellInfo, Category) },
    //{ "DispelType", 0, offsetof(SpellInfo, DispelType) },
    //{ "MechanicsType", 0, offsetof(SpellInfo, MechanicsType) },
    //{ "Attributes", 0, offsetof(SpellInfo, Attributes) },
    //{ "AttributesEx", 0, offsetof(SpellInfo, AttributesEx) },
    //{ "AttributesExB", 0, offsetof(SpellInfo, AttributesExB) },
    //{ "AttributesExC", 0, offsetof(SpellInfo, AttributesExC) },
    //{ "AttributesExD", 0, offsetof(SpellInfo, AttributesExD) },
    //{ "AttributesExE", 0, offsetof(SpellInfo, AttributesExE) },
    //{ "AttributesExF", 0, offsetof(SpellInfo, AttributesExF) },
    //{ "AttributesExG", 0, offsetof(SpellInfo, AttributesExG) },
    //{ "RequiredShapeShift", 0, offsetof(SpellInfo, RequiredShapeShift) },
    //{ "ShapeshiftExclude", 0, offsetof(SpellInfo, ShapeshiftExclude) },
    //{ "Targets", 0, offsetof(SpellInfo, Targets) },
    //{ "TargetCreatureType", 0, offsetof(SpellInfo, TargetCreatureType) },
    //{ "RequiresSpellFocus", 0, offsetof(SpellInfo, RequiresSpellFocus) },
    //{ "FacingCasterFlags", 0, offsetof(SpellInfo, FacingCasterFlags) },
    //{ "CasterAuraState", 0, offsetof(SpellInfo, CasterAuraState) },
    //{ "TargetAuraState", 0, offsetof(SpellInfo, TargetAuraState) },
    //{ "CasterAuraStateNot", 0, offsetof(SpellInfo, CasterAuraStateNot) },
    //{ "TargetAuraStateNot", 0, offsetof(SpellInfo, TargetAuraStateNot) },
    //{ "casterAuraSpell", 0, offsetof(SpellInfo, casterAuraSpell) },
    //{ "targetAuraSpell", 0, offsetof(SpellInfo, targetAuraSpell) },
    //{ "casterAuraSpellNot", 0, offsetof(SpellInfo, casterAuraSpellNot) },
    //{ "targetAuraSpellNot", 0, offsetof(SpellInfo, targetAuraSpellNot) },
    //{ "CastingTimeIndex", 0, offsetof(SpellInfo, CastingTimeIndex) },
    //{ "RecoveryTime", 0, offsetof(SpellInfo, RecoveryTime) },
    //{ "CategoryRecoveryTime", 0, offsetof(SpellInfo, CategoryRecoveryTime) },
    //{ "InterruptFlags", 0, offsetof(SpellInfo, InterruptFlags) },
    //{ "AuraInterruptFlags", 0, offsetof(SpellInfo, AuraInterruptFlags) },
    //{ "ChannelInterruptFlags", 0, offsetof(SpellInfo, ChannelInterruptFlags) },
    //{ "procFlags", 0, offsetof(SpellInfo, procFlags) },
    //{ "procChance", 0, offsetof(SpellInfo, procChance) },
    //{ "procCharges", 0, offsetof(SpellInfo, procCharges) },
    //{ "maxLevel", 0, offsetof(SpellInfo, maxLevel) },
    //{ "baseLevel", 0, offsetof(SpellInfo, baseLevel) },
    //{ "spellLevel", 0, offsetof(SpellInfo, spellLevel) },
    //{ "DurationIndex", 0, offsetof(SpellInfo, DurationIndex) },
    //{ "powerType", 0, offsetof(SpellInfo, powerType) },
    //{ "manaCost", 0, offsetof(SpellInfo, manaCost) },
    //{ "manaCostPerlevel", 0, offsetof(SpellInfo, manaCostPerlevel) },
    //{ "manaPerSecond", 0, offsetof(SpellInfo, manaPerSecond) },
    //{ "manaPerSecondPerLevel", 0, offsetof(SpellInfo, manaPerSecondPerLevel) },
    //{ "rangeIndex", 0, offsetof(SpellInfo, rangeIndex) },
    //{ "speed", 3, offsetof(SpellInfo, speed) },
#if VERSION_STRING < Cata
    //{ "modalNextSpell", 0, offsetof(SpellInfo, modalNextSpell) },
#endif
    //{ "maxstack", 0, offsetof(SpellInfo, maxstack) },
    //{ "Totem", 0, offsetof(SpellInfo, Totem[0]) },
    //{ "Reagent", 0, offsetof(SpellInfo, Reagent[0]) },
    //{ "ReagentCount", 0, offsetof(SpellInfo, ReagentCount[0]) },
    //{ "EquippedItemClass", 0, offsetof(SpellInfo, EquippedItemClass) },
    //{ "EquippedItemSubClass", 0, offsetof(SpellInfo, EquippedItemSubClass) },
#if VERSION_STRING < Cata
    //{ "RequiredItemFlags", 0, offsetof(SpellInfo, RequiredItemFlags) },
#endif
    //{ "Effect", 0, offsetof(SpellInfo, Effect[0]) },
    //{ "EffectDieSides", 0, offsetof(SpellInfo, EffectDieSides[0]) },
    //{"EffectBaseDice", 0, offsetof(SpellEntry, EffectBaseDice[0])},
    //{"EffectDicePerLevel", 3, offsetof(SpellEntry, EffectDicePerLevel[0])},
    //{ "EffectRealPointsPerLevel", 3, offsetof(SpellInfo, EffectRealPointsPerLevel[0]) },
    //{ "EffectBasePoints", 0, offsetof(SpellInfo, EffectBasePoints[0]) },
    //{ "EffectMechanic", 0, offsetof(SpellInfo, EffectMechanic[0]) },
    //{ "EffectImplicitTargetA", 0, offsetof(SpellInfo, EffectImplicitTargetA[0]) },
    //{ "EffectImplicitTargetB", 0, offsetof(SpellInfo, EffectImplicitTargetB[0]) },
    //{ "EffectRadiusIndex", 0, offsetof(SpellInfo, EffectRadiusIndex[0]) },
    //{ "EffectApplyAuraName", 0, offsetof(SpellInfo, EffectApplyAuraName[0]) },
    //{ "EffectAmplitude", 0, offsetof(SpellInfo, EffectAmplitude[0]) },
    //{ "EffectMultipleValue", 3, offsetof(SpellInfo, EffectMultipleValue[0]) },
    //{ "EffectChainTarget", 0, offsetof(SpellInfo, EffectChainTarget[0]) },
    //{ "EffectItemType", 0, offsetof(SpellInfo, EffectItemType[0]) },
    //{ "EffectMiscValue", 0, offsetof(SpellInfo, EffectMiscValue[0]) },
    //{ "EffectMiscValueB", 0, offsetof(SpellInfo, EffectMiscValueB[0]) },
    //{ "EffectTriggerSpell", 0, offsetof(SpellInfo, EffectTriggerSpell[0]) },
    //{ "EffectPointsPerComboPoint", 3, offsetof(SpellInfo, EffectPointsPerComboPoint[0]) },
#if VERSION_STRING < Cata
    //{ "EffectSpellClassMask", 0, offsetof(SpellInfo, EffectSpellClassMask[0][0]) },
#endif
    //{ "SpellVisual", 0, offsetof(SpellInfo, SpellVisual) },
    //{ "field114", 0, offsetof(SpellInfo, field114) },
    //{ "spellIconID", 0, offsetof(SpellInfo, spellIconID) },
    //{ "activeIconID", 0, offsetof(SpellInfo, activeIconID) },
#if VERSION_STRING < Cata
    //{ "spellPriority", 0, offsetof(SpellInfo, spellPriority) },
#endif
    //{ "Name", 1, offsetof(SpellInfo, Name) },
    //{ "Rank", 1, offsetof(SpellInfo, Rank) },
    //{ "Description", 1, offsetof(SpellInfo, Description) },
    //{ "BuffDescription", 1, offsetof(SpellInfo, BuffDescription) },
    //{ "ManaCostPercentage", 0, offsetof(SpellInfo, ManaCostPercentage) },
    //{ "StartRecoveryCategory", 0, offsetof(SpellInfo, StartRecoveryCategory) },
    //{ "StartRecoveryTime", 0, offsetof(SpellInfo, StartRecoveryTime) },
    //{ "MaxTargetLevel", 0, offsetof(SpellInfo, MaxTargetLevel) },
    //{ "SpellFamilyName", 0, offsetof(SpellInfo, SpellFamilyName) },
    //{ "SpellGroupType", 0, offsetof(SpellInfo, SpellGroupType[0]) },
    //{ "MaxTargets", 0, offsetof(SpellInfo, MaxTargets) },
    //{ "Spell_Dmg_Type", 0, offsetof(SpellInfo, Spell_Dmg_Type) },
    //{ "PreventionType", 0, offsetof(SpellInfo, PreventionType) },
#if VERSION_STRING < Cata
    //{ "StanceBarOrder", 0, offsetof(SpellInfo, StanceBarOrder) },
#endif
    //{ "dmg_multiplier", 3, offsetof(SpellInfo, dmg_multiplier[0]) },
#if VERSION_STRING < Cata
    //{ "MinFactionID", 0, offsetof(SpellInfo, MinFactionID) },
    //{ "MinReputation", 0, offsetof(SpellInfo, MinReputation) },
    //{ "RequiredAuraVision", 0, offsetof(SpellInfo, RequiredAuraVision) },
#endif
    //{ "TotemCategory", 0, offsetof(SpellInfo, TotemCategory[0]) },
    //{ "RequiresAreaId", 0, offsetof(SpellInfo, RequiresAreaId) },
    //{ "School", 0, offsetof(SpellInfo, School) },
    //{ "RuneCostID", 0, offsetof(SpellInfo, RuneCostID) },
    { "proc_interval", 0, offsetof(SpellInfo, custom_proc_interval) },
    { "BGR_one_buff_on_target", 0, offsetof(SpellInfo, custom_BGR_one_buff_on_target) },
    { "c_is_flags", 0, offsetof(SpellInfo, custom_c_is_flags) },
    { "RankNumber", 0, offsetof(SpellInfo, custom_RankNumber) },
    { "NameHash", 0, offsetof(SpellInfo, custom_NameHash) },
    { "ThreatForSpell", 0, offsetof(SpellInfo, custom_ThreatForSpell) },
    { "ThreatForSpellCoef", 3, offsetof(SpellInfo, custom_ThreatForSpellCoef) },
    { "base_range_or_radius_sqr", 3, offsetof(SpellInfo, custom_base_range_or_radius_sqr) },
    { "cone_width", 3, offsetof(SpellInfo, cone_width) },
    { "spell_coeff_direct", 3, offsetof(SpellInfo, spell_coeff_direct) },
    { "spell_coeff_overtime", 3, offsetof(SpellInfo, spell_coeff_overtime) },
    { "ai_target_type", 0, offsetof(SpellInfo, ai_target_type) },
    { "self_cast_only", 2, offsetof(SpellInfo, custom_self_cast_only) },
    { "apply_on_shapeshift_change", 2, offsetof(SpellInfo, custom_apply_on_shapeshift_change) },
    { "is_melee_spell", 2, offsetof(SpellInfo, custom_is_melee_spell) },
    { "is_ranged_spell", 2, offsetof(SpellInfo, custom_is_ranged_spell) },
    { "SchoolMask", 0, offsetof(SpellInfo, custom_SchoolMask) },
    { NULL, 0, 0 },
};

LuaSpellEntry GetLuaSpellEntryByName(const char* name)
{
    for (uint32 itr = 0; luaSpellVars[itr].name != NULL; itr++)
    {
        LuaSpellEntry l = luaSpellVars[itr];
        if (strcmp(l.name, name) == 0)  //they entered a correct var name
            return l;
    }
    int lElem = sizeof(luaSpellVars) / sizeof(luaSpellVars[0]) - 1;
    return luaSpellVars[lElem];
}

namespace LuaSpell
{
    int GetCaster(lua_State* L, Spell* sp)
    {
        if (!sp)
            return 0;

        if (sp->u_caster)  //unit caster
        {
            PUSH_UNIT(L, sp->u_caster);
            return 1;
        }
        else if (sp->g_caster)  //gameobject
        {
            PUSH_GO(L, sp->g_caster);
            return 1;
        }
        else if (sp->i_caster)  //item
        {
            PUSH_ITEM(L, sp->i_caster);
            return 1;
        }
        else
        {
            lua_pushnil(L);
            return 1;
        }
    }

    int GetEntry(lua_State* L, Spell* sp)
    {
        if (!sp)
            return 0;
        lua_pushinteger(L, sp->getSpellInfo()->getId());
        return 1;
    }

    int IsDuelSpell(lua_State* L, Spell* sp)
    {
        if (!sp)
            return 0;
        lua_pushboolean(L, sp->duelSpell ? 1 : 0);
        return 1;
    }

    int GetSpellType(lua_State* L, Spell* sp)
    {
        if (!sp)
            return 0;
        lua_pushinteger(L, sp->GetType());
        return 1;
    }

    int GetSpellState(lua_State* L, Spell* sp)
    {
        /*
        SPELL_STATE_NULL      = 0,
        SPELL_STATE_PREPARING = 1,
        SPELL_STATE_CASTING   = 2,
        SPELL_STATE_FINISHED  = 3,
        SPELL_STATE_IDLE      = 4
        */
        if (!sp)
            return 0;
        lua_pushinteger(L, sp->getState());
        return 1;
    }

    int Cancel(lua_State* /*L*/, Spell* sp)
    {
        if (!sp || !sp->m_caster->IsInWorld())
            return 0;
        sp->m_caster->interruptSpell(sp->getSpellInfo()->getId());
        return 0;
    }

    int Cast(lua_State* L, Spell* sp)
    {
        if (!sp)
            return 0;
        bool check = CHECK_BOOL(L, 1);
        sp->castMe(check);
        return 0;
    }

    int CanCast(lua_State* L, Spell* sp)
    {
        if (!sp)
            return 0;
        lua_pushinteger(L, sp->canCast(false, 0, 0));
        return 1;
    }

    int Finish(lua_State* /*L*/, Spell* sp)
    {
        if (!sp)
            return 0;
        sp->finish();
        return 0;
    }

    int GetTarget(lua_State* L, Spell* sp)
    {
        if (!sp || !sp->m_caster->IsInWorld())
            RET_NIL()

            if (sp->m_targets.m_unitTarget)
            {
                PUSH_UNIT(L, sp->m_caster->GetMapMgr()->GetUnit(sp->m_targets.m_unitTarget));
                return 1;
            }
            else if (sp->m_targets.m_itemTarget)
            {
                if (!sp->p_caster)
                    RET_NIL()
                    PUSH_ITEM(L, sp->p_caster->getItemInterface()->GetItemByGUID(sp->m_targets.m_itemTarget));
                return 1;
            }
            else
                RET_NIL()
    }

    int IsStealthSpell(lua_State* L, Spell* sp)
    {
        if (!sp) return 0;
        lua_pushboolean(L, sp->IsStealthSpell() ? 1 : 0);
        return 1;
    }

    int IsInvisibilitySpell(lua_State* L, Spell* sp)
    {
        if (!sp) return 0;
        lua_pushboolean(L, sp->IsInvisibilitySpell() ? 1 : 0);
        return 1;
    }

    int GetPossibleEnemy(lua_State* L, Spell* sp)
    {
        float range = (float)luaL_optnumber(L, 1, 0.0f);
        if (!sp || range < 0) return 0;
        PUSH_GUID(L, sp->GetSinglePossibleEnemy(0, range));
        return 1;
    }

    int GetPossibleFriend(lua_State* L, Spell* sp)
    {
        float range = (float)luaL_optnumber(L, 1, 0.0f);
        if (!sp || range < 0) return 0;
        PUSH_GUID(L, sp->GetSinglePossibleFriend(0, range));
        return 1;
    }

    int HasPower(lua_State* L, Spell* sp)
    {
        if (!sp) return 0;
        lua_pushboolean(L, sp->HasPower() ? 1 : 0);
        return 1;
    }

    int IsAspect(lua_State* L, Spell* sp)
    {
        if (!sp) return 0;
        lua_pushboolean(L, sp->IsAspect() ? 1 : 0);
        return 1;
    }

    int IsSeal(lua_State* L, Spell* sp)
    {
        if (!sp) return 0;
        lua_pushboolean(L, sp->IsSeal() ? 1 : 0);
        return 1;
    }

    int GetObjectType(lua_State* L, Spell* sp)
    {
        if (!sp)
        {
            lua_pushnil(L);
            return 1;
        }
        lua_pushstring(L, "Spell");
        return 1;
    }

    int SetVar(lua_State* L, Spell* sp)
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

    int GetVar(lua_State* L, Spell* sp)
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
            RET_NIL();
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

    int ResetVar(lua_State* L, Spell* sp)
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
                GET_SPELLVAR_INT(sp->getSpellInfo(), l.offset, subindex) = GET_SPELLVAR_INT(sp->m_spellInfo, l.offset, subindex);
                lua_pushboolean(L, 1);
                break;
            case 1:
                GET_SPELLVAR_CHAR(sp->getSpellInfo(), l.offset, subindex) = GET_SPELLVAR_CHAR(sp->m_spellInfo, l.offset, subindex);
                lua_pushboolean(L, 1);
                break;
            case 2:
                GET_SPELLVAR_BOOL(sp->getSpellInfo(), l.offset, subindex) = GET_SPELLVAR_BOOL(sp->m_spellInfo, l.offset, subindex);
                lua_pushboolean(L, 1);
                break;
            case 3:
                GET_SPELLVAR_FLOAT(sp->getSpellInfo(), l.offset, subindex) = GET_SPELLVAR_FLOAT(sp->m_spellInfo, l.offset, subindex);
                lua_pushboolean(L, 1);
                break;
        }
        return 1;
    }

    int ResetAllVars(lua_State* /*L*/, Spell* sp)
    {
        if (!sp)
            return 0;
        sp->m_spellInfo_override = nullptr;
        return 0;
    }

    int GetCastedItemId(lua_State* L, Spell* sp)
    {
        if (!sp)
        {
            lua_pushnil(L);
            return 1;
        }
        lua_pushnumber(L, sp->castedItemId);
        return 1;
    }
}
