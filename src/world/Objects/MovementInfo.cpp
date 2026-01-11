/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "MovementInfo.hpp"
#include "Server/OpcodeTable.hpp"

void MovementInfo::readMovementInfo(ByteBuffer& data, uint16_t opcode)
{
#if VERSION_STRING == Classic

    data >> flags >> update_time >> position >> position.o;

    if (hasMovementFlag(MOVEFLAG_TRANSPORT))
        data >> transport_guid >> transport_position >> transport_position.o;

    if (hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)))
        data >> pitch_rate;

    data >> fall_time;

    if (hasMovementFlag(MOVEFLAG_FALLING))
        data >> jump_info.velocity >> jump_info.sinAngle >> jump_info.cosAngle >> jump_info.xyspeed;

    if (hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
        data >> spline_elevation;

#elif VERSION_STRING == TBC

    data >> flags >> flags2 >> update_time >> position >> position.o;

    if (hasMovementFlag(MOVEFLAG_TRANSPORT))
        data >> transport_guid >> transport_position >> transport_position.o >> transport_time;

    if (hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || hasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
        data >> pitch_rate;

    data >> fall_time;

    if (hasMovementFlag(MOVEFLAG_FALLING))
        data >> jump_info.velocity >> jump_info.sinAngle >> jump_info.cosAngle >> jump_info.xyspeed;

    if (hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
        data >> spline_elevation;

#elif VERSION_STRING == WotLK

    data >> guid >> flags >> flags2 >> update_time >> position >> position.o;

    sLogger.debug("guid: {}, flags: {}, flags2: {}, updatetime: {}, position: ({}, {}, {}, {})",
        guid.getGuidLow(), flags, flags2, update_time, position.x, position.y, position.z, position.o);

    if (hasMovementFlag(MOVEFLAG_TRANSPORT))
    {
        WoWGuid tguid;
        data >> tguid >> transport_position >> transport_position.o >> transport_time >> transport_seat;

        transport_guid = tguid.getGuidLow();

        if (hasMovementFlag2(MOVEFLAG2_INTERPOLATED_MOVE))
            data >> transport_time2;

        sLogger.debug("tguid: {}, tposition: ({}, {}, {}, {})", transport_guid.getRawGuid(), transport_position.x, transport_position.y, transport_position.z, transport_position.o);
    }

    if (hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || hasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
        data >> pitch_rate;

    data >> fall_time;

    if (hasMovementFlag(MOVEFLAG_FALLING))
        data >> jump_info.velocity >> jump_info.sinAngle >> jump_info.cosAngle >> jump_info.xyspeed;

    if (hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
        data >> spline_elevation;

#else // >= Cata
    bool hasTransportData = false,
        hasMovementFlags = false,
        hasMovementFlags2 = false;

    switch (sOpcodeTables.getInternalIdForHex(opcode))
    {
#if VERSION_STRING == Cata
        case CMSG_CAST_SPELL:
        case CMSG_PET_CAST_SPELL:
        case CMSG_USE_ITEM:
        {
            data >> position.z;
            data >> position.y;
            data >> position.x;
            status_info.hasFallData = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            status_info.hasOrientation = !data.readBit();
            status_info.hasSpline = data.readBit();
            data.readBit();
            guid[6] = data.readBit();
            guid[4] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[3] = data.readBit();
            guid[5] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            status_info.hasPitch = !data.readBit();
            guid[7] = data.readBit();
            hasTransportData = data.readBit();
            guid[2] = data.readBit();
            hasMovementFlags = !data.readBit();
            guid[1] = data.readBit();
            guid[0] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[6]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasPitch) data >> pitch_rate;

        } break;
        case CMSG_MOVE_CHNG_TRANSPORT:
        {
            data >> position.y;
            data >> position.x;
            data >> position.z;
            guid[4] = data.readBit();
            guid[0] = data.readBit();
            guid[2] = data.readBit();
            hasTransportData = data.readBit();
            status_info.hasSpline = data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[6] = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[5] = data.readBit();
            guid[7] = data.readBit();
            status_info.hasPitch = !data.readBit();
            data.readBit();
            guid[3] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[1] = data.readBit();
            status_info.hasFallData = data.readBit();
            hasMovementFlags = !data.readBit();
            hasMovementFlags2 = !data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[3]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasTimeStamp)  data >> update_time;

        } break;
        case MSG_MOVE_FALL_LAND:
        {
            data >> position.x;
            data >> position.y;
            data >> position.z;
            hasTransportData = data.readBit();
            guid[7] = data.readBit();
            guid[1] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[3] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[6] = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            data.readBit();
            status_info.hasPitch = !data.readBit();
            guid[4] = data.readBit();
            status_info.hasSpline = data.readBit();
            guid[5] = data.readBit();
            hasMovementFlags = !data.readBit();
            status_info.hasFallData = data.readBit();
            guid[0] = data.readBit();
            guid[2] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[5]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasOrientation) data >> position.o;

        } break;
        case CMSG_MOVE_FALL_RESET:
        {
            data >> position.z;
            data >> position.x;
            data >> position.y;
            guid[1] = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            hasMovementFlags = !data.readBit();
            data.readBit();
            guid[6] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            status_info.hasPitch = !data.readBit();
            guid[3] = data.readBit();
            hasTransportData = data.readBit();
            guid[2] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[4] = data.readBit();
            guid[5] = data.readBit();
            status_info.hasSpline = data.readBit();
            guid[7] = data.readBit();
            guid[0] = data.readBit();
            status_info.hasFallData = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[6]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasPitch) data >> pitch_rate;

        } break;
        case MSG_MOVE_JUMP:
        {
            data >> position.y;
            data >> position.x;
            data >> position.z;
            status_info.hasTimeStamp = !data.readBit();
            guid[5] = data.readBit();
            hasMovementFlags = !data.readBit();
            guid[4] = data.readBit();
            guid[6] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[0] = data.readBit();
            hasTransportData = data.readBit();
            guid[3] = data.readBit();
            status_info.hasPitch = !data.readBit();
            guid[7] = data.readBit();
            status_info.hasFallData = data.readBit();
            status_info.hasSpline = data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[1] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            data.readBit();
            guid[2] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasOrientation) data >> position.o;

        } break;
        case CMSG_MOVE_SET_CAN_FLY_ACK:
        {
            data >> position.y;
            data.read_skip<uint32_t>(); //counter
            data >> position.x;
            data >> position.z;
            guid[3] = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[4] = data.readBit();
            guid[0] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            status_info.hasFallData = data.readBit();
            guid[2] = data.readBit();
            guid[5] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            hasMovementFlags2 = !data.readBit();
            data.readBit();
            guid[7] = data.readBit();
            status_info.hasSpline = data.readBit();
            guid[6] = data.readBit();
            guid[1] = data.readBit();
            hasMovementFlags = !data.readBit();
            hasTransportData = data.readBit();
            status_info.hasPitch = !data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[5]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasSplineElevation) data >> spline_elevation;

        } break;
        case MSG_MOVE_SET_FACING:
        {
            data >> position.x;
            data >> position.y;
            data >> position.z;
            guid[6] = data.readBit();
            hasTransportData = data.readBit();
            guid[4] = data.readBit();
            status_info.hasSpline = data.readBit();
            guid[0] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            status_info.hasPitch = !data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[5] = data.readBit();
            guid[7] = data.readBit();
            guid[2] = data.readBit();
            status_info.hasFallData = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            hasMovementFlags = !data.readBit();
            guid[3] = data.readBit();
            data.readBit();
            guid[1] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasPitch) data >> pitch_rate;

        } break;
        case MSG_MOVE_SET_PITCH:
        {
            data >> position.x;
            data >> position.z;
            data >> position.y;
            status_info.hasFallData = data.readBit();
            hasMovementFlags = !data.readBit();
            guid[1] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[7] = data.readBit();
            guid[3] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            hasTransportData = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[6] = data.readBit();
            status_info.hasPitch = !data.readBit();
            guid[4] = data.readBit();
            status_info.hasSpline = data.readBit();
            guid[2] = data.readBit();
            data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[0] = data.readBit();
            guid[5] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[4]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_position.x;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasOrientation) data >> position.o;

        } break;
        case MSG_MOVE_SET_RUN_MODE:
        {
            data >> position.y;
            data >> position.x;
            data >> position.z;
            status_info.hasTimeStamp = !data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[1] = data.readBit();
            status_info.hasSpline = data.readBit();
            hasMovementFlags = !data.readBit();
            guid[7] = data.readBit();
            hasTransportData = data.readBit();
            data.readBit();
            guid[0] = data.readBit();
            guid[3] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[5] = data.readBit();
            status_info.hasPitch = !data.readBit();
            guid[6] = data.readBit();
            guid[4] = data.readBit();
            status_info.hasFallData = data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[2] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[2]);
            if (status_info.hasPitch) data >> pitch_rate;
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasOrientation) data >> position.o;

        } break;
        case MSG_MOVE_SET_WALK_MODE:
        {
            data >> position.y;
            data >> position.x;
            data >> position.z;
            guid[6] = data.readBit();
            status_info.hasSpline = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[0] = data.readBit();
            guid[1] = data.readBit();
            hasMovementFlags = !data.readBit();
            status_info.hasPitch = !data.readBit();
            guid[7] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[4] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            hasTransportData = data.readBit();
            guid[2] = data.readBit();
            guid[5] = data.readBit();
            guid[3] = data.readBit();
            data.readBit();
            hasMovementFlags2 = !data.readBit();
            status_info.hasFallData = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasOrientation) data >> position.o;

        } break;
        case CMSG_MOVE_SPLINE_DONE:
        {
            data.read_skip<uint32_t>(); //counter
            data >> position.y;
            data >> position.x;
            data >> position.z;
            guid[6] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            status_info.hasFallData = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[2] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[4] = data.readBit();
            hasTransportData = data.readBit();
            guid[3] = data.readBit();
            hasMovementFlags = !data.readBit();
            guid[0] = data.readBit();
            data.readBit();
            guid[1] = data.readBit();
            guid[5] = data.readBit();
            status_info.hasPitch = !data.readBit();
            status_info.hasSpline = data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[7] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[3]);
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasOrientation) data >> position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasSplineElevation) data >> spline_elevation;

        } break;
        case MSG_MOVE_START_BACKWARD:
        {
            data >> position.x;
            data >> position.z;
            data >> position.y;
            hasTransportData = data.readBit();
            guid[3] = data.readBit();
            guid[0] = data.readBit();
            guid[2] = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[7] = data.readBit();
            status_info.hasPitch = !data.readBit();
            data.readBit();
            hasMovementFlags = !data.readBit();
            status_info.hasOrientation = !data.readBit();
            status_info.hasSpline = data.readBit();
            hasMovementFlags2 = !data.readBit();
            status_info.hasFallData = data.readBit();
            guid[5] = data.readBit();
            guid[1] = data.readBit();
            guid[4] = data.readBit();
            guid[6] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[3]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasSplineElevation) data >> spline_elevation;

        } break;
        case MSG_MOVE_START_FORWARD:
        {
            data >> position.y;
            data >> position.z;
            data >> position.x;
            guid[5] = data.readBit();
            guid[2] = data.readBit();
            guid[0] = data.readBit();
            data.readBit();
            hasMovementFlags = !data.readBit();
            guid[7] = data.readBit();
            guid[3] = data.readBit();
            guid[1] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[6] = data.readBit();
            status_info.hasSpline = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[4] = data.readBit();
            hasTransportData = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            status_info.hasPitch = !data.readBit();
            hasMovementFlags2 = !data.readBit();
            status_info.hasFallData = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[0]);
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasTimeStamp)  data >> update_time;

        } break;
        case MSG_MOVE_START_STRAFE_LEFT:
        {
            data >> position.z;
            data >> position.x;
            data >> position.y;
            status_info.hasSplineElevation = !data.readBit();
            guid[5] = data.readBit();
            status_info.hasPitch = !data.readBit();
            guid[6] = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[1] = data.readBit();
            data.readBit();
            guid[4] = data.readBit();
            guid[0] = data.readBit();
            guid[2] = data.readBit();
            status_info.hasFallData = data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[3] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[7] = data.readBit();
            status_info.hasSpline = data.readBit();
            hasMovementFlags = !data.readBit();
            hasTransportData = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[5]);
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_position.x;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasSplineElevation) data >> spline_elevation;

        } break;
        case MSG_MOVE_START_STRAFE_RIGHT:
        {
            data >> position.y;
            data >> position.x;
            data >> position.z;
            status_info.hasPitch = !data.readBit();
            guid[1] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[4] = data.readBit();
            status_info.hasSpline = data.readBit();
            data.readBit();
            guid[5] = data.readBit();
            status_info.hasFallData = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            hasMovementFlags = !data.readBit();
            guid[2] = data.readBit();
            guid[7] = data.readBit();
            guid[6] = data.readBit();
            guid[3] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            hasTransportData = data.readBit();
            guid[0] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasSplineElevation) data >> spline_elevation;

        } break;
        case MSG_MOVE_START_TURN_LEFT:
        {
            data >> position.y;
            data >> position.x;
            data >> position.z;
            data.readBit();
            guid[1] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            status_info.hasSpline = data.readBit();
            hasMovementFlags = !data.readBit();
            guid[4] = data.readBit();
            guid[2] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[5] = data.readBit();
            guid[7] = data.readBit();
            hasTransportData = data.readBit();
            guid[6] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[0] = data.readBit();
            guid[3] = data.readBit();
            status_info.hasPitch = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            status_info.hasFallData = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[1]);
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasSplineElevation) data >> spline_elevation;

        } break;
        case MSG_MOVE_START_TURN_RIGHT:
        {
            data >> position.x;
            data >> position.z;
            data >> position.y;
            guid[3] = data.readBit();
            guid[5] = data.readBit();
            hasMovementFlags = !data.readBit();
            status_info.hasSpline = data.readBit();
            guid[0] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            hasTransportData = data.readBit();
            guid[7] = data.readBit();
            data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[1] = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[6] = data.readBit();
            guid[2] = data.readBit();
            guid[4] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            status_info.hasPitch = !data.readBit();
            status_info.hasFallData = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[6]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasTimeStamp)  data >> update_time;

        } break;
        case MSG_MOVE_STOP:
        {
            data >> position.x;
            data >> position.y;
            data >> position.z;
            guid[3] = data.readBit();
            guid[6] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            status_info.hasSpline = data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[7] = data.readBit();
            hasMovementFlags = !data.readBit();
            guid[5] = data.readBit();
            status_info.hasFallData = data.readBit();
            hasMovementFlags2 = !data.readBit();
            hasTransportData = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[4] = data.readBit();
            guid[1] = data.readBit();
            data.readBit();
            guid[2] = data.readBit();
            guid[0] = data.readBit();
            status_info.hasPitch = !data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData) data >> fall_time;

        } break;
        case MSG_MOVE_STOP_STRAFE:
        {
            data >> position.y;
            data >> position.z;
            data >> position.x;
            status_info.hasPitch = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[2] = data.readBit();
            status_info.hasFallData = data.readBit();
            guid[7] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[3] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            hasMovementFlags2 = !data.readBit();
            hasTransportData = data.readBit();
            hasMovementFlags = !data.readBit();
            status_info.hasSpline = data.readBit();
            guid[0] = data.readBit();
            data.readBit();
            guid[6] = data.readBit();
            guid[5] = data.readBit();
            guid[1] = data.readBit();
            guid[4] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[0]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasTimeStamp)  data >> update_time;

        } break;
        case MSG_MOVE_STOP_TURN:
        {
            data >> position.x;
            data >> position.z;
            data >> position.y;
            guid[5] = data.readBit();
            guid[4] = data.readBit();
            status_info.hasFallData = data.readBit();
            data.readBit();
            guid[1] = data.readBit();
            guid[0] = data.readBit();
            status_info.hasSpline = data.readBit();
            hasMovementFlags = !data.readBit();
            guid[2] = data.readBit();
            guid[6] = data.readBit();
            status_info.hasPitch = !data.readBit();
            hasTransportData = data.readBit();
            guid[3] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            hasMovementFlags2 = !data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[7] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[5]);
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasOrientation) data >> position.o;

        } break;
        case MSG_MOVE_START_ASCEND:
        {
            data >> position.x;
            data >> position.y;
            data >> position.z;
            guid[0] = data.readBit();
            guid[1] = data.readBit();
            guid[7] = data.readBit();
            data.readBit();
            guid[5] = data.readBit();
            hasTransportData = data.readBit();
            guid[2] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            status_info.hasFallData = data.readBit();
            status_info.hasSpline = data.readBit();
            guid[3] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[6] = data.readBit();
            hasMovementFlags = !data.readBit();
            status_info.hasPitch = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[4] = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data >> transport_position.x;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasSplineElevation) data >> spline_elevation;

        } break;
        case MSG_MOVE_START_DESCEND:
        {
            data >> position.y;
            data >> position.z;
            data >> position.x;
            guid[0] = data.readBit();
            status_info.hasPitch = !data.readBit();
            status_info.hasFallData = data.readBit();
            guid[4] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            hasMovementFlags2 = !data.readBit();
            hasMovementFlags = !data.readBit();
            guid[6] = data.readBit();
            data.readBit();
            guid[1] = data.readBit();
            hasTransportData = data.readBit();
            status_info.hasSpline = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[5] = data.readBit();
            guid[3] = data.readBit();
            guid[7] = data.readBit();
            guid[2] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[3]);
            if (status_info.hasPitch) data >> pitch_rate;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasSplineElevation) data >> spline_elevation;

        } break;
        case MSG_MOVE_START_SWIM:
        {
            data >> position.z;
            data >> position.x;
            data >> position.y;
            guid[3] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[4] = data.readBit();
            guid[7] = data.readBit();
            data.readBit();
            status_info.hasPitch = !data.readBit();
            guid[0] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            hasMovementFlags = !data.readBit();
            status_info.hasFallData = data.readBit();
            guid[5] = data.readBit();
            hasTransportData = data.readBit();
            hasMovementFlags2 = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[6] = data.readBit();
            guid[1] = data.readBit();
            status_info.hasSpline = data.readBit();
            guid[2] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[7]);
            if (status_info.hasPitch) data >> pitch_rate;
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_seat;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasSplineElevation) data >> spline_elevation;

        } break;
        case MSG_MOVE_STOP_SWIM:
        {
            data >> position.x;
            data >> position.y;
            data >> position.z;
            status_info.hasSpline = data.readBit();
            status_info.hasPitch = !data.readBit();
            guid[5] = data.readBit();
            guid[3] = data.readBit();
            guid[7] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            hasMovementFlags = !data.readBit();
            guid[4] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[2] = data.readBit();
            guid[6] = data.readBit();
            hasTransportData = data.readBit();
            status_info.hasOrientation = !data.readBit();
            data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[1] = data.readBit();
            guid[0] = data.readBit();
            status_info.hasFallData = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[2]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_position.o;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasSplineElevation) data >> spline_elevation;

        } break;
        case MSG_MOVE_STOP_ASCEND:
        {
            data >> position.z;
            data >> position.y;
            data >> position.x;
            guid[3] = data.readBit();
            guid[2] = data.readBit();
            status_info.hasFallData = data.readBit();
            guid[7] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            status_info.hasPitch = !data.readBit();
            status_info.hasSpline = data.readBit();
            data.readBit();
            guid[1] = data.readBit();
            guid[4] = data.readBit();
            hasMovementFlags = !data.readBit();
            guid[0] = data.readBit();
            guid[6] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            hasTransportData = data.readBit();
            hasMovementFlags2 = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[5] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data >> transport_position.z;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasOrientation) data >> position.o;

        } break;
        case MSG_MOVE_START_PITCH_DOWN:
        {
            data >> position.x;
            data >> position.z;
            data >> position.y;
            hasMovementFlags = !data.readBit();
            guid[7] = data.readBit();
            guid[6] = data.readBit();
            status_info.hasPitch = !data.readBit();
            data.readBit();
            guid[1] = data.readBit();
            guid[4] = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            hasMovementFlags2 = !data.readBit();
            hasTransportData = data.readBit();
            guid[0] = data.readBit();
            guid[5] = data.readBit();
            guid[3] = data.readBit();
            status_info.hasSpline = data.readBit();
            status_info.hasOrientation = !data.readBit();
            status_info.hasFallData = data.readBit();
            guid[2] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[1]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_position.o;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasTimeStamp)  data >> update_time;

        } break;
        case MSG_MOVE_START_PITCH_UP:
        {
            data >> position.z;
            data >> position.y;
            data >> position.x;
            guid[4] = data.readBit();
            hasMovementFlags = !data.readBit();
            hasMovementFlags2 = !data.readBit();
            status_info.hasSpline = data.readBit();
            guid[2] = data.readBit();
            guid[6] = data.readBit();
            guid[3] = data.readBit();
            data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            status_info.hasFallData = data.readBit();
            guid[0] = data.readBit();
            hasTransportData = data.readBit();
            guid[1] = data.readBit();
            guid[5] = data.readBit();
            status_info.hasPitch = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[7] = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[2]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasTimeStamp)  data >> update_time;

        } break;
        case MSG_MOVE_STOP_PITCH:
        {
            data >> position.x;
            data >> position.z;
            data >> position.y;
            guid[0] = data.readBit();
            guid[5] = data.readBit();
            guid[3] = data.readBit();
            status_info.hasFallData = data.readBit();
            guid[2] = data.readBit();
            guid[4] = data.readBit();
            guid[7] = data.readBit();
            status_info.hasPitch = !data.readBit();
            status_info.hasSpline = data.readBit();
            status_info.hasOrientation = !data.readBit();
            data.readBit();
            hasMovementFlags2 = !data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[6] = data.readBit();
            guid[1] = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            hasTransportData = data.readBit();
            hasMovementFlags = !data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (hasMovementFlags) flags = data.readBits(30);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[2]);
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasPitch) data >> pitch_rate;

        } break;
