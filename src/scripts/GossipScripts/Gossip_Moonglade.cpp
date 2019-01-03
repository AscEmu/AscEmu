/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/TaxiMgr.h"
#include "Server/WorldSession.h"
#include "Units/Creatures/Creature.h"
#include "Management/Gossip/Gossip.h"
#include "Server/Script/ScriptMgr.h"

class SilvaFilnaveth_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 0);
        if (plr->getClass() == DRUID && plr->getRace() == RACE_NIGHTELF)
        {
            menu.setTextID(4914);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MG_FLY_VILLAGE), 1);
        }
        else if (plr->getClass() == DRUID && plr->getRace() == RACE_TAUREN)
            menu.setTextID(4915);
        else
            menu.setTextID(4913);

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* pCreature = (pObject->isCreature()) ? (static_cast<Creature*>(pObject)) : NULL;
        if (pCreature == NULL)
            return;
        plr->TaxiStart(sTaxiMgr.GetTaxiPath(315), 479, 0);     // Hippogryph
    }

    void Destroy() override { delete this; }

};

class BunthenPlainswind_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 0);
        if (plr->getClass() == DRUID && plr->getRace() == RACE_TAUREN)
        {
            menu.setTextID(4918);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MG_FLY_THUNDER_BLUFF), 1);
        }
        else if (plr->getClass() == DRUID && plr->getRace() == RACE_NIGHTELF)
            menu.setTextID(4917);
        else
            menu.setTextID(4916);

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* pCreature = (pObject->isCreature()) ? (static_cast<Creature*>(pObject)) : NULL;
        if (pCreature == NULL)
            return;
        plr->TaxiStart(sTaxiMgr.GetTaxiPath(316), 295, 0);     // Wyvern
    }

    void Destroy() { delete this; }

};

void SetupMoongladeGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(11800, new SilvaFilnaveth_Gossip); // Silva Fil'naveth
    mgr->register_creature_gossip(11798, new BunthenPlainswind_Gossip); // Bunthen Plainswind
}
