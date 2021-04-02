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

class Dalinda_Malem : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Dalinda_Malem)
    explicit Dalinda_Malem(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 13)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thanks, these orcs wanted me to weld in the boiler");
            getCreature()->Despawn(5000, 1000);
            getCreature()->DeleteWaypoints();
            if (getCreature()->m_escorter == nullptr)
                return;

            Player* player = getCreature()->m_escorter;
            getCreature()->m_escorter = nullptr;

            if (auto* questLog = player->getQuestLogByQuestId(1440))
                questLog->sendQuestComplete();
        }
    }
};

void SetupDesolace(ScriptMgr* mgr)
{
    mgr->register_creature_script(5644, &Dalinda_Malem::Create);
}
