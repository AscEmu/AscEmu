/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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

#ifndef OBJECTMGR_H
#define OBJECTMGR_H

#include "Chat/ChatHandler.hpp"
#include "Units/Creatures/Corpse.h"
#include "Units/Players/Player.h"
#include "Units/Creatures/Vehicle.h"
#include "Management/Guild/Guild.hpp"
#include "Storage/DBC/DBCStructures.hpp"
#include "Storage/DBC/DBCStores.h"
#include "Storage/MySQLStructures.h"
#if VERSION_STRING >= Cata
    #include "Storage/DB2/DB2Stores.h"
    #include "Storage/DB2/DB2Structures.h"
#endif
#include "Units/Creatures/CreatureDefines.hpp"
#include "Management/TransporterHandler.h"
#include "Management/Gossip/GossipDefines.hpp"
#include "Objects/GameObject.h"
#include "Spell/Spell.h"
#include "Management/Group.h"

#include <string>
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Spell/SpellTargetConstraint.h"
#include "Management/Tickets/TicketMgr.hpp"

struct WorldState
{
    uint32 field;
    uint32 value;

    WorldState()
    {
        field = 0;
        value = 0;
    }
};

// this has nothing to do with object management ;)
enum EncounterCreditType
{
    ENCOUNTER_CREDIT_KILL_CREATURE  = 0,
    ENCOUNTER_CREDIT_CAST_SPELL     = 1
};

#if VERSION_STRING >= WotLK
struct DungeonEncounter
{
    DungeonEncounter(DBC::Structures::DungeonEncounterEntry const* _dbcEntry, EncounterCreditType _creditType, uint32_t _creditEntry, uint32_t _lastEncounterDungeon)
        : dbcEntry(_dbcEntry), creditType(_creditType), creditEntry(_creditEntry), lastEncounterDungeon(_lastEncounterDungeon) { }

    DBC::Structures::DungeonEncounterEntry const* dbcEntry;
    EncounterCreditType creditType;
    uint32_t creditEntry;
    uint32_t lastEncounterDungeon;
};
#else
struct DungeonEncounter
{
    DungeonEncounter(EncounterCreditType _creditType, uint32_t _creditEntry)
        : creditEntry(_creditEntry), creditType(_creditType) { }

    uint32_t creditEntry;
    EncounterCreditType creditType;
};
#endif

typedef std::list<DungeonEncounter const*> DungeonEncounterList;
typedef std::unordered_map<uint32_t, DungeonEncounterList> DungeonEncounterContainer;

// why is this here?
struct SpellReplacement
{
    uint32 count;
    uint32* spells;
};

class Group;
class SpellInfo;

//it seems trainerspells should be part of trainer files ;)
#if VERSION_STRING >= Cata
struct TrainerSpell
{
    TrainerSpell() : spell(0), spellCost(0), reqSkill(0), reqSkillValue(0), reqLevel(0)
    {
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            learnedSpell[i] = 0;
    }

    uint32_t spell;
    uint32_t spellCost;
    uint32_t reqSkill;
    uint32_t reqSkillValue;
    uint32_t reqLevel;
    uint32_t learnedSpell[3];

    // helpers
    bool IsCastable() const
    {
        return learnedSpell[0] != spell;
    }
};
#else
struct TrainerSpell
{
    SpellInfo const* pCastSpell;
    SpellInfo const* pLearnSpell;
    SpellInfo const* pCastRealSpell;
    uint32 DeleteSpell;
    uint32 RequiredSpell;
    uint32 RequiredSkillLine;
    uint32 RequiredSkillLineValue;
    bool IsProfession;
    uint32 Cost;
    uint32 RequiredLevel;
};
#endif

// oh a trainer look it is here and not in Unit/Creature/whatever file ;)
struct Trainer
{
    uint32 SpellCount;
    std::vector<TrainerSpell> Spells;
    char* UIMessage;
    uint32 RequiredSkill;
    uint32 RequiredSkillLine;
    uint32 RequiredClass;
    uint32 RequiredRace;
    uint32 RequiredRepFaction;
    uint32 RequiredRepValue;
    uint32 TrainerType;
    uint32 Can_Train_Gossip_TextId;
    uint32 Cannot_Train_GossipTextId;
};

