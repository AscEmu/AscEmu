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
 */

#ifndef OBJECTMGR_H
#define OBJECTMGR_H

#include "Chat/ChatHandler.hpp"
#include "Units/Creatures/Corpse.h"
#include "Units/Players/Player.h"
#include "Units/Creatures/Vehicle.h"
#include "Management/Guild.h"
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

typedef std::set<uint32> InstanceBossTrashList;
struct InstanceBossInfo
{
    uint32 mapid;
    uint32 creatureid;
    InstanceBossTrashList trash;
    uint32 trashRespawnOverride;
};

struct GM_Ticket
{
    uint64 guid;
    uint64 playerGuid;
    std::string name;
    uint32 level;
    uint32 map;
    float posX;
    float posY;
    float posZ;
    std::string message;
    uint32 timestamp;
    bool deleted;
    uint64 assignedToPlayer;
    std::string comment;
};

enum
{
    GM_TICKET_CHAT_OPCODE_NEWTICKET     = 1,
    GM_TICKET_CHAT_OPCODE_LISTSTART     = 2,
    GM_TICKET_CHAT_OPCODE_LISTENTRY     = 3,
    GM_TICKET_CHAT_OPCODE_CONTENT       = 4,
    GM_TICKET_CHAT_OPCODE_APPENDCONTENT = 5,
    GM_TICKET_CHAT_OPCODE_REMOVED       = 6,
    GM_TICKET_CHAT_OPCODE_UPDATED       = 7,
    GM_TICKET_CHAT_OPCODE_ASSIGNED      = 8,
    GM_TICKET_CHAT_OPCODE_RELEASED      = 9,
    GM_TICKET_CHAT_OPCODE_COMMENT       = 10,
    GM_TICKET_CHAT_OPCODE_ONLINESTATE   = 11
};

struct SpellReplacement
{
    uint32 count;
    uint32* spells;
};

class Group;
class SpellInfo;
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

struct LevelInfo
{
    uint32 HP;
    uint32 Mana;
    uint32 Stat[5];
};

struct ReputationMod
{
    uint32 faction[2];
    int32 value;
    uint32 replimit;
};

struct InstanceReputationMod
{
    uint32 mapid;
    uint32 mob_rep_reward;
    uint32 mob_rep_limit;
    uint32 boss_rep_reward;
    uint32 boss_rep_limit;
    uint32 faction[2];
};

struct ReputationModifier
{
    uint32 entry;
    std::vector<ReputationMod> mods;
};

struct InstanceReputationModifier
{
    uint32 mapid;
    std::vector<InstanceReputationMod> mods;
};

enum AREATABLE_FLAGS
{
    AREA_CITY_AREA          = 0x0020,
    AREA_NEUTRAL_AREA       = 0x0040,
    AREA_PVP_ARENA          = 0x0080,
    AREA_CITY               = 0x0200,
    AREA_SANCTUARY          = 0x0800,
    AREA_ISLAND             = 0x1000,
    AREA_PVP_OBJECTIVE_AREA = 0x8000
};

enum AREATABLE_CATEGORY
{
    AREAC_CONTESTED          = 0,
    AREAC_ALLIANCE_TERRITORY = 2,
    AREAC_HORDE_TERRITORY    = 4,
    AREAC_SANCTUARY          = 6
};

struct SimpleEventScript
{
    uint32 eventId;
    uint8 function;
    uint8 scripttype;
    uint32 data_1;
    uint32 data_2;
    uint32 data_3;
    uint32 data_4;
    uint32 data_5;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 o;
    uint32 delay;
    uint32 nextevent;
};

