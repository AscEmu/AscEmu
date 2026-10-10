/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MovementPacketBuilder.h"
#include "Logging/Logger.hpp"
#include "Network/ByteBuffer.hpp"
#include "MoveSpline.h"
#include "Objects/Units/Unit.hpp"

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
#if VERSION_STRING == Cata
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
            WoWGuid facingGuid = moveSpline.facing.target;
            data.writeByteSeq(facingGuid[5]);
            data.writeByteSeq(facingGuid[3]);
            data.writeByteSeq(facingGuid[7]);
            data.writeByteSeq(facingGuid[1]);
            data.writeByteSeq(facingGuid[6]);
            data.writeByteSeq(facingGuid[4]);
            data.writeByteSeq(facingGuid[2]);
            data.writeByteSeq(facingGuid[0]);
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
        WoWGuid targetGuid = moveSpline.facing.target;
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

#if VERSION_STRING >= Mop
void PacketBuilder::WriteCreateData(MoveSpline const& moveSpline, ByteBuffer& data)
{
    if (!moveSpline.Finalized())
    {
        MoveSplineFlag splineFlags = moveSpline.splineflags;
        MonsterMoveType type;
        switch (moveSpline.splineflags & MoveSplineFlag::Mask_Final_Facing)
        {
            case MoveSplineFlag::Final_Target:
                type = MonsterMoveFacingTarget;
            case MoveSplineFlag::Final_Angle:
                type = MonsterMoveFacingAngle;
            case MoveSplineFlag::Final_Point:
                type = MonsterMoveFacingSpot;
            default:
                type = MonsterMoveNormal;
        }

        data << moveSpline.timePassed();
        data << float(1.f);                             // splineInfo.duration_mod_next; added in 3.1
        data << float(1.f);                             // splineInfo.duration_mod; added in 3.1

        uint32_t nodes = static_cast<uint32_t>(moveSpline.getPath().size());
        for (uint32_t i = 0; i < nodes; ++i)
        {
            data << float(moveSpline.getPath()[i].x);
            data << float(moveSpline.getPath()[i].z);
            data << float(moveSpline.getPath()[i].y);
        }

        if ((splineFlags & MoveSplineFlag::Parabolic) && moveSpline.effect_start_time < moveSpline.Duration())
            data << moveSpline.vertical_acceleration;   // added in 3.1

        data << uint8_t(type);

        if (type == MonsterMoveFacingAngle)
            data << float(moveSpline.facing.angle);

        if (type == MonsterMoveFacingSpot)
            data << moveSpline.facing.f.x << moveSpline.facing.f.z << moveSpline.facing.f.y;

        if ((splineFlags & MoveSplineFlag::Parabolic) && moveSpline.effect_start_time < moveSpline.Duration())
            data << float(moveSpline.vertical_acceleration);   // added in 3.1

        data << moveSpline.Duration();
    }

    Vector3 destination = moveSpline.isCyclic() ? Vector3::zero() : moveSpline.FinalDestination();

    data << float(destination.x);
    data << float(destination.z);
    data << moveSpline.GetId();
    data << float(destination.y);
}

void PacketBuilder::WriteCreateBits(MoveSpline const& moveSpline, ByteBuffer& data)
{
    if (!data.writeBit(!moveSpline.Finalized()))
        return;

    data.writeBit(moveSpline.splineflags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation));
    data.writeBit((moveSpline.splineflags & MoveSplineFlag::Parabolic) && moveSpline.effect_start_time < moveSpline.Duration());
    data.writeBit(0);
    data.writeBits(moveSpline.getPath().size(), 20);
    data.writeBits(uint8_t(moveSpline.spline.mode()), 2);
    data.writeBits(moveSpline.splineflags.raw(), 25);
}
#endif