#endif
        case MSG_MOVE_HEARTBEAT:
        {
#if VERSION_STRING == Cata
            data >> position.z;
            data >> position.x;
            data >> position.y;
            status_info.hasPitch = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            status_info.hasFallData = data.readBit();
            hasMovementFlags2 = !data.readBit();
            hasTransportData = data.readBit();

            guid[7] = data.readBit();
            guid[1] = data.readBit();
            guid[0] = data.readBit();
            guid[4] = data.readBit();
            guid[2] = data.readBit();

            status_info.hasOrientation = !data.readBit();

            guid[5] = data.readBit();
            guid[3] = data.readBit();

            status_info.hasSplineElevation = !data.readBit();
            status_info.hasSpline = data.readBit();
            data.readBit();

            guid[6] = data.readBit();

            hasMovementFlags = !data.readBit();

            if (hasTransportData)
            {
                status_info.hasTransportTime3 = data.readBit();
                transport_guid[4] = data.readBit();
                transport_guid[2] = data.readBit();
                status_info.hasTransportTime2 = data.readBit();
                transport_guid[5] = data.readBit();
                transport_guid[7] = data.readBit();
                transport_guid[6] = data.readBit();
                transport_guid[0] = data.readBit();
                transport_guid[3] = data.readBit();
                transport_guid[1] = data.readBit();
            }

            if (status_info.hasFallData)
                status_info.hasFallDirection = data.readBit();

            if (hasMovementFlags)
                flags = data.readBits(30);

            if (hasMovementFlags2)
                flags2 = static_cast<uint16_t>(data.readBits(12));

            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[4]);

            if (hasTransportData)
            {
                data >> transport_position.z;
                data >> transport_seat;
                data >> transport_position.o;
                data.ReadByteSeq(transport_guid[4]);
                data >> transport_position.y;
                data >> transport_time;
                data >> transport_position.x;
                data.ReadByteSeq(transport_guid[5]);
                data.ReadByteSeq(transport_guid[1]);
                data.ReadByteSeq(transport_guid[3]);
                data.ReadByteSeq(transport_guid[7]);

                if (status_info.hasTransportTime3)
                    data >> transport_time3;

                if (status_info.hasTransportTime2)
                    data >> transport_time2;

                data.ReadByteSeq(transport_guid[2]);
                data.ReadByteSeq(transport_guid[0]);
                data.ReadByteSeq(transport_guid[6]);
            }

            if (status_info.hasOrientation)
                data >> position.o;

            if (status_info.hasFallData)
            {
                data >> jump_info.velocity;
                data >> fall_time;

                if (status_info.hasFallDirection)
                {
                    data >> jump_info.xyspeed;
                    data >> jump_info.sinAngle;
                    data >> jump_info.cosAngle;
                }
            }

            if (status_info.hasPitch)
                data >> pitch_rate;

            if (status_info.hasSplineElevation)
                data >> spline_elevation;

            if (status_info.hasTimeStamp)
                data >> update_time;
