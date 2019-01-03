/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/GameEvent.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Map/WorldCreator.h"

void GameEvent::CreateNPCs()
{
    for (auto npc : npc_data)
    {
        auto mapmgr = sInstanceMgr.GetMapMgr(npc.map_id);
        if (mapmgr == nullptr)
            continue;

        Creature* c = mapmgr->CreateCreature(npc.entry);
        CreatureProperties const* cp = sMySQLStore.getCreatureProperties(npc.entry);
        if (cp == nullptr)
        {
            LOG_ERROR("try to create invalid creature %u!", npc.entry);
            continue;
        }

        c->Load(cp, npc.position_x, npc.position_y, npc.position_z, npc.orientation);
        if (npc.waypoint_group != 0)
        {
            c->LoadWaypointGroup(npc.waypoint_group);
            c->SwitchToCustomWaypoints();
        }

        // Set up spawn specific information
        c->setDisplayId(npc.displayid);
        c->SetFaction(npc.faction);

        // Equipment
        c->setVirtualItemSlotId(MELEE, sMySQLStore.getItemDisplayIdForEntry(cp->itemslot_1));
        c->setVirtualItemSlotId(OFFHAND, sMySQLStore.getItemDisplayIdForEntry(cp->itemslot_2));
        c->setVirtualItemSlotId(RANGED, sMySQLStore.getItemDisplayIdForEntry(cp->itemslot_3));

        if (npc.mountdisplayid != 0)
            c->setMountDisplayId(npc.mountdisplayid);

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

        // Set up spawn specific information
        g->setScale(gobj.scale);

        if (gobj.faction != 0)
            g->SetFaction(gobj.faction);

        g->setFlags(gobj.flags);

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
    const char* updateQuery = "REPLACE INTO event_save (event_entry, state, next_start) VALUES (%u, %u, %u)";
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
