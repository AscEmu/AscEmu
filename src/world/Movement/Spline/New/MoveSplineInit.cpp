/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MoveSplineInit.h"
#include "../world/Units/Creatures/Creature.h"
#include "../world/Units/Creatures/Vehicle.h"
#include "MoveSpline.h"
#include "MovementPacketBuilder.h"
#include "../world/Units/Unit.h"
#include "../world/Objects/Transporter.h"
#include "WorldPacket.h"

namespace MovementNew {

UnitSpeedType SelectSpeedType(uint32_t moveFlags)
{
    if (moveFlags & MOVEFLAG_FLYING)
    {
        if (moveFlags & MOVEFLAG_MOVE_BACKWARD )
            return TYPE_FLY_BACK;
        else
            return TYPE_FLY;
    }
    else if (moveFlags & MOVEFLAG_SWIMMING)
    {
        if (moveFlags & MOVEFLAG_MOVE_BACKWARD)
            return TYPE_SWIM_BACK;
        else
            return TYPE_SWIM;
    }
    else if (moveFlags & MOVEFLAG_WALK)
    {
        return TYPE_WALK;
    }
    else if (moveFlags & MOVEFLAG_MOVE_BACKWARD)
    {
        return TYPE_RUN_BACK;
    }

    // Flying creatures use MOVEFLAG_CAN_FLY or MOVEFLAG_DISABLE_GRAVITY
    // Run speed is their default flight speed.
    return TYPE_RUN;
}

int32_t MoveSplineInit::Launch()
{
    MoveSpline& move_spline = *unit->movespline;

    // Elevators also use MOVEFLAG_TRANSPORT but we do not keep track of their position changes
    bool transport = unit->hasUnitMovementFlag(MOVEFLAG_TRANSPORT) && unit->GetTransport();
    Location real_position;
    // there is a big chance that current position is unknown if current state is not finalized, need compute it
    // this also allows CalculatePath spline position and update map position in much greater intervals
    // Don't compute for transport movement if the unit is in a motion between two transports
    if (!move_spline.Finalized() && move_spline.onTransport == transport)
    {
        real_position = move_spline.ComputePosition();
    }
    else
    {
        LocationVector const pos = transport ? unit->obj_movement_info.transport_position : unit->GetPosition();
        real_position.x = pos.getPositionX();
        real_position.y = pos.getPositionY();
        real_position.z = pos.getPositionZ();
        real_position.orientation = unit->GetOrientation();
    }

    // should i do the things that user should do? - no.
    if (args.path.empty())
        return 0;

    // corrent first vertex
    args.path[0] = real_position;
    args.initialOrientation = real_position.orientation;
    args.flags.enter_cycle = args.flags.cyclic;
    move_spline.onTransport = transport;

    uint32_t moveFlags = unit->obj_movement_info.getMovementFlags();

#if VERSION_STRING <= WotLK
    moveFlags |= MOVEFLAG_SPLINE_ENABLED;
#endif

    if (!args.flags.backward)
        moveFlags = (moveFlags & ~(MOVEFLAG_MOVE_BACKWARD)) | MOVEFLAG_MOVE_FORWARD;
    else
        moveFlags = (moveFlags & ~(MOVEFLAG_MOVE_FORWARD)) | MOVEFLAG_MOVE_BACKWARD;

    if (moveFlags & MOVEFLAG_ROOTED)
        moveFlags &= ~MOVEFLAG_MOVING_MASK;

    if (!args.HasVelocity)
    {
        // If spline is initialized with SetWalk method it only means we need to select
        // walk move speed for it but not add walk flag to unit
        uint32_t moveFlagsForSpeed = moveFlags;
        if (args.walk)
            moveFlagsForSpeed |= MOVEFLAG_WALK;
        else
            moveFlagsForSpeed &= ~MOVEFLAG_WALK;

        args.velocity = unit->getSpeedRate(SelectSpeedType(moveFlagsForSpeed), true);
        if (unit->isCreature())
            if (static_cast<Creature*>(unit)->GetAIInterface()->hasCalledForHelp())
                args.velocity *= 0.66f;
    }

    // limit the speed in the same way the client does
    args.velocity = std::min(args.velocity, args.flags.catmullrom || args.flags.flying ? 50.0f : std::max(28.0f, unit->getSpeedRate(TYPE_RUN,  true) * 4.0f));

    if (!args.Validate(unit))
        return 0;

    unit->obj_movement_info.flags = moveFlags;
    move_spline.Initialize(args);

    WorldPacket data(SMSG_MONSTER_MOVE, 64);
    data << WoWGuid(unit->getGuid());
    if (transport)
    {
        data.SetOpcode(SMSG_MONSTER_MOVE_TRANSPORT);
        data << WoWGuid(unit->getTransGuid());
#if VERSION_STRING >= WotLK
        data << int8_t(unit->GetTransSeat());
#endif
    }

    PacketBuilder::WriteMonsterMove(move_spline, data);
    unit->SendMessageToSet(&data, true);

    return move_spline.Duration();
}

void MoveSplineInit::Stop()
{
    MoveSpline& move_spline = *unit->movespline;

    // No need to stop if we are not moving
    if (move_spline.Finalized())
        return;

    bool transport = unit->hasUnitMovementFlag(MOVEFLAG_TRANSPORT) && unit->getTransGuid();
    Location loc;
    if (move_spline.onTransport == transport)
    {
        loc = move_spline.ComputePosition();
    }
    else
    {
        LocationVector const pos = transport ? unit->obj_movement_info.transport_position : unit->GetPosition();
        loc.x = pos.getPositionX();
        loc.y = pos.getPositionY();
        loc.z = pos.getPositionZ();
        loc.orientation = unit->GetOrientation();
    }

    args.flags = MoveSplineFlag::Done;

#if VERSION_STRING <= WotLK
    unit->obj_movement_info.removeMovementFlag(MOVEFLAG_SPLINE_FORWARD_ENABLED);
#else
    unit->obj_movement_info.removeMovementFlag(MOVEFLAG_MOVE_FORWARD);
#endif
    move_spline.onTransport = transport;
    move_spline.Initialize(args);

    WorldPacket data(SMSG_MONSTER_MOVE, 64);
    data << WoWGuid(unit->getGuid());
    if (transport)
    {
        data.SetOpcode(SMSG_MONSTER_MOVE_TRANSPORT);
        data << WoWGuid(unit->getTransGuid());
#if VERSION_STRING >= WotLK
        data << int8_t(unit->GetTransSeat());
#endif
    }

    PacketBuilder::WriteStopMovement(loc, args.splineId, data);
    unit->SendMessageToSet(&data, true);
}

MoveSplineInit::MoveSplineInit(Unit* m) : unit(m)
{
    args.splineId = splineIdGen.NewId();
    // Elevators also use MOVEFLAG_TRANSPORT but we do not keep track of their position changes
    args.TransformForTransport = unit->hasUnitMovementFlag(MOVEFLAG_TRANSPORT) && unit->getTransGuid();
    // mix existing state into new
    args.flags.canswim = unit->CanSwim();
    args.walk = unit->hasUnitMovementFlag(MOVEFLAG_WALK);
    args.flags.flying = unit->obj_movement_info.hasMovementFlag(MOVEFLAG_FLYING_MASK);
}

MoveSplineInit::~MoveSplineInit() = default;

void MoveSplineInit::SetFacing(Vector3 const& spot)
{
    TransportPathTransform transform(unit, args.TransformForTransport);
    Vector3 finalSpot = transform(spot);
    args.facing.f.x = finalSpot.x;
    args.facing.f.y = finalSpot.y;
    args.facing.f.z = finalSpot.z;
    args.flags.EnableFacingPoint();
}

void MoveSplineInit::SetFacing(Unit const* target)
{
    args.flags.EnableFacingTarget();
    args.facing.target = target->getGuid();
}

void MoveSplineInit::SetFacing(float angle)
{
    if (args.TransformForTransport)
    {
        if (Unit* vehicle = unit->getVehicleBase())
            angle -= vehicle->GetOrientation();
        else if (Transporter* transport = unit->GetTransport())
            angle -= transport->GetOrientation();
    }

    args.facing.angle = G3D::wrap(angle, 0.f, (float)G3D::twoPi());
    args.flags.EnableFacingAngle();
}

void MoveSplineInit::MovebyPath(PointsArray const& controls, int32 path_offset)
{
    args.path_Idx_offset = path_offset;
    args.path.resize(controls.size());
    std::transform(controls.begin(), controls.end(), args.path.begin(), TransportPathTransform(unit, args.TransformForTransport));
}

void MoveSplineInit::MoveTo(float x, float y, float z, bool generatePath, bool forceDestination)
{
    MoveTo(G3D::Vector3(x, y, z), generatePath, forceDestination);
}

void MoveSplineInit::MoveTo(Vector3 const& dest, bool generatePath, bool forceDestination)
{
    if (generatePath)
    {
        // ToDo
    }

    args.path_Idx_offset = 0;
    args.path.resize(2);
    TransportPathTransform transform(unit, args.TransformForTransport);
    args.path[1] = transform(dest);
}

Vector3 TransportPathTransform::operator()(Vector3 input)
{
    if (_transformForTransport)
    {
        if (TransportBase* transport = _owner->getCurrentVehicle())
        {
            transport->CalculatePassengerOffset(input.x, input.y, input.z);
        }
        else if (TransportBase* transport = _owner->GetTransport())
        {
            transport->CalculatePassengerOffset(input.x, input.y, input.z);
        }
    }
    return input;
}
} // namespace MovementNew
