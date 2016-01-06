/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

#ifndef _OBJECTMGR_H
#define _OBJECTMGR_H

#include "Chat.h"
#include "Corpse.h"
#include "Player.h"
#include "Guild.h"
#include "DBC/DBCStructures.hpp"
#include "DBC/DBCStores.h"
#include "CreatureDefines.hpp"
#include "TransporterHandler.h"
#include "GameObject.h"
#include "Spell.h"

#include <string>

inline bool FindXinYString(std::string & x, std::string & y)
{
    return y.find(x) != std::string::npos;
}

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

#pragma pack(push,1)
struct FishingZoneEntry
{
    uint32 ZoneID;
    uint32 MinSkill;
    uint32 MaxSkill;
};

struct ZoneGuardEntry
{
    uint32 ZoneID;
    uint32 HordeEntry;
    uint32 AllianceEntry;
};

struct UnitModelSizeEntry
{
    uint32 DisplayId;
    float HalfSize;
    uint32 ModelId;
};

struct ProfessionDiscovery
{
    uint32 SpellId;
    uint32 SpellToDiscover;
    uint32 SkillValue;
    float Chance;
};

struct ItemPage
{
    uint32 id;
    char* text;
    uint32 next_page;
};

struct GossipMenuOption
{
    uint32 id;
    char* text;
};

struct WorldStringTable
{
    uint32 id;
    char* text;
};

struct CreatureText
{
    uint32 id;                  // unique id \todo remove this and use creature_entry + text_id as key
    char* text;                 
    uint32 creature_entry;      // creature entry ID
    uint32 text_id;             // text_id started with 0
    ChatMsg type;
    Languages language;
    float probability;          // chance/percent
    EmoteType emote;            // emote id on say
    uint32 duration;
    uint32 sound;               // the sound on say
    uint32 broadcast_id;        
};

struct WorldBroadCast
{
    uint32 id;
    char* text;
    uint32 percent;
};

#pragma pack(pop)

struct SpellReplacement
{
    uint32 count;
    uint32* spells;
};

class Group;

struct GossipMenuItem
{
    uint32 Id;
    uint32 IntId;
    uint8 Icon;
    uint8 Extra;
    std::string Text;
    uint32 m_gSender;
    uint32 m_gAction;
    std::string m_gBoxMessage;
    uint32 m_gBoxMoney;
};

struct PointOfInterest
{
    uint32 entry;
    float x;
    float y;
    uint32 icon;
    uint32 flags;
    uint32 data;
    const char* icon_name;
};

struct SpellEntry;
struct TrainerSpell
{
    SpellEntry* pCastSpell;
    SpellEntry* pLearnSpell;
    SpellEntry* pCastRealSpell;
    uint32 DeleteSpell;
    uint32 RequiredSpell;
    uint32 RequiredSkillLine;
    uint32 RequiredSkillLineValue;
    bool IsProfession;
    uint32 Cost;
    uint32 RequiredLevel;
};

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

class SERVER_DECL GossipMenu
{
    public:

        GossipMenu(uint64 Creature_Guid, uint32 Text_Id);
        void AddItem(GossipMenuItem* GossipItem);
        void AddItem(uint8 Icon, const char* Text, int32 Id = -1, int8 Extra = 0);
        void AddMenuItem(uint8 Icon, const char* Message, uint32 dtSender, uint32 dtAction, const char* BoxMessage, uint32 BoxMoney, bool Coded = false);
        void BuildPacket(WorldPacket & Packet);
        void SendTo(Player* Plr);
        void SendGossipMenu(uint32 TitleTextId, uint64 npcGUID);
        GossipMenuItem GetItem(uint32 Id);
        inline void SetTextID(uint32 TID) { TextId = TID; }

    protected:

