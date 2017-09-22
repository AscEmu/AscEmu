/*
 * Moon++ Scripts for Ascent MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.    If not, see <http://www.gnu.org/licenses/>.
 *
 * EasyFunctions for ASCENT v0.4
 *
 */

#ifndef _EASYFUNC_H
#define _EASYFUNC_H

#include "Units/Creatures/AIInterface.h"
#include "Management/Item.h"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include <Management/QuestLogEntry.hpp>
#include "Map/MapScriptInterface.h"
#include <Spell/Customization/SpellCustomizations.hpp>

class SCRIPT_DECL EasyFunctions
{
    public:

        static EasyFunctions GetInstance()
        {
            static EasyFunctions easy_singleton;
            return easy_singleton;
        }

        // creates a waypoint and adds it to to the creatures custom waypoints.
        // LEGACY
        void WaypointCreate(Creature* pCreature, float x, float y, float z, float o, uint32 waittime, uint32 flags, uint32 modelid)
        {
            ARCEMU_ASSERT(pCreature != NULL);

            if (!pCreature->m_custom_waypoint_map)
                pCreature->m_custom_waypoint_map = new Movement::WayPointMap;

            if (!modelid)
                modelid = pCreature->getUInt32Value(UNIT_FIELD_DISPLAYID);

            pCreature->LoadCustomWaypoint(x, y, z, o, waittime, flags, false, 0, false, 0, modelid, modelid);
        }

        // makes the creatures AI to use the custom waypoints.
        void EnableWaypoints(Creature* pCreature)
        {
            ARCEMU_ASSERT(pCreature != nullptr);
            ARCEMU_ASSERT(pCreature != NULL);
            pCreature->SwitchToCustomWaypoints();
        }

        // deletes all custom waypoint objects the creature has.
        void DeleteWaypoints(Creature* creat)
        {
            ARCEMU_ASSERT(creat != NULL);

            if (creat->m_custom_waypoint_map == NULL)
                return;

            Movement::WayPointMap::iterator i = creat->m_custom_waypoint_map->begin();

            for (; i != creat->m_custom_waypoint_map->end(); ++i)
            {
                if ((*i) != NULL)
                    delete(*i);
            }

            creat->m_custom_waypoint_map->clear();
        }

        Creature* SpawnCreature(Player* pThis, uint32 entry, float posX, float posY, float posZ, float posO, uint32 duration = 0, uint32 phase = 1)
        {
            ARCEMU_ASSERT(pThis != NULL);
            ARCEMU_ASSERT(pThis->IsInWorld());

            CreatureProperties const* p = sMySQLStore.getCreatureProperties(entry);
            if (p == nullptr)
                return NULL;

            Creature* pCreature = pThis->GetMapMgr()->CreateCreature(entry);
            pCreature->m_spawn = 0;
            pCreature->Load(p, posX, posY, posZ);
            pCreature->SetOrientation(posO);
            pCreature->Despawn(duration, 0);
            pCreature->PushToWorld(pThis->GetMapMgr());
            pCreature->Phase(PHASE_SET, phase);

            return pCreature;
        }

        GameObject* SpawnGameobject(Player* plr, uint32 entry_id, float x, float y, float z, float o, float scale, float orientation1, float orientation2, float orientation3, float orientation4)
        {
            if (plr == NULL)
                return NULL;

            auto gameobject_info = sMySQLStore.getGameObjectProperties(entry_id);
            if (gameobject_info == nullptr)
                return nullptr;

            GameObject* pC = plr->GetMapMgr()->CreateGameObject(entry_id);
            pC->m_spawn = 0;
            pC->CreateFromProto(entry_id, plr->GetMapId(), x, y, z, o, float(orientation1), float(orientation2), float(orientation3), float(orientation4));
            pC->setFloatValue(OBJECT_FIELD_SCALE_X, (float)scale);
            pC->PushToWorld(plr->GetMapMgr());

            return pC;
        }

        // creates the storage for custom waypoints. If one already exists, it is cleared.
        void CreateCustomWaypointMap(Creature* creat)
        {
            ARCEMU_ASSERT(creat != NULL);

            if (creat->m_custom_waypoint_map == NULL)
            {
                creat->m_custom_waypoint_map = new Movement::WayPointMap;
            }
            else
            {
                DeleteWaypoints(creat);
            }
        }
};

#define sEAS EasyFunctions::GetInstance()

#endif      // _EASYFUNC_H
