/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/CreatureAIScript.hpp"

enum WatchkeeperSpells
{
    // All
    SPELL_SURGE                         = 34645,
    SPELL_OVERPOWER                     = 32154,
    SPELL_RETALIATION                   = 22857,

    // Normal
    SPELL_MORTAL_WOUND                  = 30641,

    // Heroic
    SPELL_MORTAL_WOUND_H                = 36814
};

enum WatchkeeperSay
{
    WATCH_SAY_TRIGGER                   = 4870,     // Currently not added
    WATCH_SAY_HEAL                      = 4871,
    WATCH_SAY_SURGE                     = 4872,
    WATCH_SAY_AGGRO0                    = 4873,
    WATCH_SAY_AGGRO1                    = 4874,
    WATCH_SAY_AGGRO2                    = 4875,
    WATCH_SAY_KILL0                     = 4876,
    WATCH_SAY_KILL1                     = 4877,
    WATCH_SAY_DIE                       = 4878
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Watchkeeper Gargolmar
class WatchkeeperGargolmarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit WatchkeeperGargolmarAI(Creature* pCreature);

    void OnCombatStop(Unit* /*mTarget*/) override;

    void AIUpdate(unsigned long time_passed) override;

    void reset();

protected:
    bool areaTriggered = false;
    bool mCalledForHelp = false;
    bool _retaliation = false;

    CreatureAISpells* mRetaliation;
    CreatureAISpells* mMortalWound;
    CreatureAISpells* mOverpower;
};