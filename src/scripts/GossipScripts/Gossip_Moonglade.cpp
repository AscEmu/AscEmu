/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/TaxiMgr.h"
#include "Server/WorldSession.h"
#include "Units/Creatures/Creature.h"
#include "Management/Gossip/GossipScript.h"
#include "Server/Script/ScriptMgr.h"
#include "Management/Gossip/GossipMenu.h"

class SilvaFilnaveth_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 0);
        if (plr->getClass() == DRUID && plr->getRace() == RACE_NIGHTELF)
        {
            menu.setTextID(4914);
            menu.addItem(GOSSIP_ICON_CHAT, GI_MG_FLY_VILLAGE, 1);
        }
        else if (plr->getClass() == DRUID && plr->getRace() == RACE_TAUREN)
            menu.setTextID(4915);
        else
            menu.setTextID(4913);

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* pCreature = pObject->isCreature() ? static_cast<Creature*>(pObject) : NULL;
        if (pCreature == NULL)
            return;
        plr->TaxiStart(sTaxiMgr.GetTaxiPath(315), 479, 0);     // Hippogryph
    }

    void destroy() override { delete this; }

};

class BunthenPlainswind_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 0);
        if (plr->getClass() == DRUID && plr->getRace() == RACE_TAUREN)
        {
            menu.setTextID(4918);
            menu.addItem(GOSSIP_ICON_CHAT, GI_MG_FLY_THUNDER_BLUFF, 1);
        }
        else if (plr->getClass() == DRUID && plr->getRace() == RACE_NIGHTELF)
            menu.setTextID(4917);
        else
            menu.setTextID(4916);

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* pCreature = pObject->isCreature() ? static_cast<Creature*>(pObject) : NULL;
        if (pCreature == NULL)
            return;
        plr->TaxiStart(sTaxiMgr.GetTaxiPath(316), 295, 0);     // Wyvern
    }

    void destroy() { delete this; }

};

void SetupMoongladeGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(11800, new SilvaFilnaveth_Gossip); // Silva Fil'naveth
    mgr->register_creature_gossip(11798, new BunthenPlainswind_Gossip); // Bunthen Plainswind
}
