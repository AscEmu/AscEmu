/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AuraEffectModifier.hpp"

#include "SpellAura.hpp"
#include "SpellScriptDefines.hpp"
#include "Logging/Logger.hpp"
#include "Server/Script/ScriptMgr.hpp"

extern pSpellAura SpellAuraHandler[TOTAL_SPELL_AURAS];

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

void AuraEffectModifier::setEffectFixedDamage(int32_t fixedValue) { mFixedDamage = fixedValue; }
int32_t AuraEffectModifier::getEffectFixedDamage() const { return mFixedDamage; }

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

void AuraEffectModifier::applyEffect(bool apply, bool skipScriptCheck/* = false*/)
{
    // Do not apply or remove effect multiple times
    if (mActive == apply)
        return;

    mActive = apply;

    if (getAura())
    {
        if (skipScriptCheck)
        {
            (*getAura().*SpellAuraHandler[getAuraEffectType()])(this, apply);
        }
        else
        {
            const auto scriptResult = sScriptMgr.callScriptedAuraBeforeAuraEffect(getAura(), this, apply);
            if (scriptResult != SpellScriptExecuteState::EXECUTE_PREVENT)
                (*getAura().*SpellAuraHandler[getAuraEffectType()])(this, apply);
        }
    }
    else
        sLogger.failure("AuraEffectModifier::applyEffect fatal Error invalid Aura!");
}

void AuraEffectModifier::setAura(Aura* aur) { mAura = aur; }
Aura* AuraEffectModifier::getAura() const { return mAura; }
