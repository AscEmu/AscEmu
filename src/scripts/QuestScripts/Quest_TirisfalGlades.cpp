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

class TheDormantShade : public QuestScript
{
public:
    void OnQuestComplete(Player* mTarget, QuestLogEntry* qLogEntry)
    {
        Creature* creat = mTarget->GetMapMgr()->GetInterface()->SpawnCreature(1946, 2467.314f, 14.8471f, 23.5950f, 0, true, false, 0, 0);
        creat->Despawn(60000, 0);
        creat->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You have disturbed my rest. Now face my wrath!");
    }
};

class CalvinMontague : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(CalvinMontague);
    CalvinMontague(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        getCreature()->SetFaction(68);
        getCreature()->SetStandState(STANDSTATE_STAND);
    }

    void OnDamageTaken(Unit* mAttacker, uint32 fAmount)
    {
        if (getCreature()->GetHealthPct() < 10)
        {
            if (mAttacker->IsPlayer())
            {
                getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                RegisterAIUpdateEvent(1000);
                QuestLogEntry* qle = (static_cast<Player*>(mAttacker))->GetQuestLogForEntry(590);
                if (!qle)
                    return;
                qle->SendQuestComplete();
            }
        }
    }

    void AIUpdate()
    {
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Okay, okay! Enough fighting.");
        getCreature()->RemoveNegativeAuras();
        getCreature()->SetFaction(68);
        getCreature()->SetStandState(STANDSTATE_SIT);
        getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(433), true);
        sEventMgr.AddEvent(static_cast<Unit*>(getCreature()), &Unit::SetStandState, (uint8)STANDSTATE_STAND, EVENT_CREATURE_UPDATE, 18000, 0, 1);
        getCreature()->GetAIInterface()->WipeTargetList();
        getCreature()->GetAIInterface()->WipeHateList();
        getCreature()->GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);
    }
};

class ARoguesDeal : public QuestScript
{
public:

    void OnQuestStart(Player* mTarget, QuestLogEntry* qLogEntry)
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
public:
    ADD_CREATURE_FACTORY_FUNCTION(Zealot);
    Zealot(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32 iWaypointId, bool bForwards)
    {
        if (!getCreature()->HasAura(3287))
            return;
        if (iWaypointId == 2)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "My mind. . .me flesh. . .I'm. . .rotting. . . .!");
        }

        if (iWaypointId == 7)
        {
            getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(5), true);
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
