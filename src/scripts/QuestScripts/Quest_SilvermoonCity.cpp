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

class PathoftheAdept : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 1, plr->GetSession()->language);
        if (plr->hasQuestInQuestLog(9692))
            menu.addItem(GOSSIP_ICON_CHAT, 493, 1); // Take Insignia

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        plr->getItemInterface()->AddItemById(24226, 1, 0);
    }
};

class LordDawnstar : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LordDawnstar)
    explicit LordDawnstar(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
        getCreature()->setStandState(STANDSTATE_DEAD);
        getCreature()->setDeathState(CORPSE);
        getCreature()->GetAIInterface()->m_canMove = false;
    }
};

void SetupSilvermoonCity(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(17832, new PathoftheAdept());
    mgr->register_creature_script(17832, &LordDawnstar::Create);
}
