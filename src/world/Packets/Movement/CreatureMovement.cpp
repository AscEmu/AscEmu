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
            auto spline = pUnit->m_movementManager.m_spline;
            auto splineFlags = spline.GetSplineFlags();
            auto midpoints = pUnit->m_movementManager.m_spline.GetMidPoints();

            ::Packets::Movement::SmsgMonsterMove MovePacket;
            MovePacket.data << pUnit->GetNewGUID();
            // Id of first spline, so always 0
            MovePacket.data << uint8(0);
            if (spline.IsSplineEmpty())
            {
                MovePacket.data << float(pUnit->GetPositionX());
                MovePacket.data << float(pUnit->GetPositionY());
                MovePacket.data << float(pUnit->GetPositionZ());
                MovePacket.data << uint32(getMSTime());
                MovePacket.data << uint8(1);
            }
            else
            {
                auto splineStart = spline.GetFirstSplinePoint();
                MovePacket.data << splineStart.pos.x;
                MovePacket.data << splineStart.pos.y;
                MovePacket.data << splineStart.pos.z;
                MovePacket.data << splineStart.setoff;

                switch (spline.m_splineFaceType.GetFlag())
                {
                    case ::Movement::Spline::MonsterMoveFacingAngle:
                        MovePacket.data << uint8(::Movement::Spline::MonsterMoveFacingAngle);
                        MovePacket.data << spline.m_splineFaceType.GetAngle();
                        break;
                    case ::Movement::Spline::MonsterMoveFacingLocation:
                        MovePacket.data << uint8(::Movement::Spline::MonsterMoveFacingLocation);
                        MovePacket.data << spline.m_splineFaceType.GetX();
                        MovePacket.data << spline.m_splineFaceType.GetY();
                        MovePacket.data << spline.m_splineFaceType.GetZ();
                        break;
                    case ::Movement::Spline::MonsterMoveFacingTarget:
                        MovePacket.data << uint8(::Movement::Spline::MonsterMoveFacingTarget);
                        MovePacket.data << spline.m_splineFaceType.GetGuid();
                        break;
                    default:
                        MovePacket.data << uint8(0);
                        break;
                }

                splineFlags->m_splineFlagsRaw.enter_cycle = splineFlags->m_splineFlagsRaw.cyclic;
                
                MovePacket.data << splineFlags->GetFlagsForMonsterMove();

                if (splineFlags->m_splineFlagsRaw.animation)
                {
                    MovePacket.data << splineFlags->m_splineFlagsRaw.animation_id;
                    MovePacket.data << spline.m_splineTrajectoryTime;
                }

                MovePacket.data << spline.m_currentSplineTotalMoveTime;

                if (splineFlags->m_splineFlagsRaw.trajectory)
                {
                    MovePacket.data << spline.m_splineTrajectoryVertical;
                    MovePacket.data << spline.m_splineTrajectoryTime;

                }

                if (splineFlags->m_splineFlagsRaw.catmullrom)
                {
                    auto finalPoint = spline.GetLastSplinePoint();
                    auto pointCount = spline.GetSplinePoints()->size() - 1;
                    if (splineFlags->m_splineFlagsRaw.cyclic)
                    {
                        MovePacket.data << uint32(pointCount + 1);
                        // This is discarded by the client
                        MovePacket.data << finalPoint.pos.x;
                        MovePacket.data << finalPoint.pos.y;
                        MovePacket.data << finalPoint.pos.z;
                        // This is the real data
                        MovePacket.data << finalPoint.pos.x;
                        MovePacket.data << finalPoint.pos.y;
                        MovePacket.data << finalPoint.pos.z;
                    }
                    else
                    {
                        MovePacket.data << uint32(pointCount);
                        MovePacket.data << finalPoint.pos.x;
                        MovePacket.data << finalPoint.pos.y;
                        MovePacket.data << finalPoint.pos.z;
                    }
                }
                else
                {
                    // This is the index of the last spline point
                    // Possibly requires [first,last,other] ordering?
                    MovePacket.data << uint32(spline.GetSplinePoints()->size() - 1);
                    auto finalpoint = pUnit->m_movementManager.m_spline.GetLastSplinePoint();
                    MovePacket.data << finalpoint.pos.x;
                    MovePacket.data << finalpoint.pos.y;
                    MovePacket.data << finalpoint.pos.z;
                    
                    // If there are more than 2 points
                    if (spline.GetSplinePoints()->size() > 2)
                    {
                        float midx = (finalpoint.pos.x + splineStart.pos.x) * 0.5f;
                        float midy = (finalpoint.pos.y + splineStart.pos.y) * 0.5f;
                        float midz = (finalpoint.pos.z + splineStart.pos.z) * 0.5f;

                        for (auto point : midpoints)
                        {
                            float tmpx = (midx - point.pos.x) * 4;
                            float tmpy = (midy - point.pos.y) * 4;
                            float tmpz = (midz - point.pos.z) * 4;

                            //pack it
                            MovePacket.data << uint32(int(tmpx) & 0x7FF | ((int(tmpy) & 0x7FF) << 11) | ((int(tmpz) & 0x3FF) << 22));
                            
                        }
                    }
                }
            }

            pUnit->SendMessageToSet(&MovePacket.data, true);
        }
    }
} 