#else // Mop
            uint32_t forcesCount;

            data >> position.z;
            data >> position.x;
            data >> position.y;
            forcesCount = data.readBits(22);
            hasMovementFlags = !data.readBit();
            data.readBit();
            data.readBit();
            guid[3] = data.readBit();
            guid[6] = data.readBit();
            status_info.hasPitch = !data.readBit();
            data.readBit();
            data.readBit();
            guid[7] = data.readBit();
            guid[2] = data.readBit();
            guid[4] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            status_info.hasOrientation = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            hasTransportData = data.readBit();
            status_info.hasFallData = data.readBit();
            guid[5] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[1] = data.readBit();
            guid[0] = data.readBit();

            if (hasTransportData)
            {
                transport_guid[5] = data.readBit();
                transport_guid[3] = data.readBit();
                transport_guid[6] = data.readBit();
                transport_guid[0] = data.readBit();
                transport_guid[7] = data.readBit();
                status_info.hasTransportTime3 = data.readBit();
                transport_guid[1] = data.readBit();
                transport_guid[2] = data.readBit();
                transport_guid[4] = data.readBit();
                status_info.hasTransportTime2 = data.readBit();
            }

            if (hasMovementFlags)
                flags = data.readBits(30);

            if (status_info.hasFallData)
                status_info.hasFallDirection = data.readBit();

            if (hasMovementFlags2)
                flags2 = static_cast<uint16_t>(data.readBits(13));

            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[7]);

            for (uint32_t i = 0; i < forcesCount; i++)
                    data.read_skip<uint32_t>();

            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[0]);

            if (status_info.hasFallData)
            {
                if (status_info.hasFallDirection)
                {
                    data >> jump_info.sinAngle;
                    data >> jump_info.cosAngle;
                    data >> jump_info.xyspeed;
                }

                data >> jump_info.velocity;
                data >> fall_time;
            }

            if (hasTransportData)
            {
                data.ReadByteSeq(transport_guid[1]);
                data.ReadByteSeq(transport_guid[3]);
                data.ReadByteSeq(transport_guid[2]);
                data.ReadByteSeq(transport_guid[0]);

                if (status_info.hasTransportTime3)
                    data >> transport_time3;

                data >> transport_seat;
                data.ReadByteSeq(transport_guid[7]);
                data >> transport_position.x;
                data.ReadByteSeq(transport_guid[4]);

                if (status_info.hasTransportTime2)
                    data >> transport_time2;

                data >> transport_position.y;
                data.ReadByteSeq(transport_guid[6]);
                data.ReadByteSeq(transport_guid[5]);
                data >> transport_position.z;
                data >> transport_time;
                data >> transport_position.o;
            }

            //counter

            if (status_info.hasOrientation)
                data >> position.o;

            if (status_info.hasPitch)
                data >> pitch_rate;

            if (status_info.hasTimeStamp)
                data >> update_time;

            if (status_info.hasSplineElevation)
                data >> spline_elevation;
