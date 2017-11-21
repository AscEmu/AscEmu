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

#define ADD_CREATURE_FACTORY_FUNCTION(cl) public:\
static CreatureAIScript* Create(Creature* c) { return new cl(c); }
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
class SpellDesc;
class CreatureAIScript;
class Unit;

enum RangeStatus
{
    RangeStatus_Ok,
    RangeStatus_TooFar,
    RangeStatus_TooClose
};

typedef void(*EventFunc)(CreatureAIScript* pCreatureAI, int32_t pMiscVal);
typedef void(*SpellFunc)(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

typedef std::vector<Unit*> UnitArray;
typedef std::vector<SpellDesc*> SpellDescArray;
typedef std::list<SpellDesc*> SpellDescList;
typedef std::pair<uint32_t, SpellDesc*> PhaseSpellPair;
typedef std::vector<PhaseSpellPair> PhaseSpellArray;
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

#include "Chat/ChatDefines.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//Class SpellDesc
class SERVER_DECL SpellDesc
{
public:

    SpellDesc(SpellInfo* pInfo, SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32_t pCooldown, float pMinRange, float pMaxRange,
        bool pStrictRange, std::string pText, uint8_t pTextType, uint32_t pSoundId, std::string pAnnouncement);

    virtual ~SpellDesc();


    struct EmoteDesc
    {
        EmoteDesc(std::string pText, uint8_t pType, uint32_t pSoundId)
        {
            mText = (!pText.empty() ? pText : "");
            mType = pType;
            mSoundId = pSoundId;
        }

        std::string mText;
        uint8_t mType;
        uint32_t mSoundId;
    };
    typedef std::vector<EmoteDesc> EmoteArray;

    void addEmote(std::string pText, uint8_t pType = CHAT_MSG_MONSTER_YELL, uint32_t pSoundId = 0);
    void sendRandomEmote(CreatureAIScript* creatureAI);

    void setTriggerCooldown(uint32_t pCurrentTime = 0);

    void addAnnouncement(std::string pText);

    SpellInfo* mInfo;               //Spell Entry information (generally you either want a SpellInfo OR a SpellFunc, not both)
    SpellFunc mSpellFunc;           //Spell Function to be executed (generally you either want a SpellInfo OR a SpellFunc, not both)
    TargetType mTargetType;         //Target type (see class above)

    float mChance;                  //Percent of the cast of this spell in a total of 100% of the attacks
    float mCastTime;                //Duration of the spell cast (seconds). Zero means the spell is instant cast
    int32_t mCooldown;              //Spell cooldown (seconds)

    float mMinRange;                //Minimum range for spell to be cast
    float mMaxRange;                //Maximum range for spell to be cast
    bool mStrictRange;              //If true, creature won't run to (or away of) target, it will instead skip that attack

    EmoteArray mEmotes;             //Emotes (random) shouted on spell cast
    bool mEnabled;                  //True if the spell is enabled for casting, otherwise it will never be scheduled (useful for bosses with phases, etc.)
    Unit* mPredefinedTarget;        //Pre-defined Target Unit (Only valid with target type TargetGen_Predefined);

    std::string mAnnouncement;      //Announce spell cast

                                    //Those are not properties, they are data member used by the evaluation system
    uint32_t mLastCastTime;         //Last time at which the spell casted (used to implement cooldown), set to 0
};

//\brief: Used originally for SP_AI_Spell targettype and new for CreatureAISpells
enum
{
    TARGET_SELF,
    TARGET_VARIOUS,
    TARGET_ATTACKING,
    TARGET_DESTINATION,
    TARGET_SOURCE,
    TARGET_RANDOM_FRIEND,    // doesn't work yet
    TARGET_RANDOM_SINGLE,
    TARGET_RANDOM_DESTINATION,
    TARGET_CUSTOM
};

#include "Spell/Customization/SpellCustomizations.hpp"

//\brief: created by Zyres 11/13/2017 - This should replace SP_AI_Spell, ScriptSpell and SpellDesc
class SERVER_DECL CreatureAISpells
{
    public:
        CreatureAISpells(SpellInfo* spellInfo, float castChance, uint32_t targetType, uint32_t duration, uint32_t cooldown, bool forceRemove, bool isTriggered)
        {
            mSpellInfo = spellInfo;
            mCastChance = castChance;
            mTargetType = targetType;
            mDuration = duration;

            mDurationTimerId = 0;

            mCooldown = cooldown;
            mCooldownTimerId = 0;
            mForceRemoveAura = forceRemove;
            mIsTriggered = isTriggered;

            mMaxStackCount = 1;

            mMinPositionRangeToCast = 0.0f;
            mMaxPositionRangeToCast = 0.0f;

            mMinHpRangeToCast = 0;
            mMaxHpRangeToCast = 100;

            if (mSpellInfo != nullptr)
            {
                mMinPositionRangeToCast = GetMinRange(sSpellRangeStore.LookupEntry(mSpellInfo->getRangeIndex()));
                mMaxPositionRangeToCast = GetMaxRange(sSpellRangeStore.LookupEntry(mSpellInfo->getRangeIndex()));
            }

            mAttackStopTimer = 0;

            mCustomTargetCreature = nullptr;
        }

        ~CreatureAISpells()
        {
        }

        SpellInfo* mSpellInfo;
        float mCastChance;
        uint32_t mTargetType;
        uint32_t mDuration;

        void setdurationTimer(uint32_t durationTimer)
        {
            mDurationTimerId = durationTimer;
        }

        uint32_t mDurationTimerId;

        void setCooldownTimerId(uint32_t cooldownTimer)
        {
            mCooldownTimerId = cooldownTimer;
        }

        uint32_t mCooldown;
        uint32_t mCooldownTimerId;

        bool mForceRemoveAura;
        bool mIsTriggered;

        // non db script messages
        struct AISpellEmotes
        {
            AISpellEmotes(std::string pText, uint8_t pType, uint32_t pSoundId)
            {
                mText = (!pText.empty() ? pText : "");
                mType = pType;
                mSoundId = pSoundId;
            }

            std::string mText;
            uint8_t mType;
            uint32_t mSoundId;
        };
        typedef std::vector<AISpellEmotes> AISpellEmoteArray;
        AISpellEmoteArray mAISpellEmote;

        void addDBEmote(uint32_t textId);
        void addEmote(std::string pText, uint8_t pType = CHAT_MSG_MONSTER_YELL, uint32_t pSoundId = 0);

        void sendRandomEmote(CreatureAIScript* creatureAI);

        uint32_t mMaxStackCount;

        void setMaxStackCount(uint32_t stackCount) { mMaxStackCount = stackCount; }
        uint32_t getMaxStackCount() { return mMaxStackCount; }

        float mMinPositionRangeToCast;
        float mMaxPositionRangeToCast;

        bool isDistanceInRange(float targetDistance)
        {
            if (targetDistance >= mMinPositionRangeToCast && targetDistance <= mMaxPositionRangeToCast)
                return true;

            return false;
        }
        void setMinMaxDistance(float minDistance, float maxDistance)
        {
            mMinPositionRangeToCast = minDistance;
            mMaxPositionRangeToCast = maxDistance;
        }

        // if it is not a random target type it sets the hp range when the creature can cast this spell
        // if it is a random target it controles when the spell can be cast based on the target hp
        int mMinHpRangeToCast;
        int mMaxHpRangeToCast;

        bool isHpInPercentRange(int targetHp)
        {
            if (targetHp >= mMinHpRangeToCast && targetHp <= mMaxHpRangeToCast)
                return true;

            return false;
        }

        void setMinMaxPercentHp(int minHp, int maxHp)
        {
            mMinHpRangeToCast = minHp;
            mMaxHpRangeToCast = maxHp;
        }

        typedef std::vector<uint32_t> ScriptPhaseList;
        ScriptPhaseList mPhaseList;

        void setAvailableForScriptPhase(std::vector<uint32_t> phaseVector)
        {
            for (const auto& phase : phaseVector)
            {
                mPhaseList.push_back(phase);
            }
        }

        bool isAvailableForScriptPhase(uint32_t scriptPhase)
        {
            if (mPhaseList.empty())
                return true;

            for (const auto& availablePhase : mPhaseList)
            {
                if (availablePhase == scriptPhase)
                    return true;
            }

            return false;
        }

        uint32_t mAttackStopTimer;

        void setAttackStopTimer(uint32_t attackStopTime)
        {
            mAttackStopTimer = attackStopTime;
        }

        uint32_t getAttackStopTimer()
        {
            return mAttackStopTimer;
        }

        std::string mAnnouncement;
        void setAnnouncement(std::string announcement)
        {
            mAnnouncement = announcement;
        }
        void sendAnnouncement(CreatureAIScript* creatureAI);

        Creature* mCustomTargetCreature;
        void setCustomTarget(Creature* targetCreature)
        {
            mCustomTargetCreature = targetCreature;
        }

        Creature* getCustomTarget()
        {
            return mCustomTargetCreature;
        }
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
        virtual void OnScriptPhaseChange(uint32_t /*phaseId*/) {}
        virtual void OnHitBySpell(uint32_t /*pSpellId*/, Unit* /*pUnitCaster*/) {}

        //////////////////////////////////////////////////////////////////////////////////////////
        // Event default management
        //\brief: These functions are called internal for script events. Do NOT use them in your scripts!
        void _internalOnDied();
        void _internalOnTargetDied();
        void _internalOnCombatStart();
        void _internalOnCombatStop();
        void _internalAIUpdate();
        void _internalOnScriptPhaseChange();

        //////////////////////////////////////////////////////////////////////////////////////////
        // player
        Player* getNearestPlayer();

        //////////////////////////////////////////////////////////////////////////////////////////
        // creature

        CreatureAIScript* getNearestCreatureAI(uint32_t entry);

        Creature* getNearestCreature(uint32_t entry);
        Creature* getNearestCreature(float posX, float posY, float posZ, uint32_t entry);

        float getRangeToObject(Object* object);

        CreatureAIScript* spawnCreatureAndGetAIScript(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId = 0);

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
        // script phase
        //\brief: script phase is reset to 0 in _internalOnDied() and _internalOnCombatStop()

    private:

        uint32_t mScriptPhase;

    public:

        uint32_t getScriptPhase();
        void setScriptPhase(uint32_t scriptPhase);
        void resetScriptPhase();
        bool isScriptPhase(uint32_t scriptPhase);

        //////////////////////////////////////////////////////////////////////////////////////////
        // timers
        //\brief: timers are stored and updated in InstanceScript if a instance script is
        //        available (instanceUpdateFrequency). If the creature is on a map without a
        //        instance script, the timer gets updated locale (AIUpdateFrequency).
    private:

        //reference to instance time - used for creatures located on a map with a instance script.
        typedef std::list<uint32_t> creatureTimerIds;
        creatureTimerIds mCreatureTimerIds;

        //creature timer - used for creatures located on a map with NO instance script.
        typedef std::pair<uint32_t, uint32_t> CreatureTimerPair;
        typedef std::vector<CreatureTimerPair> CreatureTimerArray;

        CreatureTimerArray mCreatureTimer;

        uint32_t mCreatureTimerCount;

    public:

        uint32_t _addTimer(uint32_t durationInMs);
        uint32_t _getTimeForTimer(uint32_t timerId);
        void _removeTimer(uint32_t& timerId);
        void _resetTimer(uint32_t timerId, uint32_t durationInMs);
        bool _isTimerFinished(uint32_t timerId);
        void _cancelAllTimers();

        uint32_t _getTimerCount();

        //only for internal use!
        void updateAITimers();

        //used for debug
        void displayCreatureTimerList(Player* player);

        //////////////////////////////////////////////////////////////////////////////////////////
        // ai upodate frequency

    private:

        uint32_t mAIUpdateFrequency;

        uint32_t mCustomAIUpdateDelayTimerId;
        uint32_t mCustomAIUpdateDelay;
    public:

        //new
        void registerAiUpdateFrequency();
        void removeAiUpdateFrequency();

        //old stuff

        void SetAIUpdateFreq(uint32 pUpdateFreq);
        uint32 GetAIUpdateFreq();

        void RegisterAIUpdateEvent(uint32 frequency);
        void ModifyAIUpdateEvent(uint32 newfrequency);
        void RemoveAIUpdateEvent();

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
        //\brief: created by Zyres 11/13/2017 - This should replace AP_AI_Spell and SpellDesc

        typedef std::vector<CreatureAISpells*> CreatureAISpellsArray;
        CreatureAISpellsArray mCreatureAISpells;

    private:

    public:

        uint32_t mSpellWaitTimerId;
        bool enableCreatureAISpellSystem;

        //addAISpell(spellID, Chance, TargetType, Duration (s), waitBeforeNextCast (s))
        CreatureAISpells* addAISpell(uint32_t spellId, float castChance, uint32_t targetType, uint32_t duration = 0, uint32_t cooldown = 0, bool forceRemove = false, bool isTriggered = false)
        {
            SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
            if (spellInfo != nullptr)
            {
                uint32_t spellDuration = duration * 1000;
                if (spellDuration == 0)
                    spellDuration = spellInfo->getSpellDuration(nullptr);

                uint32_t spellCooldown = cooldown * 1000;
                if (spellCooldown == 0)
                    spellCooldown = spellInfo->getSpellDuration(nullptr);

                CreatureAISpells* newAISpell = new CreatureAISpells(spellInfo, castChance, targetType, spellDuration, spellCooldown, forceRemove, isTriggered);

                mCreatureAISpells.push_back(newAISpell);

                newAISpell->setdurationTimer(_addTimer(spellDuration));
                newAISpell->setCooldownTimerId(_addTimer(0));

                return newAISpell;
            }
            else
            {
                LOG_ERROR("tried to add invalid spell with id %u", spellId);

                // assert spellInfo can not be nullptr!
                ARCEMU_ASSERT(spellInfo != nullptr);
                return nullptr;
            }
        }

        void _applyAura(uint32_t spellId);
        void _removeAura(uint32_t spellId);
        void _removeAllAuras();

        void _removeAuraOnPlayers(uint32_t spellId);
        void _castOnInrangePlayers(uint32_t spellId, bool triggered = false);
        void _castOnInrangePlayersWithinDist(float minDistance, float maxDistance, uint32_t spellId, bool triggered = false);

        void _setTargetToChannel(Unit* target, uint32_t spellId);
        void _unsetTargetToChannel();
        Unit* _getTargetToChannel();

        Unit* mCurrentSpellTarget;
        CreatureAISpells* mLastCastedSpell;

        // only for internal use
        void newAIUpdateSpellSystem();
        void castSpellOnRandomTarget(CreatureAISpells* AiSpell);

        void oldAIUpdateSpellSystem();

        //////////////////////////////////////////////////////////////////////////////////////////
        // gameobject
        GameObject* getNearestGameObject(uint32_t entry);
        GameObject* getNearestGameObject(float posX, float posY, float posZ, uint32_t entry);

        //////////////////////////////////////////////////////////////////////////////////////////
        // chat message

        enum EmoteEventType
        {
            Event_OnCombatStart = 0,
            Event_OnTargetDied = 1,
            Event_OnDied = 2,
            Event_OnTaunt = 3,
            Event_OnIdle = 4     // new not part of db definitions!
        };

    private:

        typedef std::vector<uint32_t> definedEmoteVector;
        definedEmoteVector mEmotesOnCombatStart;
        definedEmoteVector mEmotesOnTargetDied;
        definedEmoteVector mEmotesOnDied;
        definedEmoteVector mEmotesOnTaunt;
        definedEmoteVector mEmotesOnIdle;

    public:

        void sendChatMessage(uint8_t type, uint32_t soundId, std::string text);
        void sendDBChatMessage(uint32_t textId);

        void sendRandomDBChatMessage(std::vector<uint32_t> emoteVector);

        void addEmoteForEvent(uint32_t eventType, uint32_t scriptTextId);

        void sendAnnouncement(std::string stringAnnounce);

        //////////////////////////////////////////////////////////////////////////////////////////
        // idle emote timer
        //\brief: idle timer is seperated from custom timers. If isIdleEmoteEnabled is true,
        //        a random chat message is send by _internalAIUpdate stored in mEmotesOnIdle

    private:

        bool isIdleEmoteEnabled;
        uint32_t idleEmoteTimerId;

        uint32_t idleEmoteTimeMin;
        uint32_t idleEmoteTimeMax;

    public:

        void enableOnIdleEmote(bool enable, uint32_t durationInMs = 0);
        void setIdleEmoteTimerId(uint32_t timerId);
        uint32_t getIdleEmoteTimerId();
        void resetIdleEmoteTime(uint32_t durationInMs);

        void setRandomIdleEmoteTime(uint32_t minTime, uint32_t maxTime);
        void generateNextRandomIdleEmoteTime();

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

        

        virtual void Destroy() { delete this; }

        CreatureAIScript* GetLinkedCreature() { return linkedCreatureAI; }
        void SetLinkedCreature(CreatureAIScript* creatureAI);
        void LinkedCreatureDeleted();

    protected:

        bool mDespawnWhenInactive;

    private:

        CreatureAIScript* linkedCreatureAI;

        //////////////////////////////////////////////////////////////////////////////////////////
        // OLD MoonScriptCreatureAI functions/members

    public:

        //Movement
        void MoveTo(Unit* pUnit, RangeStatusPair pRangeStatus = std::make_pair(RangeStatus_TooFar, 0.0f));
        
        void AggroNearestUnit(uint32_t pInitialThreat = 1);
        void AggroRandomUnit(uint32_t pInitialThreat = 1);
        void AggroNearestPlayer(uint32_t pInitialThreat = 1);
        void AggroRandomPlayer(uint32_t pInitialThreat = 1);

        //Spells
        SpellDesc* AddSpell(uint32_t pSpellId, TargetType pTargetType, float pChance, float pCastTime, int32_t pCooldown, float pMinRange = 0, float pMaxRange = 0, bool pStrictRange = false, std::string pText = "", uint8_t pTextType = CHAT_MSG_MONSTER_YELL, uint32_t pSoundId = 0, std::string pAnnouncement = "");
        SpellDesc* AddSpellFunc(SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32_t pCooldown, float pMinRange = 0, float pMaxRange = 0, bool pStrictRange = false, std::string pText = "", uint8_t pTextType = CHAT_MSG_MONSTER_YELL, uint32_t pSoundId = 0, std::string pAnnouncement = "");
        
        void CastSpell(SpellDesc* pSpell);
        void CastSpellNowNoScheduling(SpellDesc* pSpell);
        
        SpellDesc* FindSpellById(uint32_t pSpellId);
        SpellDesc* FindSpellByFunc(SpellFunc pFnc);

        void TriggerCooldownOnAllSpells();
        void CancelAllCooldowns();

    protected:

        bool IsSpellScheduled(SpellDesc* pSpell);
        bool CastSpellInternal(SpellDesc* pSpell, uint32_t pCurrentTime = 0);
        void CastSpellOnTarget(Unit* pTarget, TargetType pType, SpellInfo* pEntry, bool pInstant);
        int32 CalcSpellAttackTime(SpellDesc* pSpell);
        void CancelAllSpells();

        RangeStatusPair GetSpellRangeStatusToUnit(Unit* pTarget, SpellDesc* pSpell);
        Unit* GetTargetForSpell(SpellDesc* pSpell);
        Unit* GetBestPlayerTarget(TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f);
        Unit* GetBestUnitTarget(TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f);
        Unit* ChooseBestTargetInArray(UnitArray & pTargetArray, TargetFilter pFilter);
        Unit* GetNearestTargetInArray(UnitArray & pTargetArray);
        Unit* GetSecondMostHatedTargetInArray(UnitArray & pTargetArray);
        bool IsValidUnitTarget(Object* pObject, TargetFilter pFilter, float pMinRange = 0.0f, float pMaxRange = 0.0f);
        void PushRunToTargetCache(Unit* pTarget, SpellDesc* pSpell, RangeStatusPair pRangeStatus = std::make_pair(RangeStatus_TooFar, 0.0f));
        void PopRunToTargetCache();

        SpellDescArray mSpells;
        SpellDescList mQueuedSpells;
        SpellDescList mScheduledSpells;

        Unit* mRunToTargetCache;
        SpellDesc* mRunToTargetSpellCache;

        uint32_t mBaseAttackTime;

        //////////////////////////////////////////////////////////////////////////////////////////
        //MoonScriptBossAI
    public:

        //Basic Interface
        SpellDesc* AddPhaseSpell(uint32_t pPhase, SpellDesc* pSpell);
        void SetEnrageInfo(SpellDesc* pSpell, uint32_t pTriggerMilliseconds);

    protected:

        PhaseSpellArray mPhaseSpells;
        SpellDesc* mEnrageSpell;
        int32_t mEnrageTimerDuration;
        uint32_t mEnrageTimer;
};

//Premade Spell Functions
SERVER_DECL void SpellFunc_ClearHateList(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);
SERVER_DECL void SpellFunc_Disappear(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);
SERVER_DECL void SpellFunc_Reappear(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);


//Premade Event Functions
SERVER_DECL void EventFunc_ApplyAura(CreatureAIScript* pCreatureAI, int32_t pMiscVal);
SERVER_DECL void EventFunc_ChangeGoState(CreatureAIScript* pCreatureAI, int32_t pMiscVal);
SERVER_DECL void EventFunc_RemoveUnitFieldFlags(CreatureAIScript* pCreatureAI, int32_t pMiscVal);

//STL Utilities
template <class Type> inline void DeleteArray(std::vector<Type> pVector)
{
    typename std::vector<Type>::iterator Iter = pVector.begin();
    for (; Iter != pVector.end(); ++Iter)
    {
        delete(*Iter);
    }
    pVector.clear();
}


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
