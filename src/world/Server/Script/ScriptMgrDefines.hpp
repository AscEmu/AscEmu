/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

#include <cstdint>
#include <list>
#include <memory>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

class AchievementCriteriaScript;
class SpellScript;
class Creature;
class GameObjectAIScript;
class InstanceScript;
class GameObject;
class WorldMap;
class Spell;
class Aura;
class ScriptMgr;
class ServerState;
class GossipScript;
class EventScript;
class QuestScript;

namespace Arcemu
{
    class DynLib;
}

class CreatureAIScript;
class Unit;

enum ScriptTypes
{
    SCRIPT_TYPE_MISC            = 0x01,
    SCRIPT_TYPE_SCRIPT_ENGINE   = 0x20
};

#ifdef FT_ACHIEVEMENTS
typedef std::unordered_map<uint32_t, AchievementCriteriaScript*> AchievementCriteriaScripts;
#endif
typedef std::unordered_map<uint32_t, SpellScript*> SpellScripts;

// Factory Imports (from script lib)
typedef CreatureAIScript* (*exp_create_creature_ai)(Creature* pCreature);
typedef GameObjectAIScript* (*exp_create_gameobject_ai)(GameObject* pGameObject);
typedef InstanceScript* (*exp_create_instance_ai)(WorldMap* pMapMgr);

typedef bool(*exp_handle_dummy_spell)(uint8_t effectIndex, Spell* pSpell);
typedef bool(*exp_handle_script_effect)(uint8_t effectIndex, Spell* pSpell);
typedef bool(*exp_handle_dummy_aura)(uint8_t effectIndex, Aura* pAura, bool apply);

typedef void(*exp_script_register)(ScriptMgr* mgr);
typedef void(*exp_engine_reload)();
typedef void(*exp_engine_unload)();
typedef uint32_t(*exp_get_script_type)();
typedef const char*(*exp_get_version)();
typedef void(*exp_set_serverstate_singleton)(ServerState* state);

// Hashmap typedefs
typedef std::unordered_map<uint32_t, exp_create_creature_ai> CreatureCreateMap;
typedef std::unordered_map<uint32_t, exp_create_gameobject_ai> GameObjectCreateMap;
typedef std::unordered_map<uint32_t, exp_handle_dummy_aura> HandleDummyAuraMap;
typedef std::unordered_map<uint32_t, exp_handle_dummy_spell> HandleDummySpellMap;
typedef std::unordered_map< uint32_t, exp_handle_script_effect > HandleScriptEffectMap;
typedef std::unordered_map<uint32_t, exp_create_instance_ai> InstanceCreateMap;
typedef std::set<GossipScript*> CustomGossipScripts;
typedef std::unordered_map<uint32_t, GossipScript*> GossipMap;
typedef std::set<EventScript*> EventScripts;
typedef std::set<QuestScript*> QuestScripts;

typedef std::list<std::unique_ptr<Arcemu::DynLib>> DynamicLibraryMap;

enum RangeStatus
{
    RangeStatus_Ok,
    RangeStatus_TooFar,
    RangeStatus_TooClose
};

typedef void(*EventFunc)(CreatureAIScript* pCreatureAI, int32_t pMiscVal);

typedef std::vector<Unit*> UnitArray;
typedef std::pair<RangeStatus, float> RangeStatusPair;

//////////////////////////////////////////////////////////////////////////////////////////
// Instanced class created for each instance of the map, holds all scriptable exports

enum EncounterFrameType
{
#if VERSION_STRING > WotLK
    EncounterFrameSetCombatResLimit,
    EncounterFrameResetCombatResLimit,
#endif
    EncounterFrameEngage,
    EncounterFrameDisengaged,
    EncounterFrameUpdatePriority,
    EncounterFrameAddTimer,
    EncounterFrameEnableObjective,
    EncounterFrameUpdateObjective,
    EncounterFrameDisableObjective,
    EncounterFrameUnknown,
#if VERSION_STRING > WotLK
    EncounterFrameAddCombatResLimit,
#endif
};

enum EncounterStates : uint8_t
{
    NotStarted          = 0,
    InProgress          = 1,
    Failed              = 2,
    Performed           = 3,
    PreProgress         = 4,
    InvalidState        = 0xff
};

// Maybe Save more in future
struct BossInfo
{
    BossInfo() : state(InvalidState) {}
    uint32_t entry = 0;
    EncounterStates state;
};

struct ObjectData
{
    uint32_t entry;
    uint32_t type;
};

const int32_t INVALIDATE_TIMER = -1;
const uint32_t DEFAULT_DESPAWN_TIMER = 2000; // milliseconds
