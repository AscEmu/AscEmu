/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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

#ifndef BATTLEGROUNDMGR_H
#define BATTLEGROUNDMGR_H

#include "WorldPacket.h"
#include "Server/EventableObject.h"

#define ANTI_CHEAT

const uint32_t BG_SCORE_AB_BASES_ASSAULTED        = 0;
const uint32_t BG_SCORE_AB_BASES_CAPTURED         = 1;
const uint32_t BG_SCORE_AV_GRAVEYARDS_ASSAULTED   = 0;
const uint32_t BG_SCORE_AV_GRAVEYARDS_DEFENDED    = 1;
const uint32_t BG_SCORE_AV_TOWERS_ASSAULTED       = 2;
const uint32_t BG_SCORE_AV_TOWERS_DEFENDED        = 3;
const uint32_t BG_SCORE_AV_MINES_CAPTURES         = 4;
const uint32_t BG_SCORE_EOTS_FLAGS_CAPTURED       = 0;
const uint32_t BG_SCORE_WSG_FLAGS_CAPTURED        = 0;
const uint32_t BG_SCORE_WSG_FLAGS_RETURNED        = 1;
const uint32_t BG_SCORE_IOC_BASES_ASSAULTED       = 0;
const uint32_t BG_SCORE_IOC_BASES_DEFENDED        = 1;

const uint32_t SOUND_BATTLEGROUND_BEGIN           = 3439;
const uint32_t SOUND_FLAG_RESPAWN                 = 8232;
const uint32_t SOUND_HORDE_SCORES                 = 8213;
const uint32_t SOUND_ALLIANCE_SCORES              = 8173;
const uint32_t SOUND_ALLIANCE_CAPTURE             = 8174;
const uint32_t SOUND_HORDE_CAPTURE                = 8212;
const uint32_t SOUND_FLAG_RETURNED                = 8192;
const uint32_t SOUND_HORDEWINS                    = 8454;
const uint32_t SOUND_ALLIANCEWINS                 = 8455;
const uint32_t SOUND_HORDE_BGALMOSTEND            = 8456;
const uint32_t SOUND_ALLIANCE_BGALMOSTEND         = 8457;

const uint32_t BG_PREPARATION                     = 44521;
const uint32_t BG_REVIVE_PREPARATION              = 44535;
const uint32_t RESURRECT_SPELL                    = 21074; // Spirit Healer Res
const uint32_t BG_DESERTER                        = 26013;

class CBattleground;
class MapMgr;
class Player;
class Map;
class Group;
/// AV - Corpse
class Corpse;


enum BattlegroundDbcIndex
{
    BGDBC_ALTERAC_VALLEY = 1,
    BGDBC_WARSONG_GULCH = 2,
    BGDBC_ARATHI_BASIN = 3,
    BGDBC_ARENA_NAGRAND = 4,
    BGDBC_ARENA_BLADES_EDGE = 5,
    BGDBC_ARENA_ALLMAPS = 6,
    BGDBC_EYE_OF_THE_STORM = 7,
    BGDBC_RUINS_OF_LORDAERON = 8,
    BGDBC_STRAND_OF_THE_ANCIENT = 9,
    BGDBC_DALARAN_SEWERS = 10,
    BGDBC_RING_OF_VALOR = 11,
    BGDBC_ISLE_OF_CONQUEST = 30,
    BGDBC_ROWS = 30,
};

//////////////////////////////////////////////////////////////////////////////////////////
// Values based on BattlemasterList.dbc (BATTLEGROUND_NUM_TYPES = Max entry's +1)

#if VERSION_STRING != Classic
    #define BATTLEGROUND_NUM_TYPES 8
#elif VERSION_STRING != TBC
    #define BATTLEGROUND_NUM_TYPES 8
#elif VERSION_STRING != WotLK
    #define BATTLEGROUND_NUM_TYPES 33
#elif VERSION_STRING != Cata
    #define BATTLEGROUND_NUM_TYPES 657
#elif VERSION_STRING != Mop
    #define BATTLEGROUND_NUM_TYPES 758
#endif