#endif

        } break;
#if VERSION_STRING == Cata
        case CMSG_MOVE_KNOCK_BACK_ACK:
        {
            data >> position.y;
            data >> position.z;
            data.read_skip<uint32_t>(); //counter
            data >> position.x;
            guid[6] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            status_info.hasPitch = !data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[3] = data.readBit();
            guid[4] = data.readBit();
            guid[1] = data.readBit();
            guid[2] = data.readBit();
            status_info.hasSpline = data.readBit();
            guid[7] = data.readBit();
            data.readBit();
            hasMovementFlags2 = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[0] = data.readBit();
            hasMovementFlags = !data.readBit();
            hasTransportData = data.readBit();
            guid[5] = data.readBit();
            status_info.hasFallData = data.readBit();

            if (hasMovementFlags) flags = data.readBits(30);
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();

            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[7]);

            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;

            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasTimeStamp)  data >> update_time;

            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data >> transport_seat;

            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasOrientation) data >> position.o;

        } break;
        case CMSG_MOVE_NOT_ACTIVE_MOVER:
        {
            data >> position.z;
            data >> position.x;
            data >> position.y;
            hasMovementFlags2 = !data.readBit();
            hasTransportData = data.readBit();
            guid[6] = data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            guid[3] = data.readBit();
            data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[0] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[5] = data.readBit();
            status_info.hasPitch = !data.readBit();
            guid[1] = data.readBit();
            guid[4] = data.readBit();
            guid[7] = data.readBit();
            status_info.hasSpline = data.readBit();
            guid[2] = data.readBit();
            status_info.hasFallData = data.readBit();
            hasMovementFlags = !data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[3]);
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData) data >> fall_time;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data >> transport_time;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasOrientation) data >> position.o;

        } break;
        case CMSG_DISMISS_CONTROLLED_VEHICLE:
        {
            data >> position.y;
            data >> position.z;
            data >> position.x;
            status_info.hasSplineElevation = !data.readBit();
            status_info.hasFallData = data.readBit();
            hasTransportData = data.readBit();
            guid[4] = data.readBit();
            guid[7] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[2] = data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[0] = data.readBit();
            guid[6] = data.readBit();
            status_info.hasPitch = !data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            guid[1] = data.readBit();
            data.readBit();
            status_info.hasSpline = data.readBit();
            guid[3] = data.readBit();
            hasMovementFlags = !data.readBit();
            guid[5] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            data.ReadByteSeq(guid[6]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[0]);
            if (status_info.hasTimeStamp)  data >> update_time;
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasOrientation) data >> position.o;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasPitch) data >> pitch_rate;

        } break;
        case CMSG_MOVE_SET_CAN_FLY:
        {
            data >> position.z;
            data >> position.x;
            data >> position.y;
            hasTransportData = data.readBit();
            guid[1] = data.readBit();
            guid[6] = data.readBit();
            guid[4] = data.readBit();
            guid[2] = data.readBit();
            status_info.hasFallData = data.readBit();
            guid[0] = data.readBit();
            hasMovementFlags = !data.readBit();
            guid[3] = data.readBit();
            hasMovementFlags2 = !data.readBit();
            guid[5] = data.readBit();
            status_info.hasTimeStamp = !data.readBit();
            status_info.hasSplineElevation = !data.readBit();
            status_info.hasSpline = data.readBit();
            status_info.hasOrientation = !data.readBit();
            guid[7] = data.readBit();
            data.readBit();
            status_info.hasPitch = !data.readBit();
            if (hasTransportData) transport_guid[3] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime2 = data.readBit();
            if (hasTransportData) transport_guid[6] = data.readBit();
            if (hasTransportData) transport_guid[0] = data.readBit();
            if (hasTransportData) transport_guid[7] = data.readBit();
            if (hasTransportData) transport_guid[4] = data.readBit();
            if (hasTransportData) status_info.hasTransportTime3 = data.readBit();
            if (hasTransportData) transport_guid[1] = data.readBit();
            if (hasTransportData) transport_guid[2] = data.readBit();
            if (hasTransportData) transport_guid[5] = data.readBit();
            if (hasMovementFlags2) flags2 = static_cast<uint16_t>(data.readBits(12));
            if (status_info.hasFallData) status_info.hasFallDirection = data.readBit();
            if (hasMovementFlags) flags = data.readBits(30);
            data.ReadByteSeq(guid[2]);
            data.ReadByteSeq(guid[0]);
            data.ReadByteSeq(guid[4]);
            data.ReadByteSeq(guid[7]);
            data.ReadByteSeq(guid[5]);
            data.ReadByteSeq(guid[1]);
            data.ReadByteSeq(guid[3]);
            data.ReadByteSeq(guid[6]);
            if (hasTransportData) data >> transport_position.z;
            if (hasTransportData) data.ReadByteSeq(transport_guid[3]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[5]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[4]);
            if (hasTransportData) data >> transport_seat;
            if (hasTransportData) data >> transport_position.x;
            if (hasTransportData) data.ReadByteSeq(transport_guid[2]);
            if (hasTransportData && status_info.hasTransportTime2) data >> transport_time2;
            if (hasTransportData) data >> transport_position.y;
            if (hasTransportData) data.ReadByteSeq(transport_guid[1]);
            if (hasTransportData) data >> transport_time;
            if (hasTransportData) data.ReadByteSeq(transport_guid[7]);
            if (hasTransportData) data.ReadByteSeq(transport_guid[0]);
            if (hasTransportData) data >> transport_position.o;
            if (hasTransportData) data.ReadByteSeq(transport_guid[6]);
            if (hasTransportData && status_info.hasTransportTime3) data >> fall_time;
            if (status_info.hasSplineElevation) data >> spline_elevation;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.sinAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.cosAngle;
            if (status_info.hasFallData && status_info.hasFallDirection) data >> jump_info.xyspeed;
            if (status_info.hasFallData) data >> jump_info.velocity;
            if (status_info.hasFallData) data >> fall_time;
            if (status_info.hasPitch) data >> pitch_rate;
            if (status_info.hasTimeStamp)  data >> update_time;
            if (status_info.hasOrientation) data >> position.o;

        } break;
