/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "SpellDidHitResult.hpp"

#include <cstdint>

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
