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

#include <string>

#include "AchievementMgr.h"
#include "Spell/Spell.h"
#include "Management/ArenaTeam.hpp"
#include "Management/Charter.hpp"
#include "Management/Group.h"
#include "Management/Tickets/TicketMgr.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Objects/Units/Creatures/Vehicle.hpp"
#include "Server/Script/SimpleEventScript.hpp"
#include "Storage/DBC/DBCStructures.hpp"

#if VERSION_STRING >= Cata
    #include "Storage/DB2/DB2Stores.h"
    #include "Storage/DB2/DB2Structures.h"
#endif

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

typedef std::list<std::shared_ptr<DungeonEncounter>> DungeonEncounterList;

//it seems trainerspells should be part of trainer files ;)
struct TrainerSpell
{
    TrainerSpell() : castSpell(nullptr), castRealSpell(nullptr), learnSpell(nullptr), deleteSpell(0),
        requiredLevel(0), requiredSkillLine(0), requiredSkillLineValue(0), isPrimaryProfession(false),
        cost(0), isStatic(0)
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
    uint32_t HP;
    uint32_t Mana;
    uint32_t Stat[5];
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
    std::vector<std::shared_ptr<ReputationMod>> mods;
};

// reputation/instance
struct InstanceReputationModifier
{
    uint32 mapid;
    std::vector<std::shared_ptr<InstanceReputationMod>> mods;
};

typedef std::unordered_map<uint32, Player*> PlayerStorageMap;

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

    //////////////////////////////////////////////////////////////////////////////////////////
    // Arena Team
    void loadArenaTeams();

    void addArenaTeam(std::shared_ptr<ArenaTeam> _arenaTeam);
    void removeArenaTeam(std::shared_ptr<ArenaTeam> _arenaTeam);

    std::shared_ptr<ArenaTeam> getArenaTeamByName(std::string& _name, uint32_t _type);
    std::shared_ptr<ArenaTeam> getArenaTeamById(uint32_t _id);
    std::shared_ptr<ArenaTeam> getArenaTeamByGuid(uint32_t _guid, uint32_t _type);

    void updateArenaTeamRankings();
    void updateArenaTeamWeekly();
    void resetArenaTeamRatings();

private:
    std::unordered_map<uint32_t, std::shared_ptr<ArenaTeam>> m_arenaTeams;
    std::unordered_map<uint32_t, std::shared_ptr<ArenaTeam>> m_arenaTeamMap[3];
    std::mutex m_arenaTeamLock;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Charter
public:
    void loadCharters();
    void removeCharter(const std::shared_ptr<Charter>&);
    std::shared_ptr<Charter> createCharter(uint32_t _leaderGuid, CharterTypes _type);

    std::shared_ptr<Charter> getCharterByName(const std::string& _charterName, CharterTypes _type);
    std::shared_ptr<Charter> getCharter(uint32_t _charterId, CharterTypes _type);
    std::shared_ptr<Charter> getCharterByGuid(uint64_t _playerguid, CharterTypes _type);
    std::shared_ptr<Charter> getCharterByItemGuid(uint64_t _guid);

private:
    std::unordered_map<uint32_t, std::shared_ptr<Charter>> m_charters[NUM_CHARTER_TYPES];
    std::mutex m_charterLock;

    //////////////////////////////////////////////////////////////////////////////////////////
    // CachedCharacterInfo
public:
    void loadCharacters();
    void addCachedCharacterInfo(const std::shared_ptr<CachedCharacterInfo>& _characterInfo);
    std::shared_ptr<CachedCharacterInfo> getCachedCharacterInfo(uint32_t _playerGuid);
    std::shared_ptr<CachedCharacterInfo> getCachedCharacterInfoByName(std::string _playerName);
    void updateCachedCharacterInfoName(const std::shared_ptr<CachedCharacterInfo>& _characterInfo, const std::string& _newName);
    void deleteCachedCharacterInfo(uint32_t _playerGuid);

private:
    std::unordered_map<uint32_t, std::shared_ptr<CachedCharacterInfo>> m_cachedCharacterInfo;
    std::mutex m_cachedCharacterLock;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Corpse
