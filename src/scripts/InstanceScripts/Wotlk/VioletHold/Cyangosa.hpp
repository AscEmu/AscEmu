/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Instance_TheVioletHold.hpp"

namespace Cyanigosa
{
    enum Spells
    {
        SPELL_SUMMON_PLAYER         = 21150,
        SPELL_ARCANE_VACUUM         = 58694,
        SPELL_BLIZZARD              = 58693,
        SPELL_MANA_DESTRUCTION      = 59374,
        SPELL_TAIL_SWEEP            = 58690,
        SPELL_UNCONTROLLABLE_ENERGY = 58688,
        SPELL_TRANSFORM             = 58668
    };

    enum Yells
    {
        SAY_AGGRO                   = 4521,
        SAY_SLAY1                   = 4523,
        SAY_SLAY2                   = 5169,
        SAY_SLAY3                   = 5170,
        SAY_DEATH                   = 5171,
        SAY_SPAWN                   = 5172,
        SAY_DISRUPTION              = 5173,
        SAY_BREATH_ATTACK           = 5174,
        SAY_SPECIAL_ATTACK1         = 5175,
        SAY_SPECIAL_ATTACK2         = 5176
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Cyangosa AI
class CyangosaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit CyangosaAI(Creature* pCreature);

    void OnDied(Unit* /*_killer*/) override;

protected:
    InstanceScript* mInstance;
};
