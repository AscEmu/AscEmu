/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
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
#include "BattlegroundDefines.hpp"

class CBattleground;
class Player;
class Group;
class Corpse; // AV
class Arena;

typedef CBattleground* (*BattlegroundFactoryMethod)(BattlegroundMap* mgr, uint32 iid, uint32 group, uint32 type);
typedef CBattleground* (*ArenaFactoryMethod)(BattlegroundMap* mgr, uint32 iid, uint32 group, uint32 type, uint32 players_per_side);


class SERVER_DECL CBattlegroundManager : public EventableObject
{
    // Battleground Instance Map
    std::map<uint32, CBattleground*> m_instances[BATTLEGROUND_NUM_TYPES];
    Mutex m_instanceLock;

    // Max Id
    uint32 m_maxBattlegroundId[BATTLEGROUND_NUM_TYPES];

    // Queue System
    // Instance Id -> list<Player guid> [ BattlegroundType ] (instance 0 - first available)
    std::list<uint32> m_queuedPlayers[BATTLEGROUND_NUM_TYPES][BattlegroundDef::MAX_LEVEL_GROUP];

    // Instance Id -> list<Group id> [BattlegroundType][LevelGroup]
    std::list<uint32> m_queuedGroups[BATTLEGROUND_NUM_TYPES];

    Mutex m_queueLock;

    // Bg factory methods by Bg map Id
    std::map<uint32, BattlegroundFactoryMethod> bgFactories;

    // Arena factory methods
    std::vector<ArenaFactoryMethod> arenaFactories;

    // Bg map IDs by Bg type Id
    std::map<uint32, uint32> bgMaps;

    // Arena map IDs
    std::vector<uint32> arenaMaps;

    // All battlegrounds that are available in random BG queue
    std::vector<uint32> avalibleInRandom;

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
        // void RegisterBgFactory(uint32 map, BattlegroundFactoryMethod method)
        // \note   Registers the specified Battleground class factory method for
        //         the specified Battleground type.
        //         When trying to register a duplicate, the duplicate will be ignored.
        //
        // \param  uint32 map                          -  The map of the Battleground
        // \param  BattlegroundFactoryMethod method    -  The Battleground factory method
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void RegisterBgFactory(uint32 map, BattlegroundFactoryMethod method);


        //////////////////////////////////////////////////////////////////////////////////////////
        // void RegisterArenaFactory(uint32 map, ArenaFactoryMethod method)
        // \note   Registers the specified Arena class factory method for
        //         the specified Battleground type.
        //         When trying to register a duplicate, the duplicate will be ignored.
        //
        // \param  uint32 map                  -  Map id
        // \param  ArenaFactoryMethod method   -  The Arena factory method
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void RegisterArenaFactory(uint32 map, ArenaFactoryMethod method);


        //////////////////////////////////////////////////////////////////////////////////////////
        // void RegisterMapForBgType(uint32 type, uint32 map)
        // \note   Registers a Map Id for the specified Battleground type.
        //         When trying to register a duplicate, the duplicate will be ignored.
        //
        // \param  uint32 type  -  The Battleground type
        // \param  uint32 map   -  The map Id
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void RegisterMapForBgType(uint32 type, uint32 map);


        void HandleBattlegroundListPacket(WorldSession* m_session, uint32 BattlegroundType, uint8 from = 0);
        void HandleArenaJoin(WorldSession* m_session, uint32 BattlegroundType, uint8 as_group, uint8 rated_match);

        void OnPlayerLogout(Player* plr);

        void EventQueueUpdate();
        void EventQueueUpdate(bool forceStart);

        void HandleGetBattlegroundQueueCommand(WorldSession* m_session);

        void HandleBattlegroundJoin(WorldSession* m_session, WorldPacket& pck);

        void RemovePlayerFromQueues(Player* plr);
        void RemoveGroupFromQueues(Group* grp);

        CBattleground* CreateInstance(uint32 Type, uint32 LevelGroup);

        bool CanCreateInstance(uint32 Type, uint32 LevelGroup);

        void DeleteBattleground(CBattleground* bg);

        void SendBattlefieldStatus(Player* plr, BattlegroundDef::Status Status, uint32 Type, uint32 InstanceID, uint32 Time, uint32 MapId, uint8 RatedMatch);

        uint32 GetArenaGroupQInfo(Group* group, uint8_t type, uint32* avgRating);

        int CreateArenaType(int type, Group* group1, Group* group2);

        void AddPlayerToBgTeam(CBattleground* bg, std::deque<uint32> *playerVec, uint32 i, uint32 j, int Team);

        void AddPlayerToBg(CBattleground* bg, std::deque<uint32> *playerVec, uint32 i, uint32 j);

        void AddGroupToArena(CBattleground* bg, Group* group, uint32 nteam);

        uint32 GetMinimumPlayers(uint32 dbcIndex);

        uint32 GetMaximumPlayers(uint32 dbcIndex);
};

#define sBattlegroundManager CBattlegroundManager::getInstance()

#endif // BATTLEGROUNDMGR_H
