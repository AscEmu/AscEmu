/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "MovementTypedefs.h"
#include <cstdint>
#include <string>

namespace MovementMgr {
#pragma pack(push, 1)
class MoveSplineFlag
{
public:
#if VERSION_STRING <= TBC
    enum eFlags
    {
        None                    = 0x00000000,
        Done                    = 0x00000001,
        Falling                 = 0x00000002, // Affects elevation computation
        Unknown3                = 0x00000004,
        Unknown4                = 0x00000008,
        Unknown5                = 0x00000010,
        Unknown6                = 0x00000020,
        Unknown7                = 0x00000040,
        Unknown8                = 0x00000080,
        Runmode                 = 0x00000100,
        Flying                  = 0x00000200, // Smooth movement(Catmullrom interpolation mode), flying animation
        No_Spline               = 0x00000400,
        Unknown12               = 0x00000800,
        Unknown13               = 0x00001000,
        Unknown14               = 0x00002000,
        Unknown15               = 0x00004000,
        Unknown16               = 0x00008000,
        Final_Point             = 0x00010000,
        Final_Target            = 0x00020000,
        Final_Angle             = 0x00040000,
        Unknown20               = 0x00080000,
        Cyclic                  = 0x00100000,
        Enter_Cycle             = 0x00200000, // Everytimes appears with cyclic flag in monster move packet, erases first spline vertex after first cycle done
        Frozen                  = 0x00400000, // Will never arrive
        Unknown24               = 0x00800000,
        Unknown25               = 0x01000000,
        Unknown26               = 0x02000000,
        Unknown27               = 0x04000000,
        Unknown28               = 0x08000000,
        Unknown29               = 0x10000000,
        Unknown30               = 0x20000000,
        Unknown31               = 0x40000000,
        Unknown32               = 0x80000000,

        // Masks
        Mask_Final_Facing = Final_Point | Final_Target | Final_Angle,
        // flags that shouldn't be appended into SMSG_MONSTER_MOVE\SMSG_MONSTER_MOVE_TRANSPORT packet, should be more probably
        Mask_No_Monster_Move = Mask_Final_Facing | Done,
        // CatmullRom interpolation mode used
        Mask_CatmullRom = Flying
    };

    inline uint32_t& raw() { return (uint32_t&)*this; }
    inline uint32_t const& raw() const { return (uint32_t const&)*this; }

    MoveSplineFlag() { raw() = 0; }
    MoveSplineFlag(uint32_t f) { raw() = f; }

    // Constant interface
    bool isSmooth() const { return (raw() & Mask_CatmullRom) != 0; }
    bool isLinear() const { return !isSmooth(); }
    bool isFacing() const { return (raw() & Mask_Final_Facing) != 0; }

    bool hasAllFlags(uint32_t f) const { return (raw() & f) == f; }
    bool hasFlag(uint32_t f) const { return (raw() & f) != 0; }
    uint32_t operator & (uint32_t f) const { return (raw() & f); }
    uint32_t operator | (uint32_t f) const { return (raw() | f); }
    std::string ToString() const;

    // Not constant interface
    void operator &= (uint32_t f) { raw() &= f; }
    void operator |= (uint32_t f) { raw() |= f; }

    void EnableFalling() { raw() = (raw() & ~(Flying)) | Falling; }
    void EnableFlying() { raw() = (raw() & ~(Falling)) | Flying; }
    void EnableFacingPoint() { raw() = (raw() & ~Mask_Final_Facing) | Final_Point; }
    void EnableFacingAngle() { raw() = (raw() & ~Mask_Final_Facing) | Final_Angle; }
    void EnableFacingTarget() { raw() = (raw() & ~Mask_Final_Facing) | Final_Target; }

