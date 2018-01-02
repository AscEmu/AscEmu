/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum SpellCoefficientsFlags
{
    SPELL_FLAG_IS_DOT_OR_HOT_SPELL = 0x1,
    SPELL_FLAG_IS_DD_OR_DH_SPELL   = 0x2,
    SPELL_FLAG_IS_DD_DH_DOT_SPELL  = SPELL_FLAG_IS_DOT_OR_HOT_SPELL | SPELL_FLAG_IS_DD_OR_DH_SPELL,
    SPELL_FLAG_AOE_SPELL           = 0x4,
    SPELL_FLAG_ADITIONAL_EFFECT    = 0x8
};
