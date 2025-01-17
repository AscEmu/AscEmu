/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Spell/SpellInfo.hpp"

extern "C"
{
#include <lua/lua.h>
}

#include <cstdint>
#include <cstring>

#define GET_SPELLVAR_INT(proto,offset,subindex) *(int*)((char*)(proto) + (offset) + (subindex))
#define GET_SPELLVAR_CHAR(proto,offset,subindex) *(char**)((char*)(proto) + (offset) + (subindex))
#define GET_SPELLVAR_BOOL(proto,offset,subindex) *(bool*)((char*)(proto) + (offset) + (subindex))
#define GET_SPELLVAR_FLOAT(proto,offset,subindex) *(float*)((char*)(proto) + (offset) + (subindex))

struct LuaSpellEntry
{
    const char* name;
    uint32_t typeId; //0: int, 1: char*, 2: bool, 3: float
    size_t offset;
};

inline LuaSpellEntry luaSpellVars[] =
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
    //{ "modalNextSpell", 0, offsetof(SpellInfo, modalNextSpell) },
    //{ "maxstack", 0, offsetof(SpellInfo, maxstack) },
    //{ "Totem", 0, offsetof(SpellInfo, Totem[0]) },
    //{ "Reagent", 0, offsetof(SpellInfo, Reagent[0]) },
    //{ "ReagentCount", 0, offsetof(SpellInfo, ReagentCount[0]) },
    //{ "EquippedItemClass", 0, offsetof(SpellInfo, EquippedItemClass) },
    //{ "EquippedItemSubClass", 0, offsetof(SpellInfo, EquippedItemSubClass) },
    //{ "RequiredItemFlags", 0, offsetof(SpellInfo, RequiredItemFlags) },
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
    //{ "EffectSpellClassMask", 0, offsetof(SpellInfo, EffectSpellClassMask[0][0]) },
    //{ "SpellVisual", 0, offsetof(SpellInfo, SpellVisual) },
    //{ "field114", 0, offsetof(SpellInfo, field114) },
    //{ "spellIconID", 0, offsetof(SpellInfo, spellIconID) },
    //{ "activeIconID", 0, offsetof(SpellInfo, activeIconID) },
    //{ "spellPriority", 0, offsetof(SpellInfo, spellPriority) },
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
    //{ "StanceBarOrder", 0, offsetof(SpellInfo, StanceBarOrder) },
    //{ "dmg_multiplier", 3, offsetof(SpellInfo, dmg_multiplier[0]) },
    //{ "MinFactionID", 0, offsetof(SpellInfo, MinFactionID) },
    //{ "MinReputation", 0, offsetof(SpellInfo, MinReputation) },
    //{ "RequiredAuraVision", 0, offsetof(SpellInfo, RequiredAuraVision) },
    //{ "TotemCategory", 0, offsetof(SpellInfo, TotemCategory[0]) },
    //{ "RequiresAreaId", 0, offsetof(SpellInfo, RequiresAreaId) },
    //{ "School", 0, offsetof(SpellInfo, School) },
    //{ "RuneCostID", 0, offsetof(SpellInfo, RuneCostID) },
    //{ "proc_interval", 0, offsetof(SpellInfo, custom_proc_interval) },
    { "BGR_one_buff_on_target", 0, offsetof(SpellInfo, custom_BGR_one_buff_on_target) },
    { "c_is_flags", 0, offsetof(SpellInfo, custom_c_is_flags) },
    //{ "RankNumber", 0, offsetof(SpellInfo, custom_RankNumber) },
    //{ "NameHash", 0, offsetof(SpellInfo, custom_NameHash) },
    { "ThreatForSpell", 0, offsetof(SpellInfo, custom_ThreatForSpell) },
    { "ThreatForSpellCoef", 3, offsetof(SpellInfo, custom_ThreatForSpellCoef) },
    { "base_range_or_radius_sqr", 3, offsetof(SpellInfo, custom_base_range_or_radius_sqr) },
    { "cone_width", 3, offsetof(SpellInfo, cone_width) },
    { "spell_coeff_direct", 3, offsetof(SpellInfo, spell_coeff_direct) },
    { "spell_coeff_overtime", 3, offsetof(SpellInfo, spell_coeff_overtime) },
    { "ai_target_type", 0, offsetof(SpellInfo, ai_target_type) },
    { "self_cast_only", 2, offsetof(SpellInfo, custom_self_cast_only) },
    //{ "apply_on_shapeshift_change", 2, offsetof(SpellInfo, custom_apply_on_shapeshift_change) },
    { NULL, 0, 0 },
};

inline LuaSpellEntry GetLuaSpellEntryByName(const char* name)
{
    for (uint32_t itr = 0; luaSpellVars[itr].name != NULL; itr++)
    {
        LuaSpellEntry l = luaSpellVars[itr];
        if (strcmp(l.name, name) == 0)  //they entered a correct var name
            return l;
    }
    int lElem = sizeof(luaSpellVars) / sizeof(luaSpellVars[0]) - 1;
    return luaSpellVars[lElem];
}

class LuaSpell
{
public:
    static int GetCaster(lua_State* L, Spell* sp);
    static int GetEntry(lua_State* L, Spell* sp);
    static int IsDuelSpell(lua_State* L, Spell* sp);
    static int GetSpellType(lua_State* L, Spell* sp);
    static int GetSpellState(lua_State* L, Spell* sp);
    static int Cancel(lua_State* /*L*/, Spell* sp);
    static int Cast(lua_State* L, Spell* sp);
    static int CanCast(lua_State* L, Spell* sp);
    static int Finish(lua_State* /*L*/, Spell* sp);
    static int GetTarget(lua_State* L, Spell* sp);
    static int IsStealthSpell(lua_State* L, Spell* sp);
    static int IsInvisibilitySpell(lua_State* L, Spell* sp);
    static int GetPossibleEnemy(lua_State* L, Spell* sp);
    static int GetPossibleFriend(lua_State* L, Spell* sp);
    static int HasPower(lua_State* L, Spell* sp);
    static int IsAspect(lua_State* L, Spell* sp);
    static int IsSeal(lua_State* L, Spell* sp);
    static int GetObjectType(lua_State* L, Spell* sp);
    static int SetVar(lua_State* L, Spell* sp);
    static int GetVar(lua_State* L, Spell* sp);
    static int ResetVar(lua_State* L, Spell* sp);
    static int ResetAllVars(lua_State* /*L*/, Spell* sp);
    static int GetCastedItemId(lua_State* L, Spell* sp);
};
