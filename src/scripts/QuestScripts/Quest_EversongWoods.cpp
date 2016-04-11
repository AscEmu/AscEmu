/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2009-2012 ArcEmu Team <http://www.arcemu.org>
 * Copyright (C) 2007-2008 Moon++ Team <http://moonplusplus.info>
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
#include "../Common/EasyFunctions.h"

///////////////////////////////////////////////////////
//Quest: The Drwarfen Spy
//ID: 8486

// Anvilward say
#define ANVILWARD_SAY_1 "Very well. Let's see what you have to show me."
#define ANVILWARD_SAY_2 "What manner of trick is this, blood elf? If you seek to ambush me, I warn you I will not go down quietly!"

enum eGossipTexts
{
    ANVILWARD_1 = 8239,
    ANVILWARD_2 = 8240,
};

class SCRIPT_DECL ProspectorAnvilwardGossip : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* Plr);
    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* EnteredCode);
    void GossipEnd(Object* pObject, Player* Plr) { Plr->CloseGossip(); }
};

void ProspectorAnvilwardGossip::GossipHello(Object* pObject, Player * Plr)
{
    GossipMenu* Menu;

    Creature* pCreature = static_cast<Creature*>(pObject);
    if (pCreature == NULL)
        return;

    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), ANVILWARD_1, Plr);

    if (Plr->HasQuest(8483))
        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(460), 1);     // I need a moment of your time, Sir.

    Menu->SendTo(Plr);
}

void ProspectorAnvilwardGossip::GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char * Code)
{
    GossipMenu* Menu;
    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 8240, Plr);

    if(!pObject->IsCreature())
        return;

    Creature* pCreature = static_cast<Creature*>(pObject);

    switch (IntId)
    {
        case 1:
        {
            Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(461), 2);     // Why... yes, of course. I've something to show you right inside this building. Mr. Anvilward.
            Menu->SendTo(Plr);
        }break;
        case 2:
        {
            pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, ANVILWARD_SAY_1);
            Plr->Gossip_Complete();
            pCreature->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_QUEST);
            //Every Gossip NPC has a "StopMovement(30000)" by default.... lets overwrite it.
            pCreature->GetAIInterface()->StopMovement(10);
        }break;
    }
};

class ProspectorAnvilward : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(ProspectorAnvilward);
    ProspectorAnvilward(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pCreature->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_DONTMOVEWP);
    }

    void OnReachWP(uint32 iWaypointId, bool bForwards)
    {
        if (iWaypointId == 9)
        {
            _unit->SetFaction(38);
            _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
            _unit->Despawn(10 * 60 * 1000, 1000); //if failed allow other players to do quest from beggining
            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, ANVILWARD_SAY_2);
            //Zyres: This did not exist!
            //_unit->GetAIInterface()->setMoveType(Movement::WP_MOVE_TYPE_RUN);
            _unit->GetAIInterface()->getNextTarget();
        }
        if (iWaypointId == 10)
        {
            _unit->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_DONTMOVEWP);
        }
    }
};

void SetupEversongWoods(ScriptMgr* mgr)
{
    mgr->register_gossip_script(15420, new ProspectorAnvilwardGossip);
    mgr->register_creature_script(15420, ProspectorAnvilward::Create);
}
