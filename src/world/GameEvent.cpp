/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "StdAfx.h"

void GameEvent::CreateNPCs()
{
    for (auto npc : npc_data)
    {
        auto mapmgr = sInstanceMgr.GetMapMgr(npc.map_id);
        if (mapmgr == NULL)
            continue;

        Creature* c = mapmgr->CreateCreature(npc.entry);
        CreatureProto* cp = CreatureProtoStorage.LookupEntry(npc.entry);
        c->Load(cp, npc.position_x, npc.position_y, npc.position_z, npc.orientation);
        if (npc.waypoint_group != 0)
        {
            c->LoadWaypointGroup(npc.waypoint_group);
            c->SwitchToCustomWaypoints();
        }

        c->SetFaction(npc.faction);

        c->mEvent = this;
        bool addToWorld = true;
        if (mEventScript != nullptr)
        {
            addToWorld = mEventScript->OnCreatureLoad(this, c);
        }
        if (addToWorld)
        {
            c->AddToWorld(mapmgr);
            active_npcs.push_back(c);
        }
        else
        {
            c->Delete();
        }
    }
}

void GameEvent::CreateObjects()
{
    for (auto gobj : gameobject_data)
    {
        auto mapmgr = sInstanceMgr.GetMapMgr(gobj.map_id);
        if (mapmgr == NULL)
            continue;

        GameObject* g = mapmgr->CreateGameObject(gobj.entry);
        g->CreateFromProto(gobj.entry, gobj.map_id, gobj.position_x, gobj.position_y, gobj.position_z, gobj.facing);
        g->SetScale(gobj.scale);
        g->SetFaction(gobj.faction);
        bool addToWorld = true;
        if (mEventScript != nullptr)
        {
            addToWorld = mEventScript->OnGameObjectLoad(this, g);
        }
        if (addToWorld)
        {
            g->AddToWorld(mapmgr);
            active_gameobjects.push_back(g);
        }
        else
        {
            g->Delete();
        }
    }
}

void GameEvent::SpawnAllEntities()
{
    CreateNPCs();
    CreateObjects();
}

void GameEvent::DestroyAllEntities()
{
    for (auto npc : active_npcs)
    {
        if (mEventScript != nullptr)
            mEventScript->OnBeforeCreatureDespawn(this, npc);

        npc->Delete();

        if (mEventScript != nullptr)
            mEventScript->OnAfterCreatureDespawn(this, npc);
    }

    for (auto gameobject : active_gameobjects)
    {
        if (mEventScript != nullptr)
            mEventScript->OnBeforeGameObjectDespawn(this, gameobject);

        gameobject->Delete();

        if (mEventScript != nullptr)
            mEventScript->OnAfterGameObjectDespawn(this, gameobject);
    }

    active_npcs.clear();
    active_gameobjects.clear();
}

void GameEvent::SetState(GameEventState pState)
{
    auto oldstate = state;
    pState = OnStateChange(oldstate, pState);

    if (state == pState)
        return;

    state = pState;
}

GameEventState GameEvent::OnStateChange(GameEventState pOldState, GameEventState pNewState)
{
    if (mEventScript != nullptr)
        pNewState = mEventScript->OnEventStateChange(this, pOldState, pNewState);

    if (pOldState == pNewState)
        return pNewState;

    // Save new state to DB before calling handler
    const char* updateQuery = "REPLACE INTO event_save (eventEntry, state, next_start) VALUES (%u, %u, %u)";
    CharacterDatabase.Execute(updateQuery, event_id, pNewState, nextstart);

    bool shouldStop = true;
    bool shouldStart = true;

    switch (pNewState)
    {
    case GAMEEVENT_INACTIVE: // Despawn all entities
    case GAMEEVENT_INACTIVE_FORCED:
        if (mEventScript != nullptr)
            shouldStop = mEventScript->OnBeforeEventStop(this, pOldState);

        if (shouldStop)
            DestroyAllEntities();

        if (mEventScript != nullptr)
            mEventScript->OnAfterEventStop(this, pOldState);

        break;
    case GAMEEVENT_ACTIVE: // Spawn all entities
    case GAMEEVENT_ACTIVE_FORCED:
    case GAMEEVENT_PREPARING: // Not yet supported, but should be used for things like darkmoon preparation
        if (mEventScript != nullptr)
            shouldStart = mEventScript->OnBeforeEventStart(this, pOldState);

        if (shouldStart)
        {
            DestroyAllEntities();
            SpawnAllEntities();
        }

        if (mEventScript != nullptr)
            mEventScript->OnAfterEventStart(this, pOldState);
        break;
    }

    return pNewState;
}

bool GameEvent::StartEvent(bool forced)
{
    if (forced)
    {
        SetState(GAMEEVENT_ACTIVE_FORCED);
    }
    else
    {
        SetState(GAMEEVENT_ACTIVE);
    }

    return GetState() == GAMEEVENT_ACTIVE || GetState() == GAMEEVENT_ACTIVE_FORCED;
}

bool GameEvent::StopEvent(bool forced)
{
    if (forced)
    {
        SetState(GAMEEVENT_INACTIVE_FORCED);
    }
    else
    {
        SetState(GAMEEVENT_INACTIVE);
    }

    return GetState() == GAMEEVENT_INACTIVE || GetState() == GAMEEVENT_INACTIVE_FORCED;
}