    bool done                   : 1;
    bool falling                : 1;
    bool unknown3               : 1;
    bool unknown4               : 1;
    bool unknown5               : 1;
    bool unknown6               : 1;
    bool unknown7               : 1;
    bool unknown8               : 1;
    bool runmode                : 1;
    bool flying                 : 1;
    bool no_spline              : 1;
    bool unknown12              : 1;
    bool unknown13              : 1;
    bool unknown14              : 1;
    bool unknown15              : 1;
    bool unknown16              : 1;
    bool final_point            : 1;
    bool final_target           : 1;
    bool final_angle            : 1;
    bool unknown20              : 1;
    bool cyclic                 : 1;
    bool enter_cycle            : 1;
    bool frozen                 : 1;
    bool unknown24              : 1;
    bool unknown25              : 1;
    bool unknown26              : 1;
    bool unknown27              : 1;
    bool unknown28              : 1;
    bool unknown29              : 1;
    bool unknown30              : 1;
    bool unknown31              : 1;
    bool unknown32              : 1;
#elif VERSION_STRING == WotLK
    enum eFlags
    {
        None                    = 0x00000000,
                                                        // x00-xFF(first byte) used as animation Ids storage in pair with Animation flag
        Done                    = 0x00000100,
        Falling                 = 0x00000200,           // Affects elevation computation, can't be combined with Parabolic flag
        No_Spline               = 0x00000400,
        Parabolic               = 0x00000800,           // Affects elevation computation, can't be combined with Falling flag
        CanSwim                 = 0x00001000,
        Flying                  = 0x00002000,           // Smooth movement(Catmullrom interpolation mode), flying animation
        OrientationFixed        = 0x00004000,           // Model orientation fixed
        Final_Point             = 0x00008000,
        Final_Target            = 0x00010000,
        Final_Angle             = 0x00020000,
        Catmullrom              = 0x00040000,           // Used Catmullrom interpolation mode
        Cyclic                  = 0x00080000,           // Movement by cycled spline
        Enter_Cycle             = 0x00100000,           // Everytimes appears with cyclic flag in monster move packet, erases first spline vertex after first cycle done
        Animation               = 0x00200000,           // Plays animation after some time passed
        Frozen                  = 0x00400000,           // Will never arrive
        TransportEnter          = 0x00800000,
        TransportExit           = 0x01000000,
        Unknown7                = 0x02000000,
        Unknown8                = 0x04000000,
        Backward                = 0x08000000,
        Unknown10               = 0x10000000,
        Unknown11               = 0x20000000,
        Unknown12               = 0x40000000,
        Unknown13               = 0x80000000,

        // Masks
        Mask_Final_Facing = Final_Point | Final_Target | Final_Angle,
        // animation ids stored here, see AnimationTier enum, used with Animation flag
        Mask_Animations = 0xFF,
        // flags that shouldn't be appended into SMSG_MONSTER_MOVE\SMSG_MONSTER_MOVE_TRANSPORT packet, should be more probably
        Mask_No_Monster_Move = Mask_Final_Facing | Mask_Animations | Done,
        // CatmullRom interpolation mode used
        Mask_CatmullRom = Flying | Catmullrom,
        // Unused, not suported flags
        Mask_Unused = No_Spline | Enter_Cycle | Frozen | Unknown7 | Unknown8 | Unknown10 | Unknown11 | Unknown12 | Unknown13
    };

    inline uint32_t& raw() { return (uint32_t&)*this; }
    inline uint32_t const& raw() const { return (uint32_t const&)*this; }

    MoveSplineFlag() { raw() = 0; }
    MoveSplineFlag(uint32_t f) { raw() = f; }

    // Constant interface
    bool isSmooth() const { return (raw() & Mask_CatmullRom) != 0; }
    bool isLinear() const { return !isSmooth(); }
    bool isFacing() const { return (raw() & Mask_Final_Facing) != 0; }

    uint8_t getAnimationId() const { return animTier; }
    bool hasAllFlags(uint32_t f) const { return (raw() & f) == f; }
    bool hasFlag(uint32_t f) const { return (raw() & f) != 0; }
    uint32_t operator & (uint32_t f) const { return (raw() & f); }
    uint32_t operator | (uint32_t f) const { return (raw() | f); }
    std::string ToString() const;

    // Not constant interface
    void operator &= (uint32_t f) { raw() &= f; }
    void operator |= (uint32_t f) { raw() |= f; }

