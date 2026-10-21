/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AuraEffectModifier.hpp"

#include "Logging/Logger.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Objects/Units/Creatures/Summons/SummonHandler.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "SpellAura.hpp"
#include "SpellScriptDefines.hpp"

extern pSpellAura SpellAuraHandler[TOTAL_SPELL_AURAS];

AuraEffectModifier::AuraEffectModifier(Aura& parent) : mAura(parent)
{ }

void AuraEffectModifier::setAuraEffectType(AuraEffect type) { mAuraEffect = type; }
AuraEffect AuraEffectModifier::getAuraEffectType() const { return mAuraEffect; }

void AuraEffectModifier::setEffectDamage(int32_t value)
{
    mDamage = value;
    mRealDamage = static_cast<float_t>(value);
}
void AuraEffectModifier::setEffectDamage(float_t value)
{
    mRealDamage = value;
    mDamage = static_cast<int32_t>(std::ceil(value));
}
int32_t AuraEffectModifier::getEffectDamage() const { return mDamage; }
float_t AuraEffectModifier::getEffectFloatDamage() const { return mRealDamage; }

void AuraEffectModifier::setEffectBaseDamage(int32_t baseValue) { mBaseDamage = baseValue; }
int32_t AuraEffectModifier::getEffectBaseDamage() const { return mBaseDamage; }

void AuraEffectModifier::setEffectExtraField(int32_t extraValue) { mExtraField = extraValue; }
int32_t AuraEffectModifier::getEffectExtraField() const { return mExtraField; }

void AuraEffectModifier::setEffectExtra2Field(int32_t extraValue) { mExtraField2 = extraValue; }
int32_t AuraEffectModifier::getEffectExtra2Field() const { return mExtraField2; }

void AuraEffectModifier::setEffectMiscValue(int32_t _miscValue) { miscValue = _miscValue; }
int32_t AuraEffectModifier::getEffectMiscValue() const { return miscValue; }

void AuraEffectModifier::setEffectAmplitude(int32_t amplitude) { mAmplitude = amplitude; }
int32_t AuraEffectModifier::getEffectAmplitude() const { return mAmplitude; }

void AuraEffectModifier::setEffectDamageFraction(float_t fraction) { mDamageFraction = fraction; }
float_t AuraEffectModifier::getEffectDamageFraction() const { return mDamageFraction; }

void AuraEffectModifier::setEffectPercentModifier(float_t pctMod) { mEffectPctModifier = pctMod; }
float_t AuraEffectModifier::getEffectPercentModifier() const { return mEffectPctModifier; }

void AuraEffectModifier::setEffectDamageStatic(bool _static) { mEffectDamageStatic = _static; }
bool AuraEffectModifier::isEffectDamageStatic() const { return mEffectDamageStatic; }

void AuraEffectModifier::setEffectIndex(uint8_t _effIndex) { effIndex = _effIndex; }
uint8_t AuraEffectModifier::getEffectIndex() const { return effIndex; }

void AuraEffectModifier::setEffectActive(bool set) { mActive = set; }
bool AuraEffectModifier::isActive() const { return mActive; }

bool AuraEffectModifier::isPeriodicEffect() const
{
    switch (mAuraEffect)
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_PERIODIC_HEAL_PCT:
        case SPELL_AURA_PERIODIC_POWER_PCT:
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        case SPELL_AURA_PERIODIC_ENERGIZE:
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        case SPELL_AURA_PERIODIC_MANA_FUNNEL:
        case SPELL_AURA_PERIODIC_MANA_LEECH:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        case SPELL_AURA_PERIODIC_POWER_BURN:
#if VERSION_STRING >= TBC
        case SPELL_AURA_PERIODIC_TRIGGER_DUMMY:
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
#endif
            return true;
        default:
            return false;
    }
}

