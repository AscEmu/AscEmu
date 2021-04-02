/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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

//////////////////////////////////////////////////////////////////////////////////////////
// The Gifts of Loken
class LokensFury : public GameObjectAIScript
{
public:

    explicit LokensFury(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new LokensFury(GO); };

    void OnActivate(Player* pPlayer)
    {
        pPlayer->AddQuestKill(12965, 0, 0);
    }
};

class LokensPower : public GameObjectAIScript
{
public:

    explicit LokensPower(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new LokensPower(GO); };

    void OnActivate(Player* pPlayer)
    {
        pPlayer->AddQuestKill(12965, 1, 0);
    }
};

class LokensFavor : public GameObjectAIScript
{
public:

    explicit LokensFavor(GameObject* goinstance) : GameObjectAIScript(goinstance) {};
    static GameObjectAIScript* Create(GameObject* GO) { return new LokensFavor(GO); };

    void OnActivate(Player* pPlayer)
    {
        pPlayer->AddQuestKill(12965, 2, 0);
    }
};

class MissingScout_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        if (plr->hasQuestInQuestLog(12864))
        {
            GossipMenu menu(pObject->getGuid(), 13612, plr->GetSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 499, 1);     // Are you okay? I've come to take you back to Frosthold if you can stand.
            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                GossipMenu menu(pObject->getGuid(), 13612, plr->GetSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 500, 2);     // I'm sorry that I didn't get here sooner. What happened?
                menu.sendGossipPacket(plr);
            } break;
            case 2:
            {
                GossipMenu menu(pObject->getGuid(), 13613, plr->GetSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 501, 3);     // I'll go get some help. Hang in there.
                menu.sendGossipPacket(plr);
            } break;
            case 3:
            {
                GossipMenu::sendSimpleMenu(pObject->getGuid(), 13614, plr);

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

    mgr->register_creature_gossip(29811, new MissingScout_Gossip());
}