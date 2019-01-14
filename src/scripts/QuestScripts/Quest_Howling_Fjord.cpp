/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Management/TaxiMgr.h"

class NorthFleet : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(NorthFleet);
    explicit NorthFleet(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->AddQuestKill(11230, 0, 0);
        }
    }
};
class ChillmereScourge : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ChillmereScourge);
    explicit ChillmereScourge(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->AddQuestKill(11397, 0, 0);
        }
    }
};
class Baleheim : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Baleheim);
    explicit Baleheim(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->AddQuestKill(11283, 0, 0);
        }
    }
};

class Plaguethis_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 40002, plr->GetSession()->language);
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(464), 2); // Where would you like to fly too ?

        if (plr->HasQuest(11332))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(465), 1); // Greer, i need a Gryphon to ride and some bombs to drop on New Agamand!

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* pCreature = static_cast<Creature*>(pObject);

        switch (Id)
        {
            case 1:
            {
                Item* item = objmgr.CreateItem(33634, plr);
                if (item == nullptr)
                    return;

                item->setStackCount(10);

                if (!plr->getItemInterface()->AddItemToFreeSlot(item))
                {
                    plr->GetSession()->SendNotification("No free slots were found in your inventory!");
                    item->DeleteMe();
                }
                else
                {
                    plr->sendItemPushResultPacket(false, true, false, plr->getItemInterface()->LastSearchResult()->ContainerSlot,
                        plr->getItemInterface()->LastSearchResult()->Slot, 1, item->getEntry(), item->getPropertySeed(),
                        item->getRandomPropertiesId(), item->getStackCount());

                }

                if (pCreature->getEntry() == 23859)
                {
                    TaxiPath* path = sTaxiMgr.GetTaxiPath(745);
                    plr->TaxiStart(path, 17759, 0);
                }
                break;
            }
            case 2:
            {
                plr->GetSession()->sendTaxiList(pCreature);
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
