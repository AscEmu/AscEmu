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

class Paoka_Swiftmountain : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(Paoka_Swiftmountain);
    Paoka_Swiftmountain(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // makes no sense... why do we check on wp 72 if player has this quest.... too late?
    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 72)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "I appreciate the help you have shown Pao'ka. I hope this covers any misfortunes this deed has cost you.");
            getCreature()->Despawn(5000, 1000);
            getCreature()->DeleteWaypoints();
            if (getCreature()->m_escorter == NULL)
                return;
            Player* plr = getCreature()->m_escorter;
            getCreature()->m_escorter = NULL;

            if (plr->HasQuest(4770))
            {
                auto quest_entry = plr->GetQuestLogForEntry(4770);
                if (quest_entry == nullptr)
                    return;
                quest_entry->SendQuestComplete();
            }
        }
    }
};

class RumorsforKravel : public QuestScript
{
public:

    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();

        Creature* creat = mTarget->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ, 4452);
        if (creat == NULL)
            return;

        std::string msg = "Hahah! ";
        msg += mTarget->GetName();
        msg += ", you make quite a partner!";
        creat->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg.c_str());
        creat->Emote(EMOTE_ONESHOT_LAUGH);
    }
};


void SetupThousandNeedles(ScriptMgr* mgr)
{
    mgr->register_creature_script(10427, &Paoka_Swiftmountain::Create);
    mgr->register_quest_script(1117, new RumorsforKravel());
}
