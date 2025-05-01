/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MovementPacketBuilder.h"
#include "Logging/Logger.hpp"
#include "ByteBuffer.h"
#include "MoveSpline.h"

namespace MovementMgr {

inline void operator<<(ByteBuffer& b, Vector3 const& v)
{
    b << v.x << v.y << v.z;
}

inline void operator>>(ByteBuffer& b, Vector3& v)
{
    b >> v.x >> v.y >> v.z;
}

enum MonsterMoveType
{
    MonsterMoveNormal       = 0,
    MonsterMoveStop         = 1,
    MonsterMoveFacingSpot   = 2,
    MonsterMoveFacingTarget = 3,
    MonsterMoveFacingAngle  = 4
};

void PacketBuilder::WriteCommonMonsterMovePart(MoveSpline const& move_spline, ByteBuffer& data)
{
    MoveSplineFlag splineflags = move_spline.splineflags;
#if VERSION_STRING > TBC
    data << uint8_t(0);
#endif
    data << move_spline.spline.getPoint(move_spline.spline.first());
    data << move_spline.GetId();

    switch (splineflags & MoveSplineFlag::Mask_Final_Facing)
    {
    case MoveSplineFlag::Final_Target:
        data << uint8_t(MonsterMoveFacingTarget);
        data << move_spline.facing.target;
        break;
    case MoveSplineFlag::Final_Angle:
        data << uint8_t(MonsterMoveFacingAngle);
        data << move_spline.facing.angle;
        break;
    case MoveSplineFlag::Final_Point:
        data << uint8_t(MonsterMoveFacingSpot);
        data << move_spline.facing.f.x << move_spline.facing.f.y << move_spline.facing.f.z;
        break;
    default:
        data << uint8_t(MonsterMoveNormal);
        break;
    }

#if VERSION_STRING > WotLK
    // add fake Enter_Cycle flag - needed for client-side cyclic movement (client will erase first spline vertex after first cycle done)
    splineflags.enter_cycle = move_spline.isCyclic();
#endif
    data << uint32_t(splineflags & uint32_t(~MoveSplineFlag::Mask_No_Monster_Move));
#if VERSION_STRING > TBC
    if (splineflags.animation)
    {
        data << splineflags.getAnimationId();
        data << move_spline.effect_start_time;
    }
#endif
    data << move_spline.Duration();
#if VERSION_STRING > TBC
    if (splineflags.parabolic)
    {
        data << move_spline.vertical_acceleration;
        data << move_spline.effect_start_time;
    }
#endif
}

void PacketBuilder::WriteStopMovement(G3D::Vector3 const& pos, uint32_t splineId, ByteBuffer& data)
{
#if VERSION_STRING > TBC
    data << uint8_t(0);
#endif
    data << pos;
    data << splineId;
    data << uint8_t(MonsterMoveStop);
}

void WriteLinearPath(Spline<int32_t> const& spline, ByteBuffer& data)
{
    if (spline.getPointCount() < 3)
        sLogger.failure("WriteLinearPath: size of points is < 3, this will lead to issues!");

    uint32_t last_idx = static_cast<uint32_t>(spline.getPointCount() - 3);
    G3D::Vector3 const* real_path = &spline.getPoint(1);

    data << last_idx;
    data << real_path[last_idx]; // destination
    if (last_idx > 1)
    {
        G3D::Vector3 middle = (real_path[0] + real_path[last_idx]) / 2.f;
        G3D::Vector3 offset;
        // first and last points already appended
        for (uint32_t i = 1; i < last_idx; ++i)
        {
            offset = middle - real_path[i];
            data.appendPackXYZ(offset.x, offset.y, offset.z);
        }
    }
}

void WriteCatmullRomPath(Spline<int32_t> const& spline, ByteBuffer& data)
{
    if (spline.getPointCount() < 3)
        sLogger.failure("WriteCatmullRomPath: size of points is < 3, this will lead to issues!");

    uint32_t count = static_cast<uint32_t>(spline.getPointCount() - 3);
    data << count;
    data.append<G3D::Vector3>(&spline.getPoint(2), count);
}

void WriteCatmullRomCyclicPath(Spline<int32_t> const& spline, ByteBuffer& data)
{
#if VERSION_STRING <= WotLK
    if (spline.getPointCount() < 4)
        sLogger.failure("WriteCatmullRomCyclicPath: size of points is < 3, this will lead to issues!");

    uint32_t count = static_cast<uint32_t>(spline.getPointCount() - 4);
    data << count;
    data.append<Vector3>(&spline.getPoint(2), count);
#else
    if (spline.getPointCount() < 3)
        sLogger.failure("WriteCatmullRomCyclicPath: size of points is < 3, this will lead to issues!");

    uint32_t count = static_cast<uint32_t>(spline.getPointCount() - 3);
    data << count + 1;
    data << spline.getPoint(1); // fake point, client will erase it from the spline after first cycle done
    data.append<Vector3>(&spline.getPoint(1), count);
#endif
}

void PacketBuilder::WriteMonsterMove(MoveSpline const& move_spline, ByteBuffer& data)
{
#if VERSION_STRING <= WotLK
    WriteCommonMonsterMovePart(move_spline, data);

    const Spline<int32_t>& spline = move_spline.spline;
    MoveSplineFlag splineflags = move_spline.splineflags;
    if (splineflags & MoveSplineFlag::Mask_CatmullRom)
    {
        if (splineflags.cyclic)
            WriteCatmullRomCyclicPath(spline, data);
        else
            WriteCatmullRomPath(spline, data);
    }
    else
    {
        WriteLinearPath(spline, data);
    }
#else
    WriteCommonMonsterMovePart(move_spline, data);

    const Spline<int32_t>& spline = move_spline.spline;
    MoveSplineFlag splineflags = move_spline.splineflags;
    if (splineflags & MoveSplineFlag::UncompressedPath)
    {
        if (splineflags.cyclic)
            WriteCatmullRomCyclicPath(spline, data);
        else
            WriteCatmullRomPath(spline, data);
    }
    else
    {
        WriteLinearPath(spline, data);
    }
#endif
}

void PacketBuilder::WriteCreate(MoveSpline const& move_spline, ByteBuffer& data)
{
    {
        MoveSplineFlag const& splineFlags = move_spline.splineflags;

        data << splineFlags.raw();

        if (splineFlags.final_angle)
        {
            data << move_spline.facing.angle;
        }
        else if (splineFlags.final_target)
        {
            data << move_spline.facing.target;
        }
        else if (splineFlags.final_point)
        {
            data << move_spline.facing.f.x << move_spline.facing.f.y << move_spline.facing.f.z;
        }

        data << move_spline.timePassed();
        data << move_spline.Duration();
        data << move_spline.GetId();
#if VERSION_STRING > TBC
        data << float(1.f);                                             // splineInfo.duration_mod; added in 3.1
        data << float(1.f);                                             // splineInfo.duration_mod_next; added in 3.1

        data << move_spline.vertical_acceleration;                      // added in 3.1
        data << move_spline.effect_start_time;                          // added in 3.1
#endif
        uint32_t nodes = static_cast<uint32_t>(move_spline.getPath().size());
        data << nodes;
        data.append<G3D::Vector3>(&move_spline.getPath()[0], nodes);
#if VERSION_STRING > TBC
        data << uint8_t(move_spline.spline.mode());                     // added in 3.1
#endif
        data << (move_spline.isCyclic() ? G3D::Vector3::zero() : move_spline.FinalDestination());
    }
}
#if VERSION_STRING >= Cata
void PacketBuilder::WriteCreateData(MoveSpline const& moveSpline, ByteBuffer& data)
{
    if (!moveSpline.Finalized())
    {
        MoveSplineFlag splineFlags = moveSpline.splineflags;

        if ((splineFlags & MoveSplineFlag::Parabolic) && moveSpline.effect_start_time < moveSpline.Duration())
            data << moveSpline.vertical_acceleration;   // added in 3.1

        data << moveSpline.timePassed();

        if (splineFlags.final_angle)
            data << moveSpline.facing.angle;
        else if (splineFlags.final_target)
        {
            ObjectGuid facingGuid = moveSpline.facing.target;
            data.WriteByteSeq(facingGuid[5]);
            data.WriteByteSeq(facingGuid[3]);
            data.WriteByteSeq(facingGuid[7]);
            data.WriteByteSeq(facingGuid[1]);
            data.WriteByteSeq(facingGuid[6]);
            data.WriteByteSeq(facingGuid[4]);
            data.WriteByteSeq(facingGuid[2]);
            data.WriteByteSeq(facingGuid[0]);
        }

        uint32_t nodes = static_cast<uint32_t>(moveSpline.getPath().size());
        for (uint32_t i = 0; i < nodes; ++i)
        {
            data << float(moveSpline.getPath()[i].z);
            data << float(moveSpline.getPath()[i].x);
            data << float(moveSpline.getPath()[i].y);
        }

        if (splineFlags.final_point)
            data << moveSpline.facing.f.x << moveSpline.facing.f.z << moveSpline.facing.f.y;

        data << float(1.f);                             // splineInfo.duration_mod_next; added in 3.1
        data << moveSpline.Duration();
        if (splineFlags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation))
            data << moveSpline.effect_start_time;       // added in 3.1

        data << float(1.f);                             // splineInfo.duration_mod; added in 3.1
    }

