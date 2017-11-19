/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

 // The Gifts of Loken
class LokensFury : public GameObjectAIScript
{
public:

    LokensFury(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new LokensFury(GO); };

    void OnActivate(Player* pPlayer)
    {
        pPlayer->AddQuestKill(12965, 0, 0);
    }
};

class LokensPower : public GameObjectAIScript
{
public:

    LokensPower(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new LokensPower(GO); };

    void OnActivate(Player* pPlayer)
    {
        pPlayer->AddQuestKill(12965, 1, 0);
    }
};

class LokensFavor : public GameObjectAIScript
{
public:

    LokensFavor(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new LokensFavor(GO); };

    void OnActivate(Player* pPlayer)
    {
        pPlayer->AddQuestKill(12965, 2, 0);
    }
};

class MissingScout_Gossip : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr) override
    {
        if (plr->HasQuest(12864))
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 13612, plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(499), 1);     // Are you okay? I've come to take you back to Frosthold if you can stand.
            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 13612, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(500), 2);     // I'm sorry that I didn't get here sooner. What happened?
                menu.Send(plr);
            } break;
            case 2:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 13613, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(501), 3);     // I'll go get some help. Hang in there.
                menu.Send(plr);
            } break;
            case 3:
            {
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 13614, plr);

                plr->AddQuestKill(12864, 0, 0);
            } break;
        }
    }

};


void SetupTheStormPeaks(ScriptMgr* mgr)
{
    // The Gifts of Loken
    mgr->register_gameobject_script(192120, &LokensFury::Create);
    mgr->register_gameobject_script(192121, &LokensPower::Create);
    mgr->register_gameobject_script(192122, &LokensFavor::Create);

    Arcemu::Gossip::Script* MissingScoutGossip = new MissingScout_Gossip();
    mgr->register_creature_gossip(29811, MissingScoutGossip);
}