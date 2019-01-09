/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Storage/MySQLDataStore.hpp"
#include "Units/Creatures/Pet.h"

SpellCastResult Spell::canCast(bool tolerate, uint32_t* parameter1, uint32_t* parameter2)
{
    ////////////////////////////////////////////////////////
    // Caster checks

    if (p_caster != nullptr)
    {
        if (!getSpellInfo()->isPassive() && !m_triggeredSpell)
        {
            // You can't cast other spells if you have the player flag preventing cast
            if (p_caster->hasPlayerFlags(PLAYER_FLAG_PREVENT_SPELL_CAST))
                return SPELL_FAILED_SPELL_IN_PROGRESS;

            // Check for cooldown
            if (!p_caster->Cooldown_CanCast(m_spellInfo))
            {
                if (m_triggeredByAura)
                    return SPELL_FAILED_DONT_REPORT;
                else
                    return SPELL_FAILED_NOT_READY;
            }
        }

        if (getSpellInfo()->getAttributesExG() & ATTRIBUTESEXG_IS_CHEAT_SPELL && !p_caster->GetSession()->HasGMPermissions())
        {
            *parameter1 = SPELL_EXTRA_ERROR_GM_ONLY;
            return SPELL_FAILED_CUSTOM_ERROR;
        }

        // Battleground checks
        if (p_caster->m_bg != nullptr)
        {
            // Arena checks
            if (isArena(p_caster->m_bg->GetType()))
            {
                // Spells with longer than 10 minute cooldown cannot be casted in arena
                const auto spellCooldown = getSpellInfo()->getRecoveryTime() > getSpellInfo()->getCategoryRecoveryTime() ? getSpellInfo()->getRecoveryTime() : getSpellInfo()->getCategoryRecoveryTime();
                if (getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_NOT_IN_ARENAS || (spellCooldown > 10 * MINUTE * IN_MILLISECONDS && !(getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_NOT_IN_ARENAS)))
                    return SPELL_FAILED_NOT_IN_ARENA;
            }

            // If battleground has ended, don't allow spell casting
            if (!m_triggeredSpell && p_caster->m_bg->HasEnded())
                return SPELL_FAILED_DONT_REPORT;
        }
        else if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_BG_ONLY)
            return SPELL_FAILED_ONLY_BATTLEGROUNDS;

        // Movement check
        if (p_caster->m_isMoving)
        {
            // No need to check for other interrupt flags, client does that for us
            // Also don't cast first ranged autorepeat spell if we're moving but activate it
            // TODO: Missing cata checks, in cata you can cast some spells while moving
            if (getSpellInfo()->isRangedAutoRepeat() || getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
                return SPELL_FAILED_MOVING;
        }
    }

    if (u_caster != nullptr)
    {
        // Check if caster is alive
        if (!u_caster->isAlive() && !(getSpellInfo()->getAttributes() & ATTRIBUTES_DEAD_CASTABLE || (m_triggeredSpell && !m_triggeredByAura)))
            return SPELL_FAILED_CASTER_DEAD;

        // Check if spell requires caster to be in combat
        if (getSpellInfo()->getAttributes() & ATTRIBUTES_STOP_ATTACK && getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_UNAFFECTED_BY_SCHOOL_IMMUNITY && !u_caster->CombatStatus.IsInCombat())
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
                if (u_caster->m_auras[i]->GetSpellInfo()->isAffectingSpell(getSpellInfo()))
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
        if (getSpellInfo()->getCasterAuraState() > 0 && !u_caster->hasAuraState(AuraState(getSpellInfo()->getCasterAuraState()), getSpellInfo(), u_caster))
            return SPELL_FAILED_CASTER_AURASTATE;
        if (getSpellInfo()->getCasterAuraStateNot() > 0 && u_caster->hasAuraState(AuraState(getSpellInfo()->getCasterAuraStateNot()), getSpellInfo(), u_caster))
            return SPELL_FAILED_CASTER_AURASTATE;

        // Caster's aura spell requirements
        if (getSpellInfo()->getCasterAuraSpell() > 0 && !u_caster->HasAura(getSpellInfo()->getCasterAuraSpell()))
            return SPELL_FAILED_CASTER_AURASTATE;
        if (getSpellInfo()->getCasterAuraSpellNot() > 0)
        {
            // TODO: I leave this here for now (from my old work), but this really should be moved to wotlk spellscript -Appled
            // Paladin's Avenging Wrath / Forbearance thing
            if (getSpellInfo()->getCasterAuraSpellNot() == 61988)
            {
                if (u_caster->HasAura(61987))
                    return SPELL_FAILED_CASTER_AURASTATE;
            }
            else if (u_caster->HasAura(getSpellInfo()->getCasterAuraSpellNot()))
                return SPELL_FAILED_CASTER_AURASTATE;
        }

        if (!m_triggeredSpell)
        {
            // Out of combat spells should not be able to be casted in combat
            if (requireCombat && (getSpellInfo()->getAttributes() & ATTRIBUTES_REQ_OOC) && u_caster->CombatStatus.IsInCombat())
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
                    if (u_caster->m_auras[u]->GetSpellInfo()->isAffectingSpell(getSpellInfo()))
                    {
                        hasIgnoreShapeshiftAura = true;
                        break;
                    }
                }

                if (!hasIgnoreShapeshiftAura)
                {
                    SpellCastResult shapeError = getErrorAtShapeshiftedCast(getSpellInfo(), u_caster->getShapeShiftForm());
                    if (shapeError != SPELL_CANCAST_OK)
                        return shapeError;

                    // Stealth check
                    if (getSpellInfo()->getAttributes() & ATTRIBUTES_REQ_STEALTH && !u_caster->hasAuraWithAuraEffect(SPELL_AURA_MOD_STEALTH))
                        return SPELL_FAILED_ONLY_STEALTHED;
                }
            }
        }
    }

    // Indoor and outdoor specific spells
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        if (getSpellInfo()->getAttributes() & ATTRIBUTES_ONLY_OUTDOORS &&
            !MapManagement::AreaManagement::AreaStorage::IsOutdoor(m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
            return SPELL_FAILED_ONLY_OUTDOORS;

        if (getSpellInfo()->getAttributes() & ATTRIBUTES_ONLY_INDOORS &&
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
        if (!m_triggeredSpell && getSpellInfo()->getTargetAuraState() > 0 && !target->hasAuraState(AuraState(getSpellInfo()->getTargetAuraState()), getSpellInfo(), u_caster))
            return SPELL_FAILED_TARGET_AURASTATE;
        if (getSpellInfo()->getTargetAuraStateNot() > 0 && target->hasAuraState(AuraState(getSpellInfo()->getTargetAuraState()), getSpellInfo(), u_caster))
            return SPELL_FAILED_TARGET_AURASTATE;

        // Target's aura spell requirements
        if (getSpellInfo()->getTargetAuraSpell() > 0 && !target->HasAura(getSpellInfo()->getTargetAuraSpell()))
            return SPELL_FAILED_TARGET_AURASTATE;
        if (getSpellInfo()->getTargetAuraSpellNot() > 0)
        {
            // TODO: I leave this here for now (from my old work), but this really should be moved to wotlk spellscript -Appled
            // Paladin's Avenging Wrath / Forbearance thing
            if (getSpellInfo()->getTargetAuraSpellNot() == 61988)
            {
                if (target->HasAura(61987))
                    return SPELL_FAILED_TARGET_AURASTATE;
            }
            else if (target->HasAura(getSpellInfo()->getTargetAuraSpellNot()))
                return SPELL_FAILED_TARGET_AURASTATE;
        }

        if (target->isCorpse())
        {
            // Player can't cast spells on corpses with bones only left
            const auto targetCorpse = objmgr.GetCorpseByOwner(target->getGuidLow());
            if (targetCorpse == nullptr || !targetCorpse->IsInWorld() || targetCorpse->GetCorpseState() == CORPSE_STATE_BONES)
                return SPELL_FAILED_BAD_TARGETS;
        }

        if (getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_CANT_TARGET_SELF && m_caster == target)
            return SPELL_FAILED_BAD_TARGETS;

        // Check if spell requires target to be out of combat
        if (getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_REQ_OOC_TARGET && target->getcombatstatus()->IsInCombat())
            return SPELL_FAILED_TARGET_AFFECTING_COMBAT;

        if (!(getSpellInfo()->getAttributesExF() & ATTRIBUTESEXF_CAN_TARGET_INVISIBLE) && (u_caster != nullptr && !u_caster->canSee(target)))
            return SPELL_FAILED_BAD_TARGETS;

        if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_TARGET_ONLY_GHOSTS)
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
        if (getSpellInfo()->hasEffect(SPELL_EFFECT_RESURRECT) || getSpellInfo()->hasEffect(SPELL_EFFECT_RESURRECT_FLAT) || getSpellInfo()->hasEffect(SPELL_EFFECT_SELF_RESURRECT))
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
                if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_CANT_TARGET_TAGGED && target->IsTagged())
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
                if (getSpellInfo()->getMechanicsType() == MECHANIC_DISARMED)
                {
                    if (target->getUnitFlags() & UNIT_FLAG_DISARMED)
                        return SPELL_FAILED_TARGET_NO_WEAPONS;

                    if (target->isPlayer())
                    {
                        // Check if target has no weapon or if target is already disarmed
                        const auto mainHandWeapon = dynamic_cast<Player*>(target)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
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
                if (getSpellInfo()->getFacingCasterFlags() == SPELL_INFRONT_STATUS_REQUIRE_INFRONT || getSpellInfo()->getDmgClass() == SPELL_DMG_TYPE_RANGED)
                {
                    if (!u_caster->isInFront(target))
                        return SPELL_FAILED_UNIT_NOT_INFRONT;
                }

                // Target must be behind caster
                if (getSpellInfo()->getFacingCasterFlags() == SPELL_INFRONT_STATUS_REQUIRE_INBACK)
                {
                    if (u_caster->isInFront(target))
                        return SPELL_FAILED_UNIT_NOT_BEHIND;
                }

                // Caster must be behind the target
                if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_REQ_BEHIND_TARGET && getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_REQ_FACING_TARGET && target->isInFront(u_caster))
                {
                    // Throw spell has these attributes in 3.3.5a, ignore
                    if (getSpellInfo()->getId() != SPELL_RANGED_THROW
#if VERSION_STRING >= TBC
                        // Druid - Pounce, "Patch 2.0.3 - Pounce no longer requires the druid to be behind the target."
                        && !(getSpellInfo()->getSpellFamilyName() == 7 && getSpellInfo()->getSpellFamilyFlags(0) == 0x20000)
#endif
#if VERSION_STRING >= WotLK
                        // Rogue - Mutilate, "Patch 3.0.2 - Mutilate no longer requires you be behind the target."
                        && !(getSpellInfo()->getSpellFamilyName() == 8 && getSpellInfo()->getSpellFamilyFlags(1) == 0x200000)
#endif
                        )
                        return SPELL_FAILED_NOT_BEHIND;
                }

                // Caster must be in front of target
                // in 3.3.5a only rogue's and npcs' Gouge spell
                if ((getSpellInfo()->getAttributes() == (ATTRIBUTES_ABILITY | ATTRIBUTES_NOT_SHAPESHIFT | ATTRIBUTES_UNK20 | ATTRIBUTES_STOP_ATTACK)
                    || getSpellInfo()->getAttributesEx() & ATTRIBUTESEX_REQ_FACING_TARGET) && !target->isInFront(u_caster))
                    return SPELL_FAILED_NOT_INFRONT;
            }

            // Check if spell can be casted on dead target
            if (!((getSpellInfo()->getTargets() & (TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 | TARGET_FLAG_UNIT_CORPSE)) ||
                getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_CAN_BE_CASTED_ON_DEAD_TARGET) && !target->isAlive())
                return SPELL_FAILED_TARGETS_DEAD;

            if (target->hasAuraWithAuraEffect(SPELL_AURA_SPIRIT_OF_REDEMPTION))
                return SPELL_FAILED_BAD_TARGETS;

            // Line of Sight check
            if (worldConfig.terrainCollision.isCollisionEnabled)
            {
                if (m_caster->IsInWorld() && !(getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_IGNORE_LINE_OF_SIGHT) && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SKIP_LINE_OF_SIGHT_CHECK) &&
                    (m_caster->GetMapId() != target->GetMapId() || !m_caster->GetMapMgr()->isInLineOfSight(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ())))
                    return SPELL_FAILED_LINE_OF_SIGHT;
            }

            if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_TARGET_ONLY_PLAYERS && !target->isPlayer())
                return SPELL_FAILED_TARGET_NOT_PLAYER;
        }
    }

    // Check if spell effect requires pet target
    if (p_caster != nullptr)
    {
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (getSpellInfo()->getEffectImplicitTargetA(i) == EFF_TARGET_PET)
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
        if (m_caster->IsInWorld() && !(getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_IGNORE_LINE_OF_SIGHT) && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SKIP_LINE_OF_SIGHT_CHECK) &&
            !m_caster->GetMapMgr()->isInLineOfSight(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_targets.destination().x, m_targets.destination().y, m_targets.destination().z))
            return SPELL_FAILED_LINE_OF_SIGHT;
    }

    if (p_caster != nullptr)
    {
        // Check if spell requires certain area
        if (getSpellInfo()->getRequiresAreaId() > 0)
        {
            auto areaEntry = p_caster->GetArea();
            if (areaEntry == nullptr)
                areaEntry = sAreaStore.LookupEntry(p_caster->GetZoneId());
            if (areaEntry == nullptr)
                return SPELL_FAILED_INCORRECT_AREA;

#if VERSION_STRING == TBC
            if (getSpellInfo()->getRequiresAreaId() != areaEntry->id && getSpellInfo()->getRequiresAreaId() != areaEntry->zone)
            {
                *parameter1 = getSpellInfo()->getRequiresAreaId();
                return SPELL_FAILED_REQUIRES_AREA;
            }
#elif VERSION_STRING >= WotLK
            auto found = false;
            auto areaGroup = sAreaGroupStore.LookupEntry(getSpellInfo()->getRequiresAreaId());
            while (areaGroup != nullptr)
            {
                for (auto i = 0; i < 6; ++i)
                {
                    if (areaGroup->AreaId[i] == areaEntry->id || (areaEntry->zone != 0 && areaGroup->AreaId[i] == areaEntry->zone))
                    {
                        found = true;
                        *parameter1 = 0;
                        break;
                    }
                    else if (areaGroup->AreaId[i] != 0)
                        *parameter1 = areaGroup->AreaId[i];
                }

                if (found || areaGroup->next_group == 0)
                    break;

                areaGroup = sAreaGroupStore.LookupEntry(areaGroup->next_group);
            }

            if (!found)
                return SPELL_FAILED_REQUIRES_AREA;
#endif
        }

        // Flying mount check
        if (getSpellInfo()->getAttributesExD() & ATTRIBUTESEXD_ONLY_IN_OUTLANDS)
        {
            if (!p_caster->canUseFlyingMountHere())
            {
                if (p_caster->GetMapId() != 571 || !(getSpellInfo()->getAttributesExG() & ATTRIBUTESEXG_IGNORE_COLD_WEATHER_FLYING))
                    return SPELL_FAILED_NOT_HERE;
            }
        }

        // Check if spell can be casted while mounted or on a taxi
        if ((p_caster->hasUnitFlags(UNIT_FLAG_MOUNT) || p_caster->hasUnitFlags(UNIT_FLAG_MOUNTED_TAXI)) && !m_triggeredSpell && !getSpellInfo()->isPassive())
        {
            if (p_caster->isOnTaxi())
                return SPELL_FAILED_NOT_ON_TAXI;
            else
            {
                if (!(getSpellInfo()->getAttributes() & ATTRIBUTES_MOUNT_CASTABLE))
                    return SPELL_FAILED_NOT_MOUNTED;
            }
        }

        // Check if spell can be casted in heroic dungeons or in raids
        if (getSpellInfo()->getAttributesExF() & ATTRIBUTESEXF_NOT_IN_RAIDS_OR_HEROIC_DUNGEONS)
        {
            if (p_caster->IsInWorld() && (p_caster->GetMapMgr()->GetMapInfo()->type == INSTANCE_RAID || p_caster->GetMapMgr()->iInstanceMode == MODE_HEROIC))
                return SPELL_FAILED_NOT_IN_RAID_INSTANCE;
        }
    }

    ////////////////////////////////////////////////////////
    // Item checks

    const SpellCastResult itemCastResult = checkItems(parameter1, parameter2);
    if (itemCastResult != SPELL_CANCAST_OK)
        return itemCastResult;

    // Call legacy CanCast for yet unhandled cases
    return m_triggeredSpell || ProcedOnSpell != nullptr ? SPELL_CANCAST_OK : SpellCastResult(CanCast(tolerate));
}

