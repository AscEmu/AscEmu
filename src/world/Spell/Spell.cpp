/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Spell.h"
#include "SpellAuras.h"
#include "Definitions/AuraInterruptFlags.h"
#include "Definitions/AuraStates.h"
#include "Definitions/SpellCastTargetFlags.h"
#include "Definitions/SpellDamageType.h"
#include "Definitions/SpellEffectTarget.h"
#include "Definitions/SpellInFrontStatus.h"
#include "Definitions/SpellMechanics.h"
#include "Definitions/SpellRanged.h"

#include "Management/Battleground/Battleground.h"
#include "Management/ItemInterface.h"
#include "Map/Area/AreaStorage.hpp"
#include "Map/MapMgr.h"
#include "Objects/ObjectMgr.h"
#include "Units/Creatures/Pet.h"

bool Spell::canAttackCreatureType(Creature* target)
{
    // Skip check for Grounding Totem
    if (target->getCreatedBySpellId() == 8177)
        return true;

    const auto typeMask = GetSpellInfo()->getTargetCreatureType();
    const auto mask = 1 << (target->GetCreatureProperties()->Type - 1);
    return (target->GetCreatureProperties()->Type != 0 && typeMask != 0 && (typeMask & mask) == 0) ? false : true;
}

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
                if (GetSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_NOT_IN_ARENAS || (spellCooldown > 10 * MINUTE * IN_MILLISECONDS && !(GetSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_NOT_IN_ARENAS)))
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
            if (GetSpellInfo()->isRangedAutoRepeat() || GetSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
                return SPELL_FAILED_MOVING;
        }
    }

    if (u_caster != nullptr)
    {
        // Check if caster is alive
        if (!u_caster->isAlive() && !(GetSpellInfo()->getAttributes() & ATTRIBUTES_DEAD_CASTABLE || (m_triggeredSpell && !m_triggeredByAura)))
            return SPELL_FAILED_CASTER_DEAD;

        // Check if spell requires caster to be in combat
        if (GetSpellInfo()->getAttributes() & ATTRIBUTES_STOP_ATTACK && GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_UNAFFECTED_BY_SCHOOL_IMMUNITY && !u_caster->CombatStatus.IsInCombat())
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

        // Caster's aura state requirements
        if (GetSpellInfo()->getCasterAuraState() > 0 && !u_caster->hasAuraState(AuraState(GetSpellInfo()->getCasterAuraState()), GetSpellInfo(), u_caster))
            return SPELL_FAILED_CASTER_AURASTATE;
        if (GetSpellInfo()->getCasterAuraStateNot() > 0 && u_caster->hasAuraState(AuraState(GetSpellInfo()->getCasterAuraStateNot()), GetSpellInfo(), u_caster))
            return SPELL_FAILED_CASTER_AURASTATE;

        // Caster's aura spell requirements
        if (GetSpellInfo()->getCasterAuraSpell() > 0 && !u_caster->HasAura(GetSpellInfo()->getCasterAuraSpell()))
            return SPELL_FAILED_CASTER_AURASTATE;
        if (GetSpellInfo()->getCasterAuraSpellNot() > 0)
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

        if (!m_triggeredSpell)
        {
            // Out of combat spells should not be able to be casted in combat
            if (requireCombat && (GetSpellInfo()->getAttributes() & ATTRIBUTES_REQ_OOC) && u_caster->CombatStatus.IsInCombat())
                return SPELL_FAILED_AFFECTING_COMBAT;

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

    ////////////////////////////////////////////////////////
    // Target checks

    // Unit target
    const auto target = m_caster->GetMapMgrUnit(m_targets.m_unitTarget);
    if (target != nullptr)
    {
        // Target's aura state requirements
        if (!m_triggeredSpell && GetSpellInfo()->getTargetAuraState() > 0 && !target->hasAuraState(AuraState(GetSpellInfo()->getTargetAuraState()), GetSpellInfo(), u_caster))
            return SPELL_FAILED_TARGET_AURASTATE;
        if (GetSpellInfo()->getTargetAuraStateNot() > 0 && target->hasAuraState(AuraState(GetSpellInfo()->getTargetAuraState()), GetSpellInfo(), u_caster))
            return SPELL_FAILED_TARGET_AURASTATE;

        // Target's aura spell requirements
        if (GetSpellInfo()->getTargetAuraSpell() > 0 && !target->HasAura(GetSpellInfo()->getTargetAuraSpell()))
            return SPELL_FAILED_TARGET_AURASTATE;
        if (GetSpellInfo()->getTargetAuraSpellNot() > 0)
        {
            // TODO: I leave this here for now (from my old work), but this really should be moved to wotlk spellscript -Appled
            // Paladin's Avenging Wrath / Forbearance thing
            if (GetSpellInfo()->getTargetAuraSpellNot() == 61988)
            {
                if (target->HasAura(61987))
                    return SPELL_FAILED_TARGET_AURASTATE;
            }
            else if (target->HasAura(GetSpellInfo()->getTargetAuraSpellNot()))
                return SPELL_FAILED_TARGET_AURASTATE;
        }

        if (target->isCorpse())
        {
            // Player can't cast spells on corpses with bones only left
            const auto targetCorpse = objmgr.GetCorpseByOwner(target->getGuidLow());
            if (targetCorpse == nullptr || !targetCorpse->IsInWorld() || targetCorpse->GetCorpseState() == CORPSE_STATE_BONES)
                return SPELL_FAILED_BAD_TARGETS;
        }

        if (GetSpellInfo()->getAttributesEx() & ATTRIBUTESEX_CANT_TARGET_SELF && m_caster == target)
            return SPELL_FAILED_BAD_TARGETS;

        // Check if spell requires target to be out of combat
        if (GetSpellInfo()->getAttributesEx() & ATTRIBUTESEX_REQ_OOC_TARGET && target->getcombatstatus()->IsInCombat())
            return SPELL_FAILED_TARGET_AFFECTING_COMBAT;

        if (!(GetSpellInfo()->getAttributesExF() & ATTRIBUTESEXF_CAN_TARGET_INVISIBLE) && (u_caster != nullptr && !u_caster->canSee(target)))
            return SPELL_FAILED_BAD_TARGETS;

        if (GetSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_TARGET_ONLY_GHOSTS)
        {
            if (!target->hasAuraWithAuraEffect(SPELL_AURA_GHOST))
                return SPELL_FAILED_TARGET_NOT_GHOST;
        }
        else
        {
            if (target->hasAuraWithAuraEffect(SPELL_AURA_GHOST))
                return SPELL_FAILED_BAD_TARGETS;
        }

        // Check if target can be resurrected
        ///\ Appled's personal note: move this to effect check later
        if (GetSpellInfo()->hasEffect(SPELL_EFFECT_RESURRECT) || GetSpellInfo()->hasEffect(SPELL_EFFECT_RESURRECT_FLAT) || GetSpellInfo()->hasEffect(SPELL_EFFECT_SELF_RESURRECT))
        {
            if (target->isAlive())
                return SPELL_FAILED_TARGET_NOT_DEAD;
            if (target->hasAuraWithAuraEffect(SPELL_AURA_PREVENT_RESURRECTION))
                return SPELL_FAILED_TARGET_CANNOT_BE_RESURRECTED;
        }

        if (m_caster != target)
        {
            if (p_caster != nullptr)
            {
                // Check if caster can attack this creature type
                if (target->isCreature())
                {
                    if (!canAttackCreatureType(dynamic_cast<Creature*>(target)))
                        return SPELL_FAILED_BAD_TARGETS;
                }

                // Check if target is already tagged
                // Several spells cannot be casted at already tagged creatures
                // TODO: implement this error message for skinning, mining and herbalism (mining and herbalism cata only)
                if (GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_CANT_TARGET_TAGGED && target->IsTagged())
                {
                    if (target->GetTaggerGUID() != p_caster->getGuid())
                    {
                        // Player isn't the tagger, check if player is in group with the tagger
                        if (p_caster->InGroup())
                        {
                            const auto playerTagger = p_caster->GetMapMgrPlayer(target->GetTaggerGUID());
                            if (playerTagger == nullptr || !p_caster->GetGroup()->HasMember(playerTagger))
                                return SPELL_FAILED_CANT_CAST_ON_TAPPED;
                        }
                        else
                            return SPELL_FAILED_CANT_CAST_ON_TAPPED;
                    }
                }

                ///\ Appled's personal note: move this to effect check later
                if (GetSpellInfo()->getMechanicsType() == MECHANIC_DISARMED)
                {
                    if (target->getUnitFlags() & UNIT_FLAG_DISARMED)
                        return SPELL_FAILED_TARGET_NO_WEAPONS;

                    if (target->isPlayer())
                    {
                        // Check if target has no weapon or if target is already disarmed
                        const auto mainHandWeapon = dynamic_cast<Player*>(target)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                        if (mainHandWeapon == nullptr || mainHandWeapon->getItemProperties()->Class != ITEM_CLASS_WEAPON)
                            return SPELL_FAILED_TARGET_NO_WEAPONS;
                    }
                    else
                    {
                        if (target->getVirtualItemSlotId(MELEE) == 0)
                            return SPELL_FAILED_TARGET_NO_WEAPONS;
                    }
                }

                // GM flagged players should be immune to other players' casts, but not their own
                if (target->isPlayer() && (dynamic_cast<Player*>(target)->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) || dynamic_cast<Player*>(target)->m_isGmInvisible))
                    return SPELL_FAILED_BM_OR_INVISGOD;
            }

            // Do facing checks only for unit casters
            if (u_caster != nullptr)
            {
                // Target must be in front of caster
                // Check for generic ranged spells as well
                if (GetSpellInfo()->getFacingCasterFlags() == SPELL_INFRONT_STATUS_REQUIRE_INFRONT || GetSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_RANGED)
                {
                    if (!u_caster->isInFront(target))
                        return SPELL_FAILED_UNIT_NOT_INFRONT;
                }

                // Target must be behind caster
                if (GetSpellInfo()->getFacingCasterFlags() == SPELL_INFRONT_STATUS_REQUIRE_INBACK)
                {
                    if (u_caster->isInFront(target))
                        return SPELL_FAILED_UNIT_NOT_BEHIND;
                }

                // Caster must be behind the target
                if (GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_REQ_BEHIND_TARGET && GetSpellInfo()->getAttributesEx() & ATTRIBUTESEX_REQ_FACING_TARGET && target->isInFront(u_caster))
                {
                    // Throw spell has these attributes in 3.3.5a, ignore
                    if (GetSpellInfo()->getId() != SPELL_RANGED_THROW
#if VERSION_STRING >= TBC
                        // Druid - Pounce, "Patch 2.0.3 - Pounce no longer requires the druid to be behind the target."
                        && !(GetSpellInfo()->getSpellFamilyName() == 7 && GetSpellInfo()->getSpellFamilyFlags(0) == 0x20000)
#endif
#if VERSION_STRING >= WotLK
                        // Rogue - Mutilate, "Patch 3.0.2 - Mutilate no longer requires you be behind the target."
                        && !(GetSpellInfo()->getSpellFamilyName() == 8 && GetSpellInfo()->getSpellFamilyFlags(1) == 0x200000)
#endif
                        )
                        return SPELL_FAILED_NOT_BEHIND;
                }

                // Caster must be in front of target
                // in 3.3.5a only rogue's and npcs' Gouge spell
                if ((GetSpellInfo()->getAttributes() == (ATTRIBUTES_ABILITY | ATTRIBUTES_NOT_SHAPESHIFT | ATTRIBUTES_UNK20 | ATTRIBUTES_STOP_ATTACK)
                    || GetSpellInfo()->getAttributesEx() & ATTRIBUTESEX_REQ_FACING_TARGET) && !target->isInFront(u_caster))
                    return SPELL_FAILED_NOT_INFRONT;
            }

            // Check if spell can be casted on dead target
            if (!((GetSpellInfo()->getTargets() & (TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 | TARGET_FLAG_UNIT_CORPSE)) ||
                GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_CAN_BE_CASTED_ON_DEAD_TARGET) && !target->isAlive())
                return SPELL_FAILED_TARGETS_DEAD;

            if (target->hasAuraWithAuraEffect(SPELL_AURA_SPIRIT_OF_REDEMPTION))
                return SPELL_FAILED_BAD_TARGETS;

            // Line of Sight check
            if (worldConfig.terrainCollision.isCollisionEnabled)
            {
                if (m_caster->IsInWorld() && !(GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_IGNORE_LINE_OF_SIGHT) && !(GetSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SKIP_LINE_OF_SIGHT_CHECK) &&
                    (m_caster->GetMapId() != target->GetMapId() || !m_caster->GetMapMgr()->isInLineOfSight(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ())))
                    return SPELL_FAILED_LINE_OF_SIGHT;
            }

            if (GetSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_TARGET_ONLY_PLAYERS && !target->isPlayer())
                return SPELL_FAILED_TARGET_NOT_PLAYER;
        }
    }

    // Check if spell effect requires pet target
    if (p_caster != nullptr)
    {
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (GetSpellInfo()->getEffectImplicitTargetA(i) == EFF_TARGET_PET)
            {
                const auto pet = p_caster->GetSummon();
                if (pet == nullptr)
                    return m_triggeredByAura ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_NO_PET;
                else if (!pet->isAlive())
                    return SPELL_FAILED_TARGETS_DEAD;
            }
        }
    }

    ////////////////////////////////////////////////////////
    // Area checks

    // Check Line of Sight for spells with a destination
    if (m_targets.hasDestination() && worldConfig.terrainCollision.isCollisionEnabled)
    {
        if (m_caster->IsInWorld() && !(GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_IGNORE_LINE_OF_SIGHT) && !(GetSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SKIP_LINE_OF_SIGHT_CHECK) &&
            !m_caster->GetMapMgr()->isInLineOfSight(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_targets.destination().x, m_targets.destination().y, m_targets.destination().z))
            return SPELL_FAILED_LINE_OF_SIGHT;
    }

    if (p_caster != nullptr)
    {
        // Check if spell requires certain area
        // TODO: load area DBC file for tbc
#if VERSION_STRING >= WotLK
        if (GetSpellInfo()->getRequiresAreaId() > 0)
        {
            auto found = false;
            auto areaGroup = sAreaGroupStore.LookupEntry(GetSpellInfo()->getRequiresAreaId());
            const auto areaEntry = p_caster->GetArea();
            while (areaGroup != nullptr)
            {
                for (auto i = 0; i < 6; ++i)
                {
                    if (areaGroup->AreaId[i] == areaEntry->id || (areaEntry->zone != 0 && areaGroup->AreaId[i] == areaEntry->zone))
                        found = true;
                }

                if (found || areaGroup->next_group == 0)
                    break;

                areaGroup = sAreaGroupStore.LookupEntry(areaGroup->next_group);
            }

            if (!found)
                return SPELL_FAILED_INCORRECT_AREA;
        }
#endif

        // Check if spell can be casted in heroic dungeons or in raids
        if (GetSpellInfo()->getAttributesExF() & ATTRIBUTESEXF_NOT_IN_RAIDS_OR_HEROIC_DUNGEONS)
        {
            if (p_caster->IsInWorld() && (p_caster->GetMapMgr()->GetMapInfo()->type == INSTANCE_RAID || p_caster->GetMapMgr()->iInstanceMode == MODE_HEROIC))
                return SPELL_FAILED_NOT_IN_RAID_INSTANCE;
        }
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
