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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "Setup.h"
#include "Instance_BloodFurnace.h"


// Keli'dan the BreakerAI
class KelidanTheBreakerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KelidanTheBreakerAI);
    KelidanTheBreakerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //spells
        if (_isHeroic())
        {
            mShadowBoltVolley = AddSpell(KELIDAN_SHADOW_BOLT_VOLLEY_H, Target_Self, 25, 0, 6);
            mFireNova = AddSpell(KELIDAN_FIRE_NOVA_H, Target_Current, 15, 0, 12);
        }
        else
        {
            mShadowBoltVolley = AddSpell(KELIDAN_SHADOW_BOLT_VOLLEY, Target_Self, 25, 0, 6);
            mFireNova = AddSpell(KELIDAN_FIRE_NOVA, Target_Self, 15, 0, 12);
        }

        mBurningNova = AddSpell(KELIDAN_BURNING_NOVA, Target_Self, 0, 0, 0);
        mBurningNova->addEmote("Closer! Come closer... and burn!", CHAT_MSG_MONSTER_YELL);
        mVortex = AddSpell(KELIDAN_FIRE_NOVA, Target_Self, 0, 0, 0);
        AddSpell(KELIDAN_CORRUPTION, Target_Current, 15, 0, 10);

        mBurningNovaTimer = INVALIDATE_TIMER;
        SetAIUpdateFreq(800);

        // new
        addEmoteForEvent(Event_OnCombatStart, 4841);    // Who dares interrupt--What is this; what have you done? You'll ruin everything!
        addEmoteForEvent(Event_OnTargetDied, 4845);     // Just as you deserve!
        addEmoteForEvent(Event_OnTargetDied, 4846);     // Your friends will soon be joining you!
        addEmoteForEvent(Event_OnDied, 4848);           // Good...luck. You'll need it.
    }

    void OnCombatStart(Unit* pTarget) override
    {
        mBurningNovaTimer = _addTimer(15000);
    }

    void AIUpdate() override
    {
        if (!_isCasting())
        {
            if (mBurningNovaTimer == INVALIDATE_TIMER || _isTimerFinished(mBurningNovaTimer))
            {
                if (_isHeroic())
                    CastSpell(mVortex);
                CastSpell(mBurningNova);

                _resetTimer(mBurningNovaTimer, 30000);
            }
        }
    }

    SpellDesc* mShadowBoltVolley;
    SpellDesc* mFireNova;
    SpellDesc* mBurningNova;
    SpellDesc* mVortex;
    int32 mBurningNovaTimer;
};


// Broggok
class BroggokAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(BroggokAI);
        BroggokAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(POISON_BOLT, Target_Self, 12.0f, 0, 15);
            AddSpell(POISON_CLOUD, Target_RandomPlayerDestination, 8.0f, 0, 40, 0, 40);
            AddSpell(SLIME_SPRAY, Target_Self, 10.0f, 0, 25);
        }

        void OnDied(Unit* pKiller) override
        {
            GameObject* pDoor = getNearestGameObject(456.157349f, 34.248005f, 9.559463f, GO_BROGGOK);
            if (pDoor)
                pDoor->SetState(GO_STATE_OPEN);
        }
};


// The Maker
class TheMakerAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TheMakerAI);
        TheMakerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(DOMINATION, Target_RandomPlayer, 8.0f, 0, 30);
            AddSpell(ACID_SPRAY, Target_Self, 10.0f, 0, 20);
            AddSpell(THROW_BEAKER, Target_RandomPlayerDestination, 20.0f, 0, 0, 0, 40);

            // new
            addEmoteForEvent(Event_OnCombatStart, 4849);    // My work must not be interrupted!
            addEmoteForEvent(Event_OnCombatStart, 4850);    // Perhaps I can find a use for you...
            addEmoteForEvent(Event_OnCombatStart, 4851);    // Anger...hate... These are tools I can use.

            addEmoteForEvent(Event_OnTargetDied, 4852);     // Let's see what I can make of you!
            addEmoteForEvent(Event_OnTargetDied, 4853);     // It is pointless to resist.
            addEmoteForEvent(Event_OnDied, 4854);           // Stay away from... Me!
        }

        void OnDied(Unit* pKiller) override
        {
            GameObject* pDoor = getNearestGameObject(327.155487f, 149.753418f, 9.559869f, GO_THE_MAKER);
            if (pDoor)
                pDoor->SetState(GO_STATE_OPEN);
        }
};

void SetupBloodFurnace(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_KELIDAN_THE_BREAKER, &KelidanTheBreakerAI::Create);
    mgr->register_creature_script(CN_BROGGOK, &BroggokAI::Create);
    mgr->register_creature_script(CN_THE_MAKER, &TheMakerAI::Create);
}
