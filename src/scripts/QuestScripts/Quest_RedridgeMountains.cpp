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

class Corporal_Keeshan : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(Corporal_Keeshan);
    Corporal_Keeshan(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 19)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Tell Marshal Marris. I'm outta here!");
            getCreature()->Despawn(5000, 1000);
            getCreature()->DeleteWaypoints();
            if (getCreature()->m_escorter == NULL)
                return;

            Player* plr = getCreature()->m_escorter;
            getCreature()->m_escorter = NULL;

            auto quest_entry = plr->GetQuestLogForEntry(219);
            if (quest_entry == nullptr)
                return;
            quest_entry->SendQuestComplete();
        }
    }
};


void SetupRedrigeMountains(ScriptMgr* mgr)
{
    mgr->register_creature_script(349, &Corporal_Keeshan::Create);
}
