/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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

#define ADD_CREATURE_FACTORY_FUNCTION(cl) static CreatureAIScript* Create(Creature* c) { return new cl(c); }
#define ADD_INSTANCE_FACTORY_FUNCTION(ClassName) static InstanceScript* Create(MapMgr* pMapMgr) { return new ClassName(pMapMgr); };

class Channel;
class Guild;
struct QuestProperties;

enum ServerHookEvents
{
    SERVER_HOOK_EVENT_ON_NEW_CHARACTER      = 1,
    SERVER_HOOK_EVENT_ON_KILL_PLAYER        = 2,
    SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD  = 3,
    SERVER_HOOK_EVENT_ON_ENTER_WORLD        = 4,
    SERVER_HOOK_EVENT_ON_GUILD_JOIN         = 5,
    SERVER_HOOK_EVENT_ON_DEATH              = 6,
    SERVER_HOOK_EVENT_ON_REPOP              = 7,
    SERVER_HOOK_EVENT_ON_EMOTE              = 8,
    SERVER_HOOK_EVENT_ON_ENTER_COMBAT       = 9,
    SERVER_HOOK_EVENT_ON_CAST_SPELL         = 10,
    SERVER_HOOK_EVENT_ON_TICK               = 11,
    SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST     = 12,
    SERVER_HOOK_EVENT_ON_LOGOUT             = 13,
    SERVER_HOOK_EVENT_ON_QUEST_ACCEPT       = 14,
    SERVER_HOOK_EVENT_ON_ZONE               = 15,
    SERVER_HOOK_EVENT_ON_CHAT               = 16,
    SERVER_HOOK_EVENT_ON_LOOT               = 17,
    SERVER_HOOK_EVENT_ON_GUILD_CREATE       = 18,
    SERVER_HOOK_EVENT_ON_FULL_LOGIN         = 19,
    SERVER_HOOK_EVENT_ON_CHARACTER_CREATE   = 20,
    SERVER_HOOK_EVENT_ON_QUEST_CANCELLED    = 21,
    SERVER_HOOK_EVENT_ON_QUEST_FINISHED     = 22,
    SERVER_HOOK_EVENT_ON_HONORABLE_KILL     = 23,
    SERVER_HOOK_EVENT_ON_ARENA_FINISH       = 24,
    SERVER_HOOK_EVENT_ON_OBJECTLOOT         = 25,
    SERVER_HOOK_EVENT_ON_AREATRIGGER        = 26,
    SERVER_HOOK_EVENT_ON_POST_LEVELUP       = 27,
    SERVER_HOOK_EVENT_ON_PRE_DIE            = 28, // general unit die, not only based on players
    SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE  = 29,
    SERVER_HOOK_EVENT_ON_DUEL_FINISHED      = 30,
    SERVER_HOOK_EVENT_ON_AURA_REMOVE        = 31,
    SERVER_HOOK_EVENT_ON_RESURRECT          = 32,
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
typedef bool(*tOnCastSpell)(Player* pPlayer, SpellInfo* pSpell, Spell* spell);
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
typedef bool(*exp_handle_dummy_spell)(uint32 i, Spell* pSpell);
typedef bool(*exp_handle_script_effect)(uint32 i, Spell* pSpell);
typedef bool(*exp_handle_dummy_aura)(uint32 i, Aura* pAura, bool apply);
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

        bool CallScriptedDummySpell(uint32 uSpellId, uint32 i, Spell* pSpell);
        bool HandleScriptedSpellEffect(uint32 SpellId, uint32 i, Spell* s);
        bool CallScriptedDummyAura(uint32 uSpellId, uint32 i, Aura* pAura, bool apply);
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

class SERVER_DECL CreatureAIScript
{
    public:

        CreatureAIScript(Creature* creature);
        virtual ~CreatureAIScript();