void AuraEffectModifier::applyEffect(bool apply, bool skipScriptCheck/* = false*/)
{
    // Do not apply or remove effect multiple times
    if (mActive == apply)
        return;

    mActive = apply;

    if (skipScriptCheck)
    {
        (mAura.*SpellAuraHandler[getAuraEffectType()])(this, apply);
    }
    else
    {
        const auto scriptResult = sScriptMgr.callScriptedAuraBeforeAuraEffect(&mAura, this, apply);
        if (scriptResult != SpellScriptExecuteState::EXECUTE_PREVENT)
            (mAura.*SpellAuraHandler[getAuraEffectType()])(this, apply);
    }
}

void AuraEffectModifier::resetEffect()
{
    mAuraEffect = SPELL_AURA_NONE;
    mDamage = 0;
    mRealDamage = 0.0f;
    mBaseDamage = 0;
    mExtraField = 0;
    mExtraField2 = 0;
    miscValue = 0;
    mAmplitude = 0;
    mDamageFraction = 0.0f;
    mEffectPctModifier = 1.0f;
    mEffectDamageStatic = false;
    mActive = false;
    effIndex = 0;

    clearAreaAuraEffectTargets();
}

void AuraEffectModifier::updateAreaAuraEffectTargets()
{
    const auto* const aurSpellInfo = mAura.getSpellInfo();
    auto* const unitCaster = mAura.getUnitCaster();
    // Update only caster's area aura target map
    if (unitCaster == nullptr || unitCaster->getGuid() != mAura.getOwner()->getGuid())
        return;

    for (const auto& aurEff : unitCaster->getAuraEffectList(SPELL_AURA_SCHOOL_IMMUNITY))
    {
        if (aurEff->getAura()->getSpellInfo()->getMechanicsType() == MECHANIC_BANISHED)
        {
            // If aura owner is banished/cycloned, remove all aura targets for the duration of banish
            // Patch notes from 2.2.0 say Demonic Knowledge should not deactivate when pet is banished
            // Wowhead comment from 2.3.3 (01/28/08) also says "...other pet synergies remain when the pet is banished but SL is still affected."
            // However confirmed in classic that Demonic Knowledge is in fact removed (https://youtu.be/iX9IFuCGLiQ?t=469)
            // but in that video also Master Demonologist is removed which is casted by warlock
            // Going with original way and apply exception only for Demonic Knowledge -Appled
            if (!(aurSpellInfo->getSpellIconID() == 1876 && aurSpellInfo->getSpellFamilyName() == SPELLFAMILY_MAGE))
            {
                clearAreaAuraEffectTargets();
                return;
            }
        }
    }

    auto auraRadius = aurSpellInfo->calculateEffectRadius(effIndex, unitCaster, nullptr, &mAura);
    auraRadius = auraRadius * auraRadius;

    auto cannotBeEmptyAura = false;
    const auto effectId = aurSpellInfo->getEffect(effIndex);
    switch (effectId)
    {
        case SPELL_EFFECT_APPLY_GROUP_AREA_AURA:
        case SPELL_EFFECT_APPLY_RAID_AREA_AURA:
            _fillGroupOrRaidAreaAuraTargets(auraRadius, effectId == SPELL_EFFECT_APPLY_RAID_AREA_AURA);
            break;
        case SPELL_EFFECT_APPLY_PET_AREA_AURA:
            _fillPetAreaAuraTargets(auraRadius);
            cannotBeEmptyAura = true;
            break;
        case SPELL_EFFECT_APPLY_FRIEND_AREA_AURA:
            _fillFriendAreaAuraTargets(auraRadius);
            break;
        case SPELL_EFFECT_APPLY_ENEMY_AREA_AURA:
            _fillEnemyAreaAuraTargets(auraRadius);
            break;
#if VERSION_STRING >= TBC
        case SPELL_EFFECT_APPLY_OWNER_AREA_AURA:
            _fillOwnerAreaAuraTargets(auraRadius);
            cannotBeEmptyAura = true;
            break;
#endif
        default:
            return;
    }

    auto* const aurOwner = mAura.getOwner();
    for (auto itr = m_areaAuraTargets.begin(); itr != m_areaAuraTargets.cend();)
    {
        auto& [targetGuid, skipValidation] = (*itr);

        auto* const targetUnit = aurOwner->getWorldMapUnit(targetGuid);
        if (targetUnit == nullptr)
        {
            itr = m_areaAuraTargets.erase(itr);
            continue;
        }

        auto* const existingAura = targetUnit->getAuraWithId(aurSpellInfo->getId(), aurOwner->getGuid());
        if (skipValidation)
        {
            // Do not check freshly added targets again, check on next update
            skipValidation = false;
        }
        else
        {
            // Check if target in aura map is still valid
            auto removeAura = false;
            switch (effectId)
            {
                case SPELL_EFFECT_APPLY_GROUP_AREA_AURA:
                case SPELL_EFFECT_APPLY_RAID_AREA_AURA:
                {
                    const auto* plrOwner = mAura.getOwner()->getPlayerOwnerOrSelf();
                    if (plrOwner == nullptr)
                    {
                        // Try by unit owner
                        if (const auto* const unitOwner = mAura.getOwner()->getUnitOwner())
                            plrOwner = unitOwner->getPlayerOwnerOrSelf();
                    }

                    const auto checkRaid = effectId == SPELL_EFFECT_APPLY_RAID_AREA_AURA;
                    removeAura = !_canAddTargetToGroupOrRaidAreaAura(plrOwner, targetUnit, auraRadius, checkRaid);
                } break;
                case SPELL_EFFECT_APPLY_PET_AREA_AURA:
                {
                    const auto* const pet = targetUnit->isPet() ?
                        dynamic_cast<Pet const*>(targetUnit) :
                        nullptr;
                    removeAura = !_canAddPetToPetAreaAura(pet, auraRadius);
                } break;
                case SPELL_EFFECT_APPLY_FRIEND_AREA_AURA:
                {
                    removeAura = !_canAddTargetToFriendAreaAura(targetUnit, auraRadius);
                } break;
                case SPELL_EFFECT_APPLY_ENEMY_AREA_AURA:
                {
                    removeAura = !_canAddTargetToEnemyAreaAura(targetUnit, auraRadius);
                } break;
#if VERSION_STRING >= TBC
                case SPELL_EFFECT_APPLY_OWNER_AREA_AURA:
                {
                    removeAura = !_canAddOwnerToOwnerAreaAura(targetUnit, auraRadius);
                } break;
#endif
                default: break;
            }

            if (removeAura)
            {
                // Remove only this effect
                if (existingAura != nullptr)
                    existingAura->removeAuraEffect(effIndex);

                itr = m_areaAuraTargets.erase(itr);
                continue;
            }
        }

        if (existingAura != nullptr)
        {
            // Target has already this aura, replace duration and effect from aura
            existingAura->removeAuraEffect(effIndex);
            existingAura->setOriginalDuration(mAura.getOriginalDuration());
            existingAura->setMaxDuration(mAura.getMaxDuration());
            existingAura->setTimeLeft(mAura.getTimeLeft());
            // Periodic timer is same as casters timer
            existingAura->setPeriodicTimerFor(effIndex, mAura.getPeriodicTimerFor(effIndex));
            // Send calculated damage as base damage and set it static damage
            if (auto* const addedEffect = existingAura->addAuraEffect(mAuraEffect, mDamage, miscValue, mEffectPctModifier, true, effIndex))
                addedEffect->applyEffect(true);
        }
        else
        {
            // Create new aura for target
            auto tmpAura = sSpellMgr.newAura(aurSpellInfo, mAura.getOriginalDuration(), aurOwner, targetUnit, true);
            tmpAura->setIsAreaAura(true);
            tmpAura->setTimeLeft(mAura.getTimeLeft());
            // Periodic timer is same as casters timer
            tmpAura->setPeriodicTimerFor(effIndex, mAura.getPeriodicTimerFor(effIndex));
            // Send calculated damage as base damage and set it static damage
            tmpAura->addAuraEffect(mAuraEffect, mDamage, miscValue, mEffectPctModifier, true, effIndex);
            targetUnit->addAura(std::move(tmpAura));
        }
        ++itr;
    }

    if (cannotBeEmptyAura && m_areaAuraTargets.empty())
        mAura.removeAuraEffect(effIndex);
}

