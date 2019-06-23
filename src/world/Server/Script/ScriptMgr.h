/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
*
*/

#pragma once

#include <mutex>
#include "Management/Gossip/Gossip.h"
#include "Management/GameEventMgr.h"
#include "Units/Unit.h"
#include "Management/ArenaTeam.h"
#include "Server/ServerState.h"

#define ADD_CREATURE_FACTORY_FUNCTION(cl) public:\
static CreatureAIScript* Create(Creature* c) { return new cl(c); }

//#define ADD_INSTANCE_FACTORY_FUNCTION(ClassName) static InstanceScript* Create(MapMgr* pMapMgr) { return new ClassName(pMapMgr); };

class Channel;
class Guild;
struct QuestProperties;

enum ServerHookEvents
{
    //////////////////////////////////////////////////////////////////////////////////////////   
    //Register Server Hooks
    // Server Hook callbacks can be made by using the function RegisterServerHook(EventId, function)

    SERVER_HOOK_EVENT_ON_NEW_CHARACTER                   = 1,  // -- (event, pName, int Race, int Class)
    SERVER_HOOK_EVENT_ON_KILL_PLAYER                     = 2,  // -- (event, pKiller, pVictim)
    SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD               = 3,  // -- (event, pPlayer)                 / a new created character enters for first time the world
    SERVER_HOOK_EVENT_ON_ENTER_WORLD                     = 4,  // -- (event, pPlayer)                 / a character enters the world (login) or moves to another map
    SERVER_HOOK_EVENT_ON_GUILD_JOIN                      = 5,  // -- (event, pPlayer, str GuildName)
    SERVER_HOOK_EVENT_ON_DEATH                           = 6,  // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_REPOP                           = 7,  // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_EMOTE                           = 8,  // -- (event, pPlayer, pUnit, EmoteId)
    SERVER_HOOK_EVENT_ON_ENTER_COMBAT                    = 9,  // -- (event, pPlayer, pTarget)
    SERVER_HOOK_EVENT_ON_CAST_SPELL                      = 10, // -- (event, pPlayer, SpellId, pSpellObject)
    SERVER_HOOK_EVENT_ON_TICK                            = 11, // -- No arguments passed.
    SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST                  = 12, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_LOGOUT                          = 13, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_QUEST_ACCEPT                    = 14, // -- (event, pPlayer, QuestId, pQuestGiver)
    SERVER_HOOK_EVENT_ON_ZONE                            = 15, // -- (event, pPlayer, ZoneId, OldZoneId)
    SERVER_HOOK_EVENT_ON_CHAT                            = 16, // -- (event, pPlayer, str Message, Type, Language, Misc)
    SERVER_HOOK_EVENT_ON_LOOT                            = 17, // -- (event, pPlayer, pTarget, Money, ItemId)
    SERVER_HOOK_EVENT_ON_GUILD_CREATE                    = 18, // -- (event, pPlayer, pGuildName)
    SERVER_HOOK_EVENT_ON_FULL_LOGIN                      = 19, // -- (event, pPlayer)                 / a character enters the world (login)
    SERVER_HOOK_EVENT_ON_CHARACTER_CREATE                = 20, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_QUEST_CANCELLED                 = 21, // -- (event, pPlayer, QuestId)
    SERVER_HOOK_EVENT_ON_QUEST_FINISHED                  = 22, // -- (event, pPlayer, QuestId, pQuestGiver)
    SERVER_HOOK_EVENT_ON_HONORABLE_KILL                  = 23, // -- (event, pPlayer, pKilled)
    SERVER_HOOK_EVENT_ON_ARENA_FINISH                    = 24, // -- (event, pPlayer, str TeamName, bWinner, bRated)
    SERVER_HOOK_EVENT_ON_OBJECTLOOT                      = 25, // -- (event, pPlayer, pTarget, Money, ItemId)
    SERVER_HOOK_EVENT_ON_AREATRIGGER                     = 26, // -- (event, pPlayer, AreaTriggerId)
    SERVER_HOOK_EVENT_ON_POST_LEVELUP                    = 27, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_PRE_DIE                         = 28, // -- (event, pKiller, pDied)          / general unit die, not only based on players
    SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE               = 29, // -- (event, pPlayer, SkillId, SkillLevel)
    SERVER_HOOK_EVENT_ON_DUEL_FINISHED                   = 30, // -- (event, pWinner, pLoser)
    SERVER_HOOK_EVENT_ON_AURA_REMOVE                     = 31, // -- (event, pAuraObject)
    SERVER_HOOK_EVENT_ON_RESURRECT                       = 32, // -- (event, pPlayer)
    NUM_SERVER_HOOKS
};

