/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
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
#include "Management/ItemInterface.h"
#include "Management/ObjectMgr.hpp"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Objects/Item.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/Script/CreatureAIScript.hpp"

class NorthFleet : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NorthFleet(c); }
    explicit NorthFleet(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->addQuestKill(11230, 0, 0);
        }
    }
};

class ChillmereScourge : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ChillmereScourge(c); }
    explicit ChillmereScourge(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->addQuestKill(11397, 0, 0);
        }
    }
};

class Baleheim : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Baleheim(c); }
    explicit Baleheim(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->addQuestKill(11283, 0, 0);
        }
    }
};

class Plaguethis_Gossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 40002, plr->getSession()->language);
        menu.addItem(GOSSIP_ICON_CHAT, 464, 2); // Where would you like to fly too ?

        if (plr->hasQuestInQuestLog(11332))
            menu.addItem(GOSSIP_ICON_CHAT, 465, 1); // Greer, i need a Gryphon to ride and some bombs to drop on New Agamand!

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* pCreature = static_cast<Creature*>(pObject);

        switch (Id)
        {
            case 1:
            {
                plr->getItemInterface()->AddItemById(33634, 10, 0);

                if (pCreature->getEntry() == 23859)
                {
                    plr->activateTaxiPathTo(745, pCreature);
                }
                break;
            }
            case 2:
            {
                plr->getSession()->sendTaxiMenu(pCreature);
                break;
            }
        }
    }
};

void SetupHowlingFjord(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(23859, new Plaguethis_Gossip());

    mgr->register_creature_script(23643, &ChillmereScourge::Create);
    mgr->register_creature_script(23645, &ChillmereScourge::Create);
    mgr->register_creature_script(23644, &ChillmereScourge::Create);
    mgr->register_creature_script(24540, &ChillmereScourge::Create);
    mgr->register_creature_script(24485, &ChillmereScourge::Create);
    mgr->register_creature_script(23653, &Baleheim::Create);
    mgr->register_creature_script(23655, &Baleheim::Create);
    mgr->register_creature_script(24015, &Baleheim::Create);
    mgr->register_creature_script(23866, &NorthFleet::Create);
    mgr->register_creature_script(23934, &NorthFleet::Create);
    mgr->register_creature_script(23946, &NorthFleet::Create);
    mgr->register_creature_script(23794, &NorthFleet::Create);
    mgr->register_creature_script(23793, &NorthFleet::Create);
}
