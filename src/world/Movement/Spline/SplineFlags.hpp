/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "WorldConf.h"

namespace Movement { namespace Spline {
#pragma pack(push, 1)
    struct SplineFlagsData
    {
#if VERSION_STRING < Cata
        uint8_t animation_id : 8; // 8
        bool done : 1;
        bool falling : 1;
        bool no_spline : 1;
        bool trajectory : 1; // 12
        bool walkmode : 1;
        bool flying : 1;
        bool knockback : 1;             // fixed orientation
        bool finalpoint : 1; // 16
        bool finaltarget : 1;
        bool finalangle : 1;
        bool catmullrom : 1;
        bool cyclic : 1; // 20
        bool enter_cycle : 1;
        bool animation : 1;
        bool frozen : 1;
        bool enter_transport : 1; // 24
        bool exit_transport : 1;
        bool unk1 : 1;
        bool unk2 : 1;
        bool invert_orientation : 1; // 28
        bool unk3 : 1;
        bool unk4 : 1;
        bool unk5 : 1;
        bool unk6 : 1; // 32
#else
        uint8_t animation_id     : 3;
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
        bool trajectory          : 1;
        bool finalpoint          : 1;
        bool finaltarget         : 1;
        bool finalangle          : 1;
        bool unknown7            : 1;
        bool unknown8            : 1;
        bool unknown9            : 1;
#endif

        inline uint32_t& as_uint32() { return reinterpret_cast<uint32_t&>(*this); }
    };
#pragma pack(pop)

    struct SplineFlags
    {
    protected:
        void UnsetAllFacingFlags();

    public:
        SplineFlagsData m_splineFlagsRaw;
        uint32_t GetFlagsForMonsterMove();
        void SetFacingPointFlag();
        void SetFacingTargetFlag();
        void SetFacingAngleFlag();
    };
}}
