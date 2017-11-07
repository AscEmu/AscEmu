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

class PathoftheAdept : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr)
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 1, plr->GetSession()->language);
        if (plr->HasQuest(9692))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(493), 1);     // Take Insignia

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* EnteredCode, uint32 gossipId)
    {
        plr->GetItemInterface()->AddItemById(24226, 1, 0);
    }
};

class LordDawnstar : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(LordDawnstar);
    LordDawnstar(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        getCreature()->setUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        getCreature()->SetStandState(STANDSTATE_DEAD);
        getCreature()->setDeathState(CORPSE);
        getCreature()->GetAIInterface()->m_canMove = false;
    }
};


void SetupSilvermoonCity(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* LordGossip = new PathoftheAdept();
    mgr->register_creature_gossip(17832, LordGossip);

    mgr->register_creature_script(17832, &LordDawnstar::Create);
}
