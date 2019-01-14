/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

enum
{
    BALOS_FRIENDLY_TIMER = 120,
};

class BalosJackenQAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BalosJackenQAI);
    explicit BalosJackenQAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        friendlyTimer = BALOS_FRIENDLY_TIMER;
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32 fAmount) override
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
            getCreature()->RemoveNegativeAuras();
            getCreature()->SetFaction(35);
            getCreature()->SetHealthPct(100);
            getCreature()->GetAIInterface()->WipeTargetList();
            getCreature()->GetAIInterface()->WipeHateList();
            getCreature()->GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
            _setMeleeDisabled(true);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            //remove not_selectable flag:
            getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            // decrease timer
            friendlyTimer--;
        }
        else if (friendlyTimer == 0)
        {
            // set Balos Jacken unfriendly and reset FriendlyTimer
            getCreature()->SetFaction(14);
            _setMeleeDisabled(false);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
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
    ADD_CREATURE_FACTORY_FUNCTION(OverlordMokMorokk);
    explicit OverlordMokMorokk(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setStandState(STANDSTATE_STAND);
    }

    void OnDamageTaken(Unit* mAttacker, uint32 fAmount) override
    {
        uint32 chance = Util::getRandomUInt(100);
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
                QuestLogEntry* qle = (static_cast<Player*>(mAttacker))->GetQuestLogForEntry(1173);
                if (!qle)
                    return;
                qle->SendQuestComplete();
            }
        }
    }

    void AIUpdate() override
    {
        getCreature()->RemoveNegativeAuras();
        getCreature()->SetFaction(29);
        getCreature()->SetHealthPct(100);
        getCreature()->GetAIInterface()->WipeTargetList();
        getCreature()->GetAIInterface()->WipeHateList();
        getCreature()->GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
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

        Creature* Overlord = mTarget->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ, 4500);

        if (Overlord == NULL)
            return;

        std::string say = "Puny ";
        say += mTarget->getName().c_str();
        say += " wanna fight Overlord Mok'Morokk? Me beat you! Me boss here!";
        Overlord->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, say.c_str());
        Overlord->SetFaction(72);
        Overlord->GetAIInterface()->setMeleeDisabled(false);
        Overlord->GetAIInterface()->SetAllowedToEnterCombat(true);
    }
};

class PrivateHendel : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PrivateHendel);
    explicit PrivateHendel(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->SetFaction(12);
        getCreature()->setStandState(STANDSTATE_STAND);
    }

    void OnDamageTaken(Unit* mAttacker, uint32 fAmount) override
    {
        if (getCreature()->getHealth() - fAmount <= getCreature()->getMaxHealth() * 0.37f)
        {
            if (mAttacker->isPlayer())
            {
                getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                RegisterAIUpdateEvent(1000);
                QuestLogEntry* qle = (static_cast<Player*>(mAttacker))->GetQuestLogForEntry(1324);
                if (!qle)
                    return;
                qle->SendQuestComplete();
            }
        }
    }

    void AIUpdate() override
    {
        getCreature()->Emote(EMOTE_STATE_KNEEL);
        getCreature()->RemoveNegativeAuras();
        getCreature()->SetFaction(12);
        getCreature()->SetHealthPct(100);
        getCreature()->GetAIInterface()->WipeTargetList();
        getCreature()->GetAIInterface()->WipeHateList();
        getCreature()->GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
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

        Creature* Dashel = mTarget->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ, 4966);

        if (Dashel == nullptr)
            return;

        Dashel->SetFaction(72);
        Dashel->GetAIInterface()->setMeleeDisabled(false);
        Dashel->GetAIInterface()->SetAllowedToEnterCombat(true);
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
