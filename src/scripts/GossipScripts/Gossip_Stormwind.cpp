/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/TaxiMgr.h"
#include "Spell/SpellMgr.h"
#include "Server/WorldSession.h"
#include "Units/Creatures/Creature.h"
#include "Management/Gossip/GossipScript.h"
#include "Server/Script/ScriptMgr.h"
#include "Management/Gossip/GossipMenu.h"

class ArchmageMalin_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 11469);

        if (plr->HasQuest(11223))
            menu.addItem(GOSSIP_ICON_CHAT, GI_SW_ARCHMAGE_JAINA, 1);

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        static_cast<Creature*>(pObject)->castSpell(plr, sSpellMgr.getSpellInfo(42711), true);
        GossipMenu::senGossipComplete(plr);
    }

    void destroy() override { delete this; }
};

//This is when you talk to Thargold Ironwing...He will fly you through Stormwind Harbor to check it out.
class SWHarborFlyAround : public GossipScript
{
public:

    void onHello(Object* pObject, Player* Plr) override
    {
        GossipMenu menu(pObject->getGuid(), 13454);
        menu.addItem(GOSSIP_ICON_CHAT, GI_SW_HARBOR_FLY_YES, 1);
        menu.addItem(GOSSIP_ICON_CHAT, GI_SW_HARBOR_FLY_NO, 2);

        menu.sendGossipPacket(Plr);
    }
    void onSelectOption(Object* /*pObject*/, Player* Plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        GossipMenu::senGossipComplete(Plr);
        if (1 == Id)
            Plr->TaxiStart(sTaxiMgr.GetTaxiPath(1041), 25679, 0);
    }

    void destroy() override { delete this; }
};

void SetupStormwindGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(2708, new ArchmageMalin_Gossip); // Archmage Malin
    mgr->register_creature_gossip(29154, new SWHarborFlyAround);
}