    void EnableAnimation(uint8_t anim) { raw() = (raw() & ~(Mask_Animations | Falling | Parabolic)) | Animation | anim; }
    void EnableParabolic() { raw() = (raw() & ~(Mask_Animations | Falling | Animation)) | Parabolic; }
    void EnableFalling() { raw() = (raw() & ~(Mask_Animations | Parabolic | Flying | Animation)) | Falling; }
    void EnableFlying() { raw() = (raw() & ~(Falling | Catmullrom)) | Flying; }
    void EnableCatmullRom() { raw() = (raw() & ~Flying) | Catmullrom; }
    void EnableFacingPoint() { raw() = (raw() & ~Mask_Final_Facing) | Final_Point; }
    void EnableFacingAngle() { raw() = (raw() & ~Mask_Final_Facing) | Final_Angle; }
    void EnableFacingTarget() { raw() = (raw() & ~Mask_Final_Facing) | Final_Target; }
    void EnableTransportEnter() { raw() = (raw() & ~TransportExit) | TransportEnter; }
    void EnableTransportExit() { raw() = (raw() & ~TransportEnter) | TransportExit; }

    uint8_t animTier         : 8;
    bool done                : 1;
    bool falling             : 1;
    bool no_spline           : 1;
    bool parabolic           : 1;
    bool canswim             : 1;
    bool flying              : 1;
    bool orientationFixed    : 1;
    bool final_point         : 1;
    bool final_target        : 1;
    bool final_angle         : 1;
    bool catmullrom          : 1;
    bool cyclic              : 1;
    bool enter_cycle         : 1;
    bool animation           : 1;
    bool frozen              : 1;
    bool transportEnter      : 1;
    bool transportExit       : 1;
    bool unknown7            : 1;
    bool unknown8            : 1;
    bool backward            : 1;
    bool unknown10           : 1;
    bool unknown11           : 1;
    bool unknown12           : 1;
    bool unknown13           : 1;
#elif VERSION_STRING >= Cata
    enum eFlags
    {
        None                    = 0x00000000,
                                                        // x00-x07 used as animation Ids storage in pair with Animation flag
        Unknown0                = 0x00000008,           // NOT VERIFIED
        FallingSlow             = 0x00000010,
        Done                    = 0x00000020,
        Falling                 = 0x00000040,           // Affects elevation computation, can't be combined with Parabolic flag
        No_Spline               = 0x00000080,
        Unknown2                = 0x00000100,           // NOT VERIFIED
        Flying                  = 0x00000200,           // Smooth movement(Catmullrom interpolation mode), flying animation
        OrientationFixed        = 0x00000400,           // Model orientation fixed
        Catmullrom              = 0x00000800,           // Used Catmullrom interpolation mode
        Cyclic                  = 0x00001000,           // Movement by cycled spline
        Enter_Cycle             = 0x00002000,           // Everytimes appears with cyclic flag in monster move packet, erases first spline vertex after first cycle done
        Frozen                  = 0x00004000,           // Will never arrive
        TransportEnter          = 0x00008000,
        TransportExit           = 0x00010000,
        Unknown3                = 0x00020000,           // NOT VERIFIED
        Unknown4                = 0x00040000,           // NOT VERIFIED
        OrientationInversed     = 0x00080000,
        SmoothGroundPath        = 0x00100000,
        Walkmode                = 0x00200000,
        UncompressedPath        = 0x00400000,
        Unknown6                = 0x00800000,           // NOT VERIFIED
        Animation               = 0x01000000,           // Plays animation after some time passed
        Parabolic               = 0x02000000,           // Affects elevation computation, can't be combined with Falling flag
        Final_Point             = 0x04000000,
        Final_Target            = 0x08000000,
        Final_Angle             = 0x10000000,
        Unknown7                = 0x20000000,           // NOT VERIFIED
        Unknown8                = 0x40000000,           // NOT VERIFIED
        Unknown9                = 0x80000000,           // NOT VERIFIED