        virtual void OnCombatStart(Unit* /*mTarget*/) {}
        virtual void OnCombatStop(Unit* /*mTarget*/) {}
        virtual void OnDamageTaken(Unit* /*mAttacker*/, uint32 /*fAmount*/) {}
        virtual void OnCastSpell(uint32 /*iSpellId*/) {}
        virtual void OnTargetParried(Unit* /*mTarget*/) {}
        virtual void OnTargetDodged(Unit* /*mTarget*/) {}
        virtual void OnTargetBlocked(Unit* /*mTarget*/, int32 /*iAmount*/) {}
        virtual void OnTargetCritHit(Unit* /*mTarget*/, int32 /*fAmount*/) {}
        virtual void OnTargetDied(Unit* /*mTarget*/) {}
        virtual void OnParried(Unit* /*mTarget*/) {}
        virtual void OnDodged(Unit* /*mTarget*/) {}
        virtual void OnBlocked(Unit* /*mTarget*/, int32 /*iAmount*/) {}
        virtual void OnCritHit(Unit* /*mTarget*/, int32 /*fAmount*/) {}
        virtual void OnHit(Unit* /*mTarget*/, float /*fAmount*/) {}
        virtual void OnDied(Unit* /*mKiller*/) {}
        virtual void OnAssistTargetDied(Unit* /*mAssistTarget*/) {}
        virtual void OnFear(Unit* /*mFeared*/, uint32 /*iSpellId*/) {}
        virtual void OnFlee(Unit* /*mFlee*/) {}
        virtual void OnCallForHelp() {}
        virtual void OnLoad() {}
        virtual void OnDespawn() {}
        virtual void OnReachWP(uint32 /*iWaypointId*/, bool /*bForwards*/) {}
        virtual void OnLootTaken(Player* /*pPlayer*/, ItemProperties const* /*pItemPrototype*/) {}
        virtual void AIUpdate() {}
        virtual void OnEmote(Player* /*pPlayer*/, EmoteType /*Emote*/) {}
        virtual void StringFunctionCall(int) {}

        virtual void OnEnterVehicle() {}
        virtual void OnExitVehicle() {}
        virtual void OnFirstPassengerEntered(Unit* /*passenger*/) {}
        virtual void OnVehicleFull() {}
        virtual void OnLastPassengerLeft(Unit* /*passenger*/) {}

        // MIT start
        //////////////////////////////////////////////////////////////////////////////////////////
        // Event default management
        //\NOTE: These functions are called internal for script events. Do NOT use them in your scripts!
        void _internalOnDiedCleanup();

        //////////////////////////////////////////////////////////////////////////////////////////
        // player
        Player* getNearestPlayer();

        //////////////////////////////////////////////////////////////////////////////////////////
        // creature
        Creature* getNearestCreature(uint32_t entry);
        Creature* getNearestCreature(float posX, float posY, float posZ, uint32_t entry);

        float getRangeToObject(Object* object);

        Creature* spawnCreature(uint32_t entry, LocationVector pos, uint32_t factionId = 0);
        Creature* spawnCreature(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId = 0);
        void despawn(uint32_t delay = 2000, uint32_t respawnTime = 0);

        bool isAlive();

        // AIAgent
        void setAIAgent(AI_Agent agent);
        uint8_t getAIAgent();

        // movement
        void setRooted(bool set);
        bool isRooted();

        void setFlyMode(bool fly);

        // single point movement
        void moveTo(float posX, float posY, float posZ, bool setRun = true);
        void moveToUnit(Unit* unit);
        void moveToSpawn();
        void stopMovement();

        // wp movement
        Movement::WayPoint* CreateWaypoint(int pId, uint32 pWaittime, uint32 pMoveFlag, Movement::Location pCoords);
        void AddWaypoint(Movement::WayPoint* pWayPoint);
        void ForceWaypointMove(uint32 pWaypointId);
        void SetWaypointToMove(uint32 pWaypointId);
        void StopWaypointMovement();
        void SetWaypointMoveType(Movement::WaypointMovementScript wp_move_script_type);
        uint32 GetCurrentWaypoint();
        size_t GetWaypointCount();
        bool HasWaypoints();

        //////////////////////////////////////////////////////////////////////////////////////////
        // combat setup
        bool canEnterCombat();
        void setCanEnterCombat(bool enterCombat);
        bool _isInCombat();
        void _delayNextAttack(int32_t milliseconds);
        void _setDespawnWhenInactive(bool setDespawnWhenInactive);
        bool _isDespawnWhenInactiveSet();

