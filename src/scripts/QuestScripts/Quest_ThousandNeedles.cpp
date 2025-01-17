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
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"

class Paoka_Swiftmountain : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Paoka_Swiftmountain(c); }
    explicit Paoka_Swiftmountain(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // makes no sense... why do we check on wp 72 if player has this quest.... too late?
    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (iWaypointId == 72)
        {
            getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "I appreciate the help you have shown Pao'ka. I hope this covers any misfortunes this deed has cost you.");
            getCreature()->Despawn(5000, 1000);
            getCreature()->stopMoving();
            if (getCreature()->m_escorter == nullptr)
                return;

            Player* player = getCreature()->m_escorter;
            getCreature()->m_escorter = nullptr;

            if (auto* questLog = player->getQuestLogByQuestId(4770))
                questLog->sendQuestComplete();
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

        Creature* creat = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(SSX, SSY, SSZ, 4452);
        if (creat == nullptr)
            return;

        std::string msg = "Hahah! ";
        msg += mTarget->getName();
        msg += ", you make quite a partner!";
        creat->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg.c_str());
        creat->emote(EMOTE_ONESHOT_LAUGH);
    }
};

void SetupThousandNeedles(ScriptMgr* mgr)
{
    mgr->register_creature_script(10427, &Paoka_Swiftmountain::Create);
    mgr->register_quest_script(1117, new RumorsforKravel());
}
