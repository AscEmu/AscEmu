/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Data/Flags.h"
#include "Units/UnitDefines.hpp"
#include "WorldPacket.h"
#include "MovementDefines.h"
#include "LocationVector.h"


#if VERSION_STRING <= WotLK
struct MovementInfo
{
    uint32_t flags;

#if VERSION_STRING < WotLK
    uint8_t flags2;
#else
    uint16_t flags2;
#endif

    uint32_t update_time;
    LocationVector position;
    uint64_t transport_guid;
    LocationVector transport_position;
    uint32_t transport_time;
#if VERSION_STRING == WotLK
    uint8_t transport_seat;
    uint32_t transport_time2;
#endif
    /*
    *  -1.55   looking down
    *  0       looking forward
    *  +1.55   looking up
    */
    float pitch_rate;
    uint32_t fall_time;
    float redirect_velocity;
    float redirect_sin;
    float redirect_cos;
    // ReSharper disable once CppInconsistentNaming
    float redirect_2d_speed;
    float spline_elevation;
    uint32_t unk_13;

    MovementInfo() :
        flags(0),
        flags2(0),
        update_time(0),
        position(0.f, 0.f, 0.f, 0.f),
        transport_guid(0),
        transport_position(0.f, 0.f, 0.f, 0.f),
        transport_time(0),
#if VERSION_STRING == WotLK
        transport_seat(0),
        transport_time2(0.f),
#endif
        pitch_rate(0.f),
        fall_time(0),
        redirect_velocity(0.f),
        redirect_sin(0.f),
        redirect_cos(0.f),
        redirect_2d_speed(0.f),
        spline_elevation(0.f),
        unk_13(0)
    {
    }

    bool hasFlag(uint32_t flag) const { return flags & flag; }
    bool isOnTransport() const { return hasFlag(MOVEFLAG_TRANSPORT); }
    bool isSwimming() const { return hasFlag(MOVEFLAG_SWIMMING); }
    bool isFlying() const { return hasFlag(MOVEFLAG_FLYING); }
    bool isSwimmingOrFlying() const { return isSwimming() || isFlying(); }
    bool isFalling() const { return hasFlag(MOVEFLAG_FALLING); }
    bool isRedirected() const { return hasFlag(MOVEFLAG_REDIRECTED); }
    bool isFallingOrRedirected() const { return isFalling() || isRedirected(); }
    bool isSplineMover() const { return hasFlag(MOVEFLAG_SPLINE_MOVER); }

    bool hasFlag2(uint32_t flag2) const { return (flags2 & flag2) != 0; }

#if VERSION_STRING == WotLK
    bool isInterpolated() const { return hasFlag2(MOVEFLAG2_INTERPOLATED_MOVE); }
#endif
};
#endif

#if VERSION_STRING >= Cata

struct MovementInfo
{
        MovementInfo() : flags(MOVEFLAG_NONE), flags2(MOVEFLAG2_NONE), update_time(0),
            transport_time(0), transport_seat(-1), transport_time2(0), pitch_rate(0.0f), fall_time(0), spline_elevation(0.0f), byte_parameter(0) {}

        ObjectGuid const& getGuid() const { return guid; }
        ObjectGuid const& getGuid2() const { return guid2; }

        MovementFlags getMovementFlags() const { return MovementFlags(flags); }
        void addMovementFlag(MovementFlags _flags) { flags |= _flags; }
        void setMovementFlags(MovementFlags _flags) { flags = _flags; }
        bool hasMovementFlag(MovementFlags _flags) const { return (flags & _flags) != 0; }
        void removeMovementFlag(MovementFlags _flags) { flags &= ~_flags; }

        MovementFlags2 getMovementFlags2() const { return MovementFlags2(flags2); }
        void addMovementFlags2(MovementFlags2 _flags2) { flags2 |= _flags2; }
        bool hasMovementFlag2(MovementFlags2 _flags2) const { return (flags2 & _flags2) != 0; }

