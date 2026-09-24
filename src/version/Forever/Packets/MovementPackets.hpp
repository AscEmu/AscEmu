#pragma once

#include "version/Forever/Packets/Packet.hpp"
#include "Objects/MovementInfo.hpp"
#include "shared/WoWGuid.hpp"

#include <cstdint>

namespace AscEmu::Version::Forever::Packets
{
    struct MovementStatus
    {
        WoWGuid moverGuid{};
        uint64_t flags{0};
        uint32_t moveTime{0};
        LocationVector position{};
        float pitch{0.0f};
        float stepUpStartElevation{0.0f};
        float gravityModifier{1.0f};

        bool hasTransport{false};
        WoWGuid transportGuid{};
        LocationVector transportPosition{};
        int8_t transportSeat{-1};
        uint32_t transportTime{0};
        uint32_t transportPrevTime{0};
        uint32_t transportVehicleId{0};

        bool hasFallData{false};
        uint32_t fallTime{0};
        float fallVelocity{0.0f};
        bool hasFallDirection{false};
        float fallSinAngle{0.0f};
        float fallCosAngle{0.0f};
        float fallXYSpeed{0.0f};
    };

    // Forever 69913 uses the modern retail MovementInfo wire layout.  Keep the
    // decoder version-local instead of forcing this data through the legacy
    // MovementCodec/MoP descriptor tables.
    bool readMovementStatus(Packet& packet, MovementStatus& status);
    MovementInfo toLegacyMovementInfo(MovementStatus const& status);
}