// Unlike WriteCreateData/WriteCreateBits above (which embed spline info inside an object-create
// update block that is itself entirely emitted from a per-version buildMovementUpdate() function,
// so compile-time gating is fine there), SmsgMonsterMoveSpline.h dispatches to these functions via
// a RUNTIME m_protocol.isMop() check inside a single function body shared by all versions. Both
// branches of that runtime check must compile regardless of which VERSION_STRING this binary is
// built for, so these overloads (and their helpers) are intentionally NOT wrapped in
// "#if VERSION_STRING == Mop" - unlike WriteCreateData/WriteCreateBits, nothing they reference
// (Unit, MoveSpline, WoWGuid, ByteBuffer) is actually version-gated at compile time, so there is
// no real need to restrict them, and doing so would break non-Mop builds at link time.
// This is a completely different, bit-packed layout compared to the pre-Mop WriteMonsterMove/
// WriteStopMovement above: it embeds the mover's guid via bit-packing, and (since Mop has no
// separate SMSG_MONSTER_MOVE_TRANSPORT opcode) it also always embeds the transport guid, transport
// offsets and vehicle seat directly in the body.
void WriteLinearPathMop(Spline<int32_t> const& spline, ByteBuffer& data)
{
    uint32_t last_idx = static_cast<uint32_t>(spline.getPointCount() - 3);
    G3D::Vector3 const* real_path = &spline.getPoint(1);

    if (last_idx > 0)
    {
        G3D::Vector3 middle = (real_path[0] + real_path[last_idx]) / 2.f;
        G3D::Vector3 offset;
        for (uint32_t i = 0; i < last_idx; ++i)
        {
            offset = middle - real_path[i];
            data.appendPackXYZ(offset.x, offset.y, offset.z);
        }
    }
}

void WriteUncompressedPathMop(Spline<int32_t> const& spline, ByteBuffer& data)
{
    for (int i = 1; i < static_cast<int>(spline.getPointCount()) - 1; ++i)
        data << spline.getPoint(i).y << spline.getPoint(i).x << spline.getPoint(i).z;
}

void WriteUncompressedCyclicPathMop(Spline<int32_t> const& spline, ByteBuffer& data)
{
    data << spline.getPoint(1).y << spline.getPoint(1).x << spline.getPoint(1).z; // fake point, client will erase it from the spline after first cycle done
    for (int i = 1; i < static_cast<int>(spline.getPointCount()) - 3; ++i)
        data << spline.getPoint(i).y << spline.getPoint(i).x << spline.getPoint(i).z;
}

void PacketBuilder::WriteStopMovement([[maybe_unused]] G3D::Vector3 const& pos, [[maybe_unused]] uint32_t splineId, [[maybe_unused]] ByteBuffer& data, [[maybe_unused]] Unit* unit)
{
#if VERSION_STRING >= Mop
    bool const hasVehicle = unit->getVehicle() != nullptr;
    WoWGuid guid = unit->getGuid();
    WoWGuid transport = unit->getTransGuid();

    data << float(pos.z);
    data << float(pos.x);
    data << uint32_t(splineId);
    data << float(pos.y);
    data << float(unit->GetTransOffsetY());
    data << float(unit->GetTransOffsetZ());
    data << float(unit->GetTransOffsetX());

    data.writeBit(1); // Parabolic speed
    data.writeBit(guid[0]);
    data.writeBits(uint32_t(MonsterMoveStop), 3);
    data.writeBit(1);
    data.writeBit(1);
    data.writeBit(!hasVehicle);
    data.writeBits(0, 20);
    data.writeBit(1);
    data.writeBit(guid[3]);
    data.writeBit(1);
    data.writeBit(1);
    data.writeBit(1);
    data.writeBit(1);
    data.writeBit(guid[7]);
    data.writeBit(guid[4]);
    data.writeBit(1);
    data.writeBit(guid[5]);
    data.writeBits(0, 22); // WP count
    data.writeBit(guid[6]);
    data.writeBit(0); // Fake bit
    data.writeBit(transport[7]);
    data.writeBit(transport[1]);
    data.writeBit(transport[3]);
    data.writeBit(transport[0]);
    data.writeBit(transport[6]);
    data.writeBit(transport[4]);
    data.writeBit(transport[5]);
    data.writeBit(transport[2]);
    data.writeBit(0); // Send no block
    data.writeBit(0);
    data.writeBit(guid[2]);
    data.writeBit(guid[1]);

    data.flushBits();

    data.writeByteSeq(guid[1]);
    data.writeByteSeq(transport[6]);
    data.writeByteSeq(transport[4]);
    data.writeByteSeq(transport[1]);
    data.writeByteSeq(transport[7]);
    data.writeByteSeq(transport[0]);
    data.writeByteSeq(transport[3]);
    data.writeByteSeq(transport[5]);
    data.writeByteSeq(transport[2]);
    data.writeByteSeq(guid[5]);
    data.writeByteSeq(guid[3]);
    data.writeByteSeq(guid[6]);
    data.writeByteSeq(guid[0]);
    if (hasVehicle)
        data << uint8_t(unit->GetTransSeat());
    data.writeByteSeq(guid[7]);
    data.writeByteSeq(guid[2]);
    data.writeByteSeq(guid[4]);
#endif
}

