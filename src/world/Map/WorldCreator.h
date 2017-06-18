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
 */


#ifndef WORLDCREATOR_H
#define WORLDCREATOR_H

#include "WorldCreatorDefines.hpp"
#include "Management/Group.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"

extern const char* InstanceAbortMessages[];

class Map;
class MapMgr;

class Object;
class Group;
class Player;
class Battleground;

class SERVER_DECL FormationMgr : public Singleton < FormationMgr >
{
    std::map<uint32, Formation*> m_formations;

    public:

        typedef std::map<uint32, Formation*> FormationMap;
        FormationMgr();
        ~FormationMgr();

        Formation* GetFormation(uint32 sqlid)
        {
            FormationMap::iterator itr = m_formations.find(sqlid);
            return (itr == m_formations.end()) ? 0 : itr->second;
        }
};

class SERVER_DECL Instance
{
    public:

        uint32 m_instanceId;
        uint32 m_mapId;
        MapMgr* m_mapMgr;
        uint32 m_creatorGuid;
        uint32 m_creatorGroup;
        bool m_persistent;
        uint8 m_difficulty;
        std::set<uint32> m_killedNpcs;
        time_t m_creation;
        time_t m_expiration;
        MapInfo const* m_mapInfo;
        bool m_isBattleground;

        void LoadFromDB(Field* fields);
        void SaveToDB();
        void DeleteFromDB();
};

typedef std::unordered_map<uint32, Instance*> InstanceMap;

class SERVER_DECL InstanceMgr
{
    friend class MapMgr;

    public:

        InstanceMgr();
        ~InstanceMgr();

        inline Map* GetMap(uint32 mapid)
        {
            if (mapid >= NUM_MAPS)
                return NULL;
            else
                return m_maps[mapid];
        }

        uint32 PreTeleport(uint32 mapid, Player* plr, uint32 instanceid);
        MapMgr* GetInstance(Object* obj);
        uint32 GenerateInstanceID();

        void Load(TaskList* l);

        // deletes all instances owned by this player.
        void ResetSavedInstances(Player* plr);

        // deletes all instances owned by this group
        void OnGroupDestruction(Group* pGroup);

        // player left a group, boot him out of any instances he's not supposed to be in.
        void PlayerLeftGroup(Group* pGroup, Player* pPlayer);

        // has an instance expired?
        // can a player join?
        inline bool PlayerOwnsInstance(Instance* pInstance, Player* pPlayer)
        {
            // Expired?
            if (pInstance->m_expiration && (UNIXTIME + 20) >= pInstance->m_expiration)
            {
                _DeleteInstance(pInstance, true);
                return false;
            }

            // Persistent instance handling
            if (pInstance->m_persistent)
            {
                return (pPlayer->GetPersistentInstanceId(pInstance->m_mapId, pInstance->m_difficulty) == pInstance->m_instanceId);
            }
            // Default instance handling
            else if ((pPlayer->GetGroup() && pInstance->m_creatorGroup == pPlayer->GetGroup()->GetID()) || pPlayer->GetLowGUID() == pInstance->m_creatorGuid)
            {
                return true;
            }

            return false;
        }

        // has an instance expired?
        inline bool HasInstanceExpired(Instance* pInstance)
        {
            // expired?
            if (pInstance->m_expiration && (UNIXTIME + 20) >= pInstance->m_expiration)
                return true;

            return false;
        }

        // check for expired instances
        void CheckForExpiredInstances();

        // delete all instances
        void Shutdown();

        // packets, w000t! we all love packets!
        void BuildRaidSavedInstancesForPlayer(Player* plr);
        void BuildSavedInstancesForPlayer(Player* plr);
        MapMgr* CreateBattlegroundInstance(uint32 mapid);

        // Create an instance for Level 3 gm command
        MapMgr* CreateInstance(uint32 instanceType, uint32 mapid);

        // A (should be) safe way for scripts to delete an active instance
        void SafeDeleteInstance(MapMgr* mgr);

        // this only frees the instance pointer, not the mapmgr itself
        void DeleteBattlegroundInstance(uint32 mapid, uint32 instanceid);
        MapMgr* GetMapMgr(uint32 mapId);

        bool InstanceExists(uint32 mapid, uint32 instanceId)
        {
            return GetInstanceByIds(mapid, instanceId) != NULL;
        }

        Instance* GetInstanceByIds(uint32 mapid, uint32 instanceId)
        {
            if (mapid > NUM_MAPS)
                return NULL;
            if (mapid == NUM_MAPS)
            {
                Instance* in;
                for (uint32 i = 0; i < NUM_MAPS; ++i)
                {
                    in = GetInstanceByIds(i, instanceId);
                    if (in != NULL)
                        return in;
                }
                return NULL;
            }
            InstanceMap* map = m_instances[mapid];
            if (map == NULL)
                return NULL;
            InstanceMap::iterator instance = map->find(instanceId);
            return instance == map->end() ? NULL : instance->second;
        }

    private:

        void _LoadInstances();
        void _CreateMap(uint32 mapid);
        MapMgr* _CreateInstance(Instance* in);
        MapMgr* _CreateInstance(uint32 mapid, uint32 instanceid);        // only used on main maps!
        bool _DeleteInstance(Instance* in, bool ForcePlayersOut);

        uint32 m_InstanceHigh;

        Mutex m_mapLock;
        Map* m_maps[NUM_MAPS];
        InstanceMap* m_instances[NUM_MAPS];
        MapMgr* m_singleMaps[NUM_MAPS];
        time_t m_nextInstanceReset[NUM_MAPS];
};

extern SERVER_DECL InstanceMgr sInstanceMgr;

#endif      //WORLDCREATOR_H
