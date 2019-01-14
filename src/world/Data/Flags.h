/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"

#if VERSION_STRING < Cata
enum MovementFlags
{
    // Byte 1 (Resets on Movement Key Press)
    MOVEFLAG_MOVE_STOP                  = 0x00000000,   //verified
    MOVEFLAG_MOVE_FORWARD               = 0x00000001,   //verified
    MOVEFLAG_MOVE_BACKWARD              = 0x00000002,   //verified
    MOVEFLAG_STRAFE_LEFT                = 0x00000004,   //verified
    MOVEFLAG_STRAFE_RIGHT               = 0x00000008,   //verified
    MOVEFLAG_TURN_LEFT                  = 0x00000010,   //verified
    MOVEFLAG_TURN_RIGHT                 = 0x00000020,   //verified
    MOVEFLAG_PITCH_DOWN                 = 0x00000040,   //Unconfirmed
    MOVEFLAG_PITCH_UP                   = 0x00000080,   //Unconfirmed

    // Byte 2 (Resets on Situation Change)
    MOVEFLAG_WALK                       = 0x00000100,   //verified
    MOVEFLAG_TRANSPORT                  = 0x00000200,
    MOVEFLAG_DISABLEGRAVITY             = 0x00000400,   // Zyres: disable gravity
    MOVEFLAG_ROOTED                     = 0x00000800,   //verified
    MOVEFLAG_REDIRECTED                 = 0x00001000,   //Unconfirmed, should be MOVEFLAG_JUMPING
    MOVEFLAG_FALLING                    = 0x00002000,   //verified
    MOVEFLAG_FALLING_FAR                = 0x00004000,   //verified
    MOVEFLAG_FREE_FALLING               = 0x00008000,   //half verified

    // Byte 3 (Set by server. TB = Third Byte. Completely unconfirmed.)
    MOVEFLAG_TB_PENDING_STOP            = 0x00010000,   // (MOVEFLAG_PENDING_STOP)
    MOVEFLAG_TB_PENDING_UNSTRAFE        = 0x00020000,   // (MOVEFLAG_PENDING_UNSTRAFE)
    MOVEFLAG_TB_PENDING_FALL            = 0x00040000,   // (MOVEFLAG_PENDING_FALL)
    MOVEFLAG_TB_PENDING_FORWARD         = 0x00080000,   // (MOVEFLAG_PENDING_FORWARD)
    MOVEFLAG_TB_PENDING_BACKWARD        = 0x00100000,   // (MOVEFLAG_PENDING_BACKWARD)
    MOVEFLAG_SWIMMING                   = 0x00200000,   //  verified
    MOVEFLAG_ASCENDING                  = 0x00400000,
    MOVEFLAG_DESCENDING                 = 0x00800000,

    // Byte 4 (Script Based Flags. Never reset, only turned on or off.)
    MOVEFLAG_CAN_FLY                    = 0x01000000,   // Zyres: can_fly
    MOVEFLAG_FLYING                     = 0x02000000,   // Zyres: flying
    MOVEFLAG_SPLINE_MOVER               = 0x04000000,   // Zyres: spl elevation
    MOVEFLAG_SPLINE_ENABLED             = 0x08000000,
    MOVEFLAG_WATER_WALK                 = 0x10000000,
    MOVEFLAG_FEATHER_FALL               = 0x20000000,   // Does not negate fall damage.
    MOVEFLAG_HOVER                      = 0x40000000,
    //MOVEFLAG_LOCAL                    = 0x80000000,   // Zyres: commented unused 2015/12/20