// isn't it part of player info? hmmmmm....
struct LevelInfo
{
    uint32 HP;
    uint32 Mana;
    uint32 Stat[5];
};

//player too?!?
struct ReputationMod
{
    uint32 faction[2];
    int32 value;
    uint32 replimit;
};

// reputation/instance
struct InstanceReputationMod
{
    uint32 mapid;
    uint32 mob_rep_reward;
    uint32 mob_rep_limit;
    uint32 boss_rep_reward;
    uint32 boss_rep_limit;
    uint32 faction[2];
};

// reputation/instance
struct ReputationModifier
{
    uint32 entry;
    std::vector<ReputationMod> mods;
};

// reputation/instance
struct InstanceReputationModifier
{
    uint32 mapid;
    std::vector<InstanceReputationMod> mods;
};

#include "Server/Script/SimpleEventScript.hpp"

#include "Management/Charter.hpp"

typedef std::unordered_map<uint32, Player*> PlayerStorageMap;

#if VERSION_STRING > TBC
typedef std::list<DBC::Structures::AchievementCriteriaEntry const*> AchievementCriteriaEntryList;
#endif

//\TODO is this really needed since c++11?
#ifndef WIN32
typedef std::map<std::string, PlayerInfo*> PlayerNameStringIndexMap;
#else
/// vc++ has the type for a string hash already, so we don't need to do anything special
typedef std::unordered_map<std::string, PlayerInfo*> PlayerNameStringIndexMap;
#endif

#if VERSION_STRING >= Cata
// spell_id  req_spell
typedef std::multimap<uint32_t, uint32_t> SpellRequiredMap;
typedef std::pair<SpellRequiredMap::const_iterator, SpellRequiredMap::const_iterator> SpellRequiredMapBounds;

// req_spell spell_id
typedef std::multimap<uint32_t, uint32_t> SpellsRequiringSpellMap;
typedef std::pair<SpellsRequiringSpellMap::const_iterator, SpellsRequiringSpellMap::const_iterator> SpellsRequiringSpellMapBounds;

// skill line ability
typedef std::multimap<uint32_t, DBC::Structures::SkillLineAbilityEntry const*> SkillLineAbilityMap;
typedef std::pair<SkillLineAbilityMap::const_iterator, SkillLineAbilityMap::const_iterator> SkillLineAbilityMapBounds;
#endif

// finally we are here, the base class of this file ;)
class PlayerCache;
class SERVER_DECL ObjectMgr : public EventableObject
{
    private:
        ObjectMgr() = default;
        ~ObjectMgr() = default;

    public:
        //MIT
        static ObjectMgr& getInstance();
        void initialize();
        void finalize();

        ObjectMgr(ObjectMgr&&) = delete;
        ObjectMgr(ObjectMgr const&) = delete;
        ObjectMgr& operator=(ObjectMgr&&) = delete;
        ObjectMgr& operator=(ObjectMgr const&) = delete;

        void generateDatabaseGossipMenu(Object* object, uint32_t gossipMenuId, Player* player, uint32_t forcedTextId = 0);
        void generateDatabaseGossipOptionAndSubMenu(Object* object, Player* player, uint32_t gossipItemId, uint32_t gossipMenuId);
        //MIT END

        void LoadCreatureWaypoints();
        void LoadCreatureTimedEmotes();

        TimedEmoteList* GetTimedEmoteList(uint32 spawnid);

        // Set typedef's
        typedef std::unordered_map<uint32, Group*>                                                  GroupMap;
        typedef std::unordered_map<uint32, DBC::Structures::SkillLineAbilityEntry const*>           SLMap;
        typedef std::unordered_map<uint32, std::vector<CreatureItem>*>                              VendorMap;
        typedef std::unordered_map<uint32, Trainer*>                                                TrainerMap;
        typedef std::unordered_map<uint32, ReputationModifier*>                                     ReputationModMap;
        typedef std::unordered_map<uint32, Corpse*>                                                 CorpseMap;
        typedef std::unordered_map<uint32, PlayerCache*>                                            PlayerCacheMap;

