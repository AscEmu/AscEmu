/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Custom Flags to help with spell processing
enum SpellIsFlags
{
    SPELL_FLAG_IS_DAMAGING                       = 0x1,
    SPELL_FLAG_IS_HEALING                        = 0x2,
    SPELL_FLAG_IS_TARGETINGSTEALTHED             = 0x4,
    SPELL_FLAG_IS_REQUIRECOOLDOWNUPDATE          = 0x8,
    SPELL_FLAG_IS_POISON                         = 0x10,
    SPELL_FLAG_IS_FINISHING_MOVE                 = 0x20,
    SPELL_FLAG_IS_NOT_USING_DMG_BONUS            = 0x40,
    SPELL_FLAG_IS_CHILD_SPELL                    = 0x80,
    SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_ON_PET    = 0x100,
    SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_PET_OWNER = 0x200,
    SPELL_FLAG_IS_EXPIREING_WITH_PET             = 0x400,
    SPELL_FLAG_IS_EXPIREING_ON_PET               = 0x800,
    SPELL_FLAG_IS_FORCEDDEBUFF                   = 0x1000,
    SPELL_FLAG_IS_FORCEDBUFF                     = 0x2000,
    SPELL_FLAG_IS_INHERITING_LEVEL               = 0x4000,
    SPELL_FLAG_IS_MAXSTACK_FOR_DEBUFF            = 0x8000
};