    if (!moveSpline.isCyclic())
    {
        Vector3 dest = moveSpline.FinalDestination();
        data << float(dest.z);
        data << float(dest.x);
        data << float(dest.y);
    }
    else
        data << Vector3::zero();

    data << moveSpline.GetId();
}

void PacketBuilder::WriteCreateBits(MoveSpline const& moveSpline, ByteBuffer& data)
{
    if (!data.writeBit(!moveSpline.Finalized()))
        return;

    data.writeBits(uint8_t(moveSpline.spline.mode()), 2);
    data.writeBit(moveSpline.splineflags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation));
    data.writeBits(moveSpline.getPath().size(), 22);
    switch (moveSpline.splineflags & MoveSplineFlag::Mask_Final_Facing)
    {
    case MoveSplineFlag::Final_Target:
    {
        ObjectGuid targetGuid = moveSpline.facing.target;
        data.writeBits(2, 2);
        data.writeBit(targetGuid[4]);
        data.writeBit(targetGuid[3]);
        data.writeBit(targetGuid[7]);
        data.writeBit(targetGuid[2]);
        data.writeBit(targetGuid[6]);
        data.writeBit(targetGuid[1]);
        data.writeBit(targetGuid[0]);
        data.writeBit(targetGuid[5]);
        break;
    }
    case MoveSplineFlag::Final_Angle:
        data.writeBits(0, 2);
        break;
    case MoveSplineFlag::Final_Point:
        data.writeBits(1, 2);
        break;
    default:
        data.writeBits(3, 2);
        break;
    }

    data.writeBit((moveSpline.splineflags & MoveSplineFlag::Parabolic) && moveSpline.effect_start_time < moveSpline.Duration());
    data.writeBits(moveSpline.splineflags.raw(), 25);
}
#endif
void PacketBuilder::WriteSplineSync(MoveSpline const& move_spline, ByteBuffer& data)
{
    data << (float)move_spline.timePassed() / move_spline.Duration();
}
} // namespace MovementMgr