        void _setMeleeDisabled(bool disable);
        bool _isMeleeDisabled();
        void _setRangedDisabled(bool disable);
        bool _isRangedDisabled();
        void _setCastDisabled(bool disable);
        bool _isCastDisabled();
        void _setTargetingDisabled(bool disable);
        bool _isTargetingDisabled();

        void _clearHateList();
        void _wipeHateList();
        int32_t _getHealthPercent();
        int32_t _getManaPercent();
        void _regenerateHealth();

        bool _isCasting();

        bool _isHeroic();

        //////////////////////////////////////////////////////////////////////////////////////////
        // timers
        //\NOTE timers are stored and updated in InstanceScript every second and is no longer bound to AIUpdate().
        //      they require a active InstanceScript. In case of Questscripts use AIInterface functions!
        //      this solution works fine, most custom AIUpdate frequencies can be replaced by these timers.
    private:

        typedef std::list<uint32_t> creatureTimerIds;
        creatureTimerIds mCreatureTimerIds;

    public:

        uint32_t _addTimer(uint32_t durationInMs);
        uint32_t _getTimeForTimer(uint32_t timerId);
        void _removeTimer(uint32_t& timerId);
        void _resetTimer(uint32_t timerId, uint32_t durationInMs);
        bool _isTimerFinished(uint32_t timerId);
        void _cancelAllTimers();

        uint32_t _getTimerCount() { return mCreatureTimerIds.size(); }

        //////////////////////////////////////////////////////////////////////////////////////////
        // appearance
        void _setScale(float scale);
        float _getScale();
        void _setDisplayId(uint32_t displayId);
        void _setWieldWeapon(bool setWieldWeapon);
        void _setDisplayWeapon(bool setMainHand, bool setOffHand);
        void _setDisplayWeaponIds(uint32_t itemId1, uint32_t itemId2);

        //////////////////////////////////////////////////////////////////////////////////////////
        // spell
        void _applyAura(uint32_t spellId);
        void _removeAura(uint32_t spellId);
        void _removeAllAuras();

        void _removeAuraOnPlayers(uint32_t spellId);
        void _castOnInrangePlayers(uint32_t spellId, bool triggered = false);
        void _castOnInrangePlayersWithinDist(float minDistance, float maxDistance, uint32_t spellId, bool triggered = false);

        //////////////////////////////////////////////////////////////////////////////////////////
        // gameobject
        GameObject* getNearestGameObject(uint32_t entry);
        GameObject* getNearestGameObject(float posX, float posY, float posZ, uint32_t entry);

        //////////////////////////////////////////////////////////////////////////////////////////
        // chat message
        void sendChatMessage(uint8_t type, uint32_t soundId, std::string text);
        void sendDBChatMessage(uint32_t textId);

        //////////////////////////////////////////////////////////////////////////////////////////
        // basic
    private:

        Creature* _creature;

    public:

        Creature* getCreature() { return _creature; }

        //////////////////////////////////////////////////////////////////////////////////////////
        // instance

        InstanceScript* getInstanceScript();

        // MIT end

        void RegisterAIUpdateEvent(uint32 frequency);
        void ModifyAIUpdateEvent(uint32 newfrequency);
        void RemoveAIUpdateEvent();

        virtual void Destroy() { delete this; }

        CreatureAIScript* GetLinkedCreature() { return linkedCreatureAI; }
        void SetLinkedCreature(CreatureAIScript* creatureAI);
        void LinkedCreatureDeleted();

    protected:

        bool mDespawnWhenInactive;

    private:

        CreatureAIScript* linkedCreatureAI;
};

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
/// Instanced class created for each instance of the map, holds all scriptable exports
//////////////////////////////////////////////////////////////////////////////////////////
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
        virtual GameObject* GetObjectForOpenLock(Player* /*pCaster*/, Spell* /*pSpell*/, SpellInfo* /*pSpellEntry*/) { return NULL; }
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
        bool OnCastSpell(Player* pPlayer, SpellInfo* pSpell, Spell* spell);
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
