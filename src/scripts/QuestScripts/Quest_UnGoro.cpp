/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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

class RingoDeadNPC : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RingoDeadNPC)
    explicit RingoDeadNPC(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setStandState(STANDSTATE_DEAD);
        getCreature()->setDeathState(CORPSE);
        getCreature()->GetAIInterface()->m_canMove = false;
    }
};


class NorthernPylon : public GameObjectAIScript
{
public:

    explicit NorthernPylon(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new NorthernPylon(GO); }

    void OnActivate(Player* pPlayer) override
    {
        if (pPlayer->HasFinishedQuest(4284))
        {
            pPlayer->AddQuestKill(4285, 0, 0);
        }
        else
        {
            pPlayer->BroadcastMessage("You need to have completed the quest : Crystals of Power");
        }
    }
};


class EasternPylon : public GameObjectAIScript
{
public:

    explicit EasternPylon(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new EasternPylon(GO); }

    void OnActivate(Player* pPlayer) override
    {
        if (pPlayer->HasFinishedQuest(4284))
        {
            pPlayer->AddQuestKill(4287, 0, 0);
        }
        else
        {
            pPlayer->BroadcastMessage("You need to have completed the quest : Crystals of Power");
        }
    }
};


class WesternPylon : public GameObjectAIScript
{
public:

    explicit WesternPylon(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new WesternPylon(GO); }

    void OnActivate(Player* pPlayer) override
    {
        if (pPlayer->HasFinishedQuest(4284))
        {
            pPlayer->AddQuestKill(4288, 0, 0);
        }
        else
        {
            pPlayer->BroadcastMessage("You need to have completed the quest : Crystals of Power");
        }
    }
};


class A_Me01 : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(A_Me01)
    explicit A_Me01(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 28)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Tr..........");
            getCreature()->Despawn(5000, 1000);
            getCreature()->DeleteWaypoints();
            if (getCreature()->m_escorter == nullptr)
                return;

            Player* player = getCreature()->m_escorter;
            getCreature()->m_escorter = nullptr;

            if (auto* questLog = player->getQuestLogByQuestId(4245))
                questLog->sendQuestComplete();
        }
    }
};

void SetupUnGoro(ScriptMgr* mgr)
{
    mgr->register_creature_script(9999, &RingoDeadNPC::Create);
    mgr->register_gameobject_script(164955, &NorthernPylon::Create);
    mgr->register_gameobject_script(164957, &EasternPylon::Create);
    mgr->register_gameobject_script(164956, &WesternPylon::Create);
    mgr->register_creature_script(9623, &A_Me01::Create);
}
