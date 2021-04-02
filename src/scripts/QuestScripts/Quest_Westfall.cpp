/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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

class The_Defias_Traitor : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(The_Defias_Traitor)
    explicit The_Defias_Traitor(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 19)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Tell your master that this is where Van Cleef is hiding. I'm outta here!");
            getCreature()->Despawn(5000, 1000);
            getCreature()->DeleteWaypoints();

            if (getCreature()->m_escorter == nullptr)
                return;

            Player* player = getCreature()->m_escorter;
            getCreature()->m_escorter = nullptr;

            if (auto* questLog = player->getQuestLogByQuestId(155))
                questLog->sendQuestComplete();
        }
    }
};

void SetupWestfall(ScriptMgr* mgr)
{
    mgr->register_creature_script(467, &The_Defias_Traitor::Create);
}
