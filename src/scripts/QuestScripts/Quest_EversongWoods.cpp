/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2009-2012 ArcEmu Team <http://www.arcemu.org>
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
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/Script/CreatureAIScript.hpp"

enum
{
    // Prospector Anvilward
    ANVILWARD_1 = 8239,
    ANVILWARD_2 = 8240,
};

//////////////////////////////////////////////////////////////////////////////////////////
//Quest: The Drwarfen Spy
//ID: 8486
class ProspectorAnvilwardGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* Plr) override;
    void onSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
    void destroy() override { delete this; }
};

void ProspectorAnvilwardGossip::onHello(Object* pObject, Player * Plr)
{
    GossipMenu menu(pObject->getGuid(), ANVILWARD_1, Plr->getSession()->language);
    if (Plr->hasQuestInQuestLog(8483))
        menu.addItem(GOSSIP_ICON_CHAT, 460, 1);     // I need a moment of your time, Sir.

    menu.sendGossipPacket(Plr);
}

void ProspectorAnvilwardGossip::onSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/)
{
    switch (Id)
    {
        case 1:
        {
            GossipMenu menu(pObject->getGuid(), 8240, Plr->getSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 461, 2);     // Why... yes, of course. I've something to show you right inside this building. Mr. Anvilward.
            menu.sendGossipPacket(Plr);
        }break;
        case 2:
        {
            Creature* pCreature = static_cast<Creature*>(pObject);

            pCreature->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Very well. Let's see what you have to show me.");
            GossipMenu::senGossipComplete(Plr);

            pCreature->getMovementManager()->movePath(pCreature->getWaypointPath(), false);
            pCreature->pauseMovement(10);
        }break;
    }
};

class ProspectorAnvilward : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ProspectorAnvilward(c); }
    explicit ProspectorAnvilward(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        stopMovement();
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (iWaypointId == 9)
        {
            getCreature()->setFaction(38);
            getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
            getCreature()->Despawn(10 * 60 * 1000, 1000); //if failed allow other players to do quest from beggining
            getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "What manner of trick is this, blood elf? If you seek to ambush me, I warn you I will not go down quietly!");
            getCreature()->getThreatManager().getCurrentVictim();
        }
        if (iWaypointId == 10)
        {
            getCreature()->stopMoving();
        }
    }
};

void SetupEversongWoods(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(15420, new ProspectorAnvilwardGossip());
    mgr->register_creature_script(15420, ProspectorAnvilward::Create);
}
