/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
#include "Management/QuestLogEntry.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Utilities/Random.hpp"

enum
{
    BALOS_FRIENDLY_TIMER = 120,
};

class BalosJackenQAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BalosJackenQAI(c); }
    explicit BalosJackenQAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        friendlyTimer = BALOS_FRIENDLY_TIMER;
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t fAmount) override
    {
        // If Balos Jacken HP - fAmount < 20%
        if (getCreature()->getHealth() - fAmount <= getCreature()->getMaxHealth() * 0.2f)
        {
            //Missing: modify fAmount to prevent Balos Jacken death.
            //{...}
            //force player to loose target and stop melee auto-attack:
            getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            //start AIUpdate
            RegisterAIUpdateEvent(1000);
        }
    }

    void AIUpdate() override
    {
        if (friendlyTimer == BALOS_FRIENDLY_TIMER)
        {
            // set Balos Jacken friendly and start friendlyTimer cooldown
            getCreature()->removeAllNegativeAuras();
            getCreature()->setFaction(35);
            getCreature()->setHealthPct(100);
            getCreature()->getThreatManager().clearAllThreat();
            getCreature()->getThreatManager().removeMeFromThreatLists();
            getCreature()->getAIInterface()->handleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
            _setMeleeDisabled(true);
            getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
            //remove not_selectable flag:
            getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            // decrease timer
            friendlyTimer--;
        }
        else if (friendlyTimer == 0)
        {
            // set Balos Jacken unfriendly and reset FriendlyTimer
            getCreature()->setFaction(14);
            _setMeleeDisabled(false);
            getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
            friendlyTimer = BALOS_FRIENDLY_TIMER;
            RemoveAIUpdateEvent();
        }
        else
        {
            //friendlyTimer decrease
            friendlyTimer--;
        }
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        RemoveAIUpdateEvent();
    }

protected:
    short friendlyTimer;
};

class OverlordMokMorokk : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OverlordMokMorokk(c); }
    explicit OverlordMokMorokk(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setStandState(STANDSTATE_STAND);
    }

    void OnDamageTaken(Unit* mAttacker, uint32_t fAmount) override
    {
        const uint32_t chance = Util::getRandomUInt(100);
        if (chance < 25)
        {
            getCreature()->castSpell(mAttacker, sSpellMgr.getSpellInfo(6749), true);
        }

        if (getCreature()->getHealth() - fAmount <= getCreature()->getMaxHealth() * 0.3f)
        {
            if (mAttacker->isPlayer())
            {
                getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                RegisterAIUpdateEvent(1000);
                if (auto* questLog = dynamic_cast<Player*>(mAttacker)->getQuestLogByQuestId(1173))
                    questLog->sendQuestComplete();
            }
        }
    }

    void AIUpdate() override
    {
        getCreature()->removeAllNegativeAuras();
        getCreature()->setFaction(29);
        getCreature()->setHealthPct(100);
        getCreature()->getThreatManager().clearAllThreat();
        getCreature()->getThreatManager().removeMeFromThreatLists();
        getCreature()->getAIInterface()->handleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
        _setMeleeDisabled(true);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    }
};

class ChallengeOverlordMokMorokk : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();

        Creature* Overlord = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(SSX, SSY, SSZ, 4500);

        if (Overlord == nullptr)
            return;

        std::string say = "Puny ";
        say += mTarget->getName();
        say += " wanna fight Overlord Mok'Morokk? Me beat you! Me boss here!";
        Overlord->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, say.c_str());
        Overlord->setFaction(72);
        Overlord->getAIInterface()->setMeleeDisabled(false);
        Overlord->getAIInterface()->setAllowedToEnterCombat(true);
    }
};

class PrivateHendel : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PrivateHendel(c); }
    explicit PrivateHendel(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setFaction(12);
        getCreature()->setStandState(STANDSTATE_STAND);
    }

    void OnDamageTaken(Unit* mAttacker, uint32_t fAmount) override
    {
        if (getCreature()->getHealth() - fAmount <= getCreature()->getMaxHealth() * 0.37f)
        {
            if (mAttacker->isPlayer())
            {
                getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                RegisterAIUpdateEvent(1000);

                if (auto* questLog = dynamic_cast<Player*>(mAttacker)->getQuestLogByQuestId(1324))
                    questLog->sendQuestComplete();
            }
        }
    }

    void AIUpdate() override
    {
        getCreature()->emote(EMOTE_STATE_KNEEL);
        getCreature()->removeAllNegativeAuras();
        getCreature()->setFaction(12);
        getCreature()->setHealthPct(100);
        getCreature()->getThreatManager().clearAllThreat();
        getCreature()->getThreatManager().removeMeFromThreatLists();
        getCreature()->getAIInterface()->handleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
        _setMeleeDisabled(true);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    }
};

class TheMissingDiplomat2 : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();

        Creature* Dashel = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(SSX, SSY, SSZ, 4966);

        if (Dashel == nullptr)
            return;

        Dashel->setFaction(72);
        Dashel->getAIInterface()->setMeleeDisabled(false);
        Dashel->getAIInterface()->setAllowedToEnterCombat(true);
    }
};

void SetupDustwallowMarsh(ScriptMgr* mgr)
{
    mgr->register_creature_script(5089, &BalosJackenQAI::Create);
    mgr->register_creature_script(4500, &OverlordMokMorokk::Create);
    mgr->register_quest_script(1173, new ChallengeOverlordMokMorokk());
    mgr->register_creature_script(4966, &PrivateHendel::Create);
    mgr->register_quest_script(1324, new TheMissingDiplomat2());
}
