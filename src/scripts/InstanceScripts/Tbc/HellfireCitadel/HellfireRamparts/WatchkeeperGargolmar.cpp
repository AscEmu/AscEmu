/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "WatchkeeperGargolmar.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Watchkeeper Gargolmar
WatchkeeperGargolmarAI::WatchkeeperGargolmarAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Spells
    auto surge = addAISpell(SPELL_SURGE, 20.0f, TARGET_RANDOM_SINGLE, 0, 15);
    surge->addDBEmote(WATCH_SAY_SURGE);
    surge->setMinMaxDistance(5.0f, 40.0f);

    mOverpower = addAISpell(SPELL_OVERPOWER, 10.0f, TARGET_ATTACKING, 0, 5);
    mRetaliation = addAISpell(SPELL_RETALIATION, 0.0f, TARGET_SELF);

    if (isHeroic())
        mMortalWound = addAISpell(SPELL_MORTAL_WOUND_H, 15.0f, TARGET_ATTACKING, 0, 12);
    else
        mMortalWound = addAISpell(SPELL_MORTAL_WOUND, 15.0f, TARGET_ATTACKING, 0, 12);

    // Emotes
    addEmoteForEvent(Event_OnCombatStart, WATCH_SAY_AGGRO0);      // What have we here?
    addEmoteForEvent(Event_OnCombatStart, WATCH_SAY_AGGRO1);      // This may hurt a little....
    addEmoteForEvent(Event_OnCombatStart, WATCH_SAY_AGGRO2);      // I'm going to enjoy this...
    addEmoteForEvent(Event_OnTargetDied, WATCH_SAY_KILL0);        // Say farewell!
    addEmoteForEvent(Event_OnTargetDied, WATCH_SAY_KILL1);        // Much too easy.
    addEmoteForEvent(Event_OnDied, WATCH_SAY_DIE);                // Hahah.. <cough> ..argh!
}

CreatureAIScript* WatchkeeperGargolmarAI::Create(Creature* pCreature) { return new WatchkeeperGargolmarAI(pCreature); }

void WatchkeeperGargolmarAI::OnCombatStop(Unit* /*mTarget*/)
{
    reset();
}

void WatchkeeperGargolmarAI::AIUpdate(unsigned long /*time_passed*/)
{
    if (getCreature()->getHealthPct() <= 40 && !mCalledForHelp)
    {
        sendDBChatMessage(WATCH_SAY_HEAL);      // Heal me, quickly!
        mCalledForHelp = true;
    }

    if (getCreature()->getHealthPct() <= 20 && !_retaliation)
    {
        _castAISpell(mRetaliation);
        getCreature()->setAttackTimer(MELEE, 1500);
        _retaliation = true;
    }
}

void WatchkeeperGargolmarAI::reset()
{
    areaTriggered = false;
    mCalledForHelp = false;
    _retaliation = false;
}
