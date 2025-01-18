/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Objects/Object.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Objects/Units/Creatures/Creature.h"

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

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* pCreature = pObject->isCreature() ? static_cast<Creature*>(pObject) : NULL;
        if (pCreature == NULL)
            return;
        plr->activateTaxiPathTo(315, pCreature);     // Hippogryph
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

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* pCreature = pObject->isCreature() ? static_cast<Creature*>(pObject) : NULL;
        if (pCreature == NULL)
            return;
        plr->activateTaxiPathTo(316, pCreature);     // Wyvern
    }

    void destroy() override { delete this; }
};

void SetupMoongladeGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(11800, new SilvaFilnaveth_Gossip); // Silva Fil'naveth
    mgr->register_creature_gossip(11798, new BunthenPlainswind_Gossip); // Bunthen Plainswind
}