SpellCastResult Spell::checkItems(uint32_t* parameter1, uint32_t* parameter2) const
{
    if (p_caster == nullptr)
        return SPELL_CANCAST_OK;

    // If spell is casted from an enchant scroll
    auto scrollItem = false;
    // If spell is casted on an armor vellum or on a weapon vellum
    auto vellumTarget = false;

    // Casted by an item
    if (i_caster != nullptr)
    {
        if (!p_caster->hasItem(i_caster->getEntry()))
            return SPELL_FAILED_ITEM_GONE;

        // Check if the item is in trade window
        // todo: fix me!
        /*if (p_caster->GetTradeTarget() != nullptr)
        {
            for (auto i = 0; i < 7; ++i)
            {
                if (p_caster->getTradeItem(i) == nullptr)
                    continue;
                if (p_caster->getTradeItem(i) == i_caster)
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
            }
        }*/

        const auto itemProperties = i_caster->getItemProperties();
        if (itemProperties == nullptr)
            return SPELL_FAILED_ITEM_GONE;

        // Check if the item is an enchant scroll
        if (itemProperties->Flags & ITEM_FLAG_ENCHANT_SCROLL)
            scrollItem = true;

        // Check if the item has any charges left
        for (uint8_t i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (itemProperties->Spells[i].Charges > 0 && i_caster->getSpellCharges(i) == 0)
                return SPELL_FAILED_NO_CHARGES_REMAIN;
        }

        // Check zone
        if (itemProperties->ZoneNameID > 0 && itemProperties->ZoneNameID != p_caster->GetZoneId())
            return SPELL_FAILED_INCORRECT_AREA;
        // Check map
        if (itemProperties->MapID > 0 && itemProperties->MapID != p_caster->GetMapId())
            return SPELL_FAILED_INCORRECT_AREA;

        // Check health and power for consumables (potions, healthstones, mana items etc)
        if (itemProperties->Class == ITEM_CLASS_CONSUMABLE)
        {
            const auto targetUnit = p_caster->GetMapMgrUnit(m_targets.m_unitTarget);
            if (targetUnit != nullptr)
            {
                SpellCastResult errorMessage = SPELL_CANCAST_OK;
                for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    // Pet related effects are handled later
                    if (getSpellInfo()->getEffectImplicitTargetA(i) == EFF_TARGET_PET)
                        continue;

                    // +HP items
                    if (getSpellInfo()->getEffect(i) == SPELL_EFFECT_HEAL)
                    {
                        // Check if target has full health
                        if (targetUnit->getHealthPct() == 100)
                        {
                            errorMessage = SPELL_FAILED_ALREADY_AT_FULL_HEALTH;
                            continue;
                        }
                        else
                        {
                            errorMessage = SPELL_CANCAST_OK;
                            break;
                        }
                    }

                    // +Mana/Power items
                    if (getSpellInfo()->getEffect(i) == SPELL_EFFECT_ENERGIZE)
                    {
                        // Check if the spell has valid power type
                        if (getSpellInfo()->getEffectMiscValue(i) < 0
#if VERSION_STRING <= TBC
                            || getSpellInfo()->getEffectMiscValue(i) > POWER_TYPE_HAPPINESS)
#else
                            || getSpellInfo()->getEffectMiscValue(i) > POWER_TYPE_RUNIC_POWER)
#endif
                        {
                            errorMessage = SPELL_FAILED_ALREADY_AT_FULL_POWER;
                            continue;
                        }

                        // Check if target has full powers
                        const auto powerType = PowerType(getSpellInfo()->getEffectMiscValue(i));
                        if (targetUnit->getPowerPct(powerType) == 100)
                        {
                            errorMessage = powerType == POWER_TYPE_MANA ? SPELL_FAILED_ALREADY_AT_FULL_MANA : SPELL_FAILED_ALREADY_AT_FULL_POWER;
                            continue;
                        }
                        else
                        {
                            errorMessage = SPELL_CANCAST_OK;
                            break;
                        }
                    }
                }

                if (errorMessage != SPELL_CANCAST_OK)
                    return errorMessage;
            }
        }
    }

    // Casted on an item
    if (m_targets.m_itemTarget > 0)
    {
        Item* targetItem = nullptr;
        // Check if the targeted item is in the trade window
        if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
        {
            // Only enchanting and lockpicking effects can be used in trade window
            if (getSpellInfo()->getEffect(0) == SPELL_EFFECT_OPEN_LOCK ||
                getSpellInfo()->getEffect(0) == SPELL_EFFECT_ENCHANT_ITEM ||
                getSpellInfo()->getEffect(0) == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY)
            {
                if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_ENCHANT_OWN_ONLY)
                    return SPELL_FAILED_NOT_TRADEABLE;

                // todo: implement trade checks here when trading is fixed -Appled
            }
            else
                return SPELL_FAILED_NOT_TRADEABLE;
        }
        else
            targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.m_itemTarget);

        if (targetItem == nullptr)
            return SPELL_FAILED_ITEM_GONE;

        if (!targetItem->fitsToSpellRequirements(getSpellInfo()))
            return SPELL_FAILED_BAD_TARGETS;

        // Prevent exploiting (enchanting broken items and stacking them)
        if (targetItem->getDurability() == 0 && targetItem->getMaxDurability() != 0)
            return SPELL_FAILED_BAD_TARGETS;

        if ((getSpellInfo()->getEquippedItemClass() == ITEM_CLASS_ARMOR && targetItem->getItemProperties()->Class == ITEM_CLASS_TRADEGOODS && targetItem->getItemProperties()->SubClass == ITEM_SUBCLASS_ARMOR_ENCHANTMENT) ||
            (getSpellInfo()->getEquippedItemClass() == ITEM_CLASS_WEAPON && targetItem->getItemProperties()->Class == ITEM_CLASS_TRADEGOODS && targetItem->getItemProperties()->SubClass == ITEM_SUBCLASS_WEAPON_ENCHANTMENT))
            vellumTarget = true;
    }
    // Spell requires an item to be equipped
    else if (m_targets.m_itemTarget == 0 && getSpellInfo()->getEquippedItemClass() >= 0)
    {
        auto hasItemWithProperType = false;
        switch (getSpellInfo()->getEquippedItemClass())
        {
            // Spell requires a melee weapon or a ranged weapon
            case ITEM_CLASS_WEAPON:
            {
                for (int16_t i = EQUIPMENT_SLOT_MAINHAND; i <= EQUIPMENT_SLOT_RANGED; ++i)
                {
                    const auto inventoryItem = p_caster->getItemInterface()->GetInventoryItem(i);
                    if (inventoryItem != nullptr)
                    {
                        // Check if the weapon slot is disarmed
                        if ((i == EQUIPMENT_SLOT_MAINHAND && p_caster->hasUnitFlags(UNIT_FLAG_DISARMED))
#if VERSION_STRING >= TBC
                            || (i == EQUIPMENT_SLOT_OFFHAND && p_caster->hasUnitFlags2(UNIT_FLAG2_DISARM_OFFHAND))
                            || (i == EQUIPMENT_SLOT_RANGED && p_caster->hasUnitFlags2(UNIT_FLAG2_DISARM_RANGED))
#endif
                            )
                            continue;

                        // Check for proper item class and subclass
                        if (inventoryItem->fitsToSpellRequirements(getSpellInfo()))
                        {
                            hasItemWithProperType = true;
                            break;
                        }
                    }
                }
            } break;
            // Spell requires an armor piece (like shield)
            case ITEM_CLASS_ARMOR:
            {
                // Check first if spell requires a shield equipped
                Item* inventoryItem = nullptr;
                if (getSpellInfo()->getEquippedItemSubClass() & (1 << ITEM_SUBCLASS_ARMOR_SHIELD))
                {
                    inventoryItem = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    if (inventoryItem != nullptr)
                    {
#if VERSION_STRING >= TBC
                        // Check for offhand disarm
                        if (!p_caster->hasUnitFlags2(UNIT_FLAG2_DISARM_OFFHAND))
#endif
                        {
                            // Check for proper item class and subclass
                            if (inventoryItem->fitsToSpellRequirements(getSpellInfo()))
                            {
                                hasItemWithProperType = true;
                                break;
                            }
                        }
                    }
                }

                // Check for other armor pieces
                for (int16_t i = EQUIPMENT_SLOT_HEAD; i < EQUIPMENT_SLOT_MAINHAND; ++i)
                {
                    inventoryItem = p_caster->getItemInterface()->GetInventoryItem(i);
                    if (inventoryItem != nullptr)
                    {
                        // Check for proper item class and subclass
                        if (inventoryItem->fitsToSpellRequirements(getSpellInfo()))
                        {
                            hasItemWithProperType = true;
                            break;
                        }
                    }
                }

                // No need to check further if found already
                if (hasItemWithProperType)
                    break;

                // Ranged slot can have an item classified as armor (no need to check for disarm in these cases)
                inventoryItem = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                if (inventoryItem != nullptr)
                {
                    // Check for proper item class and subclass
                    if (inventoryItem->fitsToSpellRequirements(getSpellInfo()))
                    {
                        hasItemWithProperType = true;
                        break;
                    }
                }
            } break;
            default:
                break;
        }

        if (!hasItemWithProperType)
        {
            *parameter1 = getSpellInfo()->getEquippedItemClass();
            *parameter2 = getSpellInfo()->getEquippedItemSubClass();
            return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
        }

        // Temporary helper lambda
        auto hasEquippableWeapon = [&](Item const* weapon) -> bool
        {
            if (weapon == nullptr)
                return false;
            if (weapon->getItemProperties()->MaxDurability > 0 && weapon->getDurability() == 0)
                return false;
            return weapon->fitsToSpellRequirements(getSpellInfo());
        };

        // Check if spell explicitly requires a main hand weapon
        if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_REQUIRES_MAIN_HAND_WEAPON)
        {
            if (!hasEquippableWeapon(p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)))
                return SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND;
        }

        // Check if spell explicitly requires an offhand weapon
        if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_REQUIRES_OFFHAND_WEAPON)
        {
            if (!hasEquippableWeapon(p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND)))
                return SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND;
        }
    }

    // Check if the spell requires any reagents or tools (skip enchant scrolls)
    if (i_caster == nullptr || !(i_caster->getItemProperties()->Flags & ITEM_FLAG_ENCHANT_SCROLL))
    {
        // Spells with ATTRIBUTESEXE_REAGENT_REMOVAL attribute won't take reagents if player has UNIT_FLAG_NO_REAGANT_COST flag
        auto checkForReagents = !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_REAGENT_REMOVAL && p_caster->hasUnitFlags(UNIT_FLAG_NO_REAGANT_COST));
        if (checkForReagents)
        {
#if VERSION_STRING >= WotLK
            // Check for spells which remove the reagent cost for a spell
            // e.g. Glyph of Slow Fall or Glyph of Levitate
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (getSpellInfo()->getSpellFamilyFlags(i) == 0)
                    continue;
                if (getSpellInfo()->getSpellFamilyFlags(i) & p_caster->getNoReagentCost(i))
                {
                    checkForReagents = false;
                    break;
                }
            }
