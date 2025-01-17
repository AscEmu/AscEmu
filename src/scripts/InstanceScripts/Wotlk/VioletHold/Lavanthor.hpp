/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace Lavanthor
{
    enum Spells
    {
        SPELL_CAUTERIZING_FLAMES        = 59466, // Only in heroic
        SPELL_FIREBOLT                  = 54235,
        SPELL_FLAME_BREATH              = 54282,
        SPELL_LAVA_BURN                 = 54249
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Lavanthor AI
class LavanthorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit LavanthorAI(Creature* pCreature);

    void OnLoad() override;
    void OnDied(Unit* /*_killer*/) override;
    void justReachedSpawn() override;

protected:
    InstanceScript* mInstance;
};