#endif
        default:
            sLogger.failure("Unsupported MovementInfo::Read for 0x{:X} ({})!", opcode, sOpcodeTables.getInternalIdForHex(opcode));
            break;
    }
#endif
}

void MovementInfo::writeMovementInfo(ByteBuffer& data, uint16_t opcode, bool withGuid/* = true*/) const
{
#if VERSION_STRING == Classic
    if (withGuid)
        data << guid;

    data << flags << update_time << position << position.o;

    if (hasMovementFlag(MOVEFLAG_TRANSPORT))
        data << transport_guid << transport_position << transport_position.o;

    if (hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)))
        data << pitch_rate;

    data << fall_time;

    if (hasMovementFlag(MOVEFLAG_FALLING))
        data << jump_info.velocity << jump_info.sinAngle << jump_info.cosAngle << jump_info.xyspeed;

    if (hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
        data << spline_elevation;

#elif VERSION_STRING == TBC

    if (withGuid)
        data << guid;

    data << flags << flags2 << update_time << position << position.o;

    if (hasMovementFlag(MOVEFLAG_TRANSPORT))
        data << transport_guid << transport_position << transport_position.o << transport_time;

    if (hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || hasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
        data << pitch_rate;

    data << fall_time;

    if (hasMovementFlag(MOVEFLAG_FALLING))
        data << jump_info.velocity << jump_info.sinAngle << jump_info.cosAngle << jump_info.xyspeed;

    if (hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
        data << spline_elevation;

#elif VERSION_STRING == WotLK

    if (withGuid)
        data << guid;

    data << flags << flags2 << update_time << position << position.o;

    if (hasMovementFlag(MOVEFLAG_TRANSPORT))
    {
        data << transport_guid << transport_position << transport_position.o << transport_time << transport_seat;

        if (hasMovementFlag2(MOVEFLAG2_INTERPOLATED_MOVE))
            data << transport_time2;
    }

    if (hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || hasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
        data << pitch_rate;

    data << fall_time;

    if (hasMovementFlag(MOVEFLAG_FALLING))
        data << jump_info.velocity << jump_info.sinAngle << jump_info.cosAngle << jump_info.xyspeed;

    if (hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
        data << spline_elevation;

#else // >= Cata
    bool hasTransportData = !transport_guid.isEmpty();

    switch (opcode)
    {
#if VERSION_STRING == Cata
        case SMSG_MOVE_UPDATE_KNOCK_BACK:
        {
            data.writeBit(false);
            data.writeBit(guid[4]);
            data.writeBit(!flags);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[1]);
            data.writeBit(guid[0]);
            data.writeBit(guid[3]);
            data.writeBit(guid[2]);
            data.writeBit(guid[7]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(hasTransportData);

            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[3]);

            data.writeBit(guid[5]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(!flags2);
            data.writeBit(guid[6]);

            if (flags) data.writeBits(flags, 30);
            data.writeBit(status_info.hasFallData);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.writeBit(!status_info.hasOrientation);
            if (flags2) data.writeBits(flags2, 12);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            data.WriteByteSeq(guid[3]);

            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);

            if (status_info.hasPitch) data << float(pitch_rate);
            data << float(position.z);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            data << float(position.x);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[1]);
            data << float(position.y);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[5]);


        } break;
#endif
        case SMSG_PLAYER_MOVE:
        {
#if VERSION_STRING == Cata
            data.writeBit(status_info.hasFallData);
            data.writeBit(guid[3]);
            data.writeBit(guid[6]);
            data.writeBit(!flags2);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[0]);
            data.writeBit(guid[1]);

            if (flags2)
                data.writeBits(flags2, 12);

            data.writeBit(guid[7]);
            data.writeBit(!flags);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[2]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(false);
            data.writeBit(guid[4]);

            if (status_info.hasFallData)
                data.writeBit(status_info.hasFallDirection);

            data.writeBit(guid[5]);
            data.writeBit(hasTransportData);

            if (flags)
                data.writeBits(flags, 30);

            if (hasTransportData)
            {
                data.writeBit(transport_guid[3]);
                data.writeBit(status_info.hasTransportTime3);
                data.writeBit(transport_guid[6]);
                data.writeBit(transport_guid[1]);
                data.writeBit(transport_guid[7]);
                data.writeBit(transport_guid[0]);
                data.writeBit(transport_guid[4]);
                data.writeBit(status_info.hasTransportTime2);
                data.writeBit(transport_guid[5]);
                data.writeBit(transport_guid[2]);
            }

            data.writeBit(!status_info.hasPitch);

            data.WriteByteSeq(guid[5]);

            if (status_info.hasFallData)
            {
                if (status_info.hasFallDirection)
                {
                    data << float(jump_info.xyspeed);
                    data << float(jump_info.cosAngle);
                    data << float(jump_info.sinAngle);
                }

                data << float(jump_info.velocity);
                data << uint32_t(fall_time);
            }

            if (status_info.hasSplineElevation)
                data << float(spline_elevation);

            data.WriteByteSeq(guid[7]);
            data << float(position.y);
            data.WriteByteSeq(guid[3]);

            if (hasTransportData)
            {
                if (status_info.hasTransportTime3)
                    data << uint32_t(transport_time3);

                data.WriteByteSeq(transport_guid[6]);
                data << int8_t(transport_seat);
                data.WriteByteSeq(transport_guid[5]);
                data << float(transport_position.x);
                data.WriteByteSeq(transport_guid[1]);
                data << float(normalizeOrientation(transport_position.o));
                data.WriteByteSeq(transport_guid[2]);

                if (status_info.hasTransportTime2)
                    data << uint32_t(transport_time2);

                data.WriteByteSeq(transport_guid[0]);
                data << float(transport_position.z);
                data.WriteByteSeq(transport_guid[7]);
                data.WriteByteSeq(transport_guid[4]);
                data.WriteByteSeq(transport_guid[3]);
                data << float(transport_position.y);
                data << uint32_t(transport_time);
            }

            data.WriteByteSeq(guid[4]);
            data << float(position.x);
            data.WriteByteSeq(guid[6]);
            data << float(position.z);

            if (status_info.hasTimeStamp)
                data << Util::getMSTime();

            data.WriteByteSeq(guid[2]);
            if (status_info.hasPitch)
                data << float(pitch_rate);

            data.WriteByteSeq(guid[0]);

            if (status_info.hasOrientation)
                data << float(normalizeOrientation(position.o));

            data.WriteByteSeq(guid[1]);
#else // Mop
            data.writeBit(!status_info.hasPitch);
            data.writeBit(guid[2]);
            data.writeBit(0);
            data.writeBit(0);
            data.writeBit(guid[0]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(status_info.hasFallData);
            data.writeBit(false);
            data.writeBit(guid[3]);
            if (status_info.hasFallData)
                data.writeBit(status_info.hasFallDirection);

            data.writeBit(hasTransportData);
            data.writeBit(guid[4]);

            if (hasTransportData)
            {
                data.writeBit(transport_guid[5]);
                data.writeBit(transport_guid[4]);
                data.writeBit(transport_guid[7]);
                data.writeBit(transport_guid[2]);
                data.writeBit(transport_guid[6]);
                data.writeBit(status_info.hasTransportTime2);
                data.writeBit(transport_guid[3]);
                data.writeBit(transport_guid[1]);
                data.writeBit(status_info.hasTransportTime3);
                data.writeBit(transport_guid[0]);
            }

            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(!flags);
            data.writeBit(0);

            if (flags)
                data.writeBits(flags, 30);

            data.writeBit(!flags2);
            data.writeBit(guid[7]);
            data.writeBit(guid[1]);

            if (status_info.hasTimeStamp)
                data << Util::getMSTime();

            if (flags2)
                data.writeBits(flags2, 13);

            data.writeBit(guid[5]);

            data.writeBits(0, 22);
            data.writeBit(guid[6]);
            data << float(position.y);

            if (hasTransportData)
            {
                data.WriteByteSeq(transport_guid[7]);

                if (status_info.hasTransportTime2)
                    data << uint32_t(transport_time2);

                data << float(transport_position.x);
                data.WriteByteSeq(transport_guid[5]);
                data << int8_t(transport_seat);
                data.WriteByteSeq(transport_guid[2]);
                data.WriteByteSeq(transport_guid[0]);
                data.WriteByteSeq(transport_guid[3]);
                data << uint32_t(transport_time);
                data.WriteByteSeq(transport_guid[4]);
                data << float(transport_position.z);
                data.WriteByteSeq(transport_guid[1]);
                data << float(transport_position.y);
                data << float(normalizeOrientation(transport_position.o));
                data.WriteByteSeq(transport_guid[6]);

                if (status_info.hasTransportTime3)
                    data << uint32_t(transport_time3);
            }
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[1]);
            data << float(transport_position.z);

            if (status_info.hasTimeStamp)
                data << Util::getMSTime();

            if (status_info.hasOrientation)
                data << float(normalizeOrientation(position.o));

            data.WriteByteSeq(guid[3]);
            if (status_info.hasFallData)
            {
                if (status_info.hasFallDirection)
                {
                    data << float(jump_info.sinAngle);
                    data << float(jump_info.xyspeed);
                    data << float(jump_info.cosAngle);
                }
                data << float(jump_info.velocity);
                data << uint32_t(fall_time);
            }
            data.WriteByteSeq(guid[0]);

            if (status_info.hasPitch)
                data << float(pitch_rate);

            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[6]);

            if (status_info.hasSplineElevation)
                data << float(spline_elevation);

            data.writeBits(0, 22);
            data << float(transport_position.x);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[7]);
#endif

        } break;