void PacketBuilder::WriteMonsterMove([[maybe_unused]] MoveSpline const& moveSpline, [[maybe_unused]] ByteBuffer& data, [[maybe_unused]] Unit* unit)
{
#if VERSION_STRING >= Mop
    bool const hasVehicle = unit->getVehicle() != nullptr;
    WoWGuid guid = unit->getGuid();
    WoWGuid transport = unit->getTransGuid();

    MonsterMoveType type;
    switch (moveSpline.splineflags & MoveSplineFlag::Mask_Final_Facing)
    {
        case MoveSplineFlag::Final_Target: type = MonsterMoveFacingTarget; break;
        case MoveSplineFlag::Final_Angle:  type = MonsterMoveFacingAngle;  break;
        case MoveSplineFlag::Final_Point:  type = MonsterMoveFacingSpot;   break;
        default:                           type = MonsterMoveNormal;      break;
    }

    bool const hasParabolicSpeed = (moveSpline.splineflags & MoveSplineFlag::Parabolic) != 0 &&
        moveSpline.effect_start_time < moveSpline.Duration();
    bool const hasParabolicTime = (moveSpline.splineflags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation)) != 0;

    G3D::Vector3 const& firstPoint = moveSpline.spline.getPoint(moveSpline.spline.first());
    data << float(firstPoint.z);
    data << float(firstPoint.x);
    data << uint32_t(moveSpline.GetId());
    data << float(firstPoint.y);
    data << float(unit->GetTransOffsetY());
    data << float(unit->GetTransOffsetZ());
    data << float(unit->GetTransOffsetX());

    data.writeBit(!hasParabolicSpeed);
    data.writeBit(guid[0]);
    data.writeBits(uint32_t(type), 3);

    if (type == MonsterMoveFacingTarget)
    {
        WoWGuid targetGuid = moveSpline.facing.target;
        data.writeBit(targetGuid[6]);
        data.writeBit(targetGuid[4]);
        data.writeBit(targetGuid[3]);
        data.writeBit(targetGuid[0]);
        data.writeBit(targetGuid[5]);
        data.writeBit(targetGuid[7]);
        data.writeBit(targetGuid[1]);
        data.writeBit(targetGuid[2]);
    }

    data.writeBit(!hasParabolicTime);
    data.writeBit(1);
    data.writeBit(!hasVehicle);

    uint32_t const uncompressedSplineCount = moveSpline.splineflags & MoveSplineFlag::UncompressedPath
        ? (moveSpline.splineflags.cyclic
            ? static_cast<uint32_t>(moveSpline.spline.getPointCount() - 3)
            : static_cast<uint32_t>(moveSpline.spline.getPointCount() - 2))
        : 1;
    data.writeBits(uncompressedSplineCount, 20);

    data.writeBit(!moveSpline.splineflags.raw());
    data.writeBit(guid[3]);
    data.writeBit(1);
    data.writeBit(1);
    data.writeBit(1);
    data.writeBit(!moveSpline.Duration());
    data.writeBit(guid[7]);
    data.writeBit(guid[4]);
    data.writeBit(1);
    data.writeBit(guid[5]);

    int32_t const compressedSplineCount = moveSpline.splineflags & MoveSplineFlag::UncompressedPath
        ? 0
        : static_cast<int32_t>(moveSpline.spline.getPointCount() - 3);
    data.writeBits(compressedSplineCount, 22);

    data.writeBit(guid[6]);
    data.writeBit(0); // Fake bit

    data.writeBit(transport[7]);
    data.writeBit(transport[1]);
    data.writeBit(transport[3]);
    data.writeBit(transport[0]);
    data.writeBit(transport[6]);
    data.writeBit(transport[4]);
    data.writeBit(transport[5]);
    data.writeBit(transport[2]);

    data.writeBit(0);
    data.writeBit(0);
    data.writeBit(guid[2]);
    data.writeBit(guid[1]);

    data.flushBits();

    if (compressedSplineCount)
        WriteLinearPathMop(moveSpline.spline, data);

    data.writeByteSeq(guid[1]);
    data.writeByteSeq(transport[6]);
    data.writeByteSeq(transport[4]);
    data.writeByteSeq(transport[1]);
    data.writeByteSeq(transport[7]);
    data.writeByteSeq(transport[0]);
    data.writeByteSeq(transport[3]);
    data.writeByteSeq(transport[5]);
    data.writeByteSeq(transport[2]);

    if (moveSpline.splineflags & MoveSplineFlag::UncompressedPath)
    {
        if (moveSpline.splineflags.cyclic)
            WriteUncompressedCyclicPathMop(moveSpline.spline, data);
        else
            WriteUncompressedPathMop(moveSpline.spline, data);
    }
    else
    {
        G3D::Vector3 const& point = moveSpline.spline.getPoint(static_cast<int>(moveSpline.spline.getPointCount()) - 2);
        data << point.y << point.x << point.z;
    }

    if (type == MonsterMoveFacingTarget)
    {
        WoWGuid targetGuid = moveSpline.facing.target;
        data.writeByteSeq(targetGuid[5]);
        data.writeByteSeq(targetGuid[7]);
        data.writeByteSeq(targetGuid[0]);
        data.writeByteSeq(targetGuid[4]);
        data.writeByteSeq(targetGuid[3]);
        data.writeByteSeq(targetGuid[2]);
        data.writeByteSeq(targetGuid[6]);
        data.writeByteSeq(targetGuid[1]);
    }

    data.writeByteSeq(guid[5]);

    if (hasParabolicSpeed)
        data << float(moveSpline.vertical_acceleration);

    if (hasParabolicTime)
        data << uint32_t(moveSpline.effect_start_time);

    if (type == MonsterMoveFacingAngle)
        data << float(moveSpline.facing.angle);

    data.writeByteSeq(guid[3]);

    if (moveSpline.splineflags.raw())
        data << uint32_t(moveSpline.splineflags.raw());

    data.writeByteSeq(guid[6]);

    if (type == MonsterMoveFacingSpot)
        data << moveSpline.facing.f.x << moveSpline.facing.f.y << moveSpline.facing.f.z;

    data.writeByteSeq(guid[0]);
    if (hasVehicle)
        data << uint8_t(unit->GetTransSeat());
    data.writeByteSeq(guid[7]);
    data.writeByteSeq(guid[2]);
    data.writeByteSeq(guid[4]);

    if (moveSpline.Duration())
        data << uint32_t(moveSpline.Duration());
