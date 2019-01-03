/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_BloodFurnace.h"

class KelidanTheBreakerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KelidanTheBreakerAI);
    explicit KelidanTheBreakerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
        {
            mShadowBoltVolley = addAISpell(KELIDAN_SHADOW_BOLT_VOLLEY_H, 25.0f, TARGET_SELF, 0, 6);
            mFireNova = addAISpell(KELIDAN_FIRE_NOVA_H, 15.0f, TARGET_ATTACKING, 0, 12);
        }
        else
        {
            mShadowBoltVolley = addAISpell(KELIDAN_SHADOW_BOLT_VOLLEY, 25.0f, TARGET_SELF, 0, 6);
            mFireNova = addAISpell(KELIDAN_FIRE_NOVA, 15.0f, TARGET_SELF, 0, 12);
        }

        mBurningNova = addAISpell(KELIDAN_BURNING_NOVA, 0.0f, TARGET_SELF, 0, 0);
        mBurningNova->addEmote("Closer! Come closer... and burn!", CHAT_MSG_MONSTER_YELL);

        mVortex = addAISpell(KELIDAN_FIRE_NOVA, 0.0f, TARGET_SELF, 0, 0);
        addAISpell(KELIDAN_CORRUPTION, 15.0f, TARGET_ATTACKING, 0, 10);

        mBurningNovaTimerId = 0;
        SetAIUpdateFreq(800);

        addEmoteForEvent(Event_OnCombatStart, 4841);    // Who dares interrupt--What is this; what have you done? You'll ruin everything!
        addEmoteForEvent(Event_OnTargetDied, 4845);     // Just as you deserve!
        addEmoteForEvent(Event_OnTargetDied, 4846);     // Your friends will soon be joining you!
        addEmoteForEvent(Event_OnDied, 4848);           // Good...luck. You'll need it.
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mBurningNovaTimerId = _addTimer(15000);
    }

    void AIUpdate() override
    {
        if (getScriptPhase() == 1 && !_isCasting())
        {
            if (_isTimerFinished(mBurningNovaTimerId))
            {
                if (_isHeroic())
                    _castAISpell(mVortex);

                _castAISpell(mBurningNova);

                _resetTimer(mBurningNovaTimerId, 30000);
            }
        }
    }

    CreatureAISpells* mShadowBoltVolley;
    CreatureAISpells* mFireNova;
    CreatureAISpells* mBurningNova;
    CreatureAISpells* mVortex;
    uint32_t mBurningNovaTimerId;
};

class BroggokAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BroggokAI);
    explicit BroggokAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(POISON_BOLT, 12.0f, TARGET_SELF, 0, 15);
        addAISpell(SLIME_SPRAY, 10.0f, TARGET_SELF, 0, 25);

        auto poisonCloud = addAISpell(POISON_CLOUD, 8.0f, TARGET_RANDOM_DESTINATION, 0, 40);
        poisonCloud->setMinMaxDistance(0.0f, 40.0f);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        GameObject* pDoor = getNearestGameObject(456.157349f, 34.248005f, 9.559463f, GO_BROGGOK);
        if (pDoor)
            pDoor->setState(GO_STATE_OPEN);
    }
};

class TheMakerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TheMakerAI);
    explicit TheMakerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DOMINATION, 8.0f, TARGET_RANDOM_SINGLE);
        addAISpell(ACID_SPRAY, 10.0f, TARGET_SELF);

        auto throwBreaker = addAISpell(THROW_BEAKER, 20.0f, TARGET_RANDOM_DESTINATION);
        throwBreaker->setMinMaxDistance(0.0f, 40.0f);

        addEmoteForEvent(Event_OnCombatStart, 4849);    // My work must not be interrupted!
        addEmoteForEvent(Event_OnCombatStart, 4850);    // Perhaps I can find a use for you...
        addEmoteForEvent(Event_OnCombatStart, 4851);    // Anger...hate... These are tools I can use.

        addEmoteForEvent(Event_OnTargetDied, 4852);     // Let's see what I can make of you!
        addEmoteForEvent(Event_OnTargetDied, 4853);     // It is pointless to resist.
        addEmoteForEvent(Event_OnDied, 4854);           // Stay away from... Me!
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        GameObject* pDoor = getNearestGameObject(327.155487f, 149.753418f, 9.559869f, GO_THE_MAKER);
        if (pDoor)
            pDoor->setState(GO_STATE_OPEN);
    }
};

void SetupBloodFurnace(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_KELIDAN_THE_BREAKER, &KelidanTheBreakerAI::Create);
    mgr->register_creature_script(CN_BROGGOK, &BroggokAI::Create);
    mgr->register_creature_script(CN_THE_MAKER, &TheMakerAI::Create);
}