        // Masks
        Mask_Final_Facing = Final_Point | Final_Target | Final_Angle,
        // animation ids stored here, see AnimationTier enum, used with Animation flag
        Mask_Animations = 0x7,
        // flags that shouldn't be appended into SMSG_MONSTER_MOVE\SMSG_MONSTER_MOVE_TRANSPORT packet, should be more probably
        Mask_No_Monster_Move = Mask_Final_Facing | Mask_Animations | Done,
        // CatmullRom interpolation mode used
        Mask_CatmullRom = Flying | Catmullrom,
        // Unused, not suported flags
        Mask_Unused = No_Spline | Enter_Cycle | Frozen | Unknown0 | Unknown2 | Unknown3 | Unknown4 | Unknown6 | Unknown7 | Unknown8 | Unknown9
    };

    inline uint32_t& raw() { return (uint32_t&)*this; }
    inline uint32_t const& raw() const { return (uint32_t const&)*this; }

    MoveSplineFlag() { raw() = 0; }
    MoveSplineFlag(uint32_t f) { raw() = f; }
    MoveSplineFlag(MoveSplineFlag const& f) { raw() = f.raw(); }

    // Constant interface

    bool isSmooth() const { return (raw() & Catmullrom) != 0; }
    bool isLinear() const { return !isSmooth(); }
    bool isFacing() const { return (raw() & Mask_Final_Facing) != 0; }

    uint8_t getAnimationId() const { return animTier; }
    bool hasAllFlags(uint32_t f) const { return (raw() & f) == f; }
    bool hasFlag(uint32_t f) const { return (raw() & f) != 0; }
    uint32_t operator & (uint32_t f) const { return (raw() & f); }
    uint32_t operator | (uint32_t f) const { return (raw() | f); }
    std::string ToString() const;

    // Not constant interface

    void operator &= (uint32_t f) { raw() &= f; }
    void operator |= (uint32_t f) { raw() |= f; }

    void EnableAnimation(uint8_t anim) { raw() = (raw() & ~(Mask_Animations | Falling | Parabolic | FallingSlow)) | Animation | (anim & Mask_Animations); }
    void EnableParabolic() { raw() = (raw() & ~(Mask_Animations | Falling | Animation | FallingSlow)) | Parabolic; }
    void EnableFlying() { raw() = (raw() & ~(Falling)) | Flying; }
    void EnableFalling() { raw() = (raw() & ~(Mask_Animations | Parabolic | Animation | Flying)) | Falling; }
    void EnableCatmullRom() { raw() = (raw() & ~SmoothGroundPath) | Catmullrom; }
    void EnableFacingPoint() { raw() = (raw() & ~Mask_Final_Facing) | Final_Point; }
    void EnableFacingAngle() { raw() = (raw() & ~Mask_Final_Facing) | Final_Angle; }
    void EnableFacingTarget() { raw() = (raw() & ~Mask_Final_Facing) | Final_Target; }
    void EnableTransportEnter() { raw() = (raw() & ~TransportExit) | TransportEnter; }
    void EnableTransportExit() { raw() = (raw() & ~TransportEnter) | TransportExit; }

    uint8_t animTier         : 3;
    bool unknown0            : 1;
    bool fallingSlow         : 1;
    bool done                : 1;
    bool falling             : 1;
    bool no_spline           : 1;
    bool unknown2            : 1;
    bool flying              : 1;
    bool orientationFixed    : 1;
    bool catmullrom          : 1;
    bool cyclic              : 1;
    bool enter_cycle         : 1;
    bool frozen              : 1;
    bool transportEnter      : 1;
    bool transportExit       : 1;
    bool unknown3            : 1;
    bool unknown4            : 1;
    bool orientationInversed : 1;
    bool smoothGroundPath    : 1;
    bool walkmode            : 1;
    bool uncompressedPath    : 1;
    bool unknown6            : 1;
    bool animation           : 1;
    bool parabolic           : 1;
    bool final_point         : 1;
    bool final_target        : 1;
    bool final_angle         : 1;
    bool unknown7            : 1;
    bool unknown8            : 1;
    bool unknown9            : 1;
#endif
};
#pragma pack(pop)

} // namespace MovementMgr