public:
    void loadCorpsesForInstance(WorldMap* _worldMap) const;
    std::shared_ptr<Corpse> loadCorpseByGuid(uint32_t _corpseGuid) const;
    std::shared_ptr<Corpse> createCorpse();

    void addCorpse(const std::shared_ptr<Corpse>&);
    void removeCorpse(const std::shared_ptr<Corpse>&);

    std::shared_ptr<Corpse> getCorpseByGuid(uint32_t _corpseGuid);
    std::shared_ptr<Corpse> getCorpseByOwner(uint32_t _playerGuid);

    void unloadCorpseCollector();
    void addCorpseDespawnTime(const std::shared_ptr<Corpse>& _corpse);
    void delinkCorpseForPlayer(const Player* _player);

private:
    std::unordered_map<uint32_t, std::shared_ptr<Corpse>> m_corpses;
    std::mutex m_corpseLock;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Vendors
public:
    void loadVendors();

    std::vector<CreatureItem>* getVendorList(uint32_t _entry);
    void setVendorList(uint32_t _entry, std::vector<CreatureItem>* _list);

private:
    std::unordered_map<uint32_t, std::vector<CreatureItem>*> m_vendors;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Achievement - global achievement information
#if VERSION_STRING > TBC
public:
    void loadAchievementCriteriaList();
    void loadAchievementRewards();
    void loadCompletedAchievements();

    AchievementReward const* getAchievementReward(uint32_t _entry, uint8_t _gender);

    AchievementCriteriaEntryList const& getAchievementCriteriaByType(AchievementCriteriaTypes _type);

    void addCompletedAchievement(uint32_t _achievementId);
    bool isInCompletedAchievements(uint32_t _achievementId);

private:
    AchievementRewardsMap m_achievementRewards;

    AchievementCriteriaEntryList m_AchievementCriteriasByType[ACHIEVEMENT_CRITERIA_TYPE_TOTAL];

#if VERSION_STRING > WotLK
    AchievementCriteriaEntryList m_GuildAchievementCriteriasByType[ACHIEVEMENT_CRITERIA_TYPE_TOTAL];
#endif

    std::set<uint32_t> m_allCompletedAchievements;
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Reputation Mods
public:
    typedef std::unordered_map<uint32_t, std::shared_ptr<ReputationModifier>> ReputationModMap;
    void loadReputationModifiers();
    void loadReputationModifierTable(const char* _tableName, ReputationModMap& _reputationModMap);
    void loadInstanceReputationModifiers();

    std::shared_ptr<ReputationModifier> getReputationModifier(uint32_t _entry, uint32_t _factionId);
    bool handleInstanceReputationModifiers(Player* _player, Unit* _unitVictim);

private:
    ReputationModMap m_reputationFaction;
    ReputationModMap m_reputationCreature;
    std::unordered_map<uint32_t, std::shared_ptr<InstanceReputationModifier>> m_reputationInstance;

//////////////////////////////////////////////////////////////////////////////////////////
// Group
public:
    void loadGroups();
    void loadGroupInstances();

    void addGroup(std::shared_ptr<Group> _group);
    void removeGroup(std::shared_ptr<Group> _group);

    std::shared_ptr<Group> getGroupByLeader(Player* _player);
    std::shared_ptr<Group> getGroupById(uint32_t _id);

private:
    std::mutex m_groupLock;
    std::unordered_map<uint32_t, std::shared_ptr<Group>> m_groups;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
public:
    void generateDatabaseGossipMenu(Object* _object, uint32_t _gossipMenuId, Player* _player, uint32_t _forcedTextId = 0);
    void generateDatabaseGossipOptionAndSubMenu(Object* _object, Player* _player, uint32_t _gossipItemId, uint32_t _gossipMenuId);

    void loadTrainerSpellSets();
    std::vector<TrainerSpell> getTrainerSpellSetById(uint32_t _id);

    void loadTrainers();
    std::shared_ptr<Trainer> getTrainer(uint32_t _entry);

    // Preload CreatureDisplayInfoStore and CreatureModelDataStore to avoid DBC lookup calls
    void loadCreatureDisplayInfo();
    CreatureDisplayInfoData const* getCreatureDisplayInfoData(uint32_t _displayId) const;

    Player* createPlayerByGuid(uint8_t _class, uint32_t _guid);

    GameObject* createGameObjectByGuid(uint32_t _id, uint32_t _guid);

    void loadInstanceEncounters();
    DungeonEncounterList const* getDungeonEncounterList(uint32_t _mapId, uint8_t _difficulty);

    void loadCreatureMovementOverrides();
    void checkCreatureMovement(uint32_t _id, CreatureMovementData& _creatureMovement);
    CreatureMovementData const* getCreatureMovementOverride(uint32_t _spawnId) const;

    void loadWorldStateTemplates();
    std::multimap<uint32_t, WorldState>* getWorldStatesForMap(uint32_t _map) const;

    void loadCreatureTimedEmotes();
    TimedEmoteList* getTimedEmoteList(uint32_t _spawnId);

    void generateLevelUpInfo();
    std::shared_ptr<LevelInfo> getLevelInfo(uint32_t _race, uint32_t _class, uint32_t _level);

