/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

struct SpellTargetMod
{
    SpellTargetMod(uint64_t targetGuid, uint8_t targetModType) : targetGuid(targetGuid),
                                                                 targetModType(targetModType)
    {
    }

    uint64_t targetGuid;
    uint8_t targetModType;
};
