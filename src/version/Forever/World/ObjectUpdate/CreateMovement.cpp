/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CreateMovement.hpp"

#include "../ProtocolLayout.hpp"

#include "Network/ByteBuffer.hpp"

#include <array>
#include <cstring>

namespace AscEmu::Version::Forever::ObjectUpdate::Detail
{
    namespace
    {
        // Forever minimal stationary-unit create movement profile.
        // The variable GUID and position/orientation are generated per object;
        // this suffix was observed byte-identical across multiple stationary
        // Forever stationary retail creature samples. Its individual fields are
        // intentionally left semantically unnamed until separately proven.
        inline constexpr std::array<uint8_t, 135> STATIONARY_UNIT_MOVEMENT_SUFFIX = {
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x80,0x3F,0x00,0x00,0x00,0x00,0x20,0x40,0x00,0x00,0x00,0x41,0x00,0x00,
            0x90,0x40,0x71,0x1C,0x97,0x40,0x00,0x00,0x20,0x40,0x00,0x00,0xE0,0x40,0x00,0x00,
            0x90,0x40,0xDB,0x0F,0x49,0x40,0xDB,0x0F,0x49,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
            0x80,0x3F,0x00,0x00,0x00,0x40,0x00,0x00,0x82,0x42,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x40,0x40,0x00,0x00,0x20,0x41,0x00,0x00,0xC8,0x42,0xDB,0x0F,0xC9,0x3F,0xAA,0x61,
            0x1C,0x40,0xDB,0x0F,0x49,0x40,0xDB,0x0F,0xC9,0x40,0xDB,0x0F,0xC9,0x3F,0xE4,0xCB,
            0x96,0x40,0x00,0x00,0xF0,0x41,0x00,0x00,0xA0,0x42,0x00,0x00,0x30,0x40,0x00,0x00,
            0xE0,0x40,0xCD,0xCC,0xCC,0x3E,0x00
        };

        // Forever self-player movement defaults. The live position and movement
        // flags are patched by the serializer below; the remaining bytes are
        // Forever protocol defaults whose semantics are not yet named.
        inline constexpr std::array<uint8_t, 181> SELF_PLAYER_MOVEMENT_DEFAULTS = {
            0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0xC5,0x1C,0x5A,0xBA,0xCD,0xD7,0x0B,0xC6,
            0x35,0x7E,0x04,0xC3,0xF9,0x0F,0xA7,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,
            0x10,0x00,0x00,0x00,0x20,0x40,0x00,0x00,0xE0,0x40,0x00,0x00,0x90,0x40,0x71,0x1C,
            0x97,0x40,0x00,0x00,0x20,0x40,0x00,0x00,0xE0,0x40,0x00,0x00,0x90,0x40,0xDB,0x0F,
            0x49,0x40,0xDB,0x0F,0x49,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x00,0x40,0x00,0x00,0x82,0x42,0x00,0x00,0x80,0x3F,0x00,0x00,0x40,0x40,0x00,0x00,
            0x20,0x41,0x00,0x00,0xC8,0x42,0xDB,0x0F,0xC9,0x3F,0xAA,0x61,0x1C,0x40,0xDB,0x0F,
            0x49,0x40,0xDB,0x0F,0xC9,0x40,0xDB,0x0F,0xC9,0x3F,0xE4,0xCB,0x96,0x40,0x00,0x00,
            0xF0,0x41,0x00,0x00,0xA0,0x42,0x00,0x00,0x30,0x40,0x00,0x00,0xE0,0x40,0xCD,0xCC,
            0xCC,0x3E,0x80,0xB5,0x16,0x6C,0x00,0xCD,0xD7,0x0B,0xC6,0x35,0x7E,0x04,0xC3,0xF9,
            0x0F,0xA7,0x42,0x00,0x00
        };
    }

