/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
    CN_ELIZA = 314,
    ELIZA_FROST_NOVA = 11831,
    ELIZA_FROSTBOLT = 20819,
    ELIZA_SUMMON_GUARD = 3107,
};

class ElizaAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ElizaAI);
    explicit ElizaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mElizaCombatTimer = 0;
        setCanEnterCombat(false);

        addAISpell(ELIZA_FROST_NOVA, 10.0f, TARGET_ATTACKING, 0, 1, false, true);
        addAISpell(ELIZA_FROSTBOLT, 20.0f, TARGET_ATTACKING, 3, 1);

        mSummonGuard = addAISpell(ELIZA_SUMMON_GUARD, 0.0f, TARGET_SELF);

        mElizaGuard = nullptr;
    }

    void OnCombatStart(Unit*) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Wait...you are not my husband. But he must have sent you. And you...look..delicious!");
        mElizaCombatTimer = _addTimer(4000);
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mElizaCombatTimer))
        {
            setCanEnterCombat(true);
            _removeTimer(mElizaCombatTimer);
        }
        if (_getHealthPercent() >= 10 && _getHealthPercent() <= 98 && !_isCasting())
        {
            mElizaGuard = getCreature()->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), 1871);
            if (mElizaGuard == nullptr)
            {
                _castAISpell(mSummonGuard);
            }
        }
    }

    uint32 mElizaCombatTimer;
    CreatureAISpells* mSummonGuard;
    Creature* mElizaGuard;
};

class SummonElizaQuest : public QuestScript
{
public:

    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* Eliza = mTarget->GetMapMgr()->CreateAndSpawnCreature(314, -10271.127f, 53.784f, 42.711f, 1.72f);
        if (Eliza != nullptr)
            Eliza->Despawn(300000, 0);    // Should it be that much ?
    }
};

void SetupDuskwood(ScriptMgr* mgr)
{
    mgr->register_quest_script(254, new SummonElizaQuest());
    mgr->register_creature_script(CN_ELIZA, &ElizaAI::Create);
}