        uint32 TextId;
        uint64 CreatureGuid;
        std::vector<GossipMenuItem> Menu;
};


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
        std::string UnkString;
        uint32 Data[7];
        uint16 Unk1;
        uint32 Unk2;
        uint32 Unk3;
        uint32 PetitionSignerCount;

        Charter(Field* fields);
        Charter(uint32 id, uint32 leader, uint32 type) : CharterType(type), LeaderGuid(leader), CharterId(id)
        {
            SignatureCount = 0;
            ItemGuid = 0;
            Slots = GetNumberOfSlotsByType();
            Signatures = new uint32[Slots];
            memset(Signatures, 0, sizeof(uint32)*Slots);
            Unk1 = 0;
            Unk2 = 0;
            Unk3 = 0;
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

typedef std::map<uint32, std::list<SpellEntry*>* >                  OverrideIdMap;
typedef std::unordered_map<uint32, Player*>                     PlayerStorageMap;
typedef std::list<GM_Ticket*>                                       GmTicketList;
typedef std::map<uint32, InstanceBossInfo*>                         InstanceBossInfoMap;
#ifdef ENABLE_ACHIEVEMENTS
typedef std::list<AchievementCriteriaEntry const*>                    AchievementCriteriaEntryList;
#endif

#ifndef WIN32
#define arcemu_USE_MAP_PLAYER_INDEX
#ifdef arcemu_USE_MAP_PLAYER_INDEX

/// you can use the string map (slower)
typedef std::map<std::string, PlayerInfo*> PlayerNameStringIndexMap;


#else            /// or

/// gcc has no default hash for string type,
/// so we have to make an explicit hash template here
template<>
struct __gnu_cxx::hash<std::string>
{
    size_t operator()(std::string & tbh) const
    {
        /// simple crc32 hash for now, we may need to change this later however
        return size_t(crc32((const unsigned char*)tbh.c_str(), tbh.length()));
    }
}

typedef std::unordered_map<std::string, PlayerInfo*> PlayerNameStringIndexMap;

#endif
#else

/// vc++ has the type for a string hash already, so we don't need to do anything special
typedef std::unordered_map<std::string, PlayerInfo*> PlayerNameStringIndexMap;

#endif


class PlayerCache;
class SERVER_DECL ObjectMgr : public Singleton < ObjectMgr >, public EventableObject
{
    public:

        ObjectMgr();
        ~ObjectMgr();

        void LoadCreatureWaypoints();
        void LoadCreatureTimedEmotes();

        TimedEmoteList* GetTimedEmoteList(uint32 spawnid);

        // other objects
        typedef std::tr1::unordered_map<uint32, AreaTrigger> AreaTriggerContainer;

        // Set typedef's
        typedef std::unordered_map<uint32, Group*>                        GroupMap;

        // HashMap typedef's
        typedef std::unordered_map<uint64, Item*>                       ItemMap;
        typedef std::unordered_map<uint32, CorpseData*>                 CorpseCollectorMap;
        typedef std::unordered_map<uint32, PlayerInfo*>                 PlayerNameMap;
        typedef std::unordered_map<uint32, PlayerCreateInfo*>           PlayerCreateInfoMap;
        typedef std::unordered_map<uint32, Guild*>                      GuildMap;
        typedef std::unordered_map<uint32, DBC::Structures::SkillLineAbilityEntry const*>             SLMap;
        typedef std::unordered_map<uint32, std::vector<CreatureItem>*>  VendorMap;
        typedef std::unordered_map<uint32, Transporter*>                TransportMap;
        typedef std::unordered_map<uint32, Trainer*>                    TrainerMap;
        typedef std::unordered_map<uint32, std::vector<TrainerSpell*> > TrainerSpellMap;
        typedef std::unordered_map<uint32, ReputationModifier*>         ReputationModMap;
        typedef std::unordered_map<uint32, Corpse*>                     CorpseMap;
        typedef std::unordered_map<uint32, PlayerCache*>                PlayerCacheMap;

        // Map typedef's
        typedef std::map<uint32, LevelInfo*>                                LevelMap;
        typedef std::map<std::pair<uint32, uint32>, LevelMap*>                  LevelInfoMap;
        typedef std::map<int32, std::list<ItemPrototype*>* >               ItemSetContentMap;
        typedef std::map<int32, uint32>               ItemSetDefinedContentMap;
        typedef std::map<uint32, uint32>                                    NpcToGossipTextMap;
        typedef std::map<uint32, std::set<SpellEntry*> >                         PetDefaultSpellMap;
        typedef std::map<uint32, uint32>                                    PetSpellCooldownMap;
        typedef std::multimap <uint32, uint32>                              BCEntryStorage;
        typedef std::map<uint32, SpellTargetConstraint*>                  SpellTargetConstraintMap;
        typedef std::map<std::pair<uint32, uint8>, CreatureProtoDifficulty*>               CreatureProtoDifficultyMap;

        // object holders
        GmTicketList GM_TicketList;
        OverrideIdMap mOverrideIdMap;
        InstanceBossInfoMap* m_InstanceBossInfoMap[NUM_MAPS];
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

        bool HasGroupedSetBonus(int32 itemset);
        uint32 GetGroupedSetBonus(int32 itemset);

        void GroupVoiceReconnected();

        void LoadGroups();

        // player names
        void AddPlayerInfo(PlayerInfo* pn);
        PlayerInfo* GetPlayerInfo(uint32 guid);
        PlayerInfo* GetPlayerInfoByName(const char* name);
        void RenamePlayerInfo(PlayerInfo* pn, const char* oldname, const char* newname);
        void DeletePlayerInfo(uint32 guid);
        PlayerCreateInfo* GetPlayerCreateInfo(uint8 race, uint8 class_) const;

        // Guild
        void AddGuild(Guild* pGuild);
        uint32 GetTotalGuildCount();
        bool RemoveGuild(uint32 guildId);
        Guild* GetGuild(uint32 guildId);
        Guild* GetGuildByLeaderGuid(uint64 leaderGuid);
        Guild* GetGuildByGuildName(std::string guildName);

        //Corpse Stuff
        Corpse* GetCorpseByOwner(uint32 ownerguid);
        void CorpseCollectorUnload();
        void DespawnCorpse(uint64 Guid);
        void CorpseAddEventDespawn(Corpse* pCorpse);
        void DelinkPlayerCorpses(Player* pOwner);
        Corpse* CreateCorpse();
        void AddCorpse(Corpse*);
        void RemoveCorpse(Corpse*);
        Corpse* GetCorpse(uint32 corpseguid);

        uint32 GetGossipTextForNpc(uint32 ID);

        // Gm Tickets
        void AddGMTicket(GM_Ticket* ticket, bool startup = false);
        void UpdateGMTicket(GM_Ticket* ticket);
        void RemoveGMTicketByPlayer(uint64 playerGuid);
        void RemoveGMTicket(uint64 ticketGuid);
        void DeleteGMTicketPermanently(uint64 ticketGuid);
        void DeleteAllRemovedGMTickets();
        GM_Ticket* GetGMTicket(uint64 ticketGuid);
        GM_Ticket* GetGMTicketByPlayer(uint64 playerGuid);
        //std::list<GM_Ticket*>* GetGMTicketsByPlayer(uint64 playerGuid);

        DBC::Structures::SkillLineAbilityEntry const* GetSpellSkill(uint32 id);
        SpellEntry* GetNextSpellRank(SpellEntry* sp, uint32 level);

        //Vendors
        std::vector<CreatureItem> *GetVendorList(uint32 entry);
        void SetVendorList(uint32 Entry, std::vector<CreatureItem>* list_);

        std::list<ItemPrototype*>* GetListForItemSet(int32 setid);

        Pet* CreatePet(uint32 entry);
        // This is a cataclysm feature
        //void ObjectMgr::CreateCharCreationPet(uint32 entry, uint32 ownerGUID);

        uint32 GenerateArenaTeamId();

        Player* CreatePlayer(uint8 _class);
        PlayerStorageMap _players;
        RWLock _playerslock;

        void AddPlayer(Player* p); //add it to global storage
        void RemovePlayer(Player* p);


        // Serialization
#ifdef ENABLE_ACHIEVEMENTS
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
        void LoadAreaTrigger();
        void LoadQuests();
        void LoadPlayersInfo();
        void LoadPlayerCreateInfo();
        void LoadGuilds();
        Corpse* LoadCorpse(uint32 guid);
        void LoadCorpses(MapMgr* mgr);
        void LoadGMTickets();
        void SaveGMTicket(GM_Ticket* ticket, QueryBuffer* buf);
        void LoadInstanceBossInfos();
        void LoadAuctions();
        void LoadAuctionItems();
        void LoadSpellSkills();
        void LoadVendors();
        void ReloadVendors();
        void LoadAIThreatToSpellId();
        void LoadSpellProcs();
        void LoadSpellEffectsOverride();
        void LoadReputationModifierTable(const char* tablename, ReputationModMap* dmap);
        void LoadReputationModifiers();
        ReputationModifier* GetReputationModifier(uint32 entry_id, uint32 faction_id);

        void SetHighestGuids();
        uint32 GenerateLowGuid(uint32 guidhigh);
        uint32 GenerateMailID();
        uint32 GenerateReportID();
        uint32 GenerateTicketID();
        uint32 GenerateEquipmentSetID();

        void LoadTransporters();
        void ProcessGameobjectQuests();
        void AddTransport(Transporter* pTransporter);

        void LoadTrainers();
        Trainer* GetTrainer(uint32 Entry);

        void LoadExtraItemStuff();
        void LoadExtraCreatureProtoStuff();
        void LoadExtraGameObjectStuff();
        void LoadProfessionDiscoveries();

        void StoreBroadCastGroupKey();

        void CreateGossipMenuForPlayer(GossipMenu** Location, uint64 Guid, uint32 TextID, Player* Plr);

        LevelInfo* GetLevelInfo(uint32 Race, uint32 Class, uint32 Level);
        void GenerateLevelUpInfo();
        void LoadXpToLevelTable();
        uint32 GetXPToLevel(uint32 level);
        void LoadDefaultPetSpells();
        std::set<SpellEntry*>* GetDefaultPetSpells(uint32 Entry);
        uint32 GetPetSpellCooldown(uint32 SpellId);
        void LoadPetSpellCooldowns();
        WayPointMap* GetWayPointMap(uint32 spawnid);
        void LoadSpellOverride();

        void ResetDailies();

        uint32 GenerateCreatureSpawnID();
        uint32 GenerateGameObjectSpawnID();

        Transporter* GetTransporter(uint32 guid);
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

        typedef std::unordered_map<uint32, NpcMonsterSay*> MonsterSayMap;
        MonsterSayMap mMonsterSays[NUM_MONSTER_SAY_EVENTS];

        NpcMonsterSay* HasMonsterSay(uint32 Entry, MONSTER_SAY_EVENTS Event);
        void LoadMonsterSay();

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


        inline GuildMap::iterator GetGuildsBegin() { return mGuild.begin(); }
        inline GuildMap::iterator GetGuildsEnd() { return mGuild.end(); }

        std::set<ProfessionDiscovery*> ProfessionDiscoveryTable;

        // cebernic: This is a perfect Broadcast system,multi-lang supported also.
        inline uint32 GetBCGroupCountByKey(uint32 Key) { return (uint32)m_BCEntryStorage.count(Key); }
        inline bool IsBCEntryStorageEmpty() { return m_BCEntryStorage.empty(); }
        inline BCEntryStorage::iterator GetBCTotalItemBegin() { return m_BCEntryStorage.begin(); }
        inline BCEntryStorage::iterator GetBCTotalItemEnd() { return m_BCEntryStorage.end(); }
        inline int CalcCurrentBCEntry()
        // func sync at MAKE_TASK(ObjectMgr, StoreBroadCastGroupKey)[world.cpp]
        {
            if (m_BCEntryStorage.empty())
                return -1;
            uint32 RandomCap = (uint32)sWorld.BCTriggerPercentCap;

            std::vector<uint32> Entries;
            BCEntryStorage::iterator it = m_BCEntryStorage.upper_bound(RandomUInt(RandomCap) + 1);
            while (it != m_BCEntryStorage.end())
            {
                Entries.push_back(it->second);
                ++it;
            }
            if (Entries.empty())
                return 0;
            uint32 n = (Entries.size() > 1 ? RandomUInt(((uint32)Entries.size()) - 1) : 0);
            return Entries[n];
        }

        AreaTrigger const* GetMapEntranceTrigger(uint32 Map) const;

#ifdef ENABLE_ACHIEVEMENTS
        void LoadAchievementCriteriaList();
        AchievementCriteriaEntryList const & GetAchievementCriteriaByType(AchievementCriteriaTypes type);
        std::set<uint32> allCompletedAchievements;
#endif

        void LoadVehicleAccessories();
        std::vector< VehicleAccessoryEntry* >* GetVehicleAccessories(uint32 creature_entry);
        void LoadWorldStateTemplates();
        std::multimap< uint32, WorldState >* GetWorldStatesForMap(uint32 map) const;

        void LoadItemsetLink();

        void LoadCreatureProtoDifficulty();
        CreatureProtoDifficulty* GetCreatureProtoDifficulty(uint32 creature_entry, uint32 difficulty_type);

    private:

        EventScriptMaps mEventScriptMaps;
        SpellEffectMaps mSpellEffectMaps;

    protected:

        BCEntryStorage m_BCEntryStorage;        /// broadcast system.
        RWLock playernamelock;

        // highest GUIDs, used for creating new objects
        Arcemu::Threading::AtomicCounter m_hiItemGuid;
        Arcemu::Threading::AtomicCounter m_hiGroupId;
        Arcemu::Threading::AtomicCounter m_hiCharterId;
        Arcemu::Threading::AtomicCounter m_hiCreatureSpawnId;
        Arcemu::Threading::AtomicCounter m_hiGameObjectSpawnId;
        Arcemu::Threading::AtomicCounter m_mailid;
        Arcemu::Threading::AtomicCounter m_reportID;
        Arcemu::Threading::AtomicCounter m_ticketid;
        Arcemu::Threading::AtomicCounter m_setGUID;
        Arcemu::Threading::AtomicCounter m_hiCorpseGuid;
        Arcemu::Threading::AtomicCounter m_hiGuildId;
        Arcemu::Threading::AtomicCounter m_hiPetGuid;
        Arcemu::Threading::AtomicCounter m_hiArenaTeamId;
        Arcemu::Threading::AtomicCounter m_hiPlayerGuid;

        RWLock m_charterLock;

        ReputationModMap m_reputation_faction;
        ReputationModMap m_reputation_creature;
        std::unordered_map<uint32, InstanceReputationModifier*> m_reputation_instance;

        std::unordered_map<uint32, Charter*> m_charters[NUM_CHARTER_TYPES];

        std::set<uint32> m_disabled_spells;

        uint64 TransportersCount;
        std::unordered_map<uint32, PlayerInfo*> m_playersinfo;
        PlayerNameStringIndexMap m_playersInfoByName;

        std::unordered_map<uint32, WayPointMap*> m_waypoints;           /// stored by spawnid
        std::unordered_map<uint32, TimedEmoteList*> m_timedemotes;      /// stored by spawnid

        AreaTriggerContainer _areaTriggerStore;

        ///// Object Tables ////
        // These tables are modified as creatures are created and destroyed in the world

        // Group List
        RWLock m_groupLock;
        GroupMap m_groups;

        /// Map of all starting infos needed for player creation
        PlayerCreateInfoMap mPlayerCreateInfo;

        /// DK: Map of all Guild's
        GuildMap mGuild;

        /// Map of all vendor goods
        VendorMap mVendors;

        /// Maps for Gossip stuff
        NpcToGossipTextMap mNpcToGossipText;

        SLMap mSpellSkills;

        /// Corpse Collector
        CorpseCollectorMap mCorpseCollector;

        TransportMap mTransports;

        ItemSetContentMap mItemSets;
        ItemSetDefinedContentMap mDefinedItemSets;

        TrainerMap mTrainers;
        LevelInfoMap mLevelInfo;
        PetDefaultSpellMap mDefaultPetSpells;
        PetSpellCooldownMap mPetSpellCooldowns;
        SpellTargetConstraintMap m_spelltargetconstraints;
        CreatureProtoDifficultyMap m_creatureProtoDifficulty;
#ifdef ENABLE_ACHIEVEMENTS
        AchievementCriteriaEntryList m_AchievementCriteriasByType[ACHIEVEMENT_CRITERIA_TYPE_TOTAL];
#endif
        std::map< uint32, std::vector<VehicleAccessoryEntry*>* > vehicle_accessories;
        std::map< uint32, std::multimap<uint32, WorldState>* > worldstate_templates;
        typedef std::vector<uint32> PlayerXPperLevel;
        PlayerXPperLevel _playerXPperLevel;
};

#define objmgr ObjectMgr::getSingleton()

#endif // _OBJECTMGR_H
