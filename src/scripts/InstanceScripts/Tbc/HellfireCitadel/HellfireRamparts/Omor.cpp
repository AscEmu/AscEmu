/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Omor.hpp"
#include "Map/AreaBoundary.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Omor The Unscarred
OmorTheUnscarredAI::OmorTheUnscarredAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Boundarys
    getCreature()->getAIInterface()->addBoundary(std::make_unique<CircleBoundary>(getCreature()->GetPosition(), 50.0), true);

    // All
    m_ShieldSpell = addAISpell(SPELL_DEMONIC_SHIELD, 30.0f, TARGET_SELF, 0, 25);
    m_ShieldSpell->setMinMaxPercentHp(0, 20);

    m_SummonSpell = addAISpell(SPELL_SUMMON_FIENDISH_HOUND, 8.0f, TARGET_SELF, 1, 20);
    m_SummonSpell->addDBEmote(SPELL_SUMMON_FIENDISH_HOUND);

    // Normal
    if (!isHeroic())
    {
        m_ShadowBoltSpell = addAISpell(SPELL_SHADOW_BOLT, 8.0f, TARGET_RANDOM_SINGLE, 3, 15, false, true);
        m_ShadowBoltSpell->setMinMaxDistance(10.0f, 60.0f);

        m_TreacherousAura = addAISpell(SPELL_TREACHEROUS_AURA, 8.0f, TARGET_RANDOM_SINGLE, 2, 35, false, true);
        m_TreacherousAura->setMinMaxDistance(0.0f, 60.0f);
        m_TreacherousAura->addDBEmote(OMOR_SAY_SUMMON);
    }

    // Heroic
    if (isHeroic())
    {
        m_ShadowBoltSpell = addAISpell(SPELL_SHADOW_BOLT_H, 8.0f, TARGET_RANDOM_SINGLE, 3, 15, false, true);
        m_ShadowBoltSpell->setMinMaxDistance(10.0f, 60.0f);

        m_BaneOfTreacheryAura = addAISpell(SPELL_BANE_OF_TREACHERY, 8.0f, TARGET_RANDOM_SINGLE, 2, 35, false, true);
        m_BaneOfTreacheryAura->setMinMaxDistance(0.0f, 60.0f);
        m_BaneOfTreacheryAura->addDBEmote(OMOR_SAY_SUMMON);
    }

    // Emotes
    addEmoteForEvent(Event_OnCombatStart, OMOR_SAY_AGGRO0);
    addEmoteForEvent(Event_OnCombatStart, OMOR_SAY_AGGRO1);
    addEmoteForEvent(Event_OnCombatStart, OMOR_SAY_AGGRO2);
    addEmoteForEvent(Event_OnTargetDied, OMOR_SAY_KILL_1);
    addEmoteForEvent(Event_OnDied, OMOR_SAY_DIE);
}

CreatureAIScript* OmorTheUnscarredAI::Create(Creature* pCreature) { return new OmorTheUnscarredAI(pCreature); }

void OmorTheUnscarredAI::OnCombatStart(Unit* /*pTarget*/)
{
    setRooted(true);
}

void OmorTheUnscarredAI::OnCombatStop(Unit* /*pTarget*/)
{
    sendDBChatMessage(OMOR_SAY_WIPE);
}

void OmorTheUnscarredAI::onSummonedCreature(Creature* summon)
{
    if (Unit* random = getBestPlayerTarget(TargetFilter_NotCurrent))
        summon->getAIInterface()->onHostileAction(random);
}