        // Map typedef's
        typedef std::map<uint32, LevelInfo*>                                                        LevelMap;
        typedef std::map<std::pair<uint32, uint32>, LevelMap*>                                      LevelInfoMap;

        typedef std::map<uint32, uint32>                                                            PetSpellCooldownMap;
        typedef std::multimap <uint32, uint32>                                                      BCEntryStorage;
        typedef std::map<uint32, SpellTargetConstraint*>                                            SpellTargetConstraintMap;

        // object holders
        PlayerCacheMap m_playerCache;
        FastMutex m_playerCacheLock;

        Player* GetPlayer(const char* name, bool caseSensitive = true);
        Player* GetPlayer(uint32 guid);

        void AddPlayerCache(uint32 guid, PlayerCache* cache);
        void RemovePlayerCache(uint32 guid);
        PlayerCache* GetPlayerCache(uint32 guid);
        PlayerCache* GetPlayerCache(const char* name, bool caseSensitive = true);

        CorpseMap m_corpses;
        Mutex _corpseslock;
        Mutex m_creatureSetMutex;

        Item* CreateItem(uint32 entry, Player* owner);
        Item* LoadItem(uint32 lowguid);

        // Groups
        Group* GetGroupByLeader(Player* pPlayer);
        Group* GetGroupById(uint32 id);

        uint32 GenerateGroupId();
        uint32 GenerateGuildId();

        void AddGroup(Group* group)
        {
            std::lock_guard<std::mutex> guard(m_groupLock);

            m_groups.insert(std::make_pair(group->GetID(), group));
        }

        void RemoveGroup(Group* group)
        {
            std::lock_guard<std::mutex> guard(m_groupLock);

            m_groups.erase(group->GetID());
        }

        void LoadGroups();

        // player names
        void AddPlayerInfo(PlayerInfo* pn);
        PlayerInfo* GetPlayerInfo(uint32 guid);
        PlayerInfo* GetPlayerInfoByName(const char* name);
        void RenamePlayerInfo(PlayerInfo* pn, const char* oldname, const char* newname);
        void DeletePlayerInfo(uint32 guid);

        //Corpse Stuff
        Corpse* GetCorpseByOwner(uint32 ownerguid);
        void CorpseCollectorUnload();
        void CorpseAddEventDespawn(Corpse* pCorpse);
        void DelinkPlayerCorpses(Player* pOwner);
        Corpse* CreateCorpse();
        void AddCorpse(Corpse*);
        void RemoveCorpse(Corpse*);
        Corpse* GetCorpse(uint32 corpseguid);

        DBC::Structures::SkillLineAbilityEntry const* GetSpellSkill(uint32 id);
        SpellInfo const* GetNextSpellRank(SpellInfo const* sp, uint32 level);

        //Vendors
        std::vector<CreatureItem> *GetVendorList(uint32 entry);
        void SetVendorList(uint32 Entry, std::vector<CreatureItem>* list_);

        Pet* CreatePet(uint32 entry);

        uint32 GenerateArenaTeamId();

        Player* CreatePlayer(uint8 _class);
        PlayerStorageMap _players;
        std::mutex _playerslock;

        void AddPlayer(Player* p); //add it to global storage
        void RemovePlayer(Player* p);

        // Serialization
#if VERSION_STRING > TBC
        void LoadCompletedAchievements();
        AchievementRewardsMap AchievementRewards;
        AchievementReward const * GetAchievementReward(uint32 entry, uint8 gender)
        {
            AchievementRewardsMapBounds bounds = AchievementRewards.equal_range(entry);
            for (AchievementRewardsMap::const_iterator iter = bounds.first; iter != bounds.second; ++iter)
            {
                if (iter->second.gender == 2 || uint8(iter->second.gender) == gender)
                    return &iter->second;
            }
            return NULL;
        }

        void LoadAchievementRewards();
#endif
        void LoadPlayersInfo();

        Corpse* LoadCorpse(uint32 guid);
        void LoadCorpses(MapMgr* mgr);
        void LoadSpellSkills();
        void LoadVendors();
        void ReloadVendors();

