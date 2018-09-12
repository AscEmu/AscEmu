/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Spell.h"
#include "SpellAuras.h"
#include "Definitions/AuraInterruptFlags.h"
#include "Definitions/AuraStates.h"
#include "Management/Battleground/Battleground.h"
#include "Map/Area/AreaStorage.hpp"

SpellCastResult Spell::canCast(bool tolerate)
{
    ////////////////////////////////////////////////////////
    // Caster checks

    if (p_caster != nullptr)
    {
        if (!GetSpellInfo()->isPassive() && !m_triggeredSpell)
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

        if (GetSpellInfo()->getAttributesExG() & ATTRIBUTESEXG_IS_CHEAT_SPELL && !p_caster->GetSession()->HasGMPermissions())
        {
            m_extraError = SPELL_EXTRA_ERROR_GM_ONLY;
            return SPELL_FAILED_CUSTOM_ERROR;
        }

        // Battleground checks
        if (p_caster->m_bg != nullptr)
        {
            // Arena checks
            if (isArena(p_caster->m_bg->GetType()))
            {
                // Spells with longer than 10 minute cooldown cannot be casted in arena
                const auto spellCooldown = GetSpellInfo()->getRecoveryTime() > GetSpellInfo()->getCategoryRecoveryTime() ? GetSpellInfo()->getRecoveryTime() : GetSpellInfo()->getCategoryRecoveryTime();
                if (GetSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_NOT_IN_ARENA || (spellCooldown > 10 * MINUTE * IN_MILLISECONDS && !(GetSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_NOT_IN_ARENA)))
                    return SPELL_FAILED_NOT_IN_ARENA;
            }

            // If battleground has ended, don't allow spell casting
            if (!m_triggeredSpell && p_caster->m_bg->HasEnded())
                return SPELL_FAILED_DONT_REPORT;
        }
        else if (GetSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_BG_ONLY)
            return SPELL_FAILED_ONLY_BATTLEGROUNDS;

        // Movement check
        if (p_caster->m_isMoving)
        {
            // No need to check for other interrupt flags, client does that for us
            // Also don't cast first ranged autorepeat spell if we're moving but activate it
            // TODO: Missing cata checks, in cata you can cast some spells while moving
            if (GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_AUTOREPEAT || GetSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
                return SPELL_FAILED_MOVING;
        }
    }

    if (u_caster != nullptr)
    {
        // Check is caster alive
        if (!u_caster->isAlive() && !(GetSpellInfo()->getAttributes() & ATTRIBUTES_DEAD_CASTABLE || (m_triggeredSpell && !m_triggeredByAura)))
            return SPELL_FAILED_CASTER_DEAD;

        // Check if spell requires caster to be in combat
        if (GetSpellInfo()->getAttributes() & ATTRIBUTES_STOP_ATTACK && GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_UNK28 && !u_caster->CombatStatus.IsInCombat())
            return SPELL_FAILED_CASTER_AURASTATE;

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

            if (tolerate)
            {
                // Shapeshift check
                auto hasIgnoreShapeshiftAura = false;
                for (auto u = MAX_TOTAL_AURAS_START; u < MAX_TOTAL_AURAS_END; ++u)
                {
                    if (u_caster->m_auras[u] == nullptr)
                        continue;
                    // If aura has ignore shapeshift type, you can use spells regardless of stance / form
                    // Auras with this type: Shadow Dance, Metamorphosis, Warbringer (in 3.3.5a)
                    if (!u_caster->m_auras[u]->GetSpellInfo()->hasEffectApplyAuraName(SPELL_AURA_IGNORE_SHAPESHIFT))
                        continue;
                    if (u_caster->m_auras[u]->GetSpellInfo()->isAffectingSpell(GetSpellInfo()))
                    {
                        hasIgnoreShapeshiftAura = true;
                        break;
                    }
                }

                if (!hasIgnoreShapeshiftAura)
                {
                    SpellCastResult shapeError = getErrorAtShapeshiftedCast(GetSpellInfo(), u_caster->getShapeShiftForm());
                    if (shapeError != SPELL_CANCAST_OK)
                        return shapeError;

                    // Stealth check
                    if (GetSpellInfo()->getAttributes() & ATTRIBUTES_REQ_STEALTH && !u_caster->hasAuraWithAuraEffect(SPELL_AURA_MOD_STEALTH))
                        return SPELL_FAILED_ONLY_STEALTHED;
                }
            }
        }
    }

    // Indoor and outdoor specific spells
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        if (GetSpellInfo()->getAttributes() & ATTRIBUTES_ONLY_OUTDOORS &&
            !MapManagement::AreaManagement::AreaStorage::IsOutdoor(m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
            return SPELL_FAILED_ONLY_OUTDOORS;

        if (GetSpellInfo()->getAttributes() & ATTRIBUTES_ONLY_INDOORS &&
            MapManagement::AreaManagement::AreaStorage::IsOutdoor(m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
            return SPELL_FAILED_ONLY_INDOORS;
    }

    // Call legacy CanCast for yet unhandled cases
    return m_triggeredSpell || ProcedOnSpell != nullptr ? SPELL_CANCAST_OK : SpellCastResult(CanCast(tolerate));
}

SpellCastResult Spell::getErrorAtShapeshiftedCast(SpellInfo const* spellInfo, const uint32_t shapeshiftForm) const
{
    // No need to check requirements for talents that learn spells
    auto talentRank = 0;
    auto talentInfo = sTalentStore.LookupEntry(spellInfo->getId());
    if (talentInfo != nullptr)
    {
        for (auto i = 0; i < 5; ++i)
        {
            if (talentInfo->RankID[i] != 0)
                talentRank = i + 1;
        }
    }

    // This is client-side only
    if (talentRank > 0 && spellInfo->hasEffect(SPELL_EFFECT_LEARN_SPELL))
        return SPELL_CANCAST_OK;

    const auto stanceMask = shapeshiftForm ? 1 << (shapeshiftForm - 1) : 0;

    // Cannot explicitly be casted in this stance/form
    if (spellInfo->getShapeshiftExclude() > 0 && spellInfo->getShapeshiftExclude() & stanceMask)
        return SPELL_FAILED_NOT_SHAPESHIFT;

    // Can explicitly be casted in this stance/form
    if (spellInfo->getRequiredShapeShift() > 0 && spellInfo->getRequiredShapeShift() & stanceMask)
        return SPELL_CANCAST_OK;

    auto actAsShifted = false;
    if (stanceMask > FORM_NORMAL)
    {
        auto shapeShift = sSpellShapeshiftFormStore.LookupEntry(shapeshiftForm);
        if (shapeShift == nullptr)
        {
            LOG_ERROR("Spell::getErrorAtShapeshiftedCast: Caster has unknown shapeshift form %u", shapeshiftForm);
            return SPELL_CANCAST_OK;
        }

        // Shapeshift acts as normal form for spells
        actAsShifted = !(shapeShift->Flags & 1);
    }

    if (actAsShifted)
    {
        // Cannot be casted while shapeshifted
        if (spellInfo->getAttributes() & ATTRIBUTES_NOT_SHAPESHIFT)
            return SPELL_FAILED_NOT_SHAPESHIFT;
        // Needs another shapeshift form
        else if (spellInfo->getRequiredShapeShift() != 0)
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }
    else
    {
        // Check if spell even requires shapeshift
        if (!(spellInfo->getAttributesExB() & ATTRIBUTESEXB_NOT_NEED_SHAPESHIFT) && spellInfo->getRequiredShapeShift() != 0)
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }
    return SPELL_CANCAST_OK;
}