enum BattleGroundTypes
{
    BATTLEGROUND_ALTERAC_VALLEY = 1,
    BATTLEGROUND_WARSONG_GULCH = 2,
    BATTLEGROUND_ARATHI_BASIN = 3,
    BATTLEGROUND_ARENA_2V2 = 4,
    BATTLEGROUND_ARENA_3V3 = 5,
    BATTLEGROUND_ARENA_5V5 = 6,
    BATTLEGROUND_EYE_OF_THE_STORM = 7,
	// WOTLK
    BATTLEGROUND_STRAND_OF_THE_ANCIENT = 9,
    BATTLEGROUND_ISLE_OF_CONQUEST = 30,
	BATTLEGROUND_RANDOM = 32,
    // Cataclysm
    BATTLEGROUND_RATED_10 = 100,
    BATTLEGROUND_RATED_15 = 101,
    BATTLEGROUND_RATED_5 = 102,
    BATTLEGROUND_TWIN_PEAKS = 108,
    BATTLEGROUND_BATTLE_FOR_GILNEAS_CITY = 120,
    BATTLEGROUND_ICECROWN_CITADEL = 441, 
    BATTLEGROUND_RUBY_SANCTUM = 443,
    BATTLEGROUND_EYE_OF_THE_STORM_RATED = 656,
    // MOP
    BATTLEGROUND_TEMPLE_OF_KOTMOGU = 699,
    BATTLEGROUND_SILVERSHARD_MINES = 708,
    BATTLEGROUND_TOL_VIRON_ARENA = 719,
    BATTLEGROUND_DEEPWIND_GORGE = 754,
    BATTLEGROUND_TIGERS_PEAK = 757,
};

inline bool isArena(uint32_t x)
{
    return (x >= BATTLEGROUND_ARENA_2V2 && x <= BATTLEGROUND_ARENA_5V5);
}

enum BattleGroundMasterTypes
{
    BGMASTER_CREATURE = 1,
    BGMASTER_OBJECT = 2,
    BGMASTER_ITEM = 3
};

enum BattleGroundStatus
{
    BGSTATUS_NOFLAGS = 0, // wtfbbq, why aren't there any flags?
    BGSTATUS_INQUEUE = 1, // Battleground has a queue, player is now in queue
    BGSTATUS_READY = 2,   // Battleground is ready to join
    BGSTATUS_TIME = 3     // Ex. Wintergrasp time remaining
};

struct BGScore
{
    uint32_t KillingBlows;
    uint32_t HonorableKills;
    uint32_t Deaths;
    uint32_t BonusHonor;
    uint32_t DamageDone;
    uint32_t HealingDone;
    uint32_t MiscData[5];

    BGScore()
    {
        KillingBlows = 0;
        HonorableKills = 0;
        Deaths = 0;
        BonusHonor = 0;
        DamageDone = 0;
        HealingDone = 0;
        std::fill(&MiscData[0], &MiscData[5], 0);
    }
};

// get level grouping for player
static inline uint32_t GetLevelGrouping(uint32_t level)
{
    if (level < 10)
        return 0;
    else if (level < 20)
        return 1;
    else if (level < 30)
        return 2;
    else if (level < 40)
        return 3;
    else if (level < 50)
        return 4;
    else if (level < 60)
        return 5;
    else if (level < 70)
        return 6;
    else if (level < 80)
        return 7;
    else
        return 8;
}

static inline uint32_t GetFieldCount(uint32_t BGType)
{
    switch (BGType)
    {
        case BATTLEGROUND_ALTERAC_VALLEY:
            return 5;
        case BATTLEGROUND_ARATHI_BASIN:
        case BATTLEGROUND_WARSONG_GULCH:
        case BATTLEGROUND_STRAND_OF_THE_ANCIENT:
        case BATTLEGROUND_ISLE_OF_CONQUEST:
            return 2;
        case BATTLEGROUND_EYE_OF_THE_STORM:
            return 1;
        default:
            return 0;
    }
}

#define MAX_LEVEL_GROUP 9
#define MINIMUM_PLAYERS_ON_EACH_SIDE_FOR_BG 1
#define MAXIMUM_BATTLEGROUNDS_PER_LEVEL_GROUP 50
#define LEVEL_GROUP_70 8

class Arena;

typedef CBattleground* (*BattlegroundFactoryMethod)(MapMgr* mgr, uint32_t iid, uint32_t group, uint32_t type);
typedef CBattleground* (*ArenaFactoryMethod)(MapMgr* mgr, uint32_t iid, uint32_t group, uint32_t type, uint32_t players_per_side);


class SERVER_DECL CBattlegroundManager : public EventableObject
{
    // Battleground Instance Map
    std::map<uint32_t, CBattleground*> m_instances[BATTLEGROUND_NUM_TYPES];
    Mutex m_instanceLock;

    // Max Id
    uint32_t m_maxBattlegroundId[BATTLEGROUND_NUM_TYPES];

    // Queue System
    // Instance Id -> list<Player guid> [ BattlegroundType ] (instance 0 - first available)
    std::list<uint32_t> m_queuedPlayers[BATTLEGROUND_NUM_TYPES][MAX_LEVEL_GROUP];