        void LoadReputationModifierTable(const char* tablename, ReputationModMap* dmap);
        void LoadReputationModifiers();
        ReputationModifier* GetReputationModifier(uint32 entry_id, uint32 faction_id);

        void SetHighestGuids();
        uint32 GenerateLowGuid(uint32 guidhigh);
        uint32 GenerateMailID();
        uint32 GenerateReportID();
        uint32 GenerateEquipmentSetID();

#if VERSION_STRING >= Cata
        // Spell Required table
        SpellRequiredMapBounds GetSpellsRequiredForSpellBounds(uint32_t spell_id) const;
        SpellsRequiringSpellMapBounds GetSpellsRequiringSpellBounds(uint32_t spell_id) const;
        bool IsSpellRequiringSpell(uint32_t spellid, uint32_t req_spellid) const;
        const SpellsRequiringSpellMap GetSpellsRequiringSpell();
        uint32_t GetSpellRequired(uint32_t spell_id) const;
        void LoadSpellRequired();

        void LoadSkillLineAbilityMap();
        SkillLineAbilityMapBounds GetSkillLineAbilityMapBounds(uint32_t spell_id) const;
#endif

        void LoadTrainers();
        Trainer* GetTrainer(uint32 Entry);

        void LoadCreatureAIAgents();

        LevelInfo* GetLevelInfo(uint32 Race, uint32 Class, uint32 Level);
        void GenerateLevelUpInfo();

        uint32 GetPetSpellCooldown(uint32 SpellId);
        void LoadPetSpellCooldowns();
        Movement::WayPointMap* GetWayPointMap(uint32 spawnid);

        void ResetDailies();

        uint32 GenerateCreatureSpawnID();
        uint32 GenerateGameObjectSpawnID();

        Charter* CreateCharter(uint32 LeaderGuid, CharterTypes Type);
        Charter* GetCharter(uint32 CharterId, CharterTypes Type);
        void RemoveCharter(Charter*);
        void LoadGuildCharters();
        Charter* GetCharterByName(std::string & charter_name, CharterTypes Type);
        Charter* GetCharterByItemGuid(uint64 guid);
        Charter* GetCharterByGuid(uint64 playerguid, CharterTypes type);

        ArenaTeam* GetArenaTeamByName(std::string & name, uint32 Type);
        ArenaTeam* GetArenaTeamById(uint32 id);
        ArenaTeam* GetArenaTeamByGuid(uint32 guid, uint32 Type);
        void UpdateArenaTeamRankings();
        void UpdateArenaTeamWeekly();
        void ResetArenaTeamRatings();
        void LoadArenaTeams();

        std::unordered_map<uint32, ArenaTeam*> m_arenaTeamMap[3];
        std::unordered_map<uint32, ArenaTeam*> m_arenaTeams;

        void RemoveArenaTeam(ArenaTeam* team);
        void AddArenaTeam(ArenaTeam* team);
        Mutex m_arenaTeamLock;

        bool HandleInstanceReputationModifiers(Player* pPlayer, Unit* pVictim);
        void LoadInstanceReputationModifiers();
        void LoadInstanceEncounters();

#if VERSION_STRING >= WotLK
        DungeonEncounterList const* GetDungeonEncounterList(uint32_t mapId, uint8_t difficulty)
        {
            std::unordered_map<uint32_t, DungeonEncounterList>::const_iterator itr = _dungeonEncounterStore.find(uint32(uint16(mapId) | (uint32(difficulty) << 16)));
            if (itr != _dungeonEncounterStore.end())
                return &itr->second;
            return NULL;
        }
#endif

#if VERSION_STRING <= TBC
        DungeonEncounterList const* GetDungeonEncounterList(uint32_t mapId, uint8 difficulty = 0)
        {
            std::unordered_map<uint32_t, DungeonEncounterList>::const_iterator itr = _dungeonEncounterStore.find(mapId);
            if (itr != _dungeonEncounterStore.end())
                return &itr->second;
            return NULL;
        }
#endif

        inline bool IsSpellDisabled(uint32 spellid)
        {
            if (m_disabled_spells.find(spellid) != m_disabled_spells.end())
                return true;
            return false;
        }

