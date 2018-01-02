/*
 Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 This file is released under the MIT license. See README-MIT for more information.
 */

#include "Setup.h"

///////////////////////////////////////////////////////
// Vayrie's Test Event
// Test event script for purpose of testing event mgr
// event_properties entry: 65
// event_properties holiday: 327

class VayrieTest : public EventScript
{
public:

    bool OnBeforeEventStart(GameEvent* pEvent, GameEventState /*pOldState*/) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        return true;
    }

    void OnAfterEventStart(GameEvent* pEvent, GameEventState /*pOldState*/) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
    }

    bool OnBeforeEventStop(GameEvent* pEvent, GameEventState /*pOldState*/) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        return true;
    }

    void OnAfterEventStop(GameEvent* pEvent, GameEventState /*pOldState*/) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
    }

    GameEventState OnEventStateChange(GameEvent* pEvent, GameEventState /*pOldState*/, GameEventState pNewState) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        return pNewState;
    }

    bool OnCreatureLoad(GameEvent* pEvent, Creature* pCreature) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pCreature != nullptr);
        return true;
    }

    void OnCreaturePushToWorld(GameEvent* pEvent, Creature* pCreature) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pCreature != nullptr);
    }

    void OnBeforeCreatureDespawn(GameEvent* pEvent, Creature* pCreature) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pCreature != nullptr);
    }

    void OnAfterCreatureDespawn(GameEvent* pEvent, Creature* pCreature) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pCreature != nullptr);
    }

    bool OnGameObjectLoad(GameEvent* pEvent, GameObject* pGameObject) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pGameObject != nullptr);
        return true;
    }

    void OnGameObjectPushToWorld(GameEvent* pEvent, GameObject* pGameObject) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pGameObject != nullptr);
    }

    void OnBeforeGameObjectDespawn(GameEvent* pEvent, GameObject* pGameObject) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pGameObject != nullptr);
    }

    void OnAfterGameObjectDespawn(GameEvent* pEvent, GameObject* pGameObject) override
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pGameObject != nullptr);
    }
};

void SetupVayrieTest(ScriptMgr* mgr)
{
    mgr->register_event_script(65, new VayrieTest);
}