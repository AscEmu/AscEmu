/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.    If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"

class ThreatFromAboveQAI : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(ThreatFromAboveQAI);
    ThreatFromAboveQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller)
    {
        if (mKiller->IsPlayer())
        {
            static_cast<Player*>(mKiller)->AddQuestKill(11096, 0, 0);
        }
    }
};

class TheInfestedProtectorsQAI : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(TheInfestedProtectorsQAI);
    TheInfestedProtectorsQAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        min = 0;
        max = 0;
        finall = 0;
    }

    void OnDied(Unit* mKiller)
    {
        if (mKiller->IsPlayer())
        {
            Player * plr = static_cast<Player*>(mKiller);
            if (plr->HasQuest(10896))
            {
                if (Rand(90))
                {
                    switch (getCreature()->GetEntry())
                    {
                        case 22307:
                            min = 4; max = 11;
                            break;
                        case 22095:
                            min = 2; max = 5;
                            break;
                    }

                    finall = min + RandomUInt(max - min);

                    float SSX = getCreature()->GetPositionX();
                    float SSY = getCreature()->GetPositionY();
                    float SSZ = getCreature()->GetPositionZ();
                    float SSO = getCreature()->GetOrientation();

                    for (uint8 i = 0; i < finall; i++)
                    {
                        Creature * NewCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(22419, SSX + RandomFloat(3.0f), SSY + RandomFloat(3.0f), SSZ, SSO + RandomFloat(1.0f), true, false, 0, 0);
                        if (NewCreature != NULL)
                            NewCreature->Despawn(120000, 0);
                    }
                }
            }
        }
    }

private:

    uint32 min;
    uint32 max;
    uint32 finall;
};

class TakenInTheNight : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(TakenInTheNight);

    TakenInTheNight(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        getCreature()->GetAIInterface()->m_canMove = false;
        getCreature()->GetAIInterface()->setCombatDisabled(true);
    }

    void OnDied(Unit* mKiller)
    {
        if (!mKiller->IsPlayer())
            return;

        Player* plr = static_cast<Player*>(mKiller);
        uint8 chance = (uint8)RandomUInt(5);
        uint32 spawn = 0;

        switch (chance)
        {
            case 0:
            case 1:
                spawn = 22459; //Freed Shat'ar Warrior
                break;

            case 2:
                spawn = 21661; //Cabal Skirmisher
                break;

            case 3:
                spawn = 16805; //Broken Skeleton
                break;

            case 4:
                spawn = 18470; //Bonelasher
                break;

            case 5:
                spawn = 22045; //Vengeful Husk
                break;
        }

        Creature* creat = plr->GetMapMgr()->CreateAndSpawnCreature(spawn, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), 0);
        if (creat == nullptr)
            return;

        creat->Despawn(1 * 60 * 1000, 0);

        if (spawn != 22459)
            return;

        creat->GetAIInterface()->m_canMove = false;
        creat->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Finally! I'm free!");

        plr->AddQuestKill(10873, 0, 0);
    }
};

class AnImproperBurial : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(AnImproperBurial);

    AnImproperBurial(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        getCreature()->SetStandState(STANDSTATE_DEAD);
        getCreature()->setDeathState(CORPSE);
        getCreature()->GetAIInterface()->m_canMove = false;
    }
};

class TheMomentofTruth : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 1, plr->GetSession()->language);
        if (plr->HasQuest(10201) && plr->GetItemInterface()->GetItemCount(28500, 0))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(497), 1);     // Try this

        menu.Send(plr);
    }

    void OnSelectOption(Object* /*pObject*/, Player* plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32 /*gossipId*/) override
    {
        plr->GetItemInterface()->RemoveItemAmt(2799, 1);

        plr->AddQuestKill(10201, 0, 0);
    }
};


void SetupTerrokarForest(ScriptMgr* mgr)
{
    mgr->register_creature_script(22144, &ThreatFromAboveQAI::Create);
    mgr->register_creature_script(22143, &ThreatFromAboveQAI::Create);
    mgr->register_creature_script(22148, &ThreatFromAboveQAI::Create);
    mgr->register_creature_script(22355, &TakenInTheNight::Create);
    mgr->register_creature_script(21859, &AnImproperBurial::Create);
    mgr->register_creature_script(21846, &AnImproperBurial::Create);
    mgr->register_creature_script(22307, &TheInfestedProtectorsQAI::Create);
    mgr->register_creature_script(22095, &TheInfestedProtectorsQAI::Create);

    Arcemu::Gossip::Script* gossip1 = new TheMomentofTruth();
    mgr->register_creature_gossip(19606, gossip1);
}