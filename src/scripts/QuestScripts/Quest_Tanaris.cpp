/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2008 WEmu Team
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

class SpiritScreeches : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr) override
    {
        if (plr->HasQuest(3520))
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 2039, plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(495), 1);     // Goodbye
            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32 /*gossipId*/) override
    {
        Creature* spirit = static_cast<Creature*>(pObject);
        spirit->Despawn(1, 0);

        plr->AddQuestKill(3520, 0, 0);
    }
};

class ScreecherSpirit : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(ScreecherSpirit);

    ScreecherSpirit(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        if (!getCreature())
            return;

        Creature* cialo = getCreature()->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), 5307);
        if (!cialo)
            return;

        if (!cialo->isAlive())
            cialo->Despawn(1, 6 * 60 * 1000);

        getCreature()->Despawn(60 * 1000, 0);
    }
};

class StewardOfTime : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr) override
    {
        if (plr->HasQuest(10279) || plr->HasFinishedQuest(10279))
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 9978, plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(496), 1);     // Please take me to the Master's Lair
            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* creat = static_cast<Creature*>(pObject);

        creat->CastSpell(plr, sSpellCustomizations.GetSpellInfo(34891), true);
    }
};


void SetupTanaris(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* Screeches = new SpiritScreeches();
    mgr->register_creature_gossip(8612, Screeches);

    mgr->register_creature_script(8612, &ScreecherSpirit::Create);

    Arcemu::Gossip::Script* StewardOfTimeGossip = new StewardOfTime();
    mgr->register_creature_gossip(20142, StewardOfTimeGossip);
}