    void writeStationaryUnitMovement(ByteBuffer& data, std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, uint32_t movementTimeMs)
    {
        // Capture-proven minimal stationary creature layout:
        //   7-byte fixed prefix
        //   ModernGUID
        //   8-byte zero block
        //   movement timestamp (uint32 ms)
        //   x/y/z/orientation
        //   byte-stable stationary movement suffix
        data.append(ProtocolLayout::Movement::StationaryUnitPrefix.data(), ProtocolLayout::Movement::StationaryUnitPrefix.size());
        data.append(packedGuid.data(), packedGuid.size());
        data.append(ProtocolLayout::Movement::StationaryUnitZeroBlock.data(), ProtocolLayout::Movement::StationaryUnitZeroBlock.size());
        data << movementTimeMs;
        data << x << y << z << orientation;
        data.append(STATIONARY_UNIT_MOVEMENT_SUFFIX.data(), STATIONARY_UNIT_MOVEMENT_SUFFIX.size());
    }

    void writeStationaryGameObjectMovement(ByteBuffer& data, float x, float y, float z, float orientation, int64_t packedLocalRotation)
    {
        // Forever stationary GameObject movement is 31 bytes:
        // flags(3), transport/time placeholder(4), position+orientation(16),
        // packed local rotation(8). This shape is byte-stable across the
        // stationary GameObjects present in the reference captures.
        data.append(ProtocolLayout::Movement::StationaryGameObjectFlags.data(), ProtocolLayout::Movement::StationaryGameObjectFlags.size());
        data << uint32_t(0);
        data << x << y << z << orientation;
        data << packedLocalRotation;
    }

    void writeItemMovement(ByteBuffer& data)
    {
        // Forever inventory-item creates carry a seven-byte zero movement header before the field payload length.
        data.append(ProtocolLayout::Movement::ItemCreateHeader.data(), ProtocolLayout::Movement::ItemCreateHeader.size());
    }

    void writePlayerMovement(ByteBuffer& data, std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation)
    {
        std::array<uint8_t, SELF_PLAYER_MOVEMENT_DEFAULTS.size()> movementTail = SELF_PLAYER_MOVEMENT_DEFAULTS;

        // The protocol default state was rooted. Do not inherit that fixed
        // movement-control state for our generated player.
        uint32_t movementFlags = 0;
        std::memcpy(&movementFlags, movementTail.data(), sizeof(movementFlags));
        movementFlags &= ~ProtocolLayout::Movement::RootedMovementFlag;
        std::memcpy(movementTail.data(), &movementFlags, sizeof(movementFlags));

        // Forever carries the self position twice in the CreateObject2
        // movement block. Keep MovementInfo and EntityPosition synchronized.
        std::memcpy(movementTail.data() + ProtocolLayout::Movement::PlayerMovementPositionOffset + 0, &x, sizeof(float));
        std::memcpy(movementTail.data() + ProtocolLayout::Movement::PlayerMovementPositionOffset + 4, &y, sizeof(float));
        std::memcpy(movementTail.data() + ProtocolLayout::Movement::PlayerMovementPositionOffset + 8, &z, sizeof(float));
        std::memcpy(movementTail.data() + ProtocolLayout::Movement::PlayerMovementPositionOffset + 12, &orientation, sizeof(float));
        std::memcpy(movementTail.data() + ProtocolLayout::Movement::PlayerEntityPositionOffset + 0, &x, sizeof(float));
        std::memcpy(movementTail.data() + ProtocolLayout::Movement::PlayerEntityPositionOffset + 4, &y, sizeof(float));
        std::memcpy(movementTail.data() + ProtocolLayout::Movement::PlayerEntityPositionOffset + 8, &z, sizeof(float));

        for (uint8_t value : ProtocolLayout::Movement::PlayerCreatePrefix)
            data << value;
        data << uint32_t(0);
        data.append(packedGuid.data(), packedGuid.size());
        data.append(movementTail.data(), movementTail.size());
    }
}
