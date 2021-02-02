/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

class ByteBuffer;

namespace G3D
{
    class Vector3;
}

namespace MovementNew {

class MoveSpline;
class PacketBuilder
{
    static void WriteCommonMonsterMovePart(MoveSpline const& mov, ByteBuffer& data);
public:
    static void WriteMonsterMove(MoveSpline const& mov, ByteBuffer& data);
    static void WriteStopMovement(G3D::Vector3 const& loc, uint32_t splineId, ByteBuffer& data);
    static void WriteCreate(MoveSpline const& mov, ByteBuffer& data);
    static void WriteSplineSync(MoveSpline const& mov, ByteBuffer& data);
};
} // namespace MovementNew