#endif
        }
        // Reagents will always be checked for items in trade window
        else if (m_targets.m_itemTarget != 0 && m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
            checkForReagents = true;

        if (checkForReagents)
        {
            for (uint8_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
            {
                if (getSpellInfo()->getReagent(i) == 0)
                    continue;

                const auto itemId = static_cast<uint32_t>(getSpellInfo()->getReagent(i));
                auto itemCount = getSpellInfo()->getReagentCount(i);

                // Some spells include the used item as one of the reagents
                // So in these cases itemCount must be incremented by one
                // e.g. item id 24502 requires 7 items but DBC data requires only 6, because the one missing item is the caster
                if (i_caster != nullptr && i_caster->getEntry() == itemId)
                {
                    const auto itemProperties = i_caster->getItemProperties();
                    for (uint8_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
                    {
                        if (itemProperties->Spells[x].Id == 0)
                            continue;
                        if (itemProperties->Spells[x].Charges == -1 && i_caster->getSpellCharges(x) <= 1)
                        {
                            ++itemCount;
                            break;
                        }
                    }
                }

                if (!p_caster->hasItem(itemId, itemCount))
                {
                    *parameter1 = itemId;
                    return SPELL_FAILED_REAGENTS;
                }
            }
        }

        // Check for totem items
        for (uint8_t i = 0; i < MAX_SPELL_TOTEMS; ++i)
        {
            if (getSpellInfo()->getTotem(i) != 0)
            {
                if (!p_caster->hasItem(getSpellInfo()->getTotem(i)))
                {
                    *parameter1 = getSpellInfo()->getTotem(i);
                    return SPELL_FAILED_TOTEMS;
                }
            }
        }

        // Check for totem category items
        for (uint8_t i = 0; i < MAX_SPELL_TOTEM_CATEGORIES; ++i)
        {
            if (getSpellInfo()->getTotemCategory(i) != 0 && !p_caster->getItemInterface()->hasItemForTotemCategory(getSpellInfo()->getTotemCategory(i)))
            {
                *parameter1 = getSpellInfo()->getTotemCategory(i);
                return SPELL_FAILED_TOTEM_CATEGORY;
            }
        }
    }

    // Special checks for different spell effects
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getSpellInfo()->getEffect(i) == 0)
            continue;

        switch (getSpellInfo()->getEffect(i))
        {
            case SPELL_EFFECT_CREATE_ITEM:
            case SPELL_EFFECT_CREATE_ITEM2:
                if (getSpellInfo()->getEffectItemType(i) != 0)
                {
                    const auto itemProperties = sMySQLStore.getItemProperties(getSpellInfo()->getEffectItemType(i));
                    if (itemProperties == nullptr)
                    {
                        LOG_ERROR("Spell::checkItems: Spell entry %u has unknown item id (%u) in SPELL_EFFECT_CREATE_ITEM effect", getSpellInfo()->getId(), getSpellInfo()->getEffectItemType(i));
                        return SPELL_FAILED_ERROR;
                    }

                    // Check if player has any free slots in the inventory
                    if (p_caster->getItemInterface()->CalculateFreeSlots(itemProperties) == 0)
                    {
                        p_caster->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
                        return SPELL_FAILED_DONT_REPORT;
                    }
                    
                    // Check other limitations
                    const auto itemErrorMessage = p_caster->getItemInterface()->CanReceiveItem(itemProperties, 1);
                    if (itemErrorMessage != INV_ERR_OK)
                    {
                        p_caster->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, itemErrorMessage);
                        return SPELL_FAILED_DONT_REPORT;
                    }
                } break;
            case SPELL_EFFECT_ENCHANT_ITEM:
                // Check only for vellums here, normal checks are done in the next case
                if (getSpellInfo()->getEffectItemType(i) != 0 && m_targets.m_itemTarget != 0 && vellumTarget)
                {
                    // Player can only enchant their own vellums
                    if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
                        return SPELL_FAILED_NOT_TRADEABLE;
                    // Scrolls (enchanted vellums) cannot be enchanted into another vellum (duping)
                    if (scrollItem)
                        return SPELL_FAILED_BAD_TARGETS;

                    const auto vellumItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.m_itemTarget);
                    if (vellumItem == nullptr)
                        return SPELL_FAILED_ITEM_NOT_FOUND;
                    // Check if vellum is appropriate target for the enchant
                    if (getSpellInfo()->getBaseLevel() > vellumItem->getItemProperties()->ItemLevel)
                        return SPELL_FAILED_LOWLEVEL;

                    const auto itemProperties = sMySQLStore.getItemProperties(getSpellInfo()->getEffectItemType(i));
                    if (itemProperties == nullptr)
                    {
                        LOG_ERROR("Spell::checkItems: Spell entry %u has unknown item id (%u) in SPELL_EFFECT_ENCHANT_ITEM effect", getSpellInfo()->getId(), getSpellInfo()->getEffectItemType(i));
                        return SPELL_FAILED_ERROR;
                    }

                    // Check if player has any free slots in the inventory
                    if (p_caster->getItemInterface()->CalculateFreeSlots(itemProperties) == 0)
                    {
                        p_caster->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    // Check other limitations
                    const auto itemErrorMessage = p_caster->getItemInterface()->CanReceiveItem(itemProperties, 1);
                    if (itemErrorMessage != INV_ERR_OK)
                    {
                        p_caster->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, itemErrorMessage);
                        return SPELL_FAILED_DONT_REPORT;
                    }
                }
            // no break here
            case SPELL_EFFECT_ADD_SOCKET:
            {
                if (m_targets.m_itemTarget == 0)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                Item* targetItem = nullptr;
                if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
                {
                    // todo: implement this when trading is fixed
                }
                else
                    targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.m_itemTarget);

                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                // Check if the item's level is high enough for the enchantment
                if (targetItem->getItemProperties()->ItemLevel < getSpellInfo()->getBaseLevel())
                    return SPELL_FAILED_LOWLEVEL;

                auto hasOnUseEffect = false;
                const auto itemProperties = targetItem->getItemProperties();
                for (uint8_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
                {
                    if (itemProperties->Spells[x].Id == 0)
                        continue;
                    if (itemProperties->Spells[x].Trigger == USE || itemProperties->Spells[x].Trigger == APPLY_AURA_ON_PICKUP)
                    {
                        hasOnUseEffect = true;
                        break;
                    }
                }

                const auto enchantEntry = sSpellItemEnchantmentStore.LookupEntry(getSpellInfo()->getEffectMiscValue(i));
                if (enchantEntry == nullptr)
                {
                    LOG_ERROR("Spell::checkItems: Spell entry %u has no valid enchantment (%u)", getSpellInfo()->getId(), getSpellInfo()->getEffectMiscValue(i));
                    return SPELL_FAILED_ERROR;
                }

                // Loop through enchantment's types
                for (uint8_t x = 0; x < 3; ++x)
                {
                    switch (enchantEntry->type[x])
                    {
                        // todo: declare these in a header file and figure out other values
                        case 7: // Enchants 'on use' enchantment to item
                            // Check if the item already has a 'on use' enchantment
                            if (hasOnUseEffect)
                                return SPELL_FAILED_ON_USE_ENCHANT;
                            break;
                        case 8: // Enchants a new prismatic socket slot to item
                            // Check if the item already has a prismatic gem slot enchanted
                            if (targetItem->getEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT) != 0)
                                return SPELL_FAILED_ITEM_ALREADY_ENCHANTED;
                            // or if the item already has the maximum amount of socket slots
                            else if (targetItem->GetSocketsCount() >= MAX_ITEM_PROTO_SOCKETS)
                                return SPELL_FAILED_MAX_SOCKETS;
                            break;
                        default:
                            break;
                    }
                }

                // Check item owner in cases where enchantment makes item soulbound
                if (targetItem->getOwner() != p_caster)
                {
                    if (enchantEntry->EnchantGroups & 0x01) // Makes item soulbound
                        return SPELL_FAILED_NOT_TRADEABLE;
                }
                break;
            }
            case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
            {
                if (m_targets.m_itemTarget == 0)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                Item* targetItem = nullptr;
                if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
                {
                    // todo: implement this when trading is fixed
                }
                else
                    targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.m_itemTarget);

                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                const auto enchantmentEntry = sSpellItemEnchantmentStore.LookupEntry(getSpellInfo()->getEffectMiscValue(i));
                if (enchantmentEntry == nullptr)
                {
                    LOG_ERROR("Spell::checkItems: Spell entry %u has no valid enchantment (%u)", getSpellInfo()->getId(), getSpellInfo()->getEffectMiscValue(i));
                    return SPELL_FAILED_ERROR;
                }

                // Check item owner in cases where enchantment makes item soulbound
                if (targetItem->getOwner() != p_caster)
                {
                    if (enchantmentEntry->EnchantGroups & 0x01) // Makes item soulbound
                        return SPELL_FAILED_NOT_TRADEABLE;
                }
                break;
            }
            case SPELL_EFFECT_DISENCHANT:
            {
                if (m_targets.m_itemTarget == 0)
                    return SPELL_FAILED_ITEM_GONE;
                // Check if the item target is in a trade window
                if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
                    return SPELL_FAILED_NOT_TRADEABLE;

                const auto targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.m_itemTarget);
                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_GONE;

                const auto itemProperties = targetItem->getItemProperties();
                // Only armor and weapon items can be disenchanted
                if (itemProperties->Class != ITEM_CLASS_ARMOR && itemProperties->Class != ITEM_CLASS_WEAPON)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                // Only items with uncommon, rare and epic quality can be disenchanted
                if (itemProperties->Quality > ITEM_QUALITY_EPIC_PURPLE || itemProperties->Quality < ITEM_QUALITY_UNCOMMON_GREEN)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                // Some items are not disenchantable
                if (itemProperties->DisenchantReqSkill <= 0)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
