#include "version/Forever/Packets/MovementPackets.hpp"
#include "version/Forever/Packets/Packet.hpp"

#include <limits>

namespace AscEmu::Version::Forever::Packets
{
    namespace
    {
        bool readModernGuid(Packet& packet, WoWGuid& guid)
        {
            if (packet.rpos() >= packet.size())
                return false;

            std::size_t consumed = 0;
            if (!WoWGuid::unpackModern(packet.contents() + packet.rpos(), packet.size() - packet.rpos(), guid, consumed))
                return false;

            packet.rpos(packet.rpos() + consumed);
            return true;
        }

        bool readTransport(Packet& packet, MovementStatus& status)
        {
            if (!readModernGuid(packet, status.transportGuid))
                return false;

            packet >> status.transportPosition.x;
            packet >> status.transportPosition.y;
            packet >> status.transportPosition.z;
            packet >> status.transportPosition.o;
            packet >> status.transportSeat;
            packet >> status.transportTime;

            const bool hasPrevTime = packet.readBit();
            const bool hasVehicleId = packet.readBit();

            if (hasPrevTime)
                packet >> status.transportPrevTime;
            if (hasVehicleId)
                packet >> status.transportVehicleId;

            return !packet.hadReadFailure();
        }
    }

    bool readMovementStatus(Packet& packet, MovementStatus& status)
    {
        if (!readModernGuid(packet, status.moverGuid))
            return false;

        packet >> status.flags;
        packet >> status.moveTime;
        packet >> status.position.x;
        packet >> status.position.y;
        packet >> status.position.z;
        packet >> status.position.o;
        packet >> status.pitch;
        packet >> status.stepUpStartElevation;

        uint32_t removedMovementForces = 0;
        uint32_t moveIndex = 0;
        packet >> removedMovementForces;
        packet >> moveIndex;
        packet >> status.gravityModifier;

        // Each removed movement force is a modern packed ObjectGuid.
        for (uint32_t i = 0; i < removedMovementForces; ++i)
        {
            WoWGuid ignored;
            if (!readModernGuid(packet, ignored))
                return false;
        }

        const bool hasStandingOnGameObject = packet.readBit();
        status.hasTransport = packet.readBit();
        status.hasFallData = packet.readBit();
        packet.readBit(); // HasSpline - not consumed by player movement yet.
        packet.readBit(); // HeightChangeFailed.
        packet.readBit(); // RemoteTimeValid.
        const bool hasInertia = packet.readBit();
        const bool hasAdvancedFlying = packet.readBit();
        const bool hasDriveStatus = packet.readBit();

        if (hasStandingOnGameObject)
        {
            WoWGuid ignored;
            if (!readModernGuid(packet, ignored))
                return false;
        }

        if (status.hasTransport && !readTransport(packet, status))
            return false;

        if (status.hasFallData)
        {
            packet >> status.fallTime;
            packet >> status.fallVelocity;
            status.hasFallDirection = packet.readBit();
            if (status.hasFallDirection)
            {
                packet >> status.fallSinAngle;
                packet >> status.fallCosAngle;
                packet >> status.fallXYSpeed;
            }
        }

        // The base core currently has no matching storage for these modern
        // extensions. Decode/skip them so packet alignment remains correct.
        if (hasInertia)
        {
            uint32_t id = 0;
            float x = 0.0f, y = 0.0f, z = 0.0f;
            uint32_t lifetime = 0;
            packet >> id >> x >> y >> z >> lifetime;
        }

        if (hasAdvancedFlying)
        {
            float forwardVelocity = 0.0f;
            float upVelocity = 0.0f;
            packet >> forwardVelocity >> upVelocity;
        }

        if (hasDriveStatus)
        {
            float speed = 0.0f;
            float movementAngle = 0.0f;
            packet >> speed >> movementAngle;
            packet.readBit(); // accelerating
            packet.readBit(); // drifting
        }

        return !packet.hadReadFailure() && packet.rpos() == packet.size();
    }

    MovementInfo toLegacyMovementInfo(MovementStatus const& status)
    {
        MovementInfo info;
        info.guid = WoWGuid(status.moverGuid.toLegacyRaw());
        info.flags = static_cast<uint32_t>(status.flags & std::numeric_limits<uint32_t>::max());
        info.update_time = status.moveTime;
        info.position = status.position;
        info.pitch_rate = status.pitch;
        info.spline_elevation = status.stepUpStartElevation;
        info.fall_time = status.fallTime;
        info.jump_info.velocity = status.fallVelocity;
        info.jump_info.sinAngle = status.fallSinAngle;
        info.jump_info.cosAngle = status.fallCosAngle;
        info.jump_info.xyspeed = status.fallXYSpeed;

        if (status.hasTransport)
        {
            info.hasTransportData = true;
            info.transport_guid = WoWGuid(status.transportGuid.toLegacyRaw());
            info.transport_position = status.transportPosition;
            info.transport_seat = static_cast<uint8_t>(status.transportSeat);
            info.transport_time = status.transportTime;
            info.transport_time2 = status.transportPrevTime;
            info.transport_time3 = status.transportVehicleId;
        }

        info.status_info.hasFallData = status.hasFallData;
        info.status_info.hasFallDirection = status.hasFallDirection;
        info.status_info.hasPitch = true;
        info.status_info.hasOrientation = true;
        info.status_info.hasTimeStamp = true;
        info.hasMovementFlags = true;
        return info;
    }
}