        void setUpdateTime(uint32_t time) { update_time = time; }
        uint32_t getUpdateTime() { return update_time; }

        LocationVector const* getPosition() const { return &position; }
        void changeOrientation(float o) { position.o = o; }
        void changePosition(float x, float y, float z, float o) { position.x = x; position.y = y; position.z = z; position.o = o; }

        float getPitch() const { return pitch_rate; }
        uint32_t fetFallTime() const { return fall_time; }
        float getSplineElevation() const { return spline_elevation; }
        int8_t fetByteParam() const { return byte_parameter; }

        struct JumpInfo
        {
            JumpInfo() : velocity(0.f), sinAngle(0.f), cosAngle(0.f), xyspeed(0.f) { }

            float velocity;
            float sinAngle;
            float cosAngle;
            float xyspeed;
        };
        JumpInfo const& getJumpInfo() const { return jump_info; }

        struct StatusInfo
        {
            StatusInfo() : hasFallData(false), hasFallDirection(false), hasOrientation(false),
                hasPitch(false), hasSpline(false), hasSplineElevation(false),
                hasTimeStamp(false), hasTransportTime2(false), hasTransportTime3(false) { }

            bool hasFallData : 1;
            bool hasFallDirection : 1;
            bool hasOrientation : 1;
            bool hasPitch : 1;
            bool hasSpline : 1;
            bool hasSplineElevation : 1;
            bool hasTimeStamp : 1;
            bool hasTransportTime2 : 1;
            bool hasTransportTime3 : 1;
        };
        StatusInfo const& getMovementStatusInfo() const { return status_info; }

        // transport
        ObjectGuid& getTransportGuid() { return transport_guid; }

        LocationVector const* getTransportPosition() const { return &transport_position; }

        int8_t getTransportSeat() const { return transport_seat; }

        uint32_t getTransportTime() const { return transport_time; }
        uint32_t getTransportTime2() const { return transport_time2; }

        void setTransportData(ObjectGuid _guid, float x, float y, float z, float o, uint32_t time, int8_t seat)
        {
            transport_guid = _guid;
            transport_position.x = x;
            transport_position.y = y;
            transport_position.z = z;
            transport_position.o = o;
            transport_time = time;
            transport_seat = seat;
        }

        void clearTransportData()
        {
            transport_guid = 0;
            transport_position.x = 0.0f;
            transport_position.y = 0.0f;
            transport_position.z = 0.0f;
            transport_position.o = 0.0f;
            transport_time = 0;
            transport_seat = -1;
        }

        void readMovementInfo(ByteBuffer& data, uint16_t opcode);
        void writeMovementInfo(ByteBuffer& data, uint16_t opcode, float custom_speed = 0.f) const;

        uint32_t flags;
        uint16_t flags2;

        ObjectGuid guid;
        ObjectGuid guid2;

        uint32_t update_time;

        LocationVector position;

        float pitch_rate;
        uint32_t fall_time;
        float spline_elevation;
        int8_t byte_parameter;

        JumpInfo jump_info;

        StatusInfo status_info;

        // transport
        ObjectGuid transport_guid;

        LocationVector transport_position;
        uint32_t transport_time;
        int8_t transport_seat;
        uint32_t transport_time2;
};

inline WorldPacket& operator<< (WorldPacket& buf, MovementInfo const& mi)
{
    mi.writeMovementInfo(buf, buf.GetOpcode());
    return buf;
}

inline WorldPacket& operator>> (WorldPacket& buf, MovementInfo& mi)
{
    mi.readMovementInfo(buf, buf.GetOpcode());
    return buf;
}

inline float normalizeOrientation(float orientation)
{
    if (orientation < 0)
    {
        float mod = orientation *-1;
        mod = fmod(mod, 2.0f * static_cast<float>(M_PI));
        mod = -mod + 2.0f * static_cast<float>(M_PI);
        return mod;
    }

    return fmod(orientation, 2.0f * static_cast<float>(M_PI));
}
#endif
