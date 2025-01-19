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
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Utilities/Random.hpp"

bool BaronVardusAllowSpawn = true;

class WantedBaronVardus : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        if (BaronVardusAllowSpawn == true)
        {
            uint32_t rand = Util::getRandomUInt(3);
            switch (rand)
            {
                case 0:
                    mTarget->getWorldMap()->getInterface()->spawnCreature(2306, LocationVector(692.64f, -904.74f, 157.79f), true, false, 0, 0)->Despawn(1800000, 0);
                    break;
                case 1:
                    mTarget->getWorldMap()->getInterface()->spawnCreature(2306, LocationVector(939.0f, -852.46f, 114.644f), true, false, 0, 0)->Despawn(1800000, 0);
                    break;
                case 2:
                    mTarget->getWorldMap()->getInterface()->spawnCreature(2306, LocationVector(1184.07f, -553.43f, 71.3346f), true, false, 0, 0)->Despawn(1800000, 0);
                    break;
                case 3:
                    mTarget->getWorldMap()->getInterface()->spawnCreature(2306, LocationVector(1001.20f, -793.93f, 108.65f), true, false, 0, 0)->Despawn(1800000, 0);
                    break;
            }
            BaronVardusAllowSpawn = false;
        }
    }
};

class Baron_Vardus : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Baron_Vardus(c); }
    explicit Baron_Vardus(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* /*mKiller*/) override
    {
        BaronVardusAllowSpawn = true;
    }
};

void SetupHillsbradFoothills(ScriptMgr* mgr)
{
    mgr->register_creature_script(2306, &Baron_Vardus::Create);
    mgr->register_quest_script(566, new WantedBaronVardus());
}