private:
    std::unordered_map<uint32_t, std::vector<TrainerSpell>*> m_trainerSpellSet;

    std::unordered_map<uint32_t, std::shared_ptr<Trainer>> m_trainers;

    std::unordered_map<uint32_t, CreatureDisplayInfoData> m_creatureDisplayInfoData;

    std::unordered_map<uint32_t, DungeonEncounterList>  m_dungeonEncounterStore;

    std::unordered_map<uint32_t, CreatureMovementData> m_creatureMovementOverrides;

    std::map<uint32_t, std::multimap<uint32_t, WorldState>*> m_worldstateTemplates;

    std::unordered_map<uint32_t, TimedEmoteList*> m_timedEmotes;

    typedef std::map<uint32_t, std::shared_ptr<LevelInfo>> LevelMap;
    typedef std::map<std::pair<uint32_t, uint32_t>, std::shared_ptr<LevelMap>> LevelInfoMap;
    LevelInfoMap m_levelInfo;

public:
        typedef std::map<uint32, uint32>                                                            PetSpellCooldownMap;
        typedef std::multimap <uint32, uint32>                                                      BCEntryStorage;

        Player* GetPlayer(const char* name, bool caseSensitive = true);
        Player* GetPlayer(uint32 guid);

        Mutex m_creatureSetMutex;

        Item* CreateItem(uint32 entry, Player* owner);
        Item* LoadItem(uint32 lowguid);

        uint32 GenerateGroupId();
        uint32 GenerateGuildId();

        Pet* CreatePet(uint32 entry);

        uint32 GenerateArenaTeamId();

        Player* CreatePlayer(uint8 _class);
        PlayerStorageMap _players;
        std::mutex _playerslock;

        void AddPlayer(Player* p); //add it to global storage
        void RemovePlayer(Player* p);

        void SetHighestGuids();
        uint32 GenerateLowGuid(uint32 guidhigh);
        uint32 GenerateMailID();
        uint32 GenerateReportID();
        uint32 GenerateEquipmentSetID();

#if VERSION_STRING > WotLK
        uint64_t generateVoidStorageItemId();
#endif

        uint32 GetPetSpellCooldown(uint32 SpellId);
        void LoadPetSpellCooldowns();

        void ResetDailies();

        uint32 GenerateCreatureSpawnID();
        uint32 GenerateGameObjectSpawnID();

        //////////////////////////////////////////////////////////////////////////////////////////
        // Event Scripts
        void LoadEventScripts();
        EventScriptBounds GetEventScripts(uint32 event_id) const;
        SpellEffectMapBounds GetSpellEffectBounds(uint32 data_1) const;
        bool CheckforScripts(Player* plr, uint32 event_id);
        bool CheckforDummySpellScripts(Player* plr, uint32 data_1);
        void EventScriptsUpdate(Player* plr, uint32 next_event);
        //////////////////////////////////////////////////////////////////////////////////////////

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

    private:
        EventScriptMaps mEventScriptMaps;
        SpellEffectMaps mSpellEffectMaps;

    protected:
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

        PetSpellCooldownMap mPetSpellCooldowns;

#ifdef FT_VEHICLES
        VehicleAccessoryContainer _vehicleAccessoryStore;
        VehicleSeatAddonContainer _vehicleSeatAddonStore;
#endif
};

#define sObjectMgr ObjectMgr::getInstance()

#endif // OBJECTMGR_H
