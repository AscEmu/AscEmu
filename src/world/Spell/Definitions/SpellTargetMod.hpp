/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "SpellDidHitResult.hpp"

struct SpellTargetMod
{
    SpellTargetMod(uint64_t targetGuid, SpellDidHitResult hitResult, SpellDidHitResult extendedHitResult)
        : targetGuid(targetGuid), hitResult(hitResult), extendedHitResult(extendedHitResult)
    {
    }

    uint64_t targetGuid;
    SpellDidHitResult hitResult;
    SpellDidHitResult extendedHitResult; // used with reflected spells
};
