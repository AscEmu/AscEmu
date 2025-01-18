/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Cyangosa.hpp"

#include "Server/Script/InstanceScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//  Moragg AI
CyangosaAI::CyangosaAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    // Spells
    addAISpell(Cyanigosa::SPELL_ARCANE_VACUUM, 75.0f, TARGET_SOURCE, 0, 10);
    addAISpell(Cyanigosa::SPELL_BLIZZARD, 75.0f, 15, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 45.0f); });
    addAISpell(Cyanigosa::SPELL_TAIL_SWEEP, 75.0f, 20, [this]() { return getBestUnitTarget(TargetFilter_Current); });
    addAISpell(Cyanigosa::SPELL_UNCONTROLLABLE_ENERGY, 75.0f, 25, [this]() { return getBestUnitTarget(TargetFilter_Current); });
    addAISpell(Cyanigosa::SPELL_MANA_DESTRUCTION, 75.0f, 25, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 50.0f); }, false, true);

    // Emotes
    addEmoteForEvent(Event_OnCombatStart, Cyanigosa::SAY_AGGRO);
    addEmoteForEvent(Event_OnTargetDied, Cyanigosa::SAY_SLAY1);
    addEmoteForEvent(Event_OnTargetDied, Cyanigosa::SAY_SLAY2);
    addEmoteForEvent(Event_OnTargetDied, Cyanigosa::SAY_SLAY3);
    addEmoteForEvent(Event_OnDied, Cyanigosa::SAY_DEATH);
}

CreatureAIScript* CyangosaAI::Create(Creature* pCreature) { return new CyangosaAI(pCreature); }

void CyangosaAI::OnDied(Unit* /*_killer*/)
{
    if (mInstance)
        mInstance->setBossState(DATA_CYANIGOSA, Performed);
}
