/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
#include "Management/QuestLogEntry.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Utilities/Random.hpp"

class Deathstalker_Erland : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Deathstalker_Erland(c); }
    explicit Deathstalker_Erland(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (iWaypointId == 9)
        {
            getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thanks, you helped me to overcome this obstacle");
            getCreature()->Despawn(5000, 1000);
            getCreature()->stopMoving();
            if (getCreature()->m_escorter == nullptr)
                return;

            Player* player = getCreature()->m_escorter;
            getCreature()->m_escorter = nullptr;

            if (auto* questLog = player->getQuestLogByQuestId(435))
                questLog->sendQuestComplete();
        }
    }
};

class Nightlash : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Nightlash(c); }
    explicit Nightlash(Creature* pCreature) : CreatureAIScript(pCreature) {}
    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            Player* mPlayer = static_cast<Player*>(mKiller);

            if (!getCreature()->getWorldMap()->getInterface()->getCreatureNearestCoords(1069.889404f, 1544.777558f, 28.331335f, 1983) && (Util::getRandomUInt(5) > 2) && mPlayer->hasQuestInQuestLog(437)) //random number I picked between 2-8
            {
                getCreature()->getWorldMap()->getInterface()->spawnCreature(1983, LocationVector(1069.889404f, 1544.777558f, 28.331335f, 3.99f), true, false, 0, 0)->Despawn(600000, 0);
                getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Nightlash avenge us!!");//not sure this is 100% blizzlike, but looks nice
            }
        }
    }
};

void SetupSilverpineForest(ScriptMgr* mgr)
{
    mgr->register_creature_script(1978, &Deathstalker_Erland::Create);
    mgr->register_creature_script(1773, &Nightlash::Create);
    mgr->register_creature_script(1772, &Nightlash::Create);
}
