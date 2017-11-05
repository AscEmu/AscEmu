/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
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

 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 /////// Eliza
const uint32 CN_ELIZA = 314;
const uint32 ELIZA_FROST_NOVA = 11831;
const uint32 ELIZA_FROSTBOLT = 20819;
const uint32 ELIZA_SUMMON_GUARD = 3107;

class ElizaAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(ElizaAI, MoonScriptCreatureAI);
    ElizaAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        mElizaCombatTimer = INVALIDATE_TIMER;
        setCanEnterCombat(false);
        AddSpell(ELIZA_FROST_NOVA, Target_Current, 10, 0, 1, 0, 10, true);
        AddSpell(ELIZA_FROSTBOLT, Target_Current, 20, 3, 1);
        mSummonGuard = AddSpell(ELIZA_SUMMON_GUARD, Target_Self, 0, 0, 0);

        sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Wait...you are not my husband. But he must have sent you. And you...look..delicious!");
        mElizaCombatTimer = _addTimer(4000);

        RegisterAIUpdateEvent(1000);
        mElizaGuard = NULL;
    }
    void AIUpdate()
    {
        ParentClass::AIUpdate();
        if (_isTimerFinished(mElizaCombatTimer))
        {
            setCanEnterCombat(true);
            AggroNearestUnit();
            _removeTimer(mElizaCombatTimer);
        }
        if (_getHealthPercent() >= 10 && _getHealthPercent() <= 98 && !_isCasting())
        {
            mElizaGuard = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), 1871);
            if (mElizaGuard == NULL)
            {
                CastSpellNowNoScheduling(mSummonGuard);
            }
        }
    }

    uint32 mElizaCombatTimer;
    SpellDesc*  mSummonGuard;
    Creature* mElizaGuard;
};

class SummonElizaQuest : public QuestScript
{
public:
    void OnQuestComplete(Player* mTarget, QuestLogEntry* qLogEntry)
    {
        Creature* Eliza = mTarget->GetMapMgr()->CreateAndSpawnCreature(314, -10271.127f, 53.784f, 42.711f, 1.72f);
        if (Eliza != nullptr)
            Eliza->Despawn(300000, 0);    // Should it be that much ?
    };
};


void SetupDuskwood(ScriptMgr* mgr)
{
    QuestScript* SummonEliza = new SummonElizaQuest();
    mgr->register_quest_script(254, SummonEliza);
    mgr->register_creature_script(CN_ELIZA, &ElizaAI::Create);
}
