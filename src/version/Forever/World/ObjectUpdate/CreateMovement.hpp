/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <span>

class ByteBuffer;

namespace AscEmu::Version::Forever::ObjectUpdate::Detail
{
    void writeStationaryUnitMovement(ByteBuffer& data, std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, uint32_t movementTimeMs);
    void writeStationaryGameObjectMovement(ByteBuffer& data, float x, float y, float z, float orientation, int64_t packedLocalRotation);
    void writeItemMovement(ByteBuffer& data);
    void writePlayerMovement(ByteBuffer& data, std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation);
}
