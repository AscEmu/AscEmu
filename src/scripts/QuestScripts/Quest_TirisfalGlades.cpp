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

class TheDormantShade : public QuestScript
{
public:

    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* creat = mTarget->GetMapMgr()->GetInterface()->SpawnCreature(1946, 2467.314f, 14.8471f, 23.5950f, 0, true, false, 0, 0);
        creat->Despawn(60000, 0);
        creat->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You have disturbed my rest. Now face my wrath!");
    }
};

class CalvinMontague : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CalvinMontague);
    explicit CalvinMontague(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->SetFaction(68);
        getCreature()->setStandState(STANDSTATE_STAND);
    }

    void OnDamageTaken(Unit* mAttacker, uint32 /*fAmount*/) override
    {
        if (getCreature()->getHealthPct() < 10)
        {
            if (mAttacker->isPlayer())
            {
                getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                QuestLogEntry* qle = (static_cast<Player*>(mAttacker))->GetQuestLogForEntry(590);
                if (!qle)
                    return;

                qle->SendQuestComplete();

                setScriptPhase(2);
            }
        }
    }

    void OnScriptPhaseChange(uint32_t phase) override
    {
        if (phase == 2)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Okay, okay! Enough fighting.");
            getCreature()->RemoveNegativeAuras();
            getCreature()->SetFaction(68);
            getCreature()->setStandState(STANDSTATE_SIT);
            getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(433), true);
            sEventMgr.AddEvent(static_cast<Unit*>(getCreature()), &Unit::setStandState, (uint8)STANDSTATE_STAND, EVENT_CREATURE_UPDATE, 18000, 0, 1);
            getCreature()->GetAIInterface()->WipeTargetList();
            getCreature()->GetAIInterface()->WipeHateList();
            getCreature()->GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
            _setMeleeDisabled(true);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        }
    }
};

class ARoguesDeal : public QuestScript
{
public:

    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();

        Creature* Dashel = mTarget->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ, 6784);

        if (Dashel == NULL)
            return;

        Dashel->SetFaction(28);
        Dashel->GetAIInterface()->setMeleeDisabled(false);
        Dashel->GetAIInterface()->SetAllowedToEnterCombat(true);
    }
};

class Zealot : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Zealot);
    explicit Zealot(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        if (!getCreature()->HasAura(3287))
            return;
        if (iWaypointId == 2)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "My mind. . .me flesh. . .I'm. . .rotting. . . .!");
        }

        if (iWaypointId == 7)
        {
            getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(5), true);
        }
    }
};

void SetupTirisfalGlades(ScriptMgr* mgr)
{
    mgr->register_quest_script(410, new TheDormantShade());
    mgr->register_creature_script(6784, &CalvinMontague::Create);
    mgr->register_quest_script(590, new ARoguesDeal());
    mgr->register_creature_script(1931, &Zealot::Create);
}
