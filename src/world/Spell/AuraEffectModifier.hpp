/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Definitions/AuraEffects.hpp"
#include "Platform/SymbolVisibility.hpp"

#include <cmath>
#include <cstdint>

class Aura;

struct SERVER_DECL AuraEffectModifier
{
public:
    explicit AuraEffectModifier(Aura& parent);

    void setAuraEffectType(AuraEffect type);
    AuraEffect getAuraEffectType() const;

    void setEffectDamage(int32_t value);
    void setEffectDamage(float_t value);
    int32_t getEffectDamage() const;
    float_t getEffectFloatDamage() const;

    void setEffectBaseDamage(int32_t baseValue);
    int32_t getEffectBaseDamage() const;

    void setEffectExtraField(int32_t extraValue);
    int32_t getEffectExtraField() const;

    void setEffectExtra2Field(int32_t extraValue);
    int32_t getEffectExtra2Field() const;

    void setEffectMiscValue(int32_t miscValue);
    int32_t getEffectMiscValue() const;

    void setEffectAmplitude(int32_t amplitude);
    int32_t getEffectAmplitude() const;

    void setEffectDamageFraction(float_t fraction);
    float_t getEffectDamageFraction() const;

    void setEffectPercentModifier(float_t pctMod);
    float_t getEffectPercentModifier() const;

    void setEffectDamageStatic(bool);
    bool isEffectDamageStatic() const;

    void setEffectIndex(uint8_t effIndex);
    uint8_t getEffectIndex() const;

    void setEffectActive(bool set);
    bool isActive() const;

    bool isPeriodicEffect() const;

    void applyEffect(bool apply, bool skipScriptCheck = false);
    void resetEffect();

    // Never nullptr
    Aura* getAura() const;

private:
    AuraEffect mAuraEffect = SPELL_AURA_NONE;   // Effect type : SPELL_AURA_NONE
    int32_t mDamage = 0;                        // Effect calculated amount
    float_t mRealDamage = 0.0f;                 // Effect exact calculated damage
    int32_t mBaseDamage = 0;                    // Effect base amount
    int32_t mExtraField = 0;                    // Extra field, e.g. used with auras that increase your spell power by % of some stat
    int32_t mExtraField2 = 0;                   // Second extra field
    int32_t miscValue = 0;                      // Misc Value
    int32_t mAmplitude = 0;                     // Effect amplitude
    float_t mDamageFraction = 0.0f;             // Leftover damage from previous tick which will be added to next tick
    float_t mEffectPctModifier = 1.0f;          // Effect percent modifier
    bool mEffectDamageStatic = false;           // If effect damage is set to static, effect will not gain spell power bonuses
    bool mActive = false;                       // Is effect active
    uint8_t effIndex = 0;

    Aura& mAura;
};
