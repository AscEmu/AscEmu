/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008 WEmu Team
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
#include "Management/QuestLogEntry.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Movement/MovementManager.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"

class ProtectingtheShipment : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();


        Creature* creat = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(SSX, SSY, SSZ, 1379);
        if (creat == nullptr)
            return;
        creat->m_escorter = mTarget;

        auto path = creat->GetScript()->getCustomPath(1);
        creat->getMovementManager()->movePath(*path, false);
        creat->pauseMovement(3000);
        creat->getAIInterface()->setAllowedToEnterCombat(false);
        creat->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Okay let's do!");
        creat->setNpcFlags(UNIT_NPC_FLAG_NONE);
    }
};

// Miran Waypoints
static LocationVector WaypointsMiran[] =
{
    { -5753.780762f, -3433.290039f, 302.628387f, 4.834769f }, //1
    { -5744.062500f, -3476.653564f, 303.269287f, 5.580896f },
    { -5674.703125f, -3543.583984f, 304.273682f, 4.775867f },
    { -5670.187500f, -3595.618164f, 312.888153f, 4.791576f },
    { -5664.515625f, -3687.601563f, 317.954590f, 4.131842f },
    { -5705.745117f, -3755.254150f, 322.452118f, 4.457779f },
    { -5711.766113f, -3778.145752f, 323.827942f, 4.473486f }  //7
};

class Miran : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Miran(c); }
    explicit Miran(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        WPCount = 7;

        for (uint8_t i = 0; i <= WPCount; ++i)
            addWaypoint(1, createWaypoint(i, 0, WAYPOINT_MOVE_TYPE_RUN, WaypointsMiran[i]));
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (iWaypointId == 7)
        {
            getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thanks, I'm outta here!");
            getCreature()->Despawn(5000, 1000);
            //getCreature()->StopMoving();

            if (getCreature()->m_escorter == nullptr)
                return;

            auto* player = getCreature()->m_escorter;
            getCreature()->m_escorter = nullptr;

            if (auto* questLog = player->getQuestLogByQuestId(309))
                questLog->sendQuestComplete();
        }
    }

    uint8_t WPCount;
};

void SetupLochModan(ScriptMgr* mgr)
{
    mgr->register_creature_script(1379, &Miran::Create);
    mgr->register_quest_script(309, new ProtectingtheShipment());
}
