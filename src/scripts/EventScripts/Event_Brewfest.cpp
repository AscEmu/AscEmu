/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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
///\details <b>Brewfest</b>\n
/// event_properties entry: 24 \n
/// event_properties holiday: 372 \n

/// Boss Coren Direbrew
enum eGossipTexts
{
    DIREBREW_1 = 15858,
    DIREBREW_2 = 15859
};

#define BOSS_DIREBREW 23872

#define NPC_ILSA 26764
#define NPC_URSULA 26822
#define NPC_MINION 26776

#define SPELL_DISARM 47310
#define SPELL_SUMMON_MINION 47375

class SCRIPT_DECL CorenDirebrewGossip : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* Plr);
    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* EnteredCode);
    void GossipEnd(Object* pObject, Player* Plr) { Plr->CloseGossip(); }
};

void CorenDirebrewGossip::GossipHello(Object* pObject, Player * Plr)
{
    GossipMenu* Menu;
    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), DIREBREW_1, Plr);
    Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(439), 1);     // Insult Coren Direbrew's brew.
    Menu->SendTo(Plr);
}

void CorenDirebrewGossip::GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char * Code)
{
    GossipMenu* Menu;
    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), DIREBREW_2, Plr);

    if (!pObject->IsCreature())
        return;

    Creature* pCreature = static_cast<Creature*>(pObject);

    switch (IntId)
    {
        case 1:
        {
            Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(440), 1);     // Fight.
            Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(441), 1);     // Apologize.
            Menu->SendTo(Plr);
        }break;
        case 2:
        {
            pCreature->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "You'll pay for this insult, $c!");
            Plr->Gossip_Complete();
            pCreature->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
            pCreature->MoveToWaypoint(1);
        }break;
    }
};

class CorenDirebrew : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(CorenDirebrew);
    CorenDirebrew(Creature* pCreature) : CreatureAIScript(pCreature)
    { }
};

void SetupBrewfest(ScriptMgr* mgr)
{
    mgr->register_gossip_script(BOSS_DIREBREW, new CorenDirebrewGossip);
    //mgr->register_creature_script(BOSS_DIREBREW, CorenDirebrew::Create);
}

//////////////////////////////////////////////////////////////////////////////////////////
///\details <b>Brew of the Month</b>\n
/// Brew of the Month Vendors \n
/// -> Accept Quest on Brewfest (qid: 12421) \n
/// -> bringt the ticket(id: 37829) to npc \n
/// -> get every month mail with the brew of the month. \n
/// -> earn achievement "Brew of the Month" (id: 2796) \n
/// -> can buy brew of the month after finishing the quest. \n
/// f.e. Larkin Thunderbrew \n
///\todo check botm npcs (vendoritems, gossip, ...)



#define BOTM_GOSSIP_TEXT "Hey there friend!  I see you've got some Brewfest tokens.  As it happens, I still have some Brewfest items for sale." // entry 12864 in npc_text table
#define BOTM_GOSSIP_ITEM "What do you have for me?"

