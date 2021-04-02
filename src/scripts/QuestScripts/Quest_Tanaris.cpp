/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

class SpiritScreeches : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        if (plr->hasQuestInQuestLog(3520))
        {
            GossipMenu menu(pObject->getGuid(), 2039, plr->GetSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 495, 1);     // Goodbye
            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        Creature* spirit = static_cast<Creature*>(pObject);
        spirit->Despawn(1, 0);

        plr->AddQuestKill(3520, 0, 0);
    }
};

class ScreecherSpirit : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ScreecherSpirit)
    explicit ScreecherSpirit(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

class StewardOfTime : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        if (plr->hasQuestInQuestLog(10279) || plr->HasFinishedQuest(10279))
        {
            GossipMenu menu(pObject->getGuid(), 9978, plr->GetSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 496, 1);     // Please take me to the Master's Lair
            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* creat = static_cast<Creature*>(pObject);

        creat->castSpell(plr, sSpellMgr.getSpellInfo(34891), true);
    }
};

void SetupTanaris(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(8612, new SpiritScreeches());

    mgr->register_creature_script(8612, &ScreecherSpirit::Create);

    mgr->register_creature_gossip(20142, new StewardOfTime());
}
