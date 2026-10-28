/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Definitions/AuraEffects.hpp"
#include "Platform/SymbolVisibility.hpp"
#include "SpellAuraDefines.hpp"

#include <cmath>
#include <cstdint>

class Aura;
class Pet;
class Player;
class Unit;

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

    void setEffectPeriodicTimer(int32_t timer);
    int32_t getEffectPeriodicTimer() const;

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

    void calculateEffectAmplitude();

    void updateAreaAuraEffectTargets(bool periodicTickForAreaAuraTargets);
    void clearAreaAuraEffectTargets();

    void update(unsigned long diff, bool updateAreaAuraTargets);

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
    int32_t mPeriodicTimer = 0;                 // Effect periodic timer
    float_t mDamageFraction = 0.0f;             // Leftover damage from previous tick which will be added to next tick
    float_t mEffectPctModifier = 1.0f;          // Effect percent modifier
    bool mEffectDamageStatic = false;           // If effect damage is set to static, effect will not gain spell power bonuses
    bool mActive = false;                       // Is effect active
    uint8_t effIndex = 0;

    void _periodicTick();

    AreaAuraList m_areaAuraTargets;

    bool _canAddTargetToGroupOrRaidAreaAura(Player const* initiatorPlayer, Unit const* target, float radius, bool checkRaid) const;
    void _fillGroupOrRaidAreaAuraTargets(float radius, bool checkRaid);
    bool _canAddPetToPetAreaAura(Pet const* pet, float radius) const;
    void _fillPetAreaAuraTargets(float radius);
    bool _canAddTargetToFriendAreaAura(Unit const* target, float radius) const;
    void _fillFriendAreaAuraTargets(float radius);
    bool _canAddTargetToEnemyAreaAura(Unit const* target, float radius) const;
    void _fillEnemyAreaAuraTargets(float radius);
    bool _canAddOwnerToOwnerAreaAura(Unit const* owner, float radius) const;
    void _fillOwnerAreaAuraTargets(float radius);

    Aura& mAura;
};
