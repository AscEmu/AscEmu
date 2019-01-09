/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

class Quest_JourneytoUndercity : public QuestScript
{
public:

    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* creat = mTarget->GetMapMgr()->GetSqlIdCreature(19175); //Lady Sylvanas Windrunner - NCDB guid
        if (creat == nullptr)
            return;

        SpawnHighborneLamenter(mTarget, 21628, 1295.222656f, 314.253998f, -57.320854f, 2.365611f);
        SpawnHighborneLamenter(mTarget, 21628, 1293.403931f, 311.264465f, -57.320854f, 1.939140f);
        SpawnHighborneLamenter(mTarget, 21628, 1286.532104f, 311.452423f, -57.320854f, 0.592182f);
        SpawnHighborneLamenter(mTarget, 21628, 1284.536011f, 314.496338f, -57.320845f, 0.580401f);

        creat->PlaySoundToSet(10896);
        creat->castSpell(creat, sSpellMgr.getSpellInfo(36568), false);

        creat->setNpcFlags(UNIT_NPC_FLAG_NONE);

        // Players can't interact with Sylvanas for 180000 ms.
        // Cast creat to an object because the EventSetUInt32Value method is in Object class.
        sEventMgr.AddEvent(static_cast<Object*>(creat), &Object::EventSetUInt32Value, (uint16)UNIT_NPC_FLAGS, (uint32)2, EVENT_SCRIPT_UPDATE_EVENT, 180000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    void SpawnHighborneLamenter(Player* pThis, uint32 entry, float posX, float posY, float posZ, float posO)
    {
        CreatureProperties const* p = sMySQLStore.getCreatureProperties(entry);
        if (p == nullptr)
            return;

        Creature* creature = pThis->GetMapMgr()->CreateCreature(entry);
        creature->m_spawn = nullptr;
        creature->Load(p, posX, posY, posZ);
        creature->SetOrientation(posO);
        creature->GetAIInterface()->setCombatDisabled(true);
        creature->GetAIInterface()->setMeleeDisabled(true);
        creature->GetAIInterface()->setTargetingDisabled(true);
        creature->PushToWorld(pThis->GetMapMgr());
        creature->Despawn(180000, 0);
        creature->setFactionTemplate(35);
        creature->setServersideFaction();
    }
};

void SetupUndercity(ScriptMgr* mgr)
{
    mgr->register_quest_script(9180, new Quest_JourneytoUndercity());
}
