/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
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

#include "Management/ArenaTeam.hpp"
#include "Management/Charter.hpp"
#include "Management/Group.h"
#include "Management/Tickets/TicketMgr.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Objects/Units/Creatures/Summons/SummonDefines.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Objects/Units/Creatures/Vehicle.hpp"
#include "Server/Script/SimpleEventScript.hpp"
#include "Spell/Spell.h"
#include "Spell/SpellTargetConstraint.hpp"
#include "Storage/DBC/DBCStructures.hpp"
#if VERSION_STRING >= Cata
    #include "Storage/DB2/DB2Stores.h"
    #include "Storage/DB2/DB2Structures.h"
#endif

#include <string>

class SpellInfo;

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
enum EncounterCreditType : uint8_t
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

//it seems trainerspells should be part of trainer files ;)
struct TrainerSpell
{
    TrainerSpell() : castSpell(nullptr), castRealSpell(nullptr), learnSpell(nullptr), deleteSpell(0),
        requiredLevel(0), requiredSkillLine(0), requiredSkillLineValue(0), isPrimaryProfession(false),
        cost(0)
    {
        for (uint8_t i = 0; i < 3; ++i)
            requiredSpell[i] = 0;
    }

    // This spell is casted on player
    SpellInfo const* castSpell;
    // The taught spell from castSpell
    SpellInfo const* castRealSpell;
    // This spell is added to player
    SpellInfo const* learnSpell;
    uint32_t deleteSpell;
    uint32_t requiredLevel;
    uint32_t requiredSpell[3];
    uint16_t requiredSkillLine;
    uint32_t requiredSkillLineValue;
    bool isPrimaryProfession;
    uint32_t cost;
    uint32_t isStatic;

    static uint8_t getMaxRequiredSpellCount()
    {
#if VERSION_STRING < Cata
        return 3;
#else
        return 2;
#endif
    }
};



// oh a trainer look it is here and not in Unit/Creature/whatever file ;)
struct Trainer
{
    uint32_t SpellCount;
    char* UIMessage;
    uint16_t RequiredSkill;
    uint32_t RequiredSkillLine;
    uint32_t RequiredClass;
    uint32_t RequiredRace;
    uint32_t RequiredRepFaction;
    uint32_t RequiredRepValue;
    uint32_t TrainerType;
    uint32_t Can_Train_Gossip_TextId;
    uint32_t Cannot_Train_GossipTextId;
    uint32_t spellset_id;
    uint32_t can_train_max_level;
    uint32_t can_train_min_skill_value;
    uint32_t can_train_max_skill_value;
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

typedef std::unordered_map<uint32, Player*> PlayerStorageMap;

#if VERSION_STRING > TBC
typedef std::list<DBC::Structures::AchievementCriteriaEntry const*> AchievementCriteriaEntryList;
#endif

typedef std::unordered_map<std::string, CachedCharacterInfo*> PlayerNameStringIndexMap;

// finally we are here, the base class of this file ;)
//MIT
class SERVER_DECL ObjectMgr : public EventableObject
{
private:
    ObjectMgr() = default;
    ~ObjectMgr() = default;

public:
    static ObjectMgr& getInstance();
    void initialize();
    void finalize();

    ObjectMgr(ObjectMgr&&) = delete;
    ObjectMgr(ObjectMgr const&) = delete;
    ObjectMgr& operator=(ObjectMgr&&) = delete;
    ObjectMgr& operator=(ObjectMgr const&) = delete;

    void generateDatabaseGossipMenu(Object* object, uint32_t gossipMenuId, Player* player, uint32_t forcedTextId = 0);
    void generateDatabaseGossipOptionAndSubMenu(Object* object, Player* player, uint32_t gossipItemId, uint32_t gossipMenuId);

    void loadTrainerSpellSets();
    std::vector<TrainerSpell> getTrainserSpellSetById(uint32_t id);
    void loadTrainers();

    // Preload CreatureDisplayInfoStore and CreatureModelDataStore to avoid DBC lookup calls
    void loadCreatureDisplayInfo();
    CreatureDisplayInfoData const* getCreatureDisplayInfoData(uint32_t displayId) const;

    Player* createPlayerByGuid(uint8_t _class, uint32_t guid);

    GameObject* createGameObjectByGuid(uint32_t id, uint32_t guid);

    //MIT END

        void LoadCreatureTimedEmotes();

        TimedEmoteList* GetTimedEmoteList(uint32 spawnid);

        // Set typedef's
        typedef std::unordered_map<uint32, Group*>                                                  GroupMap;
        typedef std::unordered_map<uint32, std::vector<CreatureItem>*>                              VendorMap;
        typedef std::unordered_map<uint32, Trainer*>                                                TrainerMap;
        typedef std::unordered_map<uint32, ReputationModifier*>                                     ReputationModMap;
        typedef std::unordered_map<uint32, Corpse*>                                                 CorpseMap;

