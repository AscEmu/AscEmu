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

extern const char* InstanceAbortMessages[];

class Map;
class MapMgr;

class Object;
class Group;
class Player;
class Battleground;

class SERVER_DECL Instance
{
    public:

        uint32_t m_instanceId;
        uint32_t m_mapId;
        MapMgr* m_mapMgr;
        uint32_t m_creatorGuid;
        uint32_t m_creatorGroup;
        bool m_persistent;
        uint8_t m_difficulty;
        std::set<uint32_t> m_killedNpcs;
        time_t m_creation;
        time_t m_expiration;
        MySQLStructure::MapInfo const* m_mapInfo;
        bool m_isBattleground;

        void LoadFromDB(Field* fields);
        void SaveToDB();
        void DeleteFromDB();

        //MIT
        bool isPersistent() const;
        bool isResetable() const;
};

typedef std::unordered_map<uint32_t, Instance*> InstanceMap;

class SERVER_DECL InstanceMgr
{
    friend class MapMgr;

    public:

        InstanceMgr() {}
        ~InstanceMgr() {}

        Map* GetMap(uint32_t mapid)
        {
            if (mapid >= MAX_NUM_MAPS)
                return nullptr;

            return m_maps[mapid];
        }

        uint32_t PreTeleport(uint32_t mapid, Player* plr, uint32_t instanceid);
        MapMgr* GetInstance(Object* obj);
        uint32_t GenerateInstanceID();

        void Load(TaskList* l);

        // deletes all instances owned by this player.
        void ResetSavedInstances(Player* plr);

        // deletes all instances owned by this group
        void OnGroupDestruction(Group* pGroup);

        // player left a group, boot him out of any instances he's not supposed to be in.
        void PlayerLeftGroup(Group* pGroup, Player* pPlayer);

        // has an instance expired?
        // can a player join?
        bool PlayerOwnsInstance(Instance* pInstance, Player* pPlayer)
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
            if ((pPlayer->GetGroup() && pInstance->m_creatorGroup == pPlayer->GetGroup()->GetID()) || pPlayer->getGuidLow() == pInstance->m_creatorGuid)
            {
                return true;
            }

            return false;
        }

        // has an instance expired?
        bool HasInstanceExpired(Instance* pInstance)
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
        MapMgr* CreateBattlegroundInstance(uint32_t mapid);

        // Create an instance for Level 3 gm command
        MapMgr* CreateInstance(uint32_t instanceType, uint32_t mapid);

        // A (should be) safe way for scripts to delete an active instance
        void SafeDeleteInstance(MapMgr* mgr);

        // this only frees the instance pointer, not the mapmgr itself
        void DeleteBattlegroundInstance(uint32_t mapid, uint32_t instanceid);
        MapMgr* GetMapMgr(uint32_t mapId);

        bool InstanceExists(uint32_t mapid, uint32_t instanceId)
        {
            return GetInstanceByIds(mapid, instanceId) != nullptr;
        }

        Instance* GetInstanceByIds(uint32_t mapid, uint32_t instanceId)
        {
            if (mapid > MAX_NUM_MAPS)
                return nullptr;

            if (mapid == MAX_NUM_MAPS)
            {
                for (uint32_t i = 0; i < MAX_NUM_MAPS; ++i)
                {
                    Instance* in = GetInstanceByIds(i, instanceId);
                    if (in != nullptr)
                        return in;
                }

                return nullptr;
            }

            InstanceMap* map = m_instances[mapid];
            if (map == nullptr)
                return nullptr;

            auto instance = map->find(instanceId);
            return instance == map->end() ? nullptr : instance->second;
        }

    private:

        void _LoadInstances();
        void _CreateMap(uint32_t mapid);
        MapMgr* _CreateInstance(Instance* in);
        MapMgr* _CreateInstance(uint32_t mapid, uint32_t instanceid);        // only used on main maps!
        bool _DeleteInstance(Instance* in, bool ForcePlayersOut);

        uint32_t m_InstanceHigh = 0;
        Map* m_maps[MAX_NUM_MAPS] = { nullptr };
        InstanceMap* m_instances[MAX_NUM_MAPS] = { nullptr };
        MapMgr* m_singleMaps[MAX_NUM_MAPS] = { nullptr };
        time_t m_nextInstanceReset[MAX_NUM_MAPS] = { 0 };

        Mutex m_mapLock;
};

extern SERVER_DECL InstanceMgr sInstanceMgr;

#endif      //WORLDCREATOR_H