void AuraEffectModifier::clearAreaAuraEffectTargets()
{
    if (m_areaAuraTargets.empty())
        return;

    const auto* const aurOwner = mAura.getOwner();
    for (auto itr = m_areaAuraTargets.cbegin(); itr != m_areaAuraTargets.cend();)
    {
        const auto& [targetGuid, skipValidation] = (*itr);

        const auto* const targetUnit = aurOwner->getWorldMapUnit(targetGuid);
        if (targetUnit == nullptr)
        {
            itr = m_areaAuraTargets.erase(itr);
            continue;
        }

        if (auto* const existingAura = targetUnit->getAuraWithId(mAura.getSpellInfo()->getId(), aurOwner->getGuid()))
            existingAura->removeAuraEffect(effIndex);

        itr = m_areaAuraTargets.erase(itr);
    }
}

Aura* AuraEffectModifier::getAura() const { return &mAura; }

bool AuraEffectModifier::_canAddTargetToGroupOrRaidAreaAura(Player const* initiatorPlayer, Unit const* target, float radius, bool checkRaid) const
{
    if (target == nullptr)
        return false;

    if (initiatorPlayer != nullptr)
    {
        // Aura owner is player or has player owner
        // Target must be either player or pet from group/raid
        const auto* targetPlrOwner = target->getPlayerOwnerOrSelf();
        if (targetPlrOwner == nullptr)
        {
            // Try by unit owner
            if (const auto* const targetUnitOwner = target->getUnitOwner())
                targetPlrOwner = targetUnitOwner->getPlayerOwnerOrSelf();
        }

        if (targetPlrOwner == nullptr)
            return false;

        if (!initiatorPlayer->isInGroup())
        {
            // Quick check, if initiator is not in group, target cannot be player
            // but allow self if aura is from pet
            if (target->isPlayer() && initiatorPlayer->getGuid() != target->getGuid())
                return false;

            // Only aura owner's pets are valid
            if (initiatorPlayer->getGuid() != targetPlrOwner->getGuid())
                return false;
        }
        else
        {
            if (checkRaid)
            {
                if (!initiatorPlayer->getGroup()->HasMember(targetPlrOwner))
                    return false;
            }
            else
            {
                if (initiatorPlayer->getGroup() != targetPlrOwner->getGroup())
                    return false;

                if (initiatorPlayer->getSubGroupSlot() != targetPlrOwner->getSubGroupSlot())
                    return false;
            }
        }
    }

    if (!target->isAlive())
        return false;

    const auto* const aurOwner = mAura.getOwner();

    // If pvp flag not enabled, cannot add pvp flagged targets
    if (!aurOwner->isPvpFlagSet() && target->isPvpFlagSet())
        return false;

    if (aurOwner->getDistanceSq(target) > radius)
        return false;

    if (aurOwner->GetInstanceID() != target->GetInstanceID())
        return false;

    // Only assistable targets
    if (!aurOwner->isValidAssistableTarget(target, mAura.getSpellInfo()))
        return false;

    return true;
}