        // Map typedef's
        typedef std::map<uint32, LevelInfo*>                                                        LevelMap;
        typedef std::map<std::pair<uint32, uint32>, LevelMap*>                                      LevelInfoMap;

        typedef std::map<uint32, uint32>                                                            PetSpellCooldownMap;
        typedef std::multimap <uint32, uint32>                                                      BCEntryStorage;

        typedef std::unordered_map<uint32_t, std::vector<TrainerSpell>*> TrainerSpellSetContainer;

        Player* GetPlayer(const char* name, bool caseSensitive = true);
        Player* GetPlayer(uint32 guid);

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

        void AddGroup(Group* group);
        void RemoveGroup(Group* group);
        void LoadGroups();
        void loadGroupInstances();

        // player names
        void AddPlayerInfo(CachedCharacterInfo* pn);
        CachedCharacterInfo* GetPlayerInfo(uint32 guid);
        CachedCharacterInfo* GetPlayerInfoByName(std::string name);
        void RenamePlayerInfo(CachedCharacterInfo* pn, std::string oldname, std::string newname);
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
        void LoadCorpses(WorldMap* mgr);
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

#if VERSION_STRING > WotLK
        uint64_t generateVoidStorageItemId();
#endif

        Trainer* GetTrainer(uint32 Entry);

        LevelInfo* GetLevelInfo(uint32 Race, uint32 Class, uint32 Level);
        void GenerateLevelUpInfo();

        uint32 GetPetSpellCooldown(uint32 SpellId);
        void LoadPetSpellCooldowns();

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

        void loadCreatureMovementOverrides();
        void checkCreatureMovement(uint32_t id, CreatureMovementData& creatureMovement);

        CreatureMovementData const* getCreatureMovementOverride(uint32_t spawnId) const
        {
            auto itr = _creatureMovementOverrides.find(spawnId);
            if (itr != _creatureMovementOverrides.end())
                return &itr->second;
            return NULL;
        }

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

#ifdef FT_VEHICLES
        void LoadVehicleAccessories();
        VehicleAccessoryList const* getVehicleAccessories(Vehicle* vehicle);
        void loadVehicleSeatAddon();

        VehicleSeatAddon const* getVehicleSeatAddon(uint32 seatId) const
        {
            VehicleSeatAddonContainer::const_iterator itr = _vehicleSeatAddonStore.find(seatId);
            if (itr == _vehicleSeatAddonStore.end())
                return nullptr;

            return &itr->second;
        }
#endif

        void LoadWorldStateTemplates();
        std::multimap< uint32, WorldState >* GetWorldStatesForMap(uint32 map) const;

    private:

        EventScriptMaps mEventScriptMaps;
        SpellEffectMaps mSpellEffectMaps;
        DungeonEncounterContainer _dungeonEncounterStore;
        std::unordered_map<uint32_t, CreatureMovementData> _creatureMovementOverrides;
        std::unordered_map<uint32_t, CreatureDisplayInfoData> _creatureDisplayInfoData;

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
#if VERSION_STRING > WotLK
        std::atomic<unsigned long> m_voidItemId;
#endif
        std::mutex m_charterLock;

        ReputationModMap m_reputation_faction;
        ReputationModMap m_reputation_creature;
        std::unordered_map<uint32, InstanceReputationModifier*> m_reputation_instance;

        std::unordered_map<uint32, Charter*> m_charters[NUM_CHARTER_TYPES];

        std::set<uint32> m_disabled_spells;

        std::unordered_map<uint32, CachedCharacterInfo*> m_playersinfo;
        PlayerNameStringIndexMap m_playersInfoByName;

        std::unordered_map<uint32, TimedEmoteList*> m_timedemotes;       /// stored by spawnid

        // Group List
        std::mutex m_groupLock;
        GroupMap m_groups;

        /// Map of all vendor goods
        VendorMap mVendors;

        TrainerSpellSetContainer m_trainerSpellSet;
        TrainerMap mTrainers;
        LevelInfoMap mLevelInfo;
        PetSpellCooldownMap mPetSpellCooldowns;
#if VERSION_STRING > TBC
        AchievementCriteriaEntryList m_AchievementCriteriasByType[ACHIEVEMENT_CRITERIA_TYPE_TOTAL];
#endif
#if VERSION_STRING > WotLK
        AchievementCriteriaEntryList m_GuildAchievementCriteriasByType[ACHIEVEMENT_CRITERIA_TYPE_TOTAL];
#endif
#ifdef FT_VEHICLES
        VehicleAccessoryContainer _vehicleAccessoryStore;
        VehicleSeatAddonContainer _vehicleSeatAddonStore;
#endif
        std::map< uint32, std::multimap<uint32, WorldState>* > worldstate_templates;
};

#define sObjectMgr ObjectMgr::getInstance()

#endif // OBJECTMGR_H
