/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef MOVEPLINE_H
#define MOVEPLINE_H

#include "Spline.h"
#include "MoveSplineInitArgs.h"
#include <G3D/Vector3.h>

enum class AnimationTier : uint8;

namespace MovementNew
{
    struct Location : public Vector3
    {
        Location() : orientation(0) { }
        Location(float x, float y, float z, float o) : Vector3(x, y, z), orientation(o) { }
        Location(Vector3 const& v) : Vector3(v), orientation(0) { }
        Location(Vector3 const& v, float o) : Vector3(v), orientation(o) { }

        float orientation;
    };

    // MoveSpline represents smooth catmullrom or linear curve and point that moves belong it
    // curve can be cyclic - in this case movement will be cyclic
    // point can have vertical acceleration motion componemt(used in fall, parabolic movement)
    class SERVER_DECL MoveSpline
    {
    public:
        typedef Spline<int32> MySpline;
        enum UpdateResult
        {
            Result_None         = 0x01,
            Result_Arrived      = 0x02,
            Result_NextCycle    = 0x04,
            Result_NextSegment  = 0x08
        };
        friend class PacketBuilder;

    protected:
        MySpline        spline;

        FacingInfo      facing;

        uint32          m_Id;

        MoveSplineFlag  splineflags;

        int32           time_passed;
        // currently duration mods are unused, but its _currently_
        //float           duration_mod;
        //float           duration_mod_next;
        float           vertical_acceleration;
        float           initialOrientation;
        int32           effect_start_time;
        int32           point_Idx;
        int32           point_Idx_offset;
        float           velocity;

        void init_spline(MoveSplineInitArgs const& args);

    protected:
        MySpline::ControlArray const& getPath() const { return spline.getPoints(); }
        void computeParabolicElevation(float& el) const;
        void computeFallElevation(float& el) const;

        UpdateResult _updateState(int32& ms_time_diff);
        int32 next_timestamp() const { return spline.length(point_Idx + 1); }
        int32 segment_time_elapsed() const { return next_timestamp() - time_passed; }
        int32 timeElapsed() const { return Duration() - time_passed; }
        int32 timePassed() const { return time_passed; }

    public:
        int32 Duration() const { return spline.length(); }
        MySpline const& _Spline() const { return spline; }
        int32 _currentSplineIdx() const { return point_Idx; }
        float Velocity() const { return velocity; }
        void _Finalize();
        void _Interrupt() { splineflags.done = true; }

    public:
        void Initialize(MoveSplineInitArgs const&);
        bool Initialized() const { return !spline.empty(); }

        MoveSpline();

        template<class UpdateHandler>
        void updateState(int32 difftime, UpdateHandler& handler)
        {
            ASSERT(Initialized());
            do
                handler(_updateState(difftime));
            while (difftime > 0);
        }

        void updateState(int32 difftime)
        {
            ASSERT(Initialized());
            do _updateState(difftime);
            while (difftime > 0);
        }

        Location ComputePosition() const;

        uint32 GetId() const { return m_Id; }
        bool Finalized() const { return splineflags.done; }
        bool isCyclic() const { return splineflags.cyclic; }
        bool isFalling() const { return splineflags.falling; }
        Vector3 FinalDestination() const { return Initialized() ? spline.getPoint(spline.last()) : Vector3(); }
        Vector3 CurrentDestination() const { return Initialized() ? spline.getPoint(point_Idx + 1) : Vector3(); }
        int32 currentPathIdx() const;

        bool HasAnimation() const { return splineflags.animation; }
        AnimationTier GetAnimationTier() const { return static_cast<AnimationTier>(splineflags.animTier); }

        bool onTransport;
        std::string ToString() const;
        bool HasStarted() const
        {
            return time_passed > 0;
        }
    };
}
#endif // MOVEPLINE_H
