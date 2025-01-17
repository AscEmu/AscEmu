/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/GameEvent.hpp"

#include "GameEventDefines.hpp"
#include "Logging/Logger.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Script/EventScript.hpp"

GameEvent::GameEvent(EventNamesQueryResult result)
{
    event_id = result.entry;
    start = result.start_time;
    end = result.end_time;
    occurence = result.occurence;
    length = result.length;
    holiday_id = result.holiday_id;
    description = std::string(result.description);
    //state = result.world_event;
    announce = result.announce;

    // Set later by event_save data, if any
    state = GAMEEVENT_INACTIVE;
    nextstart = 0;

    mEventScript = nullptr;
}

void GameEvent::SetState(GameEventState pState)
{
    auto oldstate = state;
    pState = OnStateChange(oldstate, pState);

    if (state == pState)
        return;

    state = pState;
}

GameEventState GameEvent::GetState() { return state; }

GameEventState GameEvent::OnStateChange(GameEventState pOldState, GameEventState pNewState)
{
    if (mEventScript != nullptr)
        pNewState = mEventScript->OnEventStateChange(this, pOldState, pNewState);

    if (pOldState == pNewState)
        return pNewState;

    // Save new state to DB before calling handler
    const char* updateQuery = "REPLACE INTO gameevent_save (event_entry, state, next_start) VALUES (%u, %u, %u)";
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
        SetState(GAMEEVENT_ACTIVE_FORCED);
    else
        SetState(GAMEEVENT_ACTIVE);

    return GetState() == GAMEEVENT_ACTIVE || GetState() == GAMEEVENT_ACTIVE_FORCED;
}

bool GameEvent::StopEvent(bool forced)
{
    if (forced)
        SetState(GAMEEVENT_INACTIVE_FORCED);
    else
        SetState(GAMEEVENT_INACTIVE);

    return GetState() == GAMEEVENT_INACTIVE || GetState() == GAMEEVENT_INACTIVE_FORCED;
}

bool GameEvent::isValid() const { return length > 0 && end > time(nullptr); }

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

        if (mEventScript != nullptr)
            mEventScript->OnAfterCreatureDespawn(this, npc);

        npc->Delete();
    }

    for (auto gameobject : active_gameobjects)
    {
        if (mEventScript != nullptr)
            mEventScript->OnBeforeGameObjectDespawn(this, gameobject);

        if (mEventScript != nullptr)
            mEventScript->OnAfterGameObjectDespawn(this, gameobject);

        gameobject->Delete();
    }

    active_npcs.clear();
    active_gameobjects.clear();
}

void GameEvent::CreateNPCs()
{
    for (const auto npc : npc_data)
    {
        auto mapmgr = sMapMgr.findWorldMap(npc.map_id);
        if (mapmgr == nullptr)
            continue;

        Creature* creature = mapmgr->createCreature(npc.entry);
        CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(npc.entry);
        if (creatureProperties == nullptr)
        {
            sLogger.failure("try to create invalid creature {}!", npc.entry);
            continue;
        }

        creature->Load(creatureProperties, npc.position_x, npc.position_y, npc.position_z, npc.orientation);
        if (npc.waypoint_group != 0)
        {
            // todo aaron02
        }

        // Set up spawn specific information
        creature->setDisplayId(npc.displayid);
        creature->setFaction(npc.faction);

        // Equipment
        creature->setVirtualItemSlotId(MELEE, creatureProperties->itemslot_1);
        creature->setVirtualItemSlotId(OFFHAND, creatureProperties->itemslot_2);
        creature->setVirtualItemSlotId(RANGED, creatureProperties->itemslot_3);

        if (npc.mountdisplayid != 0)
            creature->setMountDisplayId(npc.mountdisplayid);

        creature->mEvent = this;
        bool addToWorld = true;
        if (mEventScript != nullptr)
        {
            addToWorld = mEventScript->OnCreatureLoad(this, creature);
        }
        if (addToWorld)
        {
            creature->AddToWorld(mapmgr);
            active_npcs.push_back(creature);
        }
        else
        {
            creature->Delete();
        }
    }
}

void GameEvent::CreateObjects()
{
    for (auto gobj : gameobject_data)
    {
        auto mapmgr = sMapMgr.findWorldMap(gobj.map_id);
        if (mapmgr == nullptr)
            continue;

        GameObject* gameObject = mapmgr->createGameObject(gobj.entry);
        gameObject->create(gobj.entry, mapmgr, gobj.phase, LocationVector(gobj.position_x, gobj.position_y, gobj.position_z, gobj.facing), QuaternionData(), GameObject_State(gobj.state));

        // Set up spawn specific information
        if (MySQLStructure::GameObjectSpawnOverrides const* overrides = sMySQLStore.getGameObjectOverride(gobj.id))
        {
            gameObject->setScale(overrides->scale);

            if (overrides->faction != 0)
                gameObject->SetFaction(overrides->faction);

            gameObject->setFlags(overrides->flags);
        }

        bool addToWorld = true;
        if (mEventScript != nullptr)
        {
            addToWorld = mEventScript->OnGameObjectLoad(this, gameObject);
        }
        if (addToWorld)
        {
            gameObject->AddToWorld(mapmgr);
            active_gameobjects.push_back(gameObject);
        }
        else
        {
            gameObject->Delete();
        }
    }
}
