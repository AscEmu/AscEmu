/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Spell.h"
#include "SpellAuras.h"
#include "Definitions/AuraStates.h"

SpellCastResult Spell::canCast(bool tolerate)
{
    if (p_caster != nullptr && !GetSpellInfo()->isPassive() && !m_triggeredSpell)
    {
        // You can't cast other spells if you have the player flag preventing cast
        if (p_caster->hasPlayerFlags(PLAYER_FLAG_PREVENT_SPELL_CAST))
            return SPELL_FAILED_SPELL_IN_PROGRESS;

        // Check for cooldown
        if (!p_caster->Cooldown_CanCast(GetSpellInfo()))
        {
            if (m_triggeredByAura)
                return SPELL_FAILED_DONT_REPORT;
            else
                return SPELL_FAILED_NOT_READY;
        }
    }

    if (u_caster != nullptr)
    {
        auto requireCombat = true;
        if (u_caster->hasAuraWithAuraEffect(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
        {
            for (auto i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
            {
                if (u_caster->m_auras[i] == nullptr)
                    continue;
                if (!u_caster->m_auras[i]->GetSpellInfo()->hasEffectApplyAuraName(SPELL_AURA_IGNORE_TARGET_AURA_STATE))
                    continue;
                if (u_caster->m_auras[i]->GetSpellInfo()->isAffectingSpell(GetSpellInfo()))
                {
                    // Warrior's Overpower uses "combo points" based on dbc data
                    // This allows usage of Overpower if we have an affecting aura (i.e. Taste for Blood)
                    m_requiresCP = false;

                    // All these aura effects use effect index 0
                    // Allow Warrior's Charge to be casted on combat if caster has Juggernaut or Warbringer talent
                    if (u_caster->m_auras[i]->GetSpellInfo()->getEffectMiscValue(0) == 1)
                    {
                        // TODO: currently not working, serverside everything was OK but client still gives "You are in combat" error
                        requireCombat = false;
                        break;
                    }
                }
            }
        }

        if (!m_triggeredSpell)
        {
            // Out of combat spells should not be able to be casted in combat
            if (requireCombat && (GetSpellInfo()->getAttributes() & ATTRIBUTES_REQ_OOC) && u_caster->CombatStatus.IsInCombat())
                return SPELL_FAILED_AFFECTING_COMBAT;

            // Caster's aura state requirements
            if (GetSpellInfo()->getCasterAuraState() && !u_caster->hasAuraState(AuraState(GetSpellInfo()->getCasterAuraState()), GetSpellInfo(), u_caster))
                return SPELL_FAILED_CASTER_AURASTATE;
            if (GetSpellInfo()->getCasterAuraStateNot() && u_caster->hasAuraState(AuraState(GetSpellInfo()->getCasterAuraStateNot()), GetSpellInfo(), u_caster))
                return SPELL_FAILED_CASTER_AURASTATE;

            // Caster's aura spell requirements
            if (GetSpellInfo()->getCasterAuraSpell() != 0 && !u_caster->HasAura(GetSpellInfo()->getCasterAuraSpell()))
                return SPELL_FAILED_CASTER_AURASTATE;
            if (GetSpellInfo()->getCasterAuraSpellNot() != 0)
            {
                // TODO: I leave this here for now (from my old work), but this really should be moved to wotlk spellscript -Appled
                // Paladin's Avenging Wrath / Forbearance thing
                if (GetSpellInfo()->getCasterAuraSpellNot() == 61988)
                {
                    if (u_caster->HasAura(61987))
                        return SPELL_FAILED_CASTER_AURASTATE;
                }
                else if (u_caster->HasAura(GetSpellInfo()->getCasterAuraSpellNot()))
                    return SPELL_FAILED_CASTER_AURASTATE;
            }
        }
    }

    // Call legacy CanCast for yet unhandled cases
    return m_triggeredSpell || ProcedOnSpell != nullptr ? SPELL_CANCAST_OK : SpellCastResult(CanCast(tolerate));
}
