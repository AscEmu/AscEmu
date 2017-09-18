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

class RingoDeadNPC : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(RingoDeadNPC);
    RingoDeadNPC(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        _unit->SetStandState(STANDSTATE_DEAD);
        _unit->setDeathState(CORPSE);
        _unit->GetAIInterface()->m_canMove = false;
    }
};


class NorthernPylon : public GameObjectAIScript
{
public:
    NorthernPylon(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new NorthernPylon(GO); }

    void OnActivate(Player* pPlayer)
    {
        if (pPlayer->HasFinishedQuest(4284))
        {
            QuestLogEntry* en = pPlayer->GetQuestLogForEntry(4285);
            if (en && en->GetMobCount(0) < en->GetQuest()->required_mob_or_go_count[0])
            {
                uint32 newcount = en->GetMobCount(0) + 1;
                en->SetMobCount(0, newcount);
                en->SendUpdateAddKill(0);
                en->UpdatePlayerFields();
                return;
            }
        }
        else if (pPlayer->HasFinishedQuest(4284) == false)
        {
            pPlayer->BroadcastMessage("You need to have completed the quest : Crystals of Power");
        }
    }
};


class EasternPylon : public GameObjectAIScript
{
public:
    EasternPylon(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new EasternPylon(GO); }

    void OnActivate(Player* pPlayer)
    {
        if (pPlayer->HasFinishedQuest(4284))
        {
            QuestLogEntry* en = pPlayer->GetQuestLogForEntry(4287);
            if (en && en->GetMobCount(0) < en->GetQuest()->required_mob_or_go_count[0])
            {
                uint32 newcount = en->GetMobCount(0) + 1;
                en->SetMobCount(0, newcount);
                en->SendUpdateAddKill(0);
                en->UpdatePlayerFields();
                return;
            }
        }
        else if (pPlayer->HasFinishedQuest(4284) == false)
        {
            pPlayer->BroadcastMessage("You need to have completed the quest : Crystals of Power");
        }
    }
};


class WesternPylon : public GameObjectAIScript
{
public:
    WesternPylon(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new WesternPylon(GO); }

    void OnActivate(Player* pPlayer)
    {
        if (pPlayer->HasFinishedQuest(4284))
        {
            QuestLogEntry* en = pPlayer->GetQuestLogForEntry(4288);
            if (en && en->GetMobCount(0) < en->GetQuest()->required_mob_or_go_count[0])
            {
                uint32 newcount = en->GetMobCount(0) + 1;
                en->SetMobCount(0, newcount);
                en->SendUpdateAddKill(0);
                en->UpdatePlayerFields();
                return;
            }
        }
        else if (pPlayer->HasFinishedQuest(4284) == false)
        {
            pPlayer->BroadcastMessage("You need to have completed the quest : Crystals of Power");
        }
    }
};


class A_Me01 : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(A_Me01);
    A_Me01(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32 iWaypointId, bool bForwards)
    {
        if (iWaypointId == 28)
        {
            _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Tr..........");
            _unit->Despawn(5000, 1000);
            sEAS.DeleteWaypoints(_unit);
            if (_unit->m_escorter == NULL)
                return;
            Player* plr = _unit->m_escorter;
            _unit->m_escorter = NULL;

            auto quest_entry = plr->GetQuestLogForEntry(4245);
            if (quest_entry == nullptr)
                return;
            quest_entry->SendQuestComplete();
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
