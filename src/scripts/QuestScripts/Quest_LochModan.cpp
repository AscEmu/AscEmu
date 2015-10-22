/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
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

class ProtectingtheShipment : public QuestScript
{
    public:

        void OnQuestStart(Player* mTarget, QuestLogEntry* qLogEntry)
        {
            float SSX = mTarget->GetPositionX();
            float SSY = mTarget->GetPositionY();
            float SSZ = mTarget->GetPositionZ();


            Creature* creat = mTarget->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ, 1379);
            if(creat == NULL)
                return;
            creat->m_escorter = mTarget;
            creat->GetAIInterface()->setMoveType(MOVEMENTTYPE_FORWARDTHENSTOP);
            creat->GetAIInterface()->StopMovement(3000);
            creat->GetAIInterface()->SetAllowedToEnterCombat(false);
            creat->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Okay let's do!");
            creat->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

        }
};

// Miran Waypoints
static LocationExtra WaypointsMiran[] =
{
    {},
    { -5753.780762f, -3433.290039f, 301.628387f, 4.834769f, Flag_Run }, //1
    { -5744.062500f, -3476.653564f, 302.269287f, 5.580896f, Flag_Run },
    { -5674.703125f, -3543.583984f, 303.273682f, 4.775867f, Flag_Run },
    { -5670.187500f, -3595.618164f, 311.888153f, 4.791576f, Flag_Run },
    { -5664.515625f, -3687.601563f, 317.954590f, 4.131842f, Flag_Run },
    { -5705.745117f, -3755.254150f, 321.452118f, 4.457779f, Flag_Run },
    { -5711.766113f, -3778.145752f, 322.827942f, 4.473486f, Flag_Run }  //7
};

class Miran : public MoonScriptCreatureAI
{

    public:
        MOONSCRIPT_FACTORY_FUNCTION(Miran, MoonScriptCreatureAI);
        Miran(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            WPCount = 7;
            WayPoints = WaypointsMiran;

            for (int i = 1; i <= WPCount; ++i)
            {
                AddWaypoint(CreateWaypoint(i, 0, WayPoints[i].addition, WayPoints[i]));
            }

            pCreature->GetAIInterface()->setMoveType(MOVEMENTTYPE_DONTMOVEWP);
        }

        void OnReachWP(uint32 iWaypointId, bool bForwards)
        {
            if(iWaypointId == 7)
            {
                _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thanks, I'm outta here!");
                _unit->Despawn(5000, 1000);
                sEAS.DeleteWaypoints(_unit);
                if(_unit->m_escorter == NULL)
                    return;
                auto player = _unit->m_escorter;
                _unit->m_escorter = NULL;

                auto quest_entry = player->GetQuestLogForEntry(309);
                if (quest_entry == nullptr)
                    return;
                quest_entry->SendQuestComplete();
            }
        }

        uint8 WPCount;
        LocationExtra* WayPoints;
};

void SetupLochModan(ScriptMgr* mgr)
{
    mgr->register_creature_script(1379, &Miran::Create);
    mgr->register_quest_script(309, new ProtectingtheShipment());
}