#if VERSION_STRING == Cata
        case MSG_MOVE_START_BACKWARD:
        {
            data << float(position.x);
            data << float(position.z);
            data << float(position.y);
            data.writeBit(hasTransportData);
            data.writeBit(guid[3]);
            data.writeBit(guid[0]);
            data.writeBit(guid[2]);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[7]);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(false);
            data.writeBit(!flags);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!flags2);
            data.writeBit(status_info.hasFallData);
            data.writeBit(guid[5]);
            data.writeBit(guid[1]);
            data.writeBit(guid[4]);
            data.writeBit(guid[6]);
            data.writeBit(!status_info.hasSplineElevation);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (flags) data.writeBits(flags, 30);
            if (flags2) data.writeBits(flags2, 12);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[3]);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasSplineElevation) data << float(spline_elevation);

        } break;
        case MSG_MOVE_START_FORWARD:
        {
            data << float(position.y);
            data << float(position.z);
            data << float(position.x);
            data.writeBit(guid[5]);
            data.writeBit(guid[2]);
            data.writeBit(guid[0]);
            data.writeBit(false);
            data.writeBit(!flags);
            data.writeBit(guid[7]);
            data.writeBit(guid[3]);
            data.writeBit(guid[1]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[6]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(guid[4]);
            data.writeBit(hasTransportData);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(!flags2);
            data.writeBit(status_info.hasFallData);
            if (flags) data.writeBits(flags, 30);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags2) data.writeBits(flags2, 12);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[0]);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasTimeStamp) data << Util::getMSTime();

        } break;
        case MSG_MOVE_START_STRAFE_LEFT:
        {
            data << float(position.z);
            data << float(position.x);
            data << float(position.y);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(guid[5]);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(guid[6]);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[1]);
            data.writeBit(false);
            data.writeBit(guid[4]);
            data.writeBit(guid[0]);
            data.writeBit(guid[2]);
            data.writeBit(status_info.hasFallData);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[3]);
            data.writeBit(!flags2);
            data.writeBit(guid[7]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!flags);
            data.writeBit(hasTransportData);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (flags) data.writeBits(flags, 30);
            if (flags2) data.writeBits(flags2, 12);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[5]);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << float(transport_position.x);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasSplineElevation) data << float(spline_elevation);

        } break;
        case MSG_MOVE_START_STRAFE_RIGHT:
        {
            data << float(position.y);
            data << float(position.x);
            data << float(position.z);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(guid[1]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[4]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(false);
            data.writeBit(guid[5]);
            data.writeBit(status_info.hasFallData);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(!flags);
            data.writeBit(guid[2]);
            data.writeBit(guid[7]);
            data.writeBit(guid[6]);
            data.writeBit(guid[3]);
            data.writeBit(!flags2);
            data.writeBit(hasTransportData);
            data.writeBit(guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (flags2) data.writeBits(flags2, 12);
            if (flags) data.writeBits(flags, 30);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[0]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasSplineElevation) data << float(spline_elevation);

        } break;
        case MSG_MOVE_START_TURN_LEFT:
        {
            data << float(position.y);
            data << float(position.x);
            data << float(position.z);
            data.writeBit(false);
            data.writeBit(guid[1]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!flags);
            data.writeBit(guid[4]);
            data.writeBit(guid[2]);
            data.writeBit(!flags2);
            data.writeBit(guid[5]);
            data.writeBit(guid[7]);
            data.writeBit(hasTransportData);
            data.writeBit(guid[6]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(guid[0]);
            data.writeBit(guid[3]);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(status_info.hasFallData);
            if (flags2) data.writeBits(flags2, 12);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags) data.writeBits(flags, 30);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[1]);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasSplineElevation) data << float(spline_elevation);

        } break;
        case MSG_MOVE_START_TURN_RIGHT:
        {
            data << float(position.x);
            data << float(position.z);
            data << float(position.y);
            data.writeBit(guid[3]);
            data.writeBit(guid[5]);
            data.writeBit(!flags);
            data.writeBit(status_info.hasSpline);
            data.writeBit(guid[0]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(hasTransportData);
            data.writeBit(guid[7]);
            data.writeBit(false);
            data.writeBit(!flags2);
            data.writeBit(guid[1]);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[6]);
            data.writeBit(guid[2]);
            data.writeBit(guid[4]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(status_info.hasFallData);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (flags2) data.writeBits(flags2, 12);
            if (flags) data.writeBits(flags, 30);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[6]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasTimeStamp) data << Util::getMSTime();

        } break;
        case MSG_MOVE_STOP:
        {
            data << float(position.x);
            data << float(position.y);
            data << float(position.z);
            data.writeBit(guid[3]);
            data.writeBit(guid[6]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[7]);
            data.writeBit(!flags);
            data.writeBit(guid[5]);
            data.writeBit(status_info.hasFallData);
            data.writeBit(!flags2);
            data.writeBit(hasTransportData);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[4]);
            data.writeBit(guid[1]);
            data.writeBit(false);
            data.writeBit(guid[2]);
            data.writeBit(guid[0]);
            data.writeBit(!status_info.hasPitch);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (flags) data.writeBits(flags, 30);
            if (flags2) data.writeBits(flags2, 12);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[7]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData) data << uint32_t(fall_time);

        } break;
        case MSG_MOVE_STOP_STRAFE:
        {
            data << float(position.y);
            data << float(position.z);
            data << float(position.x);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[2]);
            data.writeBit(status_info.hasFallData);
            data.writeBit(guid[7]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(guid[3]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(!flags2);
            data.writeBit(hasTransportData);
            data.writeBit(!flags);
            data.writeBit(status_info.hasSpline);
            data.writeBit(guid[0]);
            data.writeBit(false);
            data.writeBit(guid[6]);
            data.writeBit(guid[5]);
            data.writeBit(guid[1]);
            data.writeBit(guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (flags) data.writeBits(flags, 30);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags2) data.writeBits(flags2, 12);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[0]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasTimeStamp) data << Util::getMSTime();

        } break;
        case MSG_MOVE_STOP_TURN:
        {
            data << float(position.x);
            data << float(position.z);
            data << float(position.y);
            data.writeBit(guid[5]);
            data.writeBit(guid[4]);
            data.writeBit(status_info.hasFallData);
            data.writeBit(false);
            data.writeBit(guid[1]);
            data.writeBit(guid[0]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!flags);
            data.writeBit(guid[2]);
            data.writeBit(guid[6]);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(hasTransportData);
            data.writeBit(guid[3]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(!flags2);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags) data.writeBits(flags, 30);
            if (flags2) data.writeBits(flags2, 12);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[5]);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));

        } break;
        case MSG_MOVE_START_ASCEND:
        {
            data << float(position.x);
            data << float(position.y);
            data << float(position.z);
            data.writeBit(guid[0]);
            data.writeBit(guid[1]);
            data.writeBit(guid[7]);
            data.writeBit(false);
            data.writeBit(guid[5]);
            data.writeBit(hasTransportData);
            data.writeBit(guid[2]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(status_info.hasFallData);
            data.writeBit(status_info.hasSpline);
            data.writeBit(guid[3]);
            data.writeBit(!flags2);
            data.writeBit(guid[6]);
            data.writeBit(!flags);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[4]);
            if (flags) data.writeBits(flags, 30);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (flags2) data.writeBits(flags2, 12);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[7]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data << float(transport_position.x);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasSplineElevation) data << float(spline_elevation);

        } break;
        case MSG_MOVE_START_DESCEND:
        {
            data << float(position.y);
            data << float(position.z);
            data << float(position.x);
            data.writeBit(guid[0]);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(status_info.hasFallData);
            data.writeBit(guid[4]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(!flags2);
            data.writeBit(!flags);
            data.writeBit(guid[6]);
            data.writeBit(false);
            data.writeBit(guid[1]);
            data.writeBit(hasTransportData);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(guid[5]);
            data.writeBit(guid[3]);
            data.writeBit(guid[7]);
            data.writeBit(guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (flags2) data.writeBits(flags2, 12);
            if (flags) data.writeBits(flags, 30);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[3]);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasSplineElevation) data << float(spline_elevation);

        } break;
        case MSG_MOVE_START_SWIM:
        {
            data << float(position.z);
            data << float(position.x);
            data << float(position.y);
            data.writeBit(guid[3]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(guid[4]);
            data.writeBit(guid[7]);
            data.writeBit(false);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(guid[0]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(!flags);
            data.writeBit(status_info.hasFallData);
            data.writeBit(guid[5]);
            data.writeBit(hasTransportData);
            data.writeBit(!flags2);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[6]);
            data.writeBit(guid[1]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (flags) data.writeBits(flags, 30);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags2) data.writeBits(flags2, 12);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[7]);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasSplineElevation) data << float(spline_elevation);

        } break;
        case MSG_MOVE_STOP_SWIM:
        {
            data << float(position.x);
            data << float(position.y);
            data << float(position.z);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(guid[5]);
            data.writeBit(guid[3]);
            data.writeBit(guid[7]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(!flags);
            data.writeBit(guid[4]);
            data.writeBit(!flags2);
            data.writeBit(guid[2]);
            data.writeBit(guid[6]);
            data.writeBit(hasTransportData);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(false);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[1]);
            data.writeBit(guid[0]);
            data.writeBit(status_info.hasFallData);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (flags) data.writeBits(flags, 30);
            if (flags2) data.writeBits(flags2, 12);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[2]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasSplineElevation) data << float(spline_elevation);

        } break;
        case MSG_MOVE_STOP_ASCEND:
        {
            data << float(position.z);
            data << float(position.y);
            data << float(position.x);
            data.writeBit(guid[3]);
            data.writeBit(guid[2]);
            data.writeBit(status_info.hasFallData);
            data.writeBit(guid[7]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(status_info.hasSpline);
            data.writeBit(false);
            data.writeBit(guid[1]);
            data.writeBit(guid[4]);
            data.writeBit(!flags);
            data.writeBit(guid[0]);
            data.writeBit(guid[6]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(hasTransportData);
            data.writeBit(!flags2);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[5]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (flags2) data.writeBits(flags2, 12);
            if (flags) data.writeBits(flags, 30);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data << float(transport_position.z);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));

        } break;

        case MSG_MOVE_START_PITCH_DOWN:
        {
            data << float(position.x);
            data << float(position.z);
            data << float(position.y);
            data.writeBit(!flags);
            data.writeBit(guid[7]);
            data.writeBit(guid[6]);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(false);
            data.writeBit(guid[1]);
            data.writeBit(guid[4]);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(!flags2);
            data.writeBit(hasTransportData);
            data.writeBit(guid[0]);
            data.writeBit(guid[5]);
            data.writeBit(guid[3]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(status_info.hasFallData);
            data.writeBit(guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (flags2) data.writeBits(flags2, 12);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags) data.writeBits(flags, 30);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[1]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasTimeStamp) data << Util::getMSTime();

        } break;
        case MSG_MOVE_START_PITCH_UP:
        {
            data << float(position.z);
            data << float(position.y);
            data << float(position.x);
            data.writeBit(guid[4]);
            data.writeBit(!flags);
            data.writeBit(!flags2);
            data.writeBit(status_info.hasSpline);
            data.writeBit(guid[2]);
            data.writeBit(guid[6]);
            data.writeBit(guid[3]);
            data.writeBit(false);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(status_info.hasFallData);
            data.writeBit(guid[0]);
            data.writeBit(hasTransportData);
            data.writeBit(guid[1]);
            data.writeBit(guid[5]);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[7]);
            if (flags2) data.writeBits(flags2, 12);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (flags) data.writeBits(flags, 30);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[2]);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasTimeStamp) data << Util::getMSTime();

        } break;
        case MSG_MOVE_STOP_PITCH:
        {
            data << float(position.x);
            data << float(position.z);
            data << float(position.y);
            data.writeBit(guid[0]);
            data.writeBit(guid[5]);
            data.writeBit(guid[3]);
            data.writeBit(status_info.hasFallData);
            data.writeBit(guid[2]);
            data.writeBit(guid[4]);
            data.writeBit(guid[7]);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(false);
            data.writeBit(!flags2);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(guid[6]);
            data.writeBit(guid[1]);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(hasTransportData);
            data.writeBit(!flags);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags2) data.writeBits(flags2, 12);
            if (flags) data.writeBits(flags, 30);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[2]);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasPitch) data << float(pitch_rate);

        } break;
        case MSG_MOVE_HEARTBEAT:
        {
            data << float(position.z);
            data << float(position.x);
            data << float(position.y);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(status_info.hasFallData);
            data.writeBit(!flags2);
            data.writeBit(hasTransportData);
            data.writeBit(guid[7]);
            data.writeBit(guid[1]);
            data.writeBit(guid[0]);
            data.writeBit(guid[4]);
            data.writeBit(guid[2]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[5]);
            data.writeBit(guid[3]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(status_info.hasSpline);
            data.writeBit(false);
            data.writeBit(guid[6]);
            data.writeBit(!flags);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags) data.writeBits(flags, 30);
            if (flags2) data.writeBits(flags2, 12);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[4]);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasTimeStamp) data << Util::getMSTime();

        } break;
        case MSG_MOVE_SET_FACING:
        {
            data << float(position.x);
            data << float(position.y);
            data << float(position.z);
            data.writeBit(guid[6]);
            data.writeBit(hasTransportData);
            data.writeBit(guid[4]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(guid[0]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(!flags2);
            data.writeBit(guid[5]);
            data.writeBit(guid[7]);
            data.writeBit(guid[2]);
            data.writeBit(status_info.hasFallData);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(!flags);
            data.writeBit(guid[3]);
            data.writeBit(false);
            data.writeBit(guid[1]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags2) data.writeBits(flags2, 12);
            if (flags) data.writeBits(flags, 30);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasPitch) data << float(pitch_rate);

        } break;
        case MSG_MOVE_SET_PITCH:
        {
            data << float(position.x);
            data << float(position.z);
            data << float(position.y);
            data.writeBit(status_info.hasFallData);
            data.writeBit(!flags);
            data.writeBit(guid[1]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[7]);
            data.writeBit(guid[3]);
            data.writeBit(!flags2);
            data.writeBit(hasTransportData);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(guid[6]);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(guid[4]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(guid[2]);
            data.writeBit(false);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[0]);
            data.writeBit(guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (flags2) data.writeBits(flags2, 12);
            if (flags) data.writeBits(flags, 30);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[4]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data << float(transport_position.x);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));

        } break;
        case MSG_MOVE_SET_RUN_MODE:
        {
            data << float(position.y);
            data << float(position.x);
            data << float(position.z);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(!flags2);
            data.writeBit(guid[1]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!flags);
            data.writeBit(guid[7]);
            data.writeBit(hasTransportData);
            data.writeBit(false);
            data.writeBit(guid[0]);
            data.writeBit(guid[3]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(guid[5]);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(guid[6]);
            data.writeBit(guid[4]);
            data.writeBit(status_info.hasFallData);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags2) data.writeBits(flags2, 12);
            if (flags) data.writeBits(flags, 30);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[2]);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));

        } break;
        case MSG_MOVE_SET_WALK_MODE:
        {
            data << float(position.y);
            data << float(position.x);
            data << float(position.z);
            data.writeBit(guid[6]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[0]);
            data.writeBit(guid[1]);
            data.writeBit(!flags);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(guid[7]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(guid[4]);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(hasTransportData);
            data.writeBit(guid[2]);
            data.writeBit(guid[5]);
            data.writeBit(guid[3]);
            data.writeBit(false);
            data.writeBit(!flags2);
            data.writeBit(status_info.hasFallData);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags) data.writeBits(flags, 30);
            if (flags2) data.writeBits(flags2, 12);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));

        } break;
        case MSG_MOVE_JUMP:
        {
            data << float(position.y);
            data << float(position.x);
            data << float(position.z);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(guid[5]);
            data.writeBit(!flags);
            data.writeBit(guid[4]);
            data.writeBit(guid[6]);
            data.writeBit(!flags2);
            data.writeBit(guid[0]);
            data.writeBit(hasTransportData);
            data.writeBit(guid[3]);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(guid[7]);
            data.writeBit(status_info.hasFallData);
            data.writeBit(status_info.hasSpline);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[1]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(false);
            data.writeBit(guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            if (flags) data.writeBits(flags, 30);
            if (flags2) data.writeBits(flags2, 12);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));

        } break;
        case MSG_MOVE_FALL_LAND:
        {
            data << float(position.x);
            data << float(position.y);
            data << float(position.z);
            data.writeBit(hasTransportData);
            data.writeBit(guid[7]);
            data.writeBit(guid[1]);
            data.writeBit(!flags2);
            data.writeBit(guid[3]);
            data.writeBit(!status_info.hasSplineElevation);
            data.writeBit(!status_info.hasOrientation);
            data.writeBit(guid[6]);
            data.writeBit(!status_info.hasTimeStamp);
            data.writeBit(false);
            data.writeBit(!status_info.hasPitch);
            data.writeBit(guid[4]);
            data.writeBit(status_info.hasSpline);
            data.writeBit(guid[5]);
            data.writeBit(!flags);
            data.writeBit(status_info.hasFallData);
            data.writeBit(guid[0]);
            data.writeBit(guid[2]);
            if (hasTransportData) data.writeBit(transport_guid[0]);
            if (hasTransportData) data.writeBit(transport_guid[5]);
            if (hasTransportData) data.writeBit(transport_guid[3]);
            if (hasTransportData) data.writeBit(transport_guid[2]);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime3);
            if (hasTransportData) data.writeBit(status_info.hasTransportTime2);
            if (hasTransportData) data.writeBit(transport_guid[6]);
            if (hasTransportData) data.writeBit(transport_guid[4]);
            if (hasTransportData) data.writeBit(transport_guid[1]);
            if (hasTransportData) data.writeBit(transport_guid[7]);
            if (flags2) data.writeBits(flags2, 12);
            if (flags) data.writeBits(flags, 30);
            if (status_info.hasFallData) data.writeBit(status_info.hasFallDirection);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[5]);
            if (hasTransportData) data << uint32_t(transport_time);
            if (hasTransportData) data << float(transport_position.z);
            if (hasTransportData) data << float(transport_position.y);
            if (hasTransportData) data << float(transport_position.x);
            if (hasTransportData) data.WriteByteSeq(transport_guid[5]);
            if (hasTransportData && status_info.hasTransportTime3) data << uint32_t(transport_time3);
            if (hasTransportData) data.WriteByteSeq(transport_guid[1]);
            if (hasTransportData) data << int8_t(transport_seat);
            if (hasTransportData && status_info.hasTransportTime2) data << uint32_t(transport_time2);
            if (hasTransportData) data.WriteByteSeq(transport_guid[4]);
            if (hasTransportData) data << float(normalizeOrientation(transport_position.o));
            if (hasTransportData) data.WriteByteSeq(transport_guid[0]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[7]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[3]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[6]);
            if (hasTransportData) data.WriteByteSeq(transport_guid[2]);
            if (status_info.hasFallData) data << float(jump_info.velocity);
            if (status_info.hasFallData) data << uint32_t(fall_time);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.cosAngle);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.xyspeed);
            if (status_info.hasFallData && status_info.hasFallDirection) data << float(jump_info.sinAngle);
            if (status_info.hasSplineElevation) data << float(spline_elevation);
            if (status_info.hasTimeStamp) data << Util::getMSTime();
            if (status_info.hasPitch) data << float(pitch_rate);
            if (status_info.hasOrientation) data << float(normalizeOrientation(position.o));

        } break;
#endif
        default:
            sLogger.failure("Unsupported MovementInfo::Write for 0x{:X} ({})!", opcode, sOpcodeTables.getInternalIdForHex(opcode));
            break;
    }

#endif
}
