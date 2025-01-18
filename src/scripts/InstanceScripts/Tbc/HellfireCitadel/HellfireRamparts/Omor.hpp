/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Server/Script/CreatureAIScript.hpp"

enum OmorSpells
{
    // All
    SPELL_DEMONIC_SHIELD                = 31901,
    SPELL_SUMMON_FIENDISH_HOUND         = 30707,
    
    // Normal
    SPELL_SHADOW_BOLT                   = 30686,
    SPELL_TREACHEROUS_AURA              = 30695,

    // Heroic
    SPELL_BANE_OF_TREACHERY             = 37566,
    SPELL_SHADOW_BOLT_H                 = 39297    
};

enum OmorSay
{
    OMOR_SAY_AGGRO0                     = 4855,
    OMOR_SAY_AGGRO1                     = 4856,
    OMOR_SAY_AGGRO2                     = 4857,
    OMOR_SAY_SUMMON                     = 4858,
    OMOR_SAY_CURSE                      = 4859,
    OMOR_SAY_KILL_1                     = 4860,
    OMOR_SAY_DIE                        = 4861,
    OMOR_SAY_WIPE                       = 4862
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Omor the Unscarred
class OmorTheUnscarredAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit OmorTheUnscarredAI(Creature* pCreature);

    void OnCombatStart(Unit* /*pTarget*/) override;
    void OnCombatStop(Unit* /*pTarget*/) override;
    void onSummonedCreature(Creature* /*summon*/) override;

protected:
    CreatureAISpells* m_SummonSpell;
    CreatureAISpells* m_ShieldSpell;
    CreatureAISpells* m_ShadowBoltSpell;
    CreatureAISpells* m_TreacherousAura;
    CreatureAISpells* m_BaneOfTreacheryAura;
};
