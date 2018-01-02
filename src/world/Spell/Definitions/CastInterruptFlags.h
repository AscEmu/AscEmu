/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CastInterruptFlags
{
    CAST_INTERRUPT_NULL                = 0x0,
    CAST_INTERRUPT_ON_MOVEMENT         = 0x1,
    CAST_INTERRUPT_PUSHBACK            = 0x2,
    CAST_INTERRUPT_ON_INTERRUPT_CAST   = 0x4,
    CAST_INTERRUPT_ON_INTERRUPT_SCHOOL = 0x8,
    CAST_INTERRUPT_ON_DAMAGE_TAKEN     = 0x10,
    CAST_INTERRUPT_ON_INTERRUPT_ALL    = 0x20
};