#endif
}

#if VERSION_STRING >= WoD
namespace
{
    // the spline flags of 6.2.4 and 7.3.5: the path flags are shared, animation and parabolic moved up, the
    // facing and the animation tier are separate fields
    uint32_t legionSplineFlags(MoveSplineFlag const& flags)
    {
        constexpr uint32_t sharedMask = 0x007FFFF8;             // falling slow .. uncompressed path
        constexpr uint32_t legionAnimation = 0x02000000;
        constexpr uint32_t legionParabolic = 0x04000000;

        uint32_t result = flags.raw() & sharedMask;
        result &= ~uint32_t(MoveSplineFlag::Done);
        if (flags.animation)
            result |= legionAnimation;
        if (flags.parabolic)
            result |= legionParabolic;
        if (flags.cyclic)
            result |= MoveSplineFlag::Enter_Cycle;
        return result;
    }

    // the spline block after the mover: id, destination, 7.x: teleport, tolerance, then the movement spline
    void writeMonsterSplineHeaderLegion(ByteBuffer& data, uint32_t splineId, [[maybe_unused]] uint8_t stopDistanceTolerance)
    {
        data << uint32_t(splineId);
        data << float(0.0f);                                    // destination
        data << float(0.0f);
        data << float(0.0f);
#if VERSION_STRING != WoD
        data.writeBit(false);                                   // cross realm teleport
        data.writeBits(stopDistanceTolerance, 3);
#endif
    }

#if VERSION_STRING == WoD
    // 6.x closes the packet with the teleport bit and the stop tolerance: 2 for a spline without flags
    void writeMonsterSplineTailWoD(ByteBuffer& data, uint32_t splineFlags)
    {
        data.writeBit(false);                                   // cross realm teleport
        data.writeBits(splineFlags != 0 ? 0 : 2, 2);
        data.flushBits();
    }
#endif

