/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AuraEffectModifier.hpp"

#include "SpellAura.hpp"
#include "SpellScriptDefines.hpp"
#include "Logging/Logger.hpp"
#include "Server/Script/ScriptMgr.hpp"

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
}

Aura* AuraEffectModifier::getAura() const { return &mAura; }
