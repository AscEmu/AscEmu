/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

#include "Setup.h"

///////////////////////////////////////////////////////
// Vayrie's Test Event
// Test event script for purpose of testing event mgr
// event_names entry: 65
// event_names holiday: 327

class VayrieTest : public EventScript
{
public:

    bool OnBeforeEventStart(GameEvent* pEvent, GameEventState pOldState)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        return true;
    }

    void OnAfterEventStart(GameEvent* pEvent, GameEventState pOldState)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
    }

    bool OnBeforeEventStop(GameEvent* pEvent, GameEventState pOldState)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        return true;
    }

    void OnAfterEventStop(GameEvent* pEvent, GameEventState pOldState)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
    }

    GameEventState OnEventStateChange(GameEvent* pEvent, GameEventState pOldState, GameEventState pNewState)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        return pNewState;
    }

    bool OnCreatureLoad(GameEvent* pEvent, Creature* pCreature)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pCreature != nullptr);
        return true;
    }

    void OnCreaturePushToWorld(GameEvent* pEvent, Creature* pCreature)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pCreature != nullptr);
    }

    void OnBeforeCreatureDespawn(GameEvent* pEvent, Creature* pCreature)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pCreature != nullptr);
    }

    void OnAfterCreatureDespawn(GameEvent* pEvent, Creature* pCreature)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pCreature != nullptr);
    }

    bool OnGameObjectLoad(GameEvent* pEvent, GameObject* pGameObject)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pGameObject != nullptr);
        return true;
    }

    void OnGameObjectPushToWorld(GameEvent* pEvent, GameObject* pGameObject)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pGameObject != nullptr);
    }

    void OnBeforeGameObjectDespawn(GameEvent* pEvent, GameObject* pGameObject)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pGameObject != nullptr);
    }

    void OnAfterGameObjectDespawn(GameEvent* pEvent, GameObject* pGameObject)
    {
        ARCEMU_ASSERT(pEvent != nullptr);
        ARCEMU_ASSERT(pGameObject != nullptr);
    }
};

void SetupVayrieTest(ScriptMgr* mgr)
{
    mgr->register_event_script(65, new VayrieTest);
}