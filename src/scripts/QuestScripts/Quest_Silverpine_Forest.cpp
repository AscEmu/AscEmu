/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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

class Deathstalker_Erland : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(Deathstalker_Erland);
    Deathstalker_Erland(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 9)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thanks, you helped me to overcome this obstacle");
            getCreature()->Despawn(5000, 1000);
            getCreature()->DeleteWaypoints();
            if (getCreature()->m_escorter == NULL)
                return;
            Player* plr = getCreature()->m_escorter;
            getCreature()->m_escorter = NULL;

            auto quest_entry = plr->GetQuestLogForEntry(435);
            if (quest_entry == nullptr)
                return;
            quest_entry->SendQuestComplete();
        }
    }
};

class Nightlash : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(Nightlash);
    Nightlash(Creature* pCreature) : CreatureAIScript(pCreature) {}
    void OnDied(Unit* mKiller) override
    {
        if (mKiller->IsPlayer())
        {
            Player* mPlayer = static_cast<Player*>(mKiller);

            if (!getCreature()->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(1069.889404f, 1544.777558f, 28.331335f, 1983) && (RandomUInt(5) > 2) && mPlayer->HasQuest(437)) //random number I picked between 2-8
            {
                getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(1983, 1069.889404f, 1544.777558f, 28.331335f, 3.99f, true, false, 0, 0)->Despawn(600000, 0);
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Nightlash avenge us!!");//not sure this is 100% blizzlike, but looks nice
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
