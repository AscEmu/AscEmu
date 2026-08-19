/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/WorldPacket.hpp"
#include "MovementDefines.hpp"
#include "Utilities/LocationVector.hpp"
#include "Map/Cells/CellHandlerDefines.hpp"

struct MovementInfo
{
    WoWGuid guid = 0;

    //note: not present in versions before MoP
    WoWGuid guid2 = 0;

    uint32_t flags = 0;
    //note: uint8_t for tbc
    uint16_t flags2 = 0;

    uint32_t update_time = 0;

    LocationVector position { 0.f, 0.f, 0.f, 0.f };

    // -1.55 looking down | 0 looking forward | +1.55 looking up
    float pitch_rate = 0.f;

    //todo: move this to jump
    uint32_t fall_time = 0;

    float spline_elevation = 0.f;

    struct JumpInfo
    {
        float velocity = 0.f;
        float sinAngle = 0.f;
        float cosAngle = 0.f;
        float xyspeed = 0.f;
    };

    JumpInfo jump_info;
    JumpInfo const& getJumpInfo() const { return jump_info; }

    //todo: build new struct for transport
    WoWGuid transport_guid = 0;
    LocationVector transport_position {0.f, 0.f, 0.f, 0.f};
    uint32_t transport_time = 0;
    uint8_t transport_seat = static_cast<uint8_t>(-1);
    uint32_t transport_time2 = 0;
    uint32_t transport_time3 = 0;   //vehicle_id

    //misc stuff
    uint32_t counter = 0;
    uint32_t forcesCount = 0;

    //note: only used by SMSG_MOVE_UPDATE_COLLISION_HEIGHT (Cata/Mop)
    float collisionHeight = 0.f;

    struct StatusInfo
    {
        bool hasFallData = false;
        bool hasFallDirection = false;
        bool hasOrientation = false;
        bool hasPitch = false;
        bool hasSpline = false;
        bool hasSplineElevation = false;
        bool hasTimeStamp = false;
        bool hasTransportTime2 = false;
        bool hasTransportTime3 = false; //hasVehicleId
    };

    //todo: move this to status
    bool hasTransportData = false;
    bool hasMovementFlags = false;
    bool hasMovementFlags2 = false;
    bool hasCount = false;

    //todo: investigate further on forced_speed
    struct ForcedSpeed
    {
        float walk = 0.f;
        float run = 0.f;
        float runBack = 0.f;
        float swim = 0.f;
        float swimBack = 0.f;
        float turn = 0.f;
        float fly = 0.f;
        float flyBack = 0.f;
        float pitch = 0.f;
    };

    float newSpeed = 0.f;
    ForcedSpeed forcedSpeed;

    StatusInfo status_info;
    StatusInfo const& getMovementStatusInfo() const { return status_info; }

    WoWGuid const& getGuid() const { return guid; }
    WoWGuid const& getGuid2() const { return guid2; }

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
    uint32_t getFallTime() const { return fall_time; }
    float getSplineElevation() const { return spline_elevation; }

    void setFallTime(uint32_t val) { fall_time = val; }

    void setTransportData(WoWGuid _guid, float x, float y, float z, float o, uint32_t time, [[maybe_unused]]int8_t seat)
    {
        transport_guid = _guid;
        transport_position.x = x;
        transport_position.y = y;
        transport_position.z = z;
        transport_position.o = o;
        transport_time = time;
        transport_seat = static_cast<uint8_t>(seat);
    }

    void clearTransportData()
    {
        transport_guid = 0;
        transport_position.x = 0.0f;
        transport_position.y = 0.0f;
        transport_position.z = 0.0f;
        transport_position.o = 0.0f;
        transport_time = 0;
        transport_seat = static_cast<uint8_t>(-1);
    }

    void readMovementInfo(ByteBuffer& data, uint16_t opcode);
    void read(WorldPacket& packet);

    void writeMovementInfo(ByteBuffer& data, uint16_t opcode, bool withGuid = true) const;
    void write(WorldPacket& packet, bool withGuid = true) const;
};

inline WorldPacket& operator<< (WorldPacket& buf, MovementInfo const& mi)
{
    mi.writeMovementInfo(buf, buf.getOpcode());
    return buf;
}

inline WorldPacket& operator>> (WorldPacket& buf, MovementInfo& mi)
{
    mi.readMovementInfo(buf, buf.getOpcode());
    return buf;
}

inline void normalizeMapCoord(float &c)
{
    if (c > Map::Terrain::MapHalfSize - 0.5f)
        c = Map::Terrain::MapHalfSize - 0.5f;
    else if (c < -(Map::Terrain::MapHalfSize - 0.5f))
        c = -(Map::Terrain::MapHalfSize - 0.5f);
}

inline bool isValidMapCoord(float c)
{
    return std::isfinite(c) && (std::fabs(c) <= Map::Terrain::MapHalfSize - 0.5f);
}

inline bool isValidMapCoord(float x, float y)
{
    return isValidMapCoord(x) && isValidMapCoord(y);
}

inline bool isValidMapCoord(float x, float y, float z)
{
    return isValidMapCoord(x, y) && isValidMapCoord(z);
}

inline bool isValidMapCoord(float x, float y, float z, float o)
{
    return isValidMapCoord(x, y, z) && std::isfinite(o);
}