#if VERSION_STRING >= TBC
                // As of patch 2.0.1 disenchanting an item requires minimum skill level
                if (static_cast<uint32_t>(itemProperties->DisenchantReqSkill) > p_caster->_GetSkillLineCurrent(SKILL_ENCHANTING))
                    return SPELL_FAILED_CANT_BE_DISENCHANTED_SKILL;
#endif
                // TODO: check does the item even have disenchant loot
                break;
            }
            case SPELL_EFFECT_PROSPECTING:
            {
                if (m_targets.m_itemTarget == 0)
                    return SPELL_FAILED_ITEM_GONE;
                // Check if the item target is in a trade window
                if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
                    return SPELL_FAILED_NOT_TRADEABLE;

                const auto targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.m_itemTarget);
                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_GONE;

                const auto itemProperties = targetItem->getItemProperties();
                // Check if the item is prospectable
                if (!(itemProperties->Flags & ITEM_FLAG_PROSPECTABLE))
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                // Check if player has enough skill in Jewelcrafting
                if (itemProperties->RequiredSkillRank > p_caster->_GetSkillLineCurrent(SKILL_JEWELCRAFTING))
                {
                    *parameter1 = itemProperties->RequiredSkill;
                    *parameter2 = itemProperties->RequiredSkillRank;
                    return SPELL_FAILED_MIN_SKILL;
                }
                // Check if player has enough ores for prospecting
                if (!p_caster->hasItem(targetItem->getEntry(), 5))
                {
                    *parameter1 = targetItem->getEntry();
                    *parameter2 = 5;
                    return SPELL_FAILED_NEED_MORE_ITEMS;
                }

                // TODO: check does the item even have prospecting loot
                break;
            }
            case SPELL_EFFECT_MILLING:
            {
                if (m_targets.m_itemTarget == 0)
                    return SPELL_FAILED_ITEM_GONE;
                // Check if the item target is in a trade window
                if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
                    return SPELL_FAILED_NOT_TRADEABLE;

                const auto targetItem = p_caster->getItemInterface()->GetItemByGUID(m_targets.m_itemTarget);
                if (targetItem == nullptr)
                    return SPELL_FAILED_ITEM_GONE;

                const auto itemProperties = targetItem->getItemProperties();
                // Check if the item is millable
                if (!(itemProperties->Flags & ITEM_FLAG_MILLABLE))
                    return SPELL_FAILED_CANT_BE_MILLED;
                // Check if player has enough skill in Inscription
                if (itemProperties->RequiredSkillRank > p_caster->_GetSkillLineCurrent(SKILL_INSCRIPTION))
                {
                    *parameter1 = itemProperties->RequiredSkill;
                    *parameter2 = itemProperties->RequiredSkillRank;
                    return SPELL_FAILED_MIN_SKILL;
                }
                if (!p_caster->hasItem(targetItem->getEntry(), 5))
                {
                    *parameter1 = targetItem->getEntry();
                    *parameter2 = 5;
                    return SPELL_FAILED_NEED_MORE_ITEMS;
                }

                // TODO: check does the item even have milling loot
                break;
            }
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            {
                // Check if spell is not ranged type
                if (getSpellInfo()->getDmgClass() != SPELL_DMG_TYPE_RANGED)
                    break;

                const auto rangedWeapon = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                if (rangedWeapon == nullptr || rangedWeapon->getItemProperties()->Class != ITEM_CLASS_WEAPON)
                    return SPELL_FAILED_EQUIPPED_ITEM;
                // Check if the item has any durability left
                if (rangedWeapon->getMaxDurability() > 0 && rangedWeapon->getDurability() == 0)
                    return SPELL_FAILED_EQUIPPED_ITEM;

#if VERSION_STRING <= WotLK
                // Check for ammunitation
                switch (rangedWeapon->getItemProperties()->SubClass)
                {
                    case ITEM_SUBCLASS_WEAPON_THROWN:
                        // todo: at some point throwing weapons used durability but at some point they used stack count,
                        // and at some point they had neither of those. Figure out which expansion/patch had which
                        if (p_caster->getItemInterface()->GetItemCount(rangedWeapon->getEntry()) == 0)
                            return SPELL_FAILED_NO_AMMO;
                        break;
                    // Check ammo for ranged weapons
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_GUN:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    {
                        // Thori'dal, the Stars' Fury has a dummy aura which makes it generate magical arrows
                        // iirc the only item with this kind of effect?
                        if (p_caster->m_requiresNoAmmo)
                            break;
                        const auto ammoId = p_caster->getAmmoId();
                        if (ammoId == 0)
                            return SPELL_FAILED_NEED_AMMO;

                        const auto ammoProperties = sMySQLStore.getItemProperties(ammoId);
                        if (ammoProperties == nullptr)
                            return SPELL_FAILED_NEED_AMMO;
                        if (ammoProperties->Class != ITEM_CLASS_PROJECTILE)
                            return SPELL_FAILED_NEED_AMMO;
                        if (ammoProperties->RequiredLevel > p_caster->getLevel())
                            return SPELL_FAILED_NEED_AMMO;

                        // Check for correct projectile type
                        if (rangedWeapon->getItemProperties()->SubClass == ITEM_SUBCLASS_WEAPON_GUN)
                        {
                            if (ammoProperties->SubClass != ITEM_SUBCLASS_PROJECTILE_BULLET)
                                return SPELL_FAILED_NEED_AMMO;
                        }
                        else
                        {
                            if (ammoProperties->SubClass != ITEM_SUBCLASS_PROJECTILE_ARROW)
                                return SPELL_FAILED_NEED_AMMO;
                        }

                        // Check if player is out of ammos
                        if (!p_caster->hasItem(ammoId))
                        {
                            p_caster->setAmmoId(0);
                            return SPELL_FAILED_NO_AMMO;
                        }
                    } break;
                    default:
                        break;
                }
#endif
                break;
            }
            default:
                break;
        }
    }

    return SPELL_CANCAST_OK;
}

