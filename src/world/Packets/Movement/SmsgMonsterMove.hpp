/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _PACKET_SMSG_MONSTER_MOVE_HPP
#define _PACKET_SMSG_MONSTER_MOVE_HPP

#include "StdAfx.h"
#include "Opcodes.h"

namespace Packets
{
    namespace Movement
    {
        class SmsgMonsterMove : Packets::ManagedPacket
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

                SmsgMonsterMove() {};
                SmsgMonsterMove(uint32 pSize) : ManagedPacket(SMSG_MONSTER_MOVE, pSize) {}
        };
    }
}

#endif // _PACKET_SMSG_MONSTER_MOVE_HPP
