/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace Moragg
{
    enum Spells
    {
        SPELL_CORROSIVE_SALIVA  = 54527,
        SPELL_OPTIC_LINK        = 54396,
        SPELL_RAY_OF_PAIN       = 54438,
        SPELL_RAY_OF_SUFFERING  = 54442,

        // Visual
        SPELL_OPTIC_LINK_LEVEL_1 = 54393,
        SPELL_OPTIC_LINK_LEVEL_2 = 54394,
        SPELL_OPTIC_LINK_LEVEL_3 = 54395
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Moragg AI
class MoraggAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit MoraggAI(Creature* pCreature);

    void OnLoad() override;
    void OnDied(Unit* /*_killer*/) override;
    void justReachedSpawn() override;

protected:
    InstanceScript* mInstance;
};
