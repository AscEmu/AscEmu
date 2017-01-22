/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "StdAfx.h"
#include "UnitDefines.hpp"

#include "Management/LootMgr.h"
#include "Spell/SpellProc.h"
#include "CombatStatus.h"

class SpellProc;
class AIInterface;
class DynamicObject;
class Object;

#include "Units/Summons/SummonHandler.h"
#include "Movement/UnitMovementManager.hpp"

class Vehicle;

//these refer to visibility ranges. We need to store each stack of the aura and not just visible count.
#define MAX_POSITIVE_VISUAL_AURAS_START 0
#define MAX_POSITIVE_VISUAL_AURAS_END 40

#define MAX_NEGATIVE_VISUAL_AURAS_START MAX_POSITIVE_VISUAL_AURAS_END       // 40 buff slots, 16 debuff slots.
#define MAX_NEGATIVE_VISUAL_AURAS_END (MAX_POSITIVE_VISUAL_AURAS_END + 16)  // 40 buff slots, 16 debuff slots.

//you hardly get to this but since i was testing i got to it :) : 20 items * 11 (enchants) + 61 talents
#define MAX_PASSIVE_AURAS_START 0                                                   // these are reserved for talents. No need to check them for removes ?
#define MAX_PASSIVE_AURAS_END (MAX_PASSIVE_AURAS_START + 140)                       // these are reserved for talents. No need to check them for removes ?

#define MAX_POSITIVE_AURAS_EXTEDED_START MAX_PASSIVE_AURAS_END                      //these are not talents.These are stacks from visible auras
#define MAX_POSITIVE_AURAS_EXTEDED_END (MAX_POSITIVE_AURAS_EXTEDED_START + 100)     //these are not talents.These are stacks from visible auras

#define MAX_NEGATIVE_AURAS_EXTEDED_START MAX_POSITIVE_AURAS_EXTEDED_END             //these are not talents.These are stacks from visible auras
#define MAX_NEGATIVE_AURAS_EXTEDED_END (MAX_NEGATIVE_AURAS_EXTEDED_START + 100)     //these are not talents.These are stacks from visible auras

#define MAX_REMOVABLE_AURAS_START (MAX_POSITIVE_AURAS_EXTEDED_START)        //do we need to handle talents at all ?
#define MAX_REMOVABLE_AURAS_END (MAX_NEGATIVE_AURAS_EXTEDED_END)            //do we need to handle talents at all ?

#define MAX_TOTAL_AURAS_START (MAX_PASSIVE_AURAS_START)
#define MAX_TOTAL_AURAS_END (MAX_REMOVABLE_AURAS_END)

bool SERVER_DECL Rand(float);

#define UF_TARGET_DIED  1
#define UF_ATTACKING    2           // this unit is attacking it's selection
#define SPELL_GROUPS    96          // This is actually on 64 bits !
#define DIMINISHING_GROUP_COUNT 15

#define UNIT_TYPE_HUMANOID_BIT (1 << (UNIT_TYPE_HUMANOID-1))    //should get computed by precompiler ;)

class Aura;
class Spell;
class AIInterface;
class GameObject;
class Pet;
class Group;

struct FactionDBC;

typedef std::unordered_map<uint32, uint64> UniqueAuraTargetMap;

//////////////////////////////////////////////////////////////////////////////////////////
/// Checks for conditions specified in subclasses on Auras. When calling operator()
/// it tells if the conditions are met.
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL AuraCondition
{
    public:

        virtual bool operator()(Aura* aura)
        {
            return true;
        }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Performs the actions specified in subclasses on the Aura, when calling operator().
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL AuraAction
{
    public:

        virtual void operator()(Aura* aura){}
};


#pragma pack(push, 1)
struct DisplayBounding
{
    uint32 displayid;
    float low[3];
    float high[3];
    float boundradius;
};
#pragma pack(pop)

struct ReflectSpellSchool
{
    uint32 spellId;
    uint32 charges;
    int32 school;
    int32 chance;
    int32 require_aura_hash;
    bool infront;
};

typedef struct
{
    uint32 spellid;
    uint32 mindmg;
    uint32 maxdmg;
} OnHitSpell;

struct AreaAura
{
    uint32 auraid;
    Unit* caster;
};

typedef struct
{
    SpellInfo* spell_info;
    uint32 charges;
} ExtraStrike;

struct AuraCheckResponse
{
    uint32 Error;
    uint32 Misc;
};

#define UNIT_SUMMON_SLOTS 6

typedef std::list<struct ProcTriggerSpellOnSpell> ProcTriggerSpellOnSpellList;
