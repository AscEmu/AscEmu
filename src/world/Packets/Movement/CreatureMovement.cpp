/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

namespace Packets
{
    namespace Movement
    {
        void SendMoveToPacket(Unit* pUnit)
        {
            ::Packets::Movement::SmsgMonsterMove MovePacket;
            MovePacket.data << pUnit->GetNewGUID();
            MovePacket.data << uint8(0);
            if (pUnit->m_movementManager.m_spline.IsSplineEmpty())
            {
                MovePacket.data << float(pUnit->GetPositionX());
                MovePacket.data << float(pUnit->GetPositionY());
                MovePacket.data << float(pUnit->GetPositionZ());
                MovePacket.data << uint32(getMSTime());
                MovePacket.data << uint8(1);
            }
            else
            {
                auto splineStart = pUnit->m_movementManager.m_spline.GetFirstSplinePoint();
                MovePacket.data << splineStart.pos.x;
                MovePacket.data << splineStart.pos.y;
                MovePacket.data << splineStart.pos.z;
                MovePacket.data << splineStart.setoff;

                if (pUnit->m_movementManager.m_spline.m_splineFaceType.GetFlag() == ::Movement::Spline::MonsterMoveFacingAngle)
                {
                    MovePacket.data << uint8(::Movement::Spline::MonsterMoveFacingAngle);
                    MovePacket.data << pUnit->m_movementManager.m_spline.m_splineFaceType.GetAngle();
                }
                else
                {
                    MovePacket.data << uint8(0);
                }

                MovePacket.data << pUnit->m_movementManager.m_spline.GetSplineFlags();
            }
        }
    }
} 
