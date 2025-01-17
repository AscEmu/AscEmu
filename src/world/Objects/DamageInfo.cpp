/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "DamageInfo.hpp"
#include "Spell/Definitions/ProcFlags.hpp"
#include "Spell/Definitions/School.hpp"
#include "Units/UnitDefines.hpp"

DamageInfo::DamageInfo()
{
    schoolMask = SCHOOL_MASK_NORMAL;
    weaponType = MELEE;
    attackerProcFlags = PROC_NULL;
    victimProcFlags = PROC_NULL;
}

uint8_t DamageInfo::getSchoolTypeFromMask() const
{
    for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
    {
        if (schoolMask & (1 << i))
            return i;
    }

    // shouldn't happen
    return SCHOOL_NORMAL;
}
