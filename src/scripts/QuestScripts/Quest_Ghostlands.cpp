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

class Prisoner12 : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 1, plr->GetSession()->language);
        if (plr->hasQuestInQuestLog(9164))
            menu.addItem(GOSSIP_ICON_CHAT, 462, 1);     // Release Him.

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        Creature* Prisoner12 = static_cast<Creature*>(pObject);

        plr->AddQuestKill(9164, 0, 0);

        Prisoner12->Despawn(5000, 6 * 60 * 1000);
        Prisoner12->setStandState(STANDSTATE_STAND);
    }
};

class Prisoner22 : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 1, plr->GetSession()->language);
        if (plr->hasQuestInQuestLog(9164))
            menu.addItem(GOSSIP_ICON_CHAT, 462, 1);     // Release Him.

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        Creature* Prisoner22 = static_cast<Creature*>(pObject);

        plr->AddQuestKill(9164, 1, 0);

        Prisoner22->Despawn(5000, 6 * 60 * 1000);
        Prisoner22->setStandState(STANDSTATE_STAND);
        Prisoner22->setEmoteState(EMOTE_ONESHOT_EAT);
    }
};

class Prisoner32 : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 1, plr->GetSession()->language);
        if (plr->hasQuestInQuestLog(9164))
            menu.addItem(GOSSIP_ICON_CHAT, 462, 1);     // Release Him.

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        Creature* Prisoner32 = static_cast<Creature*>(pObject);

        plr->AddQuestKill(9164, 2, 0);

        Prisoner32->Despawn(5000, 6 * 60 * 1000);
        Prisoner32->setStandState(STANDSTATE_STAND);
        Prisoner32->setEmoteState(EMOTE_ONESHOT_EAT);
    }
};

class PrisonersatDeatholme : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PrisonersatDeatholme)
    explicit PrisonersatDeatholme(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setStandState(STANDSTATE_DEAD);
        getCreature()->GetAIInterface()->m_canMove = false;
    }
};

class VanquishingAquantion : public GameObjectAIScript
{
public:

    explicit VanquishingAquantion(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new VanquishingAquantion(GO); }

    void OnActivate(Player* pPlayer) override
    {
        if (pPlayer->hasQuestInQuestLog(9174))
        {
            Creature* naga = pPlayer->GetMapMgr()->CreateAndSpawnCreature(16292, 7938, -7632, 114, 3.05f);
            naga->Despawn(6 * 60 * 1000, 0);
        }
    }
};

void SetupGhostlands(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(16208, new Prisoner12());
    mgr->register_creature_gossip(16206, new Prisoner22());
    mgr->register_creature_gossip(16209, new Prisoner32());

    mgr->register_creature_script(16208, &PrisonersatDeatholme::Create);
    mgr->register_creature_script(16206, &PrisonersatDeatholme::Create);
    mgr->register_creature_script(16209, &PrisonersatDeatholme::Create);

    mgr->register_gameobject_script(181157, &VanquishingAquantion::Create);
}
