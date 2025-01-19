/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Utilities/Random.hpp"

class ThreatFromAboveQAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ThreatFromAboveQAI(c); }
    explicit ThreatFromAboveQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->addQuestKill(11096, 0, 0);
        }
    }
};

class TheInfestedProtectorsQAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheInfestedProtectorsQAI(c); }
    explicit TheInfestedProtectorsQAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        min = 0;
        max = 0;
        finall = 0;
    }

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            Player * plr = static_cast<Player*>(mKiller);
            if (plr->hasQuestInQuestLog(10896))
            {
                if (Util::checkChance(90))
                {
                    switch (getCreature()->getEntry())
                    {
                        case 22307:
                            min = 4; max = 11;
                            break;
                        case 22095:
                            min = 2; max = 5;
                            break;
                    }

                    finall = min + Util::getRandomUInt(max - min);

                    float SSX = getCreature()->GetPositionX();
                    float SSY = getCreature()->GetPositionY();
                    float SSZ = getCreature()->GetPositionZ();
                    float SSO = getCreature()->GetOrientation();

                    for (uint8_t i = 0; i < finall; i++)
                    {
                        Creature * NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(22419, LocationVector(SSX + Util::getRandomFloat(3.0f), SSY + Util::getRandomFloat(3.0f), SSZ, SSO + Util::getRandomFloat(1.0f)), true, false, 0, 0);
                        if (NewCreature != nullptr)
                            NewCreature->Despawn(120000, 0);
                    }
                }
            }
        }
    }

private:
    uint32_t min;
    uint32_t max;
    uint32_t finall;
};

class TakenInTheNight : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TakenInTheNight(c); }
    explicit TakenInTheNight(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        getCreature()->getAIInterface()->setCombatDisabled(true);
    }

    void OnDied(Unit* mKiller) override
    {
        if (!mKiller->isPlayer())
            return;

        Player* plr = static_cast<Player*>(mKiller);
        uint8_t chance = (uint8_t)Util::getRandomUInt(5);
        uint32_t spawn = 0;

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

        Creature* creat = plr->getWorldMap()->createAndSpawnCreature(spawn, getCreature()->GetPosition());
        if (creat == nullptr)
            return;

        creat->Despawn(1 * 60 * 1000, 0);

        if (spawn != 22459)
            return;

        creat->setControlled(true, UNIT_STATE_ROOTED);
        creat->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Finally! I'm free!");

        plr->addQuestKill(10873, 0, 0);
    }
};

class AnImproperBurial : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AnImproperBurial(c); }
    explicit AnImproperBurial(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setStandState(STANDSTATE_DEAD);
        getCreature()->setDeathState(CORPSE);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
    }
};

class TheMomentofTruth : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 1, plr->getSession()->language);
        if (plr->hasQuestInQuestLog(10201) && plr->getItemInterface()->GetItemCount(28500, 0))
            menu.addItem(GOSSIP_ICON_CHAT, 497, 1);     // Try this

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        plr->getItemInterface()->RemoveItemAmt(2799, 1);

        plr->addQuestKill(10201, 0, 0);
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

    mgr->register_creature_gossip(19606, new TheMomentofTruth());
}