void AuraEffectModifier::_fillGroupOrRaidAreaAuraTargets(float radius, bool checkRaid)
{
    const auto* aurOwner = mAura.getOwner();
    const auto* plrOwner = aurOwner->getPlayerOwnerOrSelf();
    if (plrOwner == nullptr)
    {
        // Try by unit owner
        if (const auto* const unitOwner = aurOwner->getUnitOwner())
            plrOwner = unitOwner->getPlayerOwnerOrSelf();
    }

    // Helper lambdas
    auto findAllSummonsForTarget = [this, radius, checkRaid](Player const* plrOwner) -> void
    {
        if (plrOwner == nullptr)
            return;

        for (const auto* const summon : plrOwner->getSummonInterface()->getSummons())
        {
            // Not possible but make compiler happy
            if (summon == nullptr)
                continue;

            // In case aura owner is a summon do not add self to target map
            if (mAura.getOwner()->getGuid() == summon->getGuid())
                continue;

            const auto* const totemSummon = summon->isTotem() ? dynamic_cast<TotemSummon const*>(summon) : nullptr;
            if (totemSummon != nullptr)
            {
                // For totems get their summons as well so they can be added to target map
                for (const auto* const subSummon : totemSummon->getSummonInterface()->getSummons())
                {
                    // In case aura owner is a summon do not add self to target map
                    if (mAura.getOwner()->getGuid() == subSummon->getGuid())
                        continue;

                    if (!_canAddTargetToGroupOrRaidAreaAura(plrOwner, subSummon, radius, checkRaid))
                        continue;

                    // Skip target validation on this update
                    m_areaAuraTargets.insert_or_assign(subSummon->getGuid(), true);
                }

                // Skip adding totems since they cannot benefit from area auras
                continue;
            }

            if (!_canAddTargetToGroupOrRaidAreaAura(plrOwner, summon, radius, checkRaid))
                continue;

            // Skip target validation on this update
            m_areaAuraTargets.insert_or_assign(summon->getGuid(), true);
        }
    };

    auto findSubgroupPlayers = [this, findAllSummonsForTarget, radius, checkRaid](Player const* plrOwner, SubGroup* subGroup) -> void
    {
        if (plrOwner == nullptr || subGroup == nullptr)
            return;

        for (const auto* const cachedInfo : subGroup->getGroupMembers())
        {
            // Player owner is added later in target map
            if (plrOwner->getGuidLow() == cachedInfo->guid)
                continue;

            const auto* const grpPlayer = sObjectMgr.getPlayer(cachedInfo->guid);
            if (!_canAddTargetToGroupOrRaidAreaAura(plrOwner, grpPlayer, radius, checkRaid))
                continue;

            // Skip target validation on this update
            m_areaAuraTargets.insert_or_assign(grpPlayer->getGuid(), true);

            // Add target's summons to target map
            findAllSummonsForTarget(grpPlayer);
        }
    };

    if (plrOwner == nullptr)
    {
        // Aura owner is pure creature
        // Add nearby units with same faction to target map
        for (const auto* const obj : aurOwner->getInRangeObjectsSet())
        {
            const auto* const creatureObj = obj->isCreature() ? dynamic_cast<Creature const*>(obj) : nullptr;
            if (creatureObj == nullptr)
                continue;

            // Skip player summons
            if (creatureObj->hasUnitFlags(UNIT_FLAG_PVP_ATTACKABLE))
                continue;

            if (aurOwner->getFactionTemplate() != creatureObj->getFactionTemplate())
                continue;

            if (!_canAddTargetToGroupOrRaidAreaAura(nullptr, creatureObj, radius, checkRaid))
                continue;

            // Skip target validation on this update
            m_areaAuraTargets.insert_or_assign(creatureObj->getGuid(), true);
        }
    }
    else
    {
        if (plrOwner->isInGroup())
        {
            plrOwner->getGroup()->Lock();

            if (checkRaid)
            {
                for (uint32_t i = 0; i < plrOwner->getGroup()->GetSubGroupCount(); ++i)
                {
                    findSubgroupPlayers(plrOwner, plrOwner->getGroup()->GetSubGroup(i));
                }
            }
            else
            {
                findSubgroupPlayers(plrOwner, plrOwner->getGroup()->GetSubGroup(plrOwner->getSubGroupSlot()));
            }

            plrOwner->getGroup()->Unlock();
        }

        // Add player's own summons to target map
        findAllSummonsForTarget(plrOwner);

        // Include owner as well
        if (aurOwner->getGuid() != plrOwner->getGuid())
        {
            if (_canAddTargetToGroupOrRaidAreaAura(plrOwner, plrOwner, radius, checkRaid))
            {
                // Skip target validation on this update
                m_areaAuraTargets.insert_or_assign(plrOwner->getGuid(), true);
            }
        }
    }
}