enum class ScriptCommands : uint8
{
    SCRIPT_COMMAND_TALK                 = 0,
    SCRIPT_COMMAND_EMOTE                = 1,
    SCRIPT_COMMAND_FIELD_SET            = 2,
    SCRIPT_COMMAND_MOVE_TO              = 3,
    SCRIPT_COMMAND_FLAG_SET             = 4,
    SCRIPT_COMMAND_FLAG_REMOVE          = 5,
    SCRIPT_COMMAND_TELEPORT_TO          = 6,
    SCRIPT_COMMAND_QUEST_EXPLORED       = 7,
    SCRIPT_COMMAND_KILL_CREDIT          = 8,       //Implemented (   data_1 (spellid), data_2 (quest id), data_3 (targettype 0 Creature/ 1 Gameobject), data_4 (target id), data_5 (killcredit), delay (when script needs to start ( in ms), next_event (next event_id when you want to add more )
    SCRIPT_COMMAND_RESPAWN_GAMEOBJECT   = 9,       //Implemented (   data_1 (GoId), data_2 (respawntime), delay (when script needs to start ( in ms), next_event (next event_id when you want to add more )
    SCRIPT_COMMAND_TEMP_SUMMON_CREATURE = 10,
    SCRIPT_COMMAND_OPEN_DOOR            = 11,
    SCRIPT_COMMAND_CLOSE_DOOR           = 12,
    SCRIPT_COMMAND_ACTIVATE_OBJECT      = 13,      // Implemented ( data_1 (Go id),  when dont wanna use get pos then type in x y z the coords, delay (when script needs to start ( in ms), next_event (next event_id when you want to add more )
    SCRIPT_COMMAND_REMOVE_AURA          = 14,
    SCRIPT_COMMAND_CAST_SPELL           = 15,
    SCRIPT_COMMAND_PLAY_SOUND           = 16,
    SCRIPT_COMMAND_CREATE_ITEM          = 17,
    SCRIPT_COMMAND_DESPAWN_SELF         = 18,
    SCRIPT_COMMAND_KILL                 = 19,
    SCRIPT_COMMAND_ORIENTATION          = 20,
    SCRIPT_COMMAND_EQUIP                = 21,
    SCRIPT_COMMAND_MODEL                = 22,
    SCRIPT_COMMAND_PLAYMOVIE            = 23
};

enum class EasyScriptTypes : uint8
{
    SCRIPT_TYPE_SPELL_EFFECT            = 1,
    SCRIPT_TYPE_GAMEOBJECT              = 2,
    SCRIPT_TYPE_CREATURE                = 3,
    SCRIPT_TYPE_PLAYER                  = 4,
    SCRIPT_TYPE_DUMMY                   = 5
};

typedef std::multimap<uint32, SimpleEventScript> EventScriptMaps;
typedef std::multimap<uint32, SimpleEventScript const*> SpellEffectMaps;
typedef std::pair<EventScriptMaps::const_iterator, EventScriptMaps::const_iterator> EventScriptBounds;
typedef std::pair<SpellEffectMaps::const_iterator, SpellEffectMaps::const_iterator> SpellEffectMapBounds;

//\TODO move it to seperated file!
class Charter
{
    public:

        uint32 GetNumberOfSlotsByType()
        {
            switch(CharterType)
            {
                case CHARTER_TYPE_GUILD:
                    return 9;

                case CHARTER_TYPE_ARENA_2V2:
                    return 1;

                case CHARTER_TYPE_ARENA_3V3:
                    return 2;

                case CHARTER_TYPE_ARENA_5V5:
                    return 4;

                default:
                    return 9;
            }
        }

        uint32 SignatureCount;
        uint32* Signatures;
        uint32 CharterType;
        uint32 Slots;
        uint32 LeaderGuid;
        uint64 ItemGuid;
        uint32 CharterId;
        std::string GuildName;

        /************************************************************************/
        /* Developer Fields                                                     */
        /************************************************************************/
        uint32 PetitionSignerCount;

        Charter(Field* fields);
        Charter(uint32 id, uint32 leader, uint32 type) : CharterType(type), LeaderGuid(leader), CharterId(id)
        {
            SignatureCount = 0;
            ItemGuid = 0;
            Slots = GetNumberOfSlotsByType();
            Signatures = new uint32[Slots];
            memset(Signatures, 0, sizeof(uint32)*Slots);
            PetitionSignerCount = 0;
        }

        ~Charter()
        {
            delete [] Signatures;
        }