    // Instance Id -> list<Group id> [BattlegroundType][LevelGroup]
    std::list<uint32_t> m_queuedGroups[BATTLEGROUND_NUM_TYPES];

    Mutex m_queueLock;

    // Bg factory methods by Bg map Id
    std::map<uint32_t, BattlegroundFactoryMethod> bgFactories;

    // Arena factory methods
    std::vector<ArenaFactoryMethod> arenaFactories;

    // Bg map IDs by Bg type Id
    std::map<uint32_t, uint32_t> bgMaps;

    // Arena map IDs
    std::vector<uint32_t> arenaMaps;

    // All battlegrounds that are available in random BG queue
    std::vector<uint32_t> avalibleInRandom;

    private:

        CBattlegroundManager() = default;
        ~CBattlegroundManager() = default;

    public:

        static CBattlegroundManager& getInstance();
        void initialize();

        CBattlegroundManager(CBattlegroundManager&&) = delete;
        CBattlegroundManager(CBattlegroundManager const&) = delete;
        CBattlegroundManager& operator=(CBattlegroundManager&&) = delete;
        CBattlegroundManager& operator=(CBattlegroundManager const&) = delete;

        //////////////////////////////////////////////////////////////////////////////////////////
        // void RegisterBgFactory(uint32_t map, BattlegroundFactoryMethod method)
        // \note   Registers the specified Battleground class factory method for
        //         the specified Battleground type.
        //         When trying to register a duplicate, the duplicate will be ignored.
        //
        // \param  uint32_t map                          -  The map of the Battleground
        // \param  BattlegroundFactoryMethod method    -  The Battleground factory method
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void RegisterBgFactory(uint32_t map, BattlegroundFactoryMethod method);


        //////////////////////////////////////////////////////////////////////////////////////////
        // void RegisterArenaFactory(uint32_t map, ArenaFactoryMethod method)
        // \note   Registers the specified Arena class factory method for
        //         the specified Battleground type.
        //         When trying to register a duplicate, the duplicate will be ignored.
        //
        // \param  uint32_t map                  -  Map id
        // \param  ArenaFactoryMethod method   -  The Arena factory method
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void RegisterArenaFactory(uint32_t map, ArenaFactoryMethod method);


        //////////////////////////////////////////////////////////////////////////////////////////
        // void RegisterMapForBgType(uint32_t type, uint32_t map)
        // \note   Registers a Map Id for the specified Battleground type.
        //         When trying to register a duplicate, the duplicate will be ignored.
        //
        // \param  uint32_t type  -  The Battleground type
        // \param  uint32_t map   -  The map Id
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void RegisterMapForBgType(uint32_t type, uint32_t map);


        void HandleBattlegroundListPacket(WorldSession* m_session, uint32_t BattlegroundType, uint8_t from = 0);
        void HandleArenaJoin(WorldSession* m_session, uint32_t BattlegroundType, uint8_t as_group, uint8_t rated_match);

        void OnPlayerLogout(Player* plr);

        void EventQueueUpdate();
        void EventQueueUpdate(bool forceStart);

        void HandleGetBattlegroundQueueCommand(WorldSession* m_session);

        void HandleBattlegroundJoin(WorldSession* m_session, WorldPacket& pck);

        void RemovePlayerFromQueues(Player* plr);
        void RemoveGroupFromQueues(Group* grp);

        CBattleground* CreateInstance(uint32_t Type, uint32_t LevelGroup);

        bool CanCreateInstance(uint32_t Type, uint32_t LevelGroup);

        void DeleteBattleground(CBattleground* bg);

        void SendBattlefieldStatus(Player* plr, BattleGroundStatus Status, uint32_t Type, uint32_t InstanceID, uint32_t Time, uint32_t MapId, uint8_t RatedMatch);

        uint32_t GetArenaGroupQInfo(Group* group, int type, uint32_t* avgRating);

        int CreateArenaType(int type, Group* group1, Group* group2);

        void AddPlayerToBgTeam(CBattleground* bg, std::deque<uint32_t> *playerVec, uint32_t i, uint32_t j, int Team);

        void AddPlayerToBg(CBattleground* bg, std::deque<uint32_t> *playerVec, uint32_t i, uint32_t j);

        void AddGroupToArena(CBattleground* bg, Group* group, uint32_t nteam);

        uint32_t GetMinimumPlayers(uint32_t dbcIndex);

        uint32_t GetMaximumPlayers(uint32_t dbcIndex);
};

#define sBattlegroundManager CBattlegroundManager::getInstance()

#endif // BATTLEGROUNDMGR_H