        void LoadDisabledSpells();
        void ReloadDisabledSpells();
        void LoadSpellTargetConstraints();
        SpellTargetConstraint* GetSpellTargetConstraintForSpell(uint32 spellid);

        //////////////////////////////////////////////////////////////////////////////////////////
        // Event Scripts
        void LoadEventScripts();
        EventScriptBounds GetEventScripts(uint32 event_id) const;
        SpellEffectMapBounds GetSpellEffectBounds(uint32 data_1) const;
        bool CheckforScripts(Player* plr, uint32 event_id);
        bool CheckforDummySpellScripts(Player* plr, uint32 data_1);
        void EventScriptsUpdate(Player* plr, uint32 next_event);
        //////////////////////////////////////////////////////////////////////////////////////////

#if VERSION_STRING > TBC
        void LoadAchievementCriteriaList();
        AchievementCriteriaEntryList const & GetAchievementCriteriaByType(AchievementCriteriaTypes type);
        std::set<uint32> allCompletedAchievements;
#endif

        void LoadVehicleAccessories();
        std::vector< VehicleAccessoryEntry* >* GetVehicleAccessories(uint32 creature_entry);
        void LoadWorldStateTemplates();
        std::multimap< uint32, WorldState >* GetWorldStatesForMap(uint32 map) const;

    private:

        EventScriptMaps mEventScriptMaps;
        SpellEffectMaps mSpellEffectMaps;
        DungeonEncounterContainer _dungeonEncounterStore;

#if VERSION_STRING >= Cata
        SpellsRequiringSpellMap mSpellsReqSpell;
        SpellRequiredMap mSpellReq;
        SkillLineAbilityMap mSkillLineAbilityMap;
#endif

    protected:

        std::mutex playernamelock;

        // highest GUIDs, used for creating new objects
        std::atomic<unsigned long> m_hiItemGuid;
        std::atomic<unsigned long> m_hiGroupId;
        std::atomic<unsigned long> m_hiCharterId;
        std::atomic<unsigned long> m_hiCreatureSpawnId;
        std::atomic<unsigned long> m_hiGameObjectSpawnId;
        std::atomic<unsigned long> m_mailid;
        std::atomic<unsigned long> m_reportID;
        std::atomic<unsigned long> m_setGUID;
        std::atomic<unsigned long> m_hiCorpseGuid;
        std::atomic<unsigned long> m_hiGuildId;
        std::atomic<unsigned long> m_hiPetGuid;
        std::atomic<unsigned long> m_hiArenaTeamId;
        std::atomic<unsigned long> m_hiPlayerGuid;

        std::mutex m_charterLock;

        ReputationModMap m_reputation_faction;
        ReputationModMap m_reputation_creature;
        std::unordered_map<uint32, InstanceReputationModifier*> m_reputation_instance;

        std::unordered_map<uint32, Charter*> m_charters[NUM_CHARTER_TYPES];

        std::set<uint32> m_disabled_spells;

        std::unordered_map<uint32, PlayerInfo*> m_playersinfo;
        PlayerNameStringIndexMap m_playersInfoByName;

        std::unordered_map<uint32, Movement::WayPointMap*> mWayPointMap; /// stored by spawnid
        std::unordered_map<uint32, TimedEmoteList*> m_timedemotes;       /// stored by spawnid

        // Group List
        std::mutex m_groupLock;
        GroupMap m_groups;

        /// Map of all vendor goods
        VendorMap mVendors;

        SLMap mSpellSkills;

        TrainerMap mTrainers;
        LevelInfoMap mLevelInfo;
        PetSpellCooldownMap mPetSpellCooldowns;
        SpellTargetConstraintMap m_spelltargetconstraints;
#if VERSION_STRING > TBC
        AchievementCriteriaEntryList m_AchievementCriteriasByType[ACHIEVEMENT_CRITERIA_TYPE_TOTAL];
#endif
        std::map< uint32, std::vector<VehicleAccessoryEntry*>* > vehicle_accessories;
        std::map< uint32, std::multimap<uint32, WorldState>* > worldstate_templates;
};

#define sObjectMgr ObjectMgr::getInstance()

#endif // OBJECTMGR_H
