/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include <cstdint>

class GameObject;
class Creature;
class GameEvent;
enum GameEventState : uint8_t;

class SERVER_DECL EventScript
{
public:
    EventScript() = default;
    virtual ~EventScript() {}

    virtual bool OnBeforeEventStart(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) { return true; } // Before an event is about to be flagged as starting
    virtual void OnAfterEventStart(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) {} // After an event has spawned all entities
    virtual bool OnBeforeEventStop(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) { return true; } // Before an event is about to be flagged as stopping
    virtual void OnAfterEventStop(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) { } // After an event has despawned all entities

    virtual GameEventState OnEventStateChange(GameEvent* /*pEvent*/, GameEventState /*pOldState*/, GameEventState pNewState) { return pNewState; } // When an event changes state

    virtual bool OnCreatureLoad(GameEvent* /*pEvent*/, Creature* /*pCreature*/) { return true; } // When a creature's data has been loaded, before it is spawned
    virtual void OnCreaturePushToWorld(GameEvent* /*pEvent*/, Creature* /*pCreature*/) {} // After a creature has been added to the world
    virtual void OnBeforeCreatureDespawn(GameEvent* /*pEvent*/, Creature* /*pCreature*/) {} // Before a creature is about to be despawned
    virtual void OnAfterCreatureDespawn(GameEvent* /*pEvent*/, Creature* /*pCreature*/) {} // After a creature has been despawned

    virtual bool OnGameObjectLoad(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) { return true; } // When a game object's data has been loaded, before it is spawned
    virtual void OnGameObjectPushToWorld(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) {} // After a game object has been added to the world
    virtual void OnBeforeGameObjectDespawn(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) {} // Before a game object is about to be despawned
    virtual void OnAfterGameObjectDespawn(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) {} // After a game object is about to be despawned

    // Standard virtual methods
    virtual void OnLoad() {}
    virtual void UpdateEvent() {}
    virtual void Destroy() {}

    // Data sharing between scripts
    virtual void setInstanceData(uint32_t /*dataType*/, uint32_t /*value*/) {}
    virtual uint32_t getInstanceData(uint32_t /*data*/) const { return 0; }
    virtual void setGuidData(uint32_t /*guidType*/, uint64_t /*guidData*/) {}
    virtual uint64_t getGuidData(uint32_t /*guidType*/) const { return 0; }

    // UpdateEvent
    void RegisterUpdateEvent(uint32_t pFrequency);
    void ModifyUpdateEvent(uint32_t pNewFrequency);
    void RemoveUpdateEvent();
};