bool AuraEffectModifier::_canAddPetToPetAreaAura(Pet const* pet, float radius) const
{
    if (pet == nullptr)
        return false;

    if (!pet->isAlive())
        return false;

    if (mAura.getOwner()->getDistanceSq(pet) > radius)
        return false;

    return true;
}

void AuraEffectModifier::_fillPetAreaAuraTargets(float radius)
{
    const auto* const pet = mAura.getOwner()->getPet();
    if (!_canAddPetToPetAreaAura(pet, radius))
        return;

    // Skip target validation on this update
    m_areaAuraTargets.insert_or_assign(pet->getGuid(), true);
}

bool AuraEffectModifier::_canAddTargetToFriendAreaAura(Unit const* target, float radius) const
{
    if (target == nullptr)
        return false;

    if (!target->isAlive())
        return false;

    const auto* const aurOwner = mAura.getOwner();

    // If pvp flag not enabled, cannot add pvp flagged targets
    if (!aurOwner->isPvpFlagSet() && target->isPvpFlagSet())
        return false;

    if (aurOwner->getDistanceSq(target) > radius)
        return false;

    if (aurOwner->GetInstanceID() != target->GetInstanceID())
        return false;

    // Only assistable targets
    if (!aurOwner->isValidAssistableTarget(target, mAura.getSpellInfo()))
        return false;

    return true;
}

