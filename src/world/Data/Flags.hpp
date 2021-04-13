/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"

#if VERSION_STRING <= WotLK

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE                     = 0x0000,
    UPDATEFLAG_SELF                     = 0x0001,
    UPDATEFLAG_TRANSPORT                = 0x0002,
    UPDATEFLAG_HAS_TARGET               = 0x0004,
    UPDATEFLAG_LOWGUID                  = 0x0008,
    UPDATEFLAG_HIGHGUID                 = 0x0010,
    UPDATEFLAG_LIVING                   = 0x0020,
    UPDATEFLAG_HAS_POSITION             = 0x0040,
#if VERSION_STRING > TBC
    UPDATEFLAG_VEHICLE                  = 0x0080,
    UPDATEFLAG_POSITION                 = 0x0100,
    UPDATEFLAG_ROTATION                 = 0x0200,
    UPDATEFLAG_UNK1                     = 0x0400,
    UPDATEFLAG_ANIM_KITS                = 0x0800,
    UPDATEFLAG_TRANSPORT_ARR            = 0x1000,
    UPDATEFLAG_ENABLE_PORTALS           = 0x2000,
    UPDATEFLAG_UNK2                     = 0x4000
#endif
};

#elif VERSION_STRING == Cata
enum TrainerSpellState
{
    TRAINER_SPELL_GRAY                  = 0,
    TRAINER_SPELL_GREEN                 = 1,
    TRAINER_SPELL_RED                   = 2,
    TRAINER_SPELL_GREEN_DISABLED        = 10
};

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE                     = 0x0000,
    UPDATEFLAG_SELF                     = 0x0001,
    UPDATEFLAG_TRANSPORT                = 0x0002,
    UPDATEFLAG_HAS_TARGET               = 0x0004,
    UPDATEFLAG_UNK                      = 0x0008,
    UPDATEFLAG_LOWGUID                  = 0x0010,
    UPDATEFLAG_LIVING                   = 0x0020,
    UPDATEFLAG_HAS_POSITION             = 0x0040,
    UPDATEFLAG_VEHICLE                  = 0x0080,
    UPDATEFLAG_POSITION                 = 0x0100,
    UPDATEFLAG_ROTATION                 = 0x0200,
    UPDATEFLAG_UNK1                     = 0x0400,
    UPDATEFLAG_ANIM_KITS                = 0x0800,
    UPDATEFLAG_TRANSPORT_ARR            = 0x1000,
    UPDATEFLAG_UNK3                     = 0x2000
};
#elif VERSION_STRING == Mop
enum TrainerSpellState
{
    TRAINER_SPELL_GRAY                  = 0,
    TRAINER_SPELL_GREEN                 = 1,
    TRAINER_SPELL_RED                   = 2,
    TRAINER_SPELL_GREEN_DISABLED        = 10
};

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE                     = 0x0000,
    UPDATEFLAG_SELF                     = 0x0001,
    UPDATEFLAG_TRANSPORT                = 0x0002,
    UPDATEFLAG_HAS_TARGET               = 0x0004,
    UPDATEFLAG_LIVING                   = 0x0008,
    UPDATEFLAG_HAS_POSITION             = 0x0010, //stationary
    UPDATEFLAG_VEHICLE                  = 0x0020,
    UPDATEFLAG_POSITION                 = 0x0040, //transport position
    UPDATEFLAG_ROTATION                 = 0x0080,
    UPDATEFLAG_ANIM_KITS                = 0x0100,
};

#endif
