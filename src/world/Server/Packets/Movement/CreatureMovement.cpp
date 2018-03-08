/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Units/Unit.h"

namespace Packets
{
    namespace Movement
    {
        void SendMoveToPacket(Unit* pUnit)
        {
            auto spline = pUnit->m_movementManager.m_spline;
            auto splineFlags = spline.GetSplineFlags();
            auto midpoints = pUnit->m_movementManager.m_spline.GetMidPoints();

            WorldPacket data(SMSG_MONSTER_MOVE, 100);
            data << pUnit->GetNewGUID();
            // Id of first spline, so always 0
            data << uint8(0);
            if (spline.IsSplineEmpty())
            {
                data << float(pUnit->GetPositionX());
                data << float(pUnit->GetPositionY());
                data << float(pUnit->GetPositionZ());
                data << uint32(Util::getMSTime());
                data << uint8(1);
            }
            else
            {
                auto splineStart = spline.GetFirstSplinePoint();
                data << splineStart.pos.x;
                data << splineStart.pos.y;
                data << splineStart.pos.z;
                data << splineStart.setoff;

                switch (spline.m_splineFaceType.GetFlag())
                {
                    case ::Movement::Spline::MonsterMoveFacingAngle:
                        data << uint8(::Movement::Spline::MonsterMoveFacingAngle);
                        data << spline.m_splineFaceType.GetAngle();
                        break;
                    case ::Movement::Spline::MonsterMoveFacingLocation:
                        data << uint8(::Movement::Spline::MonsterMoveFacingLocation);
                        data << spline.m_splineFaceType.GetX();
                        data << spline.m_splineFaceType.GetY();
                        data << spline.m_splineFaceType.GetZ();
                        break;
                    case ::Movement::Spline::MonsterMoveFacingTarget:
                        data << uint8(::Movement::Spline::MonsterMoveFacingTarget);
                        data << spline.m_splineFaceType.GetGuid();
                        break;
                    default:
                        data << uint8(0);
                        break;
                }

                splineFlags->m_splineFlagsRaw.enter_cycle = splineFlags->m_splineFlagsRaw.cyclic;

                data << splineFlags->GetFlagsForMonsterMove();

                if (splineFlags->m_splineFlagsRaw.animation)
                {
                    data << splineFlags->m_splineFlagsRaw.animation_id;
                    data << spline.m_splineTrajectoryTime;
                }

                data << spline.m_currentSplineTotalMoveTime;

                if (splineFlags->m_splineFlagsRaw.trajectory)
                {
                    data << spline.m_splineTrajectoryVertical;
                    data << spline.m_splineTrajectoryTime;

                }

                if (splineFlags->m_splineFlagsRaw.catmullrom)
                {
                    auto finalPoint = spline.GetLastSplinePoint();
                    auto pointCount = spline.GetSplinePoints()->size() - 1;
                    if (splineFlags->m_splineFlagsRaw.cyclic)
                    {
                        data << uint32(pointCount + 1);
                        // This is discarded by the client
                        data << finalPoint.pos.x;
                        data << finalPoint.pos.y;
                        data << finalPoint.pos.z;
                        // This is the real data
                        data << finalPoint.pos.x;
                        data << finalPoint.pos.y;
                        data << finalPoint.pos.z;
                    }
                    else
                    {
                        data << uint32(pointCount);
                        data << finalPoint.pos.x;
                        data << finalPoint.pos.y;
                        data << finalPoint.pos.z;
                    }
                }
                else
                {
                    // This is the index of the last spline point
                    // Possibly requires [first,last,other] ordering?
                    data << uint32(spline.GetSplinePoints()->size() - 1);
                    auto finalpoint = pUnit->m_movementManager.m_spline.GetLastSplinePoint();
                    data << finalpoint.pos.x;
                    data << finalpoint.pos.y;
                    data << finalpoint.pos.z;

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
                            data << uint32(int(tmpx) & 0x7FF | ((int(tmpy) & 0x7FF) << 11) | ((int(tmpz) & 0x3FF) << 22));

                        }
                    }
                }
            }

            pUnit->SendMessageToSet(&data, true);
        }
    }
}