void AuraEffectModifier::_fillFriendAreaAuraTargets(float radius)
{
    for (const auto* const object : mAura.getOwner()->getInRangeObjectsSet())
    {
        const auto* const unit = object->isCreatureOrPlayer() ?
            dynamic_cast<Unit const*>(object) :
            nullptr;

        if (unit == nullptr)
            continue;

        if (!_canAddTargetToFriendAreaAura(unit, radius))
            continue;

        // Skip target validation on this update
        m_areaAuraTargets.insert_or_assign(unit->getGuid(), true);
    }
}

bool AuraEffectModifier::_canAddTargetToEnemyAreaAura(Unit const* target, float radius) const
{
    if (target == nullptr)
        return false;

    if (!target->isAlive())
        return false;

    const auto* const aurOwner = mAura.getOwner();

    // If pvp flag not enabled, cannot add pvp flagged targets
    if (!aurOwner->isPvpFlagSet() && target->isPvpFlagSet())
        return false;

    if (aurOwner->getDistanceSq(target) > radius)
        return false;

    if (aurOwner->GetInstanceID() != target->GetInstanceID())
        return false;

    // Only attackable targets
    if (!aurOwner->isValidAttackableTarget(target, mAura.getSpellInfo()))
        return false;

    return true;
}

void AuraEffectModifier::_fillEnemyAreaAuraTargets(float radius)
{
    for (const auto* const object : mAura.getOwner()->getInRangeObjectsSet())
    {
        const auto* const unit = object->isCreatureOrPlayer() ?
            dynamic_cast<Unit const*>(object) :
            nullptr;

        if (unit == nullptr)
            continue;

        if (!_canAddTargetToEnemyAreaAura(unit, radius))
            continue;

        // Skip target validation on this update
        m_areaAuraTargets.insert_or_assign(unit->getGuid(), true);
    }
}

bool AuraEffectModifier::_canAddOwnerToOwnerAreaAura(Unit const* owner, float radius) const
{
    if (owner == nullptr)
        return false;

    if (!owner->isAlive())
        return false;

    if (mAura.getOwner()->getDistanceSq(owner) > radius)
        return false;

    return true;
}

void AuraEffectModifier::_fillOwnerAreaAuraTargets(float radius)
{
    const auto* const aurOwner = mAura.getOwner();
    if (!aurOwner->isSummon())
        return;

    const auto* const ownerUnit = aurOwner->getUnitOwner();
    if (!_canAddOwnerToOwnerAreaAura(ownerUnit, radius))
        return;

    // Skip target validation on this update
    m_areaAuraTargets.insert_or_assign(ownerUnit->getGuid(), true);
}
