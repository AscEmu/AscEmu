/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/Script/CreatureAIScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>Brewfest</b>\n
// event_properties entry: 24 \n
// event_properties holiday: 372 \n

enum
{
    BOSS_DIREBREW = 23872,

    //NPC_ILSA = 26764,
    //NPC_URSULA = 26822,
    //NPC_MINION = 26776,

    //SPELL_DISARM = 47310,
    //SPELL_SUMMON_MINION = 47375,

    // Boss Coren Direbrew
    DIREBREW_1 = 15858,
    DIREBREW_2 = 15859
};

class SCRIPT_DECL CorenDirebrewGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* Plr) override;
    void onSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

void CorenDirebrewGossip::onHello(Object* pObject, Player * Plr)
{
    GossipMenu menu(pObject->getGuid(), DIREBREW_1, Plr->getSession()->language);
    menu.addItem(GOSSIP_ICON_CHAT, 439, 1);     // Insult Coren Direbrew's brew.
    menu.sendGossipPacket(Plr);
}

void CorenDirebrewGossip::onSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char * /*Code*/, uint32_t /*gossipId*/)
{
    Creature* pCreature = static_cast<Creature*>(pObject);

    switch (Id)
    {
        case 1:
        {
            GossipMenu menu(pObject->getGuid(), DIREBREW_2, Plr->getSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 440, 1);     // Fight.
            menu.addItem(GOSSIP_ICON_CHAT, 441, 1);     // Apologize.
            menu.sendGossipPacket(Plr);
        }break;
        case 2:
        {
            pCreature->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "You'll pay for this insult, $c!");
            GossipMenu::senGossipComplete(Plr);

            pCreature->GetScript()->DoAction(0);
        }break;
    }
};

class CorenDirebrew : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CorenDirebrew(c); }
    explicit CorenDirebrew(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // whats the correct waypoint ?
        addWaypoint(1, createWaypoint(1, 0, WAYPOINT_MOVE_TYPE_WALK, pCreature->GetPosition()));
    }

    void DoAction(int32_t const action) override
    {
        if(action == 0)
            setWaypointToMove(1, 1);
    }
};

void SetupBrewfest(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(BOSS_DIREBREW, new CorenDirebrewGossip());
    mgr->register_creature_script(BOSS_DIREBREW, CorenDirebrew::Create);
}

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>Brew of the Month</b>\n
// Brew of the Month Vendors \n
// -> Accept Quest on Brewfest (qid: 12421) \n
// -> bringt the ticket(id: 37829) to npc \n
// -> get every month mail with the brew of the month. \n
// -> earn achievement "Brew of the Month" (id: 2796) \n
// -> can buy brew of the month after finishing the quest. \n
// f.e. Larkin Thunderbrew \n
//\todo check botm npcs (vendoritems, gossip, ...)

//#define BOTM_GOSSIP_TEXT "Hey there friend!  I see you've got some Brewfest tokens.  As it happens, I still have some Brewfest items for sale." // entry 12864 in npc_text table
//#define BOTM_GOSSIP_ITEM "What do you have for me?"
