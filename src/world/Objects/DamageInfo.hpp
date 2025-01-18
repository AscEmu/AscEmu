/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum SchoolMask : uint8_t;
enum WeaponDamageType : uint8_t;
enum SpellProcFlags : uint32_t;

struct DamageInfo
{
    DamageInfo();

    SchoolMask schoolMask;

    uint32_t realDamage = 0; // the damage after resist, absorb etc
    int32_t fullDamage = 0; // the damage before resist, absorb etc
    uint32_t absorbedDamage = 0;
    uint32_t resistedDamage = 0;
    uint32_t blockedDamage = 0;

    WeaponDamageType weaponType;
    bool isHeal = false;
    bool isCritical = false;
    bool isPeriodic = false;

    uint32_t attackerProcFlags;
    uint32_t victimProcFlags;

    uint8_t getSchoolTypeFromMask() const;
};