    void writeTransportOfMoverLegion(ByteBuffer& data, Unit* unit, uint32_t realmId)
    {
        const WoWGuid transportGuid(unit->getTransGuid());
        data << transportGuid.toGuid128(realmId, unit->GetMapId());
        data << int8_t(transportGuid.isEmpty() ? -1 : unit->GetTransSeat());
    }
}

void PacketBuilder::WriteMonsterMoveLegion(MoveSpline const& moveSpline, ByteBuffer& data, Unit* unit, uint32_t realmId)
{
    MonsterMoveType type;
    switch (moveSpline.splineflags & MoveSplineFlag::Mask_Final_Facing)
    {
        case MoveSplineFlag::Final_Target: type = MonsterMoveFacingTarget; break;
        case MoveSplineFlag::Final_Angle:  type = MonsterMoveFacingAngle;  break;
        case MoveSplineFlag::Final_Point:  type = MonsterMoveFacingSpot;   break;
        default:                           type = MonsterMoveNormal;      break;
    }

    // the face field of 7.3.5: none, spot, target, angle
    uint8_t face = 0;
    switch (type)
    {
        case MonsterMoveFacingSpot:   face = 1; break;
        case MonsterMoveFacingTarget: face = 2; break;
        case MonsterMoveFacingAngle:  face = 3; break;
        default: break;
    }

    G3D::Vector3 const& firstPoint = moveSpline.spline.getPoint(moveSpline.spline.first());
    data << unit->GetNewGUID().toGuid128(realmId, unit->GetMapId());
    data << float(firstPoint.x);
    data << float(firstPoint.y);
    data << float(firstPoint.z);

    writeMonsterSplineHeaderLegion(data, moveSpline.GetId(), 0);

    const bool parabolic = moveSpline.splineflags.parabolic;
    const bool animation = moveSpline.splineflags.animation;
    const bool uncompressed = (moveSpline.splineflags & MoveSplineFlag::UncompressedPath) != 0;

    // the points: the whole path uncompressed, otherwise the last point and the packed offsets of the others
    const uint32_t pointCount = static_cast<uint32_t>(moveSpline.spline.getPointCount());
    uint32_t points = 0;
    uint32_t packedDeltas = 0;
    if (uncompressed)
    {
        points = pointCount - 3 + (moveSpline.splineflags.cyclic ? 1 : 0);
    }
    else
    {
        points = 1;
        const uint32_t lastIdx = pointCount - 3;
        packedDeltas = lastIdx > 1 ? lastIdx - 1 : 0;
    }

    const auto writeFacing = [&]()
    {
        switch (type)
        {
            case MonsterMoveFacingSpot:
                data << moveSpline.facing.f.x << moveSpline.facing.f.y << moveSpline.facing.f.z;
                break;
            case MonsterMoveFacingTarget:
                data << float(0.0f);                                  // face direction
                data << WoWGuid(moveSpline.facing.target).toGuid128(realmId, unit->GetMapId());
                break;
            case MonsterMoveFacingAngle:
                data << float(moveSpline.facing.angle);
                break;
            default:
                break;
        }
    };

    const auto writePoints = [&]()
    {
        if (uncompressed)
        {
            if (moveSpline.splineflags.cyclic)
            {
                data << moveSpline.spline.getPoint(1);                // the client erases it after the first cycle
                for (uint32_t i = 1; i < pointCount - 2; ++i)
                    data << moveSpline.spline.getPoint(i);
            }
            else
            {
                for (uint32_t i = 2; i < pointCount - 1; ++i)
                    data << moveSpline.spline.getPoint(i);
            }
        }
        else
        {
            const uint32_t lastIdx = pointCount - 3;
            G3D::Vector3 const* realPath = &moveSpline.spline.getPoint(1);

            data << realPath[lastIdx];

            if (lastIdx > 1)
            {
                G3D::Vector3 middle = (realPath[0] + realPath[lastIdx]) / 2.f;
                for (uint32_t i = 1; i < lastIdx; ++i)
                {
                    G3D::Vector3 offset = middle - realPath[i];
                    data.appendPackXYZ(offset.x, offset.y, offset.z);
                }
            }
        }
    };

    const uint32_t splineFlags = legionSplineFlags(moveSpline.splineflags);

    data << uint32_t(splineFlags);
#if VERSION_STRING != Shadowlands
    data << uint8_t(animation ? moveSpline.splineflags.animTier : 0);
    data << uint32_t(animation ? moveSpline.effect_start_time : 0);   // tier transition start
#endif
    data << int32_t(0);                                               // elapsed
    data << uint32_t(moveSpline.Duration());
#if VERSION_STRING == BfA || VERSION_STRING == Shadowlands
    data << uint32_t(0);                                              // fade object time
#else
    data << float(parabolic ? moveSpline.vertical_acceleration : 0.0f);
    data << uint32_t(parabolic ? moveSpline.effect_start_time : 0);   // special time
#endif

#if VERSION_STRING == WoD
    // 6.x: the counts are fields, the points come before the facing and the packet ends with the tolerance
    data << int32_t(points);
    data << uint8_t(0);                                               // mode
    data << uint8_t(0);                                               // vehicle exit voluntary
    writeTransportOfMoverLegion(data, unit, realmId);
    data << int32_t(packedDeltas);

    writePoints();

    data.writeBits(face, 2);
    data.writeBit(false);                                             // spline filter
    data.flushBits();

    writeFacing();
    writeMonsterSplineTailWoD(data, splineFlags);
#elif VERSION_STRING == Shadowlands
    // 9.x: the vehicle exit and the interpolation are bits behind the point count, the animation goes through
    // the tier transition block
    data << uint8_t(0);                                               // mode
    writeTransportOfMoverLegion(data, unit, realmId);

    data.writeBits(face, 2);
    data.writeBits(points, 16);
    data.writeBit(false);                                             // vehicle exit voluntary
    data.writeBit(false);                                             // interpolate
    data.writeBits(packedDeltas, 16);
    data.writeBit(false);                                             // spline filter
    data.writeBit(false);                                             // spell effect extra data
    data.writeBit(parabolic);                                         // jump extra data
    data.writeBit(false);                                             // animation tier transition
    data.writeBit(false);                                             // unknown block
    data.flushBits();

    writeFacing();
    writePoints();

    if (parabolic)
    {
        data << float(moveSpline.vertical_acceleration);              // jump gravity
        data << uint32_t(moveSpline.effect_start_time);               // start time
        data << uint32_t(0);                                          // duration
    }
#else
    data << uint8_t(0);                                               // mode
    data << uint8_t(0);                                               // vehicle exit voluntary
    writeTransportOfMoverLegion(data, unit, realmId);

    data.writeBits(face, 2);
    data.writeBits(points, 16);
    data.writeBits(packedDeltas, 16);
    data.writeBit(false);                                             // spline filter
    data.writeBit(false);                                             // spell effect extra data
#if VERSION_STRING == BfA
    data.writeBit(parabolic);                                         // jump extra data
#endif
    data.flushBits();

    writeFacing();
    writePoints();

#if VERSION_STRING == BfA
    if (parabolic)
    {
        data << float(moveSpline.vertical_acceleration);              // jump gravity
        data << uint32_t(moveSpline.effect_start_time);               // start time
        data << uint32_t(0);                                          // duration
    }
#endif
#endif
}

