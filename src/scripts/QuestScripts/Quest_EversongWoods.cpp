/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2009-2012 ArcEmu Team <http://www.arcemu.org>
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
#include "Units/Creatures/AIInterface.h"
#include "Management/Item.h"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include <Management/QuestLogEntry.hpp>
#include "Map/MapScriptInterface.h"

enum
{
    // Prospector Anvilward
    ANVILWARD_1 = 8239,
    ANVILWARD_2 = 8240,
};

//////////////////////////////////////////////////////////////////////////////////////////
//Quest: The Drwarfen Spy
//ID: 8486

class ProspectorAnvilwardGossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* Plr) override;
    void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32 gossipId) override;
    void Destroy() override { delete this; }
};

void ProspectorAnvilwardGossip::OnHello(Object* pObject, Player * Plr)
{
    Arcemu::Gossip::Menu menu(pObject->getGuid(), ANVILWARD_1, Plr->GetSession()->language);
    if (Plr->HasQuest(8483))
        menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(460), 1);     // I need a moment of your time, Sir.

    menu.Send(Plr);
}

void ProspectorAnvilwardGossip::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/)
{
    switch (Id)
    {
        case 1:
        {
            Arcemu::Gossip::Menu menu(pObject->getGuid(), 8240, Plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(461), 2);     // Why... yes, of course. I've something to show you right inside this building. Mr. Anvilward.
            menu.Send(Plr);
        }break;
        case 2:
        {
            Creature* pCreature = static_cast<Creature*>(pObject);

            pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Very well. Let's see what you have to show me.");
            Arcemu::Gossip::Menu::Complete(Plr);
            pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_QUEST);

            pCreature->GetAIInterface()->StopMovement(10);
        }break;
    }
};

class ProspectorAnvilward : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ProspectorAnvilward);
    explicit ProspectorAnvilward(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 9)
        {
            getCreature()->SetFaction(38);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
            getCreature()->Despawn(10 * 60 * 1000, 1000); //if failed allow other players to do quest from beggining
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "What manner of trick is this, blood elf? If you seek to ambush me, I warn you I will not go down quietly!");
            getCreature()->GetAIInterface()->getNextTarget();
        }
        if (iWaypointId == 10)
        {
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        }
    }
};

void SetupEversongWoods(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(15420, new ProspectorAnvilwardGossip());
    mgr->register_creature_script(15420, ProspectorAnvilward::Create);
}
