/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
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

 // Chieftain Oomooroo
class ChieftainOomoorooQAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ChieftainOomoorooQAI);
    explicit ChieftainOomoorooQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller)
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->AddQuestKill(9573, 1, 0);
        }
    }
};

void SetupAzuremystIsle(ScriptMgr* mgr)
{
    mgr->register_creature_script(17189, &ChieftainOomoorooQAI::Create);
}
