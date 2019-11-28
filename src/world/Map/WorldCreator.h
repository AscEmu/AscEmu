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


#ifndef WORLDCREATOR_H
#define WORLDCREATOR_H

#include "WorldCreatorDefines.hpp"
#include "Management/Group.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Instance.h"

extern const char* InstanceAbortMessages[];

class Map;
class MapMgr;

class Object;
class Group;
class Player;
class Battleground;

class SERVER_DECL InstanceMgr
{
    friend class MapMgr;

    typedef std::unordered_map<uint32_t, Instance*> InstanceMap;

    public:

        InstanceMgr() {}
        ~InstanceMgr() {}

        Map* GetMap(uint32_t mapid);

        uint32_t PreTeleport(uint32_t mapid, Player* plr, uint32_t instanceid);
        MapMgr* GetInstance(Object* obj);

        void Load();

        void SaveInstanceToDB(Instance* instance);

        // deletes all instances owned by this player.
        void ResetSavedInstances(Player* plr);

        // deletes all instances owned by this group
        void OnGroupDestruction(Group* pGroup);

        // player left a group, boot him out of any instances he's not supposed to be in.
        void PlayerLeftGroup(Group* pGroup, Player* pPlayer);

        // has an instance expired?
        // can a player join?
        bool PlayerOwnsInstance(Instance* pInstance, Player* pPlayer);

        // has an instance expired?
        bool HasInstanceExpired(Instance* pInstance);

        // check for expired instances
        void CheckForExpiredInstances();

        // delete all instances
        void Shutdown();

        // packets, w000t! we all love packets!
        void BuildRaidSavedInstancesForPlayer(Player* plr);
        void BuildSavedInstancesForPlayer(Player* plr);
        MapMgr* CreateBattlegroundInstance(uint32_t mapid);

        // Create an instance for Level 3 gm command
        MapMgr* CreateInstance(uint32_t instanceType, uint32_t mapid);

        // A (should be) safe way for scripts to delete an active instance
        void SafeDeleteInstance(MapMgr* mgr);

        // this only frees the instance pointer, not the mapmgr itself
        void DeleteBattlegroundInstance(uint32_t mapid, uint32_t instanceid);
        MapMgr* GetMapMgr(uint32_t mapId);

        bool InstanceExists(uint32_t mapid, uint32_t instanceId);

        Instance* GetInstanceByIds(uint32_t mapid, uint32_t instanceId);

    private:

        void _CreateMap(uint32_t mapid);
        MapMgr* _CreateInstance(Instance* in);
        MapMgr* _CreateInstance(uint32_t mapid, uint32_t instanceid);        // only used on main maps!
        bool _DeleteInstance(Instance* in, bool ForcePlayersOut);

        Mutex m_mapLock;

        //\brief: rewrite the stuff above. 1) data handling. 2) clear packet handling. 3) helper funtions. 4) optimization.
        //MIT starts

        uint32_t m_InstanceHigh {0};

        Map* m_maps[MAX_NUM_MAPS] {nullptr};
        InstanceMap* m_instances[MAX_NUM_MAPS] {nullptr};
        MapMgr* m_singleMaps[MAX_NUM_MAPS] {nullptr};
        time_t m_nextInstanceReset[MAX_NUM_MAPS] {0};

public:

    void generateInstances();
    void loadInstanceResetTimes();
    void deleteExpiredAndInvalidInstances();
    void loadAndApplySavedInstanceValues();

    uint32_t getNextInstanceId();
    //MIT ends
};

extern SERVER_DECL InstanceMgr sInstanceMgr;

#endif      //WORLDCREATOR_H