    // Masks
    MOVEFLAG_MOVING_MASK                = 0x03,
    MOVEFLAG_STRAFING_MASK              = 0x0C,
    MOVEFLAG_TURNING_MASK               = 0x30,         // MOVEFLAG_TURN_LEFT + MOVEFLAG_TURN_RIGHT
    MOVEFLAG_FALLING_MASK               = 0x6000,
    MOVEFLAG_MOTION_MASK                = 0xE00F,       // Forwards, Backwards, Strafing, Falling
    MOVEFLAG_PENDING_MASK               = 0x7F0000,
    MOVEFLAG_PENDING_STRAFE_MASK        = 0x600000,
    MOVEFLAG_PENDING_MOVE_MASK          = 0x180000,
    MOVEFLAG_FULL_FALLING_MASK          = 0xE000,
};

enum MovementFlags2
{
    MOVEFLAG2_NO_STRAFING           = 0x0001,
    MOVEFLAG2_NO_JUMPING            = 0x0002,
    MOVEFLAG2_UNK1                  = 0x0004,
    MOVEFLAG2_FULLSPEED_TURNING     = 0x0008,
    MOVEFLAG2_FULLSPEED_PITCHING    = 0x0010,
    MOVEFLAG2_ALLOW_PITCHING        = 0x0020,
    MOVEFLAG2_UNK2                  = 0x0040,
    MOVEFLAG2_UNK3                  = 0x0080,
    MOVEFLAG2_UNK4                  = 0x0100,
    MOVEFLAG2_UNK5                  = 0x0200,
    MOVEFLAG2_INTERPOLATED_MOVE     = 0x0400,
    MOVEFLAG2_INTERPOLATED_TURN     = 0x0800,
    MOVEFLAG2_INTERPOLATED_PITCH    = 0x1000
};

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE         = 0x0000,
    UPDATEFLAG_SELF         = 0x0001,
    UPDATEFLAG_TRANSPORT    = 0x0002,
    UPDATEFLAG_HAS_TARGET   = 0x0004,
    UPDATEFLAG_LOWGUID      = 0x0008,
    UPDATEFLAG_HIGHGUID     = 0x0010,
    UPDATEFLAG_LIVING       = 0x0020,
    UPDATEFLAG_HAS_POSITION = 0x0040,
#if VERSION_STRING > TBC
    UPDATEFLAG_VEHICLE      = 0x0080,
    UPDATEFLAG_POSITION     = 0x0100,
    UPDATEFLAG_ROTATION     = 0x0200,
    UPDATEFLAG_UNK1         = 0x0400,
    UPDATEFLAG_ANIM_KITS    = 0x0800,
    UPDATEFLAG_TRANSPORT_ARR = 0x1000,
    UPDATEFLAG_ENABLE_PORTALS = 0x2000,
    UPDATEFLAG_UNK2         = 0x4000
#endif
};

#else
enum TrainerSpellState
{
    TRAINER_SPELL_GRAY = 0,
    TRAINER_SPELL_GREEN = 1,
    TRAINER_SPELL_RED = 2,
    TRAINER_SPELL_GREEN_DISABLED = 10
};

#if VERSION_STRING == Cata
    #include "GameCata/Movement/MovementDefines.h"
#elif VERSION_STRING == Mop
    #include "GameMop/Movement/MovementDefines.h"
#endif

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE         = 0x0000,
    UPDATEFLAG_SELF         = 0x0001,
    UPDATEFLAG_TRANSPORT    = 0x0002,
    UPDATEFLAG_HAS_TARGET   = 0x0004,
    UPDATEFLAG_UNK          = 0x0008,
    UPDATEFLAG_LOWGUID      = 0x0010,
    UPDATEFLAG_LIVING       = 0x0020,
    UPDATEFLAG_HAS_POSITION = 0x0040,
    UPDATEFLAG_VEHICLE      = 0x0080,
    UPDATEFLAG_POSITION     = 0x0100,
    UPDATEFLAG_ROTATION     = 0x0200,
    UPDATEFLAG_UNK1         = 0x0400,
    UPDATEFLAG_ANIM_KITS    = 0x0800,
    UPDATEFLAG_TRANSPORT_ARR = 0x1000,
    UPDATEFLAG_UNK3         = 0x2000
};
#endif