        void SaveToDB();
        void Destroy();         // When item is deleted.

        void AddSignature(uint32 PlayerGuid);
        void RemoveSignature(uint32 PlayerGuid);

        inline uint32 GetLeader() { return LeaderGuid; }
        inline uint32 GetID() { return CharterId; }

        inline bool IsFull() { return (SignatureCount == Slots); }
};

typedef std::unordered_map<uint32, Player*> PlayerStorageMap;
typedef std::list<GM_Ticket*> GmTicketList;
typedef std::map<uint32, InstanceBossInfo*> InstanceBossInfoMap;

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


class PlayerCache;
class SERVER_DECL ObjectMgr : public Singleton < ObjectMgr >, public EventableObject
{
    public:
        //NIT
        void createGuardGossipMenuForPlayer(uint64_t senderGuid, uint32_t gossipMenuId, Player* player, uint32_t forcedTextId = 0);
        void createGuardGossipOptionAndSubMenu(uint64_t senderGuid, Player* player, uint32_t gossipItemId, uint32_t gossipMenuId);
        //NIT END

        ObjectMgr();
        ~ObjectMgr();

        void LoadCreatureWaypoints();
        void LoadCreatureTimedEmotes();

        TimedEmoteList* GetTimedEmoteList(uint32 spawnid);

        // Set typedef's
        typedef std::unordered_map<uint32, Group*>                      GroupMap;

        // HashMap typedef's
        typedef std::unordered_map<uint32, DBC::Structures::SkillLineAbilityEntry const*>             SLMap;
        typedef std::unordered_map<uint32, std::vector<CreatureItem>*>  VendorMap;
        typedef std::unordered_map<uint32, Trainer*>                    TrainerMap;
        typedef std::unordered_map<uint32, ReputationModifier*>         ReputationModMap;
        typedef std::unordered_map<uint32, Corpse*>                     CorpseMap;
        typedef std::unordered_map<uint32, PlayerCache*>                PlayerCacheMap;

        // Map typedef's
        typedef std::map<uint32, LevelInfo*>                            LevelMap;
        typedef std::map<std::pair<uint32, uint32>, LevelMap*>          LevelInfoMap;

        typedef std::map<uint32, uint32>                                PetSpellCooldownMap;
        typedef std::multimap <uint32, uint32>                          BCEntryStorage;
        typedef std::map<uint32, SpellTargetConstraint*>                SpellTargetConstraintMap;

        typedef std::unordered_map<uint32, Transporter*>                TransportMap;
        typedef std::set<Transporter*>                                  TransporterSet;
        typedef std::map<uint32, TransporterSet>                        TransporterMap;