enum ScriptTypes
{
    SCRIPT_TYPE_MISC            = 0x01,
    SCRIPT_TYPE_SCRIPT_ENGINE   = 0x20
};

// Hook typedefs
typedef bool(*tOnNewCharacter)(uint32 Race, uint32 Class, WorldSession* Session, const char* Name);
typedef void(*tOnKillPlayer)(Player* pPlayer, Player* pVictim);
typedef void(*tOCharacterCreate)(Player* pPlayer);
typedef void(*tOnFirstEnterWorld)(Player* pPlayer);
typedef void(*tOnEnterWorld)(Player* pPlayer);
typedef void(*tOnGuildCreate)(Player* pLeader, Guild* pGuild);
typedef void(*tOnGuildJoin)(Player* pPlayer, Guild* pGuild);
typedef void(*tOnDeath)(Player* pPlayer);
typedef bool(*tOnRepop)(Player* pPlayer);
typedef void(*tOnEmote)(Player* pPlayer, uint32 Emote, Unit* pUnit);
typedef void(*tOnEnterCombat)(Player* pPlayer, Unit* pTarget);
typedef bool(*tOnCastSpell)(Player* pPlayer, SpellInfo const* pSpell, Spell* spell);
typedef void(*tOnTick)();
typedef bool(*tOnLogoutRequest)(Player* pPlayer);
typedef void(*tOnLogout)(Player* pPlayer);
typedef void(*tOnQuestAccept)(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
typedef void(*tOnZone)(Player* pPlayer, uint32 Zone, uint32 oldzone);
typedef bool(*tOnChat)(Player* pPlayer, uint32 Type, uint32 Lang, const char* Message, const char* Misc);
typedef void(*tOnLoot)(Player* pPlayer, Unit* pTarget, uint32 Money, uint32 ItemId);
typedef bool(*ItemScript)(Item* pItem, Player* pPlayer);
typedef void(*tOnQuestCancel)(Player* pPlayer, QuestProperties const* pQuest);
typedef void(*tOnQuestFinished)(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
typedef void(*tOnHonorableKill)(Player* pPlayer, Player* pKilled);
typedef void(*tOnArenaFinish)(Player* pPlayer, ArenaTeam* pTeam, bool victory, bool rated);
typedef void(*tOnObjectLoot)(Player* pPlayer, Object* pTarget, uint32 Money, uint32 ItemId);
typedef void(*tOnAreaTrigger)(Player* pPlayer, uint32 areaTrigger);
typedef void(*tOnPostLevelUp)(Player* pPlayer);
typedef bool(*tOnPreUnitDie)(Unit* killer, Unit* target);
typedef void(*tOnAdvanceSkillLine)(Player* pPlayer, uint32 SkillLine, uint32 Current);
typedef void(*tOnDuelFinished)(Player* Winner, Player* Looser);
typedef void(*tOnAuraRemove)(Aura* aura);
typedef bool(*tOnResurrect)(Player* pPlayer);

class Spell;
class Aura;
class Creature;
class CreatureAIScript;
class EventScript;
class GameObjectAIScript;
class InstanceScript;
class ScriptMgr;
struct ItemProperties;
class QuestLogEntry;
class QuestScript;

// Factory Imports (from script lib)
typedef CreatureAIScript* (*exp_create_creature_ai)(Creature* pCreature);
typedef GameObjectAIScript* (*exp_create_gameobject_ai)(GameObject* pGameObject);
typedef InstanceScript* (*exp_create_instance_ai)(MapMgr* pMapMgr);

typedef bool(*exp_handle_dummy_spell)(uint8_t effectIndex, Spell* pSpell);
typedef bool(*exp_handle_script_effect)(uint8_t effectIndex, Spell* pSpell);
typedef bool(*exp_handle_dummy_aura)(uint8_t effectIndex, Aura* pAura, bool apply);

typedef void(*exp_script_register)(ScriptMgr* mgr);
typedef void(*exp_engine_reload)();
typedef void(*exp_engine_unload)();
typedef uint32(*exp_get_script_type)();
typedef const char*(*exp_get_version)();
typedef void(*exp_set_serverstate_singleton)(ServerState* state);

// Hashmap typedefs
typedef std::unordered_map<uint32, exp_create_creature_ai> CreatureCreateMap;
typedef std::unordered_map<uint32, exp_create_gameobject_ai> GameObjectCreateMap;
typedef std::unordered_map<uint32, exp_handle_dummy_aura> HandleDummyAuraMap;
typedef std::unordered_map<uint32, exp_handle_dummy_spell> HandleDummySpellMap;
typedef std::unordered_map< uint32, exp_handle_script_effect > HandleScriptEffectMap;
typedef std::unordered_map<uint32, exp_create_instance_ai> InstanceCreateMap;
typedef std::set<Arcemu::Gossip::Script*> CustomGossipScripts;
typedef std::unordered_map<uint32, Arcemu::Gossip::Script*> GossipMap;
typedef std::set<EventScript*> EventScripts;
typedef std::set<QuestScript*> QuestScripts;
typedef std::set<void*> ServerHookList;
typedef std::list< Arcemu::DynLib* > DynamicLibraryMap;


class SERVER_DECL ScriptMgr : public Singleton<ScriptMgr>
{
    public:
        ScriptMgr();
        ~ScriptMgr();

        friend class HookInterface;

        void LoadScripts();
        void UnloadScripts();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Dumps the IDs of the spells with dummy/script effects or dummy aura
        /// that are not yet implemented.
        ///
        /// \param none   \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        void DumpUnimplementedSpells();

        CreatureAIScript* CreateAIScriptClassForEntry(Creature* pCreature);
        GameObjectAIScript* CreateAIScriptClassForGameObject(uint32 uEntryId, GameObject* pGameObject);
        InstanceScript* CreateScriptClassForInstance(uint32 pMapId, MapMgr* pMapMgr);

        bool CallScriptedDummySpell(uint32 uSpellId, uint8_t effectIndex, Spell* pSpell);
        bool HandleScriptedSpellEffect(uint32 SpellId, uint8_t effectIndex, Spell* s);
        bool CallScriptedDummyAura(uint32 uSpellId, uint8_t effectIndex, Aura* pAura, bool apply);
        bool CallScriptedItem(Item* pItem, Player* pPlayer);

        //Single Entry Registers
        void register_creature_script(uint32 entry, exp_create_creature_ai callback);
        void register_gameobject_script(uint32 entry, exp_create_gameobject_ai callback);
        void register_dummy_aura(uint32 entry, exp_handle_dummy_aura callback);
        void register_dummy_spell(uint32 entry, exp_handle_dummy_spell callback);
        void register_script_effect(uint32 entry, exp_handle_script_effect callback);
        void register_instance_script(uint32 pMapId, exp_create_instance_ai pCallback);
        void register_hook(ServerHookEvents event, void* function_pointer);
        void register_quest_script(uint32 entry, QuestScript* qs);
        void register_event_script(uint32 entry, EventScript* es);

        // GOSSIP INTERFACE REGISTRATION
        void register_creature_gossip(uint32, Arcemu::Gossip::Script*);
        void register_item_gossip(uint32, Arcemu::Gossip::Script*);
        void register_go_gossip(uint32, Arcemu::Gossip::Script*);

        // Mutliple Entry Registers
        void register_creature_script(uint32* entries, exp_create_creature_ai callback);
        void register_gameobject_script(uint32* entries, exp_create_gameobject_ai callback);
        void register_dummy_aura(uint32* entries, exp_handle_dummy_aura callback);
        void register_dummy_spell(uint32* entries, exp_handle_dummy_spell callback);
        void register_script_effect(uint32* entries, exp_handle_script_effect callback);

        void ReloadScriptEngines();
        void UnloadScriptEngines();

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified creature id.
        // Parameter: uint32 - the id of the creature to search for.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_creature_script(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified gameobject id.
        // Parameter: uint32 - the id of the gameobject to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_gameobject_script(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified aura id.
        // Parameter: uint32 - the aura id to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_dummy_aura_script(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified dummy spell id.
        // Parameter: uint32 - the spell id to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_dummy_spell_script(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified spell id.
        // Parameter: uint32 - the spell id to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_script_effect(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified map id.
        // Parameter: uint32 - the map id to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_instance_script(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has registered the specified function ptr to the specified event.
        // Parameter: ServerHookEvents - the event number
        // Parameter: void * - the function pointer to search for.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_hook(ServerHookEvents, void*) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified quest id.
        // Parameter: uint32 - the quest id to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_quest_script(uint32) const;

        bool has_creature_gossip(uint32) const;
        bool has_item_gossip(uint32) const;
        bool has_go_gossip(uint32) const;

        Arcemu::Gossip::Script* get_creature_gossip(uint32) const;
        Arcemu::Gossip::Script* get_go_gossip(uint32) const;
        Arcemu::Gossip::Script* get_item_gossip(uint32) const;

        // Default Gossip Script Interfaces
        Arcemu::Gossip::Trainer trainerScript_;
        Arcemu::Gossip::SpiritHealer spirithealerScript_;
        Arcemu::Gossip::Banker bankerScript_;
        Arcemu::Gossip::Vendor vendorScript_;
        Arcemu::Gossip::ClassTrainer classtrainerScript_;
        Arcemu::Gossip::PetTrainer pettrainerScript_;
        Arcemu::Gossip::FlightMaster flightmasterScript_;
        Arcemu::Gossip::Auctioneer auctioneerScript_;
        Arcemu::Gossip::InnKeeper innkeeperScript_;
        Arcemu::Gossip::BattleMaster battlemasterScript_;
        Arcemu::Gossip::CharterGiver chartergiverScript_;
        Arcemu::Gossip::TabardDesigner tabardScript_;
        Arcemu::Gossip::StableMaster stablemasterScript_;
        Arcemu::Gossip::Generic genericScript_;

    protected:

        InstanceCreateMap mInstances;
        CreatureCreateMap _creatures;
        Mutex m_creaturesMutex;
        GameObjectCreateMap _gameobjects;
        HandleDummyAuraMap _auras;
        HandleDummySpellMap _spells;
        HandleScriptEffectMap SpellScriptEffects;
        DynamicLibraryMap dynamiclibs;
        ServerHookList _hooks[NUM_SERVER_HOOKS];
        CustomGossipScripts _customgossipscripts;
        EventScripts _eventscripts;
        QuestScripts _questscripts;
        GossipMap creaturegossip_, gogossip_, itemgossip_;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Base.h stuff
struct LocationExtra
{
    float x;
    float y;
    float z;
    float o;
    uint32_t addition;
};

enum TargetGenerator
{
    // Self
    TargetGen_Self,                         // Target self (Note: doesn't always mean self, also means the spell can choose various target)

    // Current
    TargetGen_Current,                      // Current highest aggro (attacking target)
    TargetGen_Destination,                  // Target is a destination coordinates (X, Y, Z)

    // Second most hated
    TargetGen_SecondMostHated,              // Second highest aggro

    // Predefined target
    TargetGen_Predefined,                   // Pre-defined target unit

    // Random Unit
    TargetGen_RandomUnit,                   // Random target unit (players, totems, pets, etc.)
    TargetGen_RandomUnitDestination,        // Random destination coordinates (X, Y, Z)
    TargetGen_RandomUnitApplyAura,          // Random target unit to self cast aura

    // Random Player
    TargetGen_RandomPlayer,                 // Random target player
    TargetGen_RandomPlayerDestination,      // Random player destination coordinates (X, Y, Z)
    TargetGen_RandomPlayerApplyAura         // Random target player to self cast aura
};

enum TargetFilter
{
    // Standard filters
    TargetFilter_None                   = 0,            // 0
    TargetFilter_Closest                = 1 << 0,       // 1
    TargetFilter_Friendly               = 1 << 1,       // 2
    TargetFilter_NotCurrent             = 1 << 2,       // 4
    TargetFilter_Wounded                = 1 << 3,       // 8
    TargetFilter_SecondMostHated        = 1 << 4,       // 16
    TargetFilter_Aggroed                = 1 << 5,       // 32
    TargetFilter_Corpse                 = 1 << 6,       // 64
    TargetFilter_InMeleeRange           = 1 << 7,       // 128
    TargetFilter_InRangeOnly            = 1 << 8,       // 256
    TargetFilter_IgnoreSpecialStates    = 1 << 9,       // 512 - not really a TargetFilter, more like requirement for spell
    TargetFilter_IgnoreLineOfSight      = 1 << 10,      // 1024

    // Predefined filters
    TargetFilter_ClosestFriendly        = TargetFilter_Closest | TargetFilter_Friendly,         // 3
    TargetFilter_ClosestNotCurrent      = TargetFilter_Closest | TargetFilter_NotCurrent,       // 5
    TargetFilter_WoundedFriendly        = TargetFilter_Wounded | TargetFilter_Friendly,         // 10
    TargetFilter_FriendlyCorpse         = TargetFilter_Corpse | TargetFilter_Friendly,          // 66
    TargetFilter_ClosestFriendlyCorpse  = TargetFilter_Closest | TargetFilter_FriendlyCorpse    // 67
};

class TargetType;
class CreatureAIScript;
class Unit;

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
//Class TargetType
class SERVER_DECL TargetType
{
public:
    TargetType(uint32_t pTargetGen = TargetGen_Self, TargetFilter pTargetFilter = TargetFilter_None, uint32_t pMinTargetNumber = 0, uint32_t pMaxTargetNumber = 0);
    ~TargetType();

    uint32_t mTargetGenerator;      // Defines what kind of target should we try to find
    TargetFilter mTargetFilter;     // Defines filter of target
    uint32_t mTargetNumber[2];      // 0: Defines min. number of creature on hatelist (0 - any, 1 - the most hated etc.)
                                    // 1: Defines max. number of creature on hatelist (0 - any, HateList.size + 1 - the least hated etc.)
};

// Pre-made TargetTypes
#define Target_Self TargetType()
#define Target_Current TargetType(TargetGen_Current)
#define Target_SecondMostHated TargetType(TargetGen_SecondMostHated)
#define Target_Destination TargetType(TargetGen_Destination)
#define Target_Predefined TargetType(TargetGen_Predefined)
#define Target_RandomPlayer TargetType(TargetGen_RandomPlayer)
#define Target_RandomPlayerNotCurrent TargetType(TargetGen_RandomPlayer, TargetFilter_NotCurrent)
#define Target_RandomPlayerDestination TargetType(TargetGen_RandomPlayerDestination)
#define Target_RandomPlayerApplyAura TargetType(TargetGen_RandomPlayerApplyAura)
#define Target_RandomUnit TargetType(TargetGen_RandomUnit)
#define Target_RandomUnitNotCurrent TargetType(TargetGen_RandomUnit, TargetFilter_NotCurrent)
#define Target_RandomDestination TargetType(TargetGen_RandomUnitDestination)
#define Target_RandomUnitApplyAura TargetType(TargetGen_RandomUnitApplyAura)
#define Target_RandomFriendly TargetType(TargetGen_RandomUnit, TargetFilter_Friendly)
#define Target_WoundedPlayer TargetType(TargetGen_RandomPlayer, TargetFilter_Wounded)
#define Target_WoundedUnit TargetType(TargetGen_RandomUnit, TargetFilter_Wounded)
#define Target_WoundedFriendly TargetType(TargetGen_RandomUnit, TargetFilter_WoundedFriendly)
#define Target_ClosestPlayer TargetType(TargetGen_RandomPlayer, TargetFilter_Closest)
#define Target_ClosestPlayerNotCurrent TargetType(TargetGen_RandomPlayer, TargetFilter_ClosestNotCurrent)
#define Target_ClosestUnit TargetType(TargetGen_RandomUnit, TargetFilter_Closest)
#define Target_ClosestUnitNotCurrent TargetType(TargetGen_RandomUnit, TargetFilter_ClosestNotCurrent)
#define Target_ClosestFriendly TargetType(TargetGen_RandomUnit, TargetFilter_ClosestFriendly)
#define Target_ClosestCorpse TargetType(TargetGen_RandomUnit, TargetFilter_ClosestFriendlyCorpse)
#define Target_RandomCorpse TargetType(TargetGen_RandomUnit, TargetFilter_FriendlyCorpse)

#include "CreatureAIScript.h"


class GameEvent;
class SERVER_DECL EventScript
{
    public:

        EventScript(){};
        virtual ~EventScript() {};

        virtual bool OnBeforeEventStart(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) { return true; } // Before an event is about to be flagged as starting
        virtual void OnAfterEventStart(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) {} // After an event has spawned all entities
        virtual bool OnBeforeEventStop(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) { return true; } // Before an event is about to be flagged as stopping
        virtual void OnAfterEventStop(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) { } // After an event has despawned all entities

        virtual GameEventState OnEventStateChange(GameEvent* /*pEvent*/, GameEventState /*pOldState*/, GameEventState pNewState) { return pNewState; } // When an event changes state

        virtual bool OnCreatureLoad(GameEvent* /*pEvent*/, Creature* /*pCreature*/) { return true; } // When a creature's data has been loaded, before it is spawned
        virtual void OnCreaturePushToWorld(GameEvent* /*pEvent*/, Creature* /*pCreature*/) {} // After a creature has been added to the world
        virtual void OnBeforeCreatureDespawn(GameEvent* /*pEvent*/, Creature* /*pCreature*/) {} // Before a creature is about to be despawned
        virtual void OnAfterCreatureDespawn(GameEvent* /*pEvent*/, Creature* /*pCreature*/) {} // After a creature has been despawned

        virtual bool OnGameObjectLoad(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) { return true; } // When a game object's data has been loaded, before it is spawned
        virtual void OnGameObjectPushToWorld(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) {} // After a game object has been added to the world
        virtual void OnBeforeGameObjectDespawn(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) {} // Before a game object is about to be despawned
        virtual void OnAfterGameObjectDespawn(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) {} // After a game object is about to be despawned

        // Standard virtual methods
        virtual void OnLoad() {}
        virtual void UpdateEvent() {}
        virtual void Destroy() {}

        // Data sharing between scripts
        virtual void setInstanceData(uint32 /*dataType*/, uint32 /*value*/) {}
        virtual uint32 getInstanceData(uint32 /*data*/) const { return 0;  }
        virtual void setGuidData(uint32 /*guidType*/, uint64 /*guidData*/) {}
        virtual uint64 getGuidData(uint32 /*guidType*/) const { return 0; }

        // UpdateEvent
        void RegisterUpdateEvent(uint32 pFrequency);
        void ModifyUpdateEvent(uint32 pNewFrequency);
        void RemoveUpdateEvent();
};

class SERVER_DECL GameObjectAIScript
{
    public:

        GameObjectAIScript(GameObject* goinstance);
        virtual ~GameObjectAIScript() {}

        virtual void OnCreate() {}
        virtual void OnSpawn() {}
        virtual void OnDespawn() {}
        virtual void OnLootTaken(Player* /*pLooter*/, ItemProperties const* /*pItemInfo*/) {}
        virtual void OnActivate(Player* /*pPlayer*/) {}
        virtual void OnDamaged(uint32 /*damage*/){}
        virtual void OnDestroyed(){}
        virtual void AIUpdate() {}
        virtual void Destroy() { delete this; }

        // Data sharing between scripts
        virtual void setGameObjectData(uint32 /*type*/) {}
        virtual uint32 getGameObjectData(uint32 /*type*/) const { return 0; }
        virtual void setGuidData(uint32 /*guidType*/, uint64 /*guidData*/) {}
        virtual uint64 getGuidData(uint32 /*guidType*/) const { return 0; }

        void RegisterAIUpdateEvent(uint32 frequency);
        void ModifyAIUpdateEvent(uint32 newfrequency);
        void RemoveAIUpdateEvent();

    protected:

        GameObject* _gameobject;
};

class SERVER_DECL QuestScript
{
    public:

        QuestScript() {};
        virtual ~QuestScript() {};

        virtual void OnQuestStart(Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
        virtual void OnQuestComplete(Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
        virtual void OnQuestCancel(Player* /*mTarget*/) {}
        virtual void OnGameObjectActivate(uint32 /*entry*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
        virtual void OnCreatureKill(uint32 /*entry*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
        virtual void OnExploreArea(uint32 /*areaId*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
        virtual void OnPlayerItemPickup(uint32 /*itemId*/, uint32 /*totalCount*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Instanced class created for each instance of the map, holds all scriptable exports
#include "Map/WorldCreator.h"

//#define UseNewMapScriptsProject

enum EncounterStates
{
    NotStarted = 0,
    InProgress = 1,
    Finished = 2,
    Performed = 3,
    PreProgress = 4,
    InvalidState = 0xff
};

typedef std::map<uint32_t, uint32_t> InstanceDataMap;

typedef std::set<Creature*> CreatureSet;
typedef std::set<GameObject*> GameObjectSet;

typedef std::pair<uint32_t, uint32_t> InstanceTimerPair;
typedef std::vector<InstanceTimerPair> InstanceTimerArray;

const int32 INVALIDATE_TIMER = -1;
const uint32 DEFAULT_DESPAWN_TIMER = 2000;      //milliseconds

const uint32_t defaultUpdateFrequency = 1000;

class SERVER_DECL InstanceScript
{
    public:

        InstanceScript(MapMgr* pMapMgr);
        virtual ~InstanceScript() {}

        // Procedures that had been here before
        virtual GameObject* GetObjectForOpenLock(Player* /*pCaster*/, Spell* /*pSpell*/, SpellInfo const* /*pSpellEntry*/) { return NULL; }
        virtual void SetLockOptions(uint32 /*pEntryId*/, GameObject* /*pGameObject*/) {}
        virtual uint32 GetRespawnTimeForCreature(uint32 /*pEntryId*/, Creature* /*pCreature*/) { return 240000; }

        // Player
        virtual void OnPlayerDeath(Player* /*pVictim*/, Unit* /*pKiller*/) {}

        // Area and AreaTrigger
        virtual void OnPlayerEnter(Player* /*pPlayer*/) {}
        virtual void OnAreaTrigger(Player* /*pPlayer*/, uint32 /*pAreaId*/) {}
        virtual void OnZoneChange(Player* /*pPlayer*/, uint32 /*pNewZone*/, uint32 /*pOldZone*/) {}

        // Creature / GameObject - part of it is simple reimplementation for easier use Creature / GO < --- > Script
        virtual void OnCreatureDeath(Creature* /*pVictim*/, Unit* /*pKiller*/) {}
        virtual void OnCreaturePushToWorld(Creature* /*pCreature*/) {}
        virtual void OnGameObjectActivate(GameObject* /*pGameObject*/, Player* /*pPlayer*/) {}
        virtual void OnGameObjectPushToWorld(GameObject* /*pGameObject*/) {}

        // Standard virtual methods
        virtual void OnLoad() {}
        virtual void UpdateEvent() {}

        virtual void Destroy() {}

        // Something to return Instance's MapMgr
        MapMgr* GetInstance() { return mInstance; }

        // MIT start
        //////////////////////////////////////////////////////////////////////////////////////////
        // data

        void addData(uint32_t data, uint32_t state = NotStarted);

        void setData(uint32_t data, uint32_t state);
        uint32_t getData(uint32_t data);
        bool isDataStateFinished(uint32_t data);

        // used for local instance data (not saved to database, only for scripting)
        virtual void SetInstanceData(uint32_t /*type*/, uint32_t /*data*/) {}
        virtual void SetInstanceData64(uint32_t /*type*/, uint64_t /*data*/) {}
        virtual uint32_t GetInstanceData(uint32_t /*type*/) const { return 0; }
        virtual uint64_t GetInstanceData64(uint32_t /*type*/) const { return 0; }
        
        //used for debug
        std::string getDataStateString(uint32_t bossEntry);

        //////////////////////////////////////////////////////////////////////////////////////////
        // encounters

        // called for all initialized instancescripts!
        void generateBossDataState();
        void sendUnitEncounter(uint32_t type, Unit* unit = nullptr, uint8_t value_a = 0, uint8_t value_b = 0);

        //used for debug
        void displayDataStateList(Player* player);

        //////////////////////////////////////////////////////////////////////////////////////////
        // timers

    private:

        InstanceTimerArray mTimers;
        uint32_t mTimerCount;

    public:

        uint32_t addTimer(uint32_t durationInMs);
        uint32_t getTimeForTimer(uint32_t timerId);
        void removeTimer(uint32_t& timerId);
        void resetTimer(uint32_t timerId, uint32_t durationInMs);
        bool isTimerFinished(uint32_t timerId);
        void cancelAllTimers();

        //only for internal use!
        void updateTimers();

        //used for debug
        void displayTimerList(Player* player);

        //////////////////////////////////////////////////////////////////////////////////////////
        // instance update

    private:

        uint32_t mUpdateFrequency;

    public:

        uint32_t getUpdateFrequency() { return mUpdateFrequency; }
        void setUpdateFrequency(uint32_t frequencyInMs) { mUpdateFrequency = frequencyInMs; }

        void registerUpdateEvent();
        void modifyUpdateEvent(uint32_t frequencyInMs);
        void removeUpdateEvent();

        //////////////////////////////////////////////////////////////////////////////////////////
        // misc

        void setCellForcedStates(float xMin, float xMax, float yMin, float yMax, bool forceActive = true);

        Creature* spawnCreature(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId = 0);
        Creature* getCreatureBySpawnId(uint32_t entry);
        Creature* GetCreatureByGuid(uint32_t guid);

        CreatureSet getCreatureSetForEntry(uint32_t entry, bool debug = false, Player* player = nullptr);
        CreatureSet getCreatureSetForEntries(std::vector<uint32_t> entryVector);

        GameObject* spawnGameObject(uint32_t entry, float posX, float posY, float posZ, float posO, bool addToWorld = true, uint32_t misc1 = 0, uint32_t phase = 0);
        GameObject* getGameObjectBySpawnId(uint32_t entry);
        GameObject* GetGameObjectByGuid(uint32_t guid);

        GameObject* getClosestGameObjectForPosition(uint32_t entry, float posX, float posY, float posZ);

        GameObjectSet getGameObjectsSetForEntry(uint32_t entry);

        float getRangeToObjectForPosition(Object* object, float posX, float posY, float posZ);

        void setGameObjectStateForEntry(uint32_t entry, uint8_t state);

    private:

        bool mSpawnsCreated;

    public:

        bool spawnsCreated() { return mSpawnsCreated; }
        void setSpawnsCreated(bool created = true) { mSpawnsCreated = created; }

    protected:

        InstanceDataMap mInstanceData;

        //MIT end

        MapMgr* mInstance;
};


class SERVER_DECL HookInterface : public Singleton<HookInterface>
{
    public:

        friend class ScriptMgr;

        bool OnNewCharacter(uint32 Race, uint32 Class, WorldSession* Session, const char* Name);
        void OnKillPlayer(Player* pPlayer, Player* pVictim);
        void OnFirstEnterWorld(Player* pPlayer);
        void OnEnterWorld(Player* pPlayer);
        void OnGuildCreate(Player* pLeader, Guild* pGuild);
        void OnGuildJoin(Player* pPlayer, Guild* pGuild);
        void OnDeath(Player* pPlayer);
        bool OnRepop(Player* pPlayer);
        void OnEmote(Player* pPlayer, uint32 Emote, Unit* pUnit);
        void OnEnterCombat(Player* pPlayer, Unit* pTarget);
        bool OnCastSpell(Player* pPlayer, SpellInfo const* pSpell, Spell* spell);
        bool OnLogoutRequest(Player* pPlayer);
        void OnLogout(Player* pPlayer);
        void OnQuestAccept(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
        void OnZone(Player* pPlayer, uint32 Zone, uint32 oldZone);
        bool OnChat(Player* pPlayer, uint32 Type, uint32 Lang, const char* Message, const char* Misc);
        void OnLoot(Player* pPlayer, Unit* pTarget, uint32 Money, uint32 ItemId);
        void OnFullLogin(Player* pPlayer);
        void OnCharacterCreate(Player* pPlayer);
        void OnQuestCancelled(Player* pPlayer, QuestProperties const* pQuest);
        void OnQuestFinished(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
        void OnHonorableKill(Player* pPlayer, Player* pKilled);
        void OnArenaFinish(Player* pPlayer, ArenaTeam* pTeam, bool victory, bool rated);
        void OnObjectLoot(Player* pPlayer, Object* pTarget, uint32 Money, uint32 ItemId);
        void OnAreaTrigger(Player* pPlayer, uint32 areaTrigger);
        void OnPostLevelUp(Player* pPlayer);
        bool OnPreUnitDie(Unit* Killer, Unit* Victim);
        void OnAdvanceSkillLine(Player* pPlayer, uint32 SkillLine, uint32 Current);
        void OnDuelFinished(Player* Winner, Player* Looser);
        void OnAuraRemove(Aura* aura);
        bool OnResurrect(Player* pPlayer);
};

#define sScriptMgr ScriptMgr::getSingleton()
#define sHookInterface HookInterface::getSingleton()