SpellCastResult Spell::getErrorAtShapeshiftedCast(SpellInfo const* spellInfo, const uint32_t shapeshiftForm) const
{
    // No need to check requirements for talents that learn spells
    uint8_t talentRank = 0;
    const auto talentInfo = sTalentStore.LookupEntry(spellInfo->getId());
    if (talentInfo != nullptr)
    {
        for (uint8_t i = 0; i < 5; ++i)
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

void Spell::sendCastResult(SpellCastResult result, uint32_t parameter1 /*= 0*/, uint32_t parameter2 /*= 0*/)
{
    if (result == SPELL_CANCAST_OK)
        return;

    SetSpellFailed();

    if (!m_caster->IsInWorld())
        return;

    Player* plr = p_caster;
    if (plr == nullptr && u_caster != nullptr)
        plr = u_caster->m_redirectSpellPackets;
    if (plr == nullptr)
        return;

    sendCastResult(plr, 0, result, parameter1, parameter2);
}

void Spell::sendCastResult(Player* caster, uint8_t castCount, SpellCastResult result, uint32_t parameter1, uint32_t parameter2)
{
    if (caster == nullptr)
        return;

    // Include missing parameters to error messages
    switch (result)
    {
        case SPELL_FAILED_ONLY_SHAPESHIFT:
            if (parameter1 == 0)
                parameter1 = getSpellInfo()->getRequiredShapeShift();
            break;
        case SPELL_FAILED_REQUIRES_AREA:
            if (parameter1 == 0)
            {
#if VERSION_STRING == TBC
                parameter1 = getSpellInfo()->getRequiresAreaId();
#elif VERSION_STRING >= WotLK
                // Send the first area id from areagroup to player
                auto areaGroup = sAreaGroupStore.LookupEntry(getSpellInfo()->getRequiresAreaId());
                for (auto i = 0; i < 6; ++i)
                {
                    if (areaGroup->AreaId[i] != 0)
                    {
                        parameter1 = areaGroup->AreaId[i];
                        break;
                    }
                }
#endif
            } break;
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS:
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND:
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND:
            if (parameter1 == 0 && parameter2 == 0)
            {
                parameter1 = getSpellInfo()->getEquippedItemClass();
                parameter2 = getSpellInfo()->getEquippedItemSubClass();
            } break;
        case SPELL_FAILED_REAGENTS:
            if (parameter1 == 0)
            {
                for (uint8_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
                {
                    if (getSpellInfo()->getReagent(i) == 0)
                        continue;
                    if (!caster->hasItem(getSpellInfo()->getReagent(i), getSpellInfo()->getReagentCount(i)))
                    {
                        parameter1 = getSpellInfo()->getReagent(i);
                        break;
                    }
                }
            } break;
        case SPELL_FAILED_TOTEMS:
            if (parameter1 == 0)
            {
                for (uint8_t i = 0; i < MAX_SPELL_TOTEMS; ++i)
                {
                    if (getSpellInfo()->getTotem(i) == 0)
                        continue;
                    if (!caster->hasItem(getSpellInfo()->getTotem(i)))
                    {
                        parameter1 = getSpellInfo()->getTotem(i);
                        break;
                    }
                }
            } break;
        case SPELL_FAILED_TOTEM_CATEGORY:
            if (parameter1 == 0)
            {
                for (uint8_t i = 0; i < MAX_SPELL_TOTEM_CATEGORIES; ++i)
                {
                    if (getSpellInfo()->getTotemCategory(i) == 0)
                        continue;
                    if (!caster->getItemInterface()->hasItemForTotemCategory(getSpellInfo()->getTotemCategory(i)))
                    {
                        parameter1 = getSpellInfo()->getTotemCategory(i);
                        break;
                    }
                }
            } break;
        case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
            if (parameter1 == 0)
                parameter1 = getSpellInfo()->getRequiresSpellFocus();
            break;
        default:
            break;
    }

    caster->sendCastFailedPacket(getSpellInfo()->getId(), result, castCount, parameter1, parameter2);
}

bool Spell::canAttackCreatureType(Creature* target) const
{
    // Skip check for Grounding Totem
    if (target->getCreatedBySpellId() == 8177)
        return true;

    const auto typeMask = getSpellInfo()->getTargetCreatureType();
    const auto mask = 1 << (target->GetCreatureProperties()->Type - 1);
    return (target->GetCreatureProperties()->Type != 0 && typeMask != 0 && (typeMask & mask) == 0) ? false : true;
}

SpellInfo const* Spell::getSpellInfo() const
{
    return m_spellInfo_override != nullptr ? m_spellInfo_override : m_spellInfo;
}
