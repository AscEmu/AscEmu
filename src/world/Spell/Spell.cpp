/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Spell.h"

SpellCastResult Spell::canCast(bool tolerate, uint32_t* parameter1, uint32_t* parameter2)
{
    if (!GetSpellInfo()->hasAttributes(ATTRIBUTES_PASSIVE) && p_caster != nullptr)
    {
        // You can't cast other spells if you have SPELL_AURA_ALLOW_ONLY_ABILITY aura (Killing Spree and Bladestorm for example)
        if (!m_triggeredSpell && p_caster->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_ALLOW_ONLY_ABILITY))
        {
            return SPELL_FAILED_SPELL_IN_PROGRESS;
        }

        // Check for cooldown
        if (!m_triggeredSpell && !p_caster->Cooldown_CanCast(GetSpellInfo()))
        {
            if (m_triggeredByAura)
                return SPELL_FAILED_DONT_REPORT;
            else
                return SPELL_FAILED_NOT_READY;
        }
    }

    /*bool requireCombat = true;
    if (u_caster != nullptr && u_caster->hasAuraWithAuraEffect(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
    {
        for (uint32_t i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
        {
            if (u_caster->m_auras[i] == nullptr)
                continue;
            if (!u_caster->m_auras[i]->GetSpellInfo()->HasEffectApplyAuraName(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
                continue;
        }
    }*/

    // Caster's aura state requirements
    /*if (GetSpellInfo()->getCasterAuraState() && !u_caster->hasAuraState(AuraStates(GetSpellInfo()->CasterAuraState), GetSpellInfo(), u_caster))
    {
        return SPELL_FAILED_CASTER_AURASTATE;
    }
    if (GetSpellInfo()->getCasterAuraStateNot() && u_caster->hasAuraState(AuraStates(GetSpellInfo()->CasterAuraStateNot), GetSpellInfo(), u_caster))
    {
        return SPELL_FAILED_CASTER_AURASTATE;
    }*/

    // Caster's aura spell requirements
    if (GetSpellInfo()->getCasterAuraSpell() != 0 && !u_caster->HasAura(GetSpellInfo()->getCasterAuraSpell()))
    {
        return SPELL_FAILED_CASTER_AURASTATE;
    }
    if (GetSpellInfo()->getCasterAuraSpellNot() != 0)
    {
        // TODO: move this to wotlk spellscript
        // Paladin's Avenging Wrath / Forbearance thing
        if (GetSpellInfo()->getCasterAuraSpellNot() == 61988)
        {
            if (u_caster->HasAura(61987))
                return SPELL_FAILED_CASTER_AURASTATE;
        }
        else if (u_caster->HasAura(GetSpellInfo()->getCasterAuraSpellNot()))
            return SPELL_FAILED_CASTER_AURASTATE;
    }

    // Legacy CanCast
    return static_cast<SpellCastResult>(CanCast(tolerate));
}