void PacketBuilder::WriteStopMovementLegion(G3D::Vector3 const& pos, uint32_t splineId, ByteBuffer& data, Unit* unit, uint32_t realmId)
{
    data << unit->GetNewGUID().toGuid128(realmId, unit->GetMapId());
    data << float(pos.x);
    data << float(pos.y);
    data << float(pos.z);

    writeMonsterSplineHeaderLegion(data, splineId, 2);

    data << uint32_t(0);                                              // flags
#if VERSION_STRING != Shadowlands
    data << uint8_t(0);                                               // animation tier
    data << uint32_t(0);                                              // tier transition start
#endif
    data << int32_t(0);                                               // elapsed
    data << uint32_t(0);                                              // move time
#if VERSION_STRING == BfA || VERSION_STRING == Shadowlands
    data << uint32_t(0);                                              // fade object time
#else
    data << float(0.0f);                                              // jump gravity
    data << uint32_t(0);                                              // special time
#endif

#if VERSION_STRING == WoD
    data << int32_t(0);                                               // points
    data << uint8_t(0);                                               // mode
    data << uint8_t(0);                                               // vehicle exit voluntary
    writeTransportOfMoverLegion(data, unit, realmId);
    data << int32_t(0);                                               // packed deltas

    data.writeBits(0, 2);                                             // face
    data.writeBit(false);                                             // spline filter
    data.flushBits();

    writeMonsterSplineTailWoD(data, 0);
#elif VERSION_STRING == Shadowlands
    data << uint8_t(0);                                               // mode
    writeTransportOfMoverLegion(data, unit, realmId);

    data.writeBits(0, 2);                                             // face
    data.writeBits(0, 16);                                            // points
    data.writeBit(false);                                             // vehicle exit voluntary
    data.writeBit(false);                                             // interpolate
    data.writeBits(0, 16);                                            // packed deltas
    data.writeBit(false);                                             // spline filter
    data.writeBit(false);                                             // spell effect extra data
    data.writeBit(false);                                             // jump extra data
    data.writeBit(false);                                             // animation tier transition
    data.writeBit(false);                                             // unknown block
    data.flushBits();
#else
    data << uint8_t(0);                                               // mode
    data << uint8_t(0);                                               // vehicle exit voluntary
    writeTransportOfMoverLegion(data, unit, realmId);

    data.writeBits(0, 2);                                             // face
    data.writeBits(0, 16);                                            // points
    data.writeBits(0, 16);                                            // packed deltas
    data.writeBit(false);                                             // spline filter
    data.writeBit(false);                                             // spell effect extra data
#if VERSION_STRING == BfA
    data.writeBit(false);                                             // jump extra data
#endif
    data.flushBits();
#endif
}
#endif

void PacketBuilder::WriteSplineSync(MoveSpline const& move_spline, ByteBuffer& data)
{
    data << (float)move_spline.timePassed() / move_spline.Duration();
}
} // namespace MovementMgr
