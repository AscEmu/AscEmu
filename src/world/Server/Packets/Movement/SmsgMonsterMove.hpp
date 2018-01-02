/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Packets/Opcode.h"
#include "Movement/UnitMovementManager.hpp"
#include "Server/Packets/ManagedPacket.hpp"

namespace Packets
{
    namespace Movement
    {
        class SmsgMonsterMove : public Packets::ManagedPacket
        {
            public:

                WoWGuid m_Guid;
                uint8 m_unk0;
                ::Movement::Point m_point;
                uint32 m_splineId;
                ::Movement::Spline::MonsterMoveFaceType m_moveType;
                uint32 m_splineFlags;
                ::Movement::Spline::SplineAnimation m_splineAnimation;
                int32 m_splineDuration;
                ::Movement::Spline::SplineParabolic m_splineParabolic;
                uint32 m_pointCount;
                std::vector<::Movement::Point> m_locations;

                SmsgMonsterMove() : ManagedPacket(SMSG_MONSTER_MOVE, 100) {};
                SmsgMonsterMove(uint32 pSize) : ManagedPacket(SMSG_MONSTER_MOVE, pSize) {}
        };
    }
}