        // object holders
        GmTicketList GM_TicketList;
        InstanceBossInfoMap* m_InstanceBossInfoMap[MAX_NUM_MAPS];
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
        Mutex _TransportLock;
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
            m_groupLock.AcquireWriteLock();
            m_groups.insert(std::make_pair(group->GetID(), group));
            m_groupLock.ReleaseWriteLock();
        }

        void RemoveGroup(Group* group)
        {
            m_groupLock.AcquireWriteLock();
            m_groups.erase(group->GetID());
            m_groupLock.ReleaseWriteLock();
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

        // Gm Tickets
        void AddGMTicket(GM_Ticket* ticket, bool startup = false);
        void UpdateGMTicket(GM_Ticket* ticket);
        void RemoveGMTicketByPlayer(uint64 playerGuid);
        void RemoveGMTicket(uint64 ticketGuid);
        void CloseTicket(uint64 ticketGuid);
        void DeleteGMTicketPermanently(uint64 ticketGuid);
        void DeleteAllRemovedGMTickets();
        GM_Ticket* GetGMTicket(uint64 ticketGuid);
        GM_Ticket* GetGMTicketByPlayer(uint64 playerGuid);

        DBC::Structures::SkillLineAbilityEntry const* GetSpellSkill(uint32 id);
        SpellInfo const* GetNextSpellRank(SpellInfo const* sp, uint32 level);

        //Vendors
        std::vector<CreatureItem> *GetVendorList(uint32 entry);
        void SetVendorList(uint32 Entry, std::vector<CreatureItem>* list_);

        Pet* CreatePet(uint32 entry);

        uint32 GenerateArenaTeamId();

        Player* CreatePlayer(uint8 _class);
        PlayerStorageMap _players;
        RWLock _playerslock;

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
        void LoadGMTickets();
        void SaveGMTicket(GM_Ticket* ticket, QueryBuffer* buf);
        void LoadInstanceBossInfos();
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
        uint32 GenerateTicketID();
        uint32 GenerateEquipmentSetID();

        /////////////////////////////////////////////////////////////////////////////////////////////
        /// Transport Handler                                                                     ///
        /////////////////////////////////////////////////////////////////////////////////////////////

        // Loads Transporters on Continents
        void LoadTransports();

        // Load Transport in Instance
        Transporter*LoadTransportInInstance(MapMgr *instance, uint32 goEntry, uint32 period);

        // Unloads Transporter from MapMgr
        void UnloadTransportFromInstance(Transporter *t);

        // Add Transporter
        void AddTransport(Transporter* transport);

        TransportMap m_Transports;

        TransporterSet m_Transporters;
        TransporterMap m_TransportersByMap;
        TransporterMap m_TransportersByInstanceIdMap;

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

        Transporter* GetTransporter(uint32 guid);
        Transporter* GetTransportOrThrow(uint32 guid);
        Transporter* GetTransporterByEntry(uint32 entry);

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

        ///////// Event Scripts ////////////////////
        void LoadEventScripts();
        EventScriptBounds GetEventScripts(uint32 event_id) const;
        SpellEffectMapBounds GetSpellEffectBounds(uint32 data_1) const;
        bool CheckforScripts(Player* plr, uint32 event_id);
        bool CheckforDummySpellScripts(Player* plr, uint32 data_1);
        void EventScriptsUpdate(Player* plr, uint32 next_event);
        ////////////////////////////////////////////

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
#if VERSION_STRING >= Cata
        SpellsRequiringSpellMap mSpellsReqSpell;
        SpellRequiredMap mSpellReq;
        SkillLineAbilityMap mSkillLineAbilityMap;
#endif

    protected:

        RWLock playernamelock;

        // highest GUIDs, used for creating new objects
        std::atomic<unsigned long> m_hiItemGuid;
        std::atomic<unsigned long> m_hiGroupId;
        std::atomic<unsigned long> m_hiCharterId;
        std::atomic<unsigned long> m_hiCreatureSpawnId;
        std::atomic<unsigned long> m_hiGameObjectSpawnId;
        std::atomic<unsigned long> m_mailid;
        std::atomic<unsigned long> m_reportID;
        std::atomic<unsigned long> m_ticketid;
        std::atomic<unsigned long> m_setGUID;
        std::atomic<unsigned long> m_hiCorpseGuid;
        std::atomic<unsigned long> m_hiGuildId;
        std::atomic<unsigned long> m_hiPetGuid;
        std::atomic<unsigned long> m_hiArenaTeamId;
        std::atomic<unsigned long> m_hiPlayerGuid;

        RWLock m_charterLock;

        ReputationModMap m_reputation_faction;
        ReputationModMap m_reputation_creature;
        std::unordered_map<uint32, InstanceReputationModifier*> m_reputation_instance;

        std::unordered_map<uint32, Charter*> m_charters[NUM_CHARTER_TYPES];

        std::set<uint32> m_disabled_spells;

        uint64 TransportersCount;
        std::unordered_map<uint32, PlayerInfo*> m_playersinfo;
        PlayerNameStringIndexMap m_playersInfoByName;

        std::unordered_map<uint32, Movement::WayPointMap*> mWayPointMap;           /// stored by spawnid
        std::unordered_map<uint32, TimedEmoteList*> m_timedemotes;      /// stored by spawnid


        // Group List
        RWLock m_groupLock;
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

#define objmgr ObjectMgr::getSingleton()

#endif // OBJECTMGR_H
