/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/TaxiMgr.h"
#include "Spell/SpellMgr.h"
#include "Server/WorldSession.h"
#include "Units/Creatures/Creature.h"
#include "Management/Gossip/Gossip.h"
#include "Server/Script/ScriptMgr.h"

class ArchmageMalin_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 11469);

        if (plr->HasQuest(11223))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_SW_ARCHMAGE_JAINA), 1);

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        static_cast<Creature*>(pObject)->castSpell(plr, sSpellMgr.getSpellInfo(42711), true);
        Arcemu::Gossip::Menu::Complete(plr);
    }

    void Destroy() override { delete this; }
};

//This is when you talk to Thargold Ironwing...He will fly you through Stormwind Harbor to check it out.
class SWHarborFlyAround : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* Plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 13454);
        menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SW_HARBOR_FLY_YES), 1);
        menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SW_HARBOR_FLY_NO), 2);

        menu.Send(Plr);
    }
    void OnSelectOption(Object* /*pObject*/, Player* Plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Arcemu::Gossip::Menu::Complete(Plr);
        if (1 == Id)
            Plr->TaxiStart(sTaxiMgr.GetTaxiPath(1041), 25679, 0);
    }

    void Destroy() override { delete this; }
};

void SetupStormwindGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(2708, new ArchmageMalin_Gossip); // Archmage Malin
    mgr->register_creature_gossip(29154, new SWHarborFlyAround);
}
