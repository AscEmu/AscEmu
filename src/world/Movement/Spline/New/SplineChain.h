/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef SPLINECHAIN_H
#define SPLINECHAIN_H

#include "MoveSplineInitArgs.h"
#include <G3D/Vector3.h>

struct SERVER_DECL SplineChainLink
{
    SplineChainLink(MovementNew::PointsArray const& points, uint32 expectedDuration, uint32 msToNext, float velocity) : Points(points), ExpectedDuration(expectedDuration), TimeToNext(msToNext), Velocity(velocity) { }
    template <typename iteratorType> SplineChainLink(iteratorType begin, iteratorType end, uint32 expectedDuration, uint32 msToNext, float velocity) : Points(begin, end), ExpectedDuration(expectedDuration), TimeToNext(msToNext), Velocity(velocity) { }
    SplineChainLink(uint32 expectedDuration, uint32 msToNext, float velocity) : Points(), ExpectedDuration(expectedDuration), TimeToNext(msToNext), Velocity(velocity) { }
    MovementNew::PointsArray Points;
    uint32 ExpectedDuration;
    uint32 TimeToNext;
    float Velocity;
};

struct SERVER_DECL SplineChainResumeInfo
{
    SplineChainResumeInfo() : PointID(0), Chain(nullptr), IsWalkMode(false), SplineIndex(0), PointIndex(0), TimeToNext(0) { }
    SplineChainResumeInfo(uint32 id, std::vector<SplineChainLink> const* chain, bool walk, uint8 splineIndex, uint8 wpIndex, uint32 msToNext) :
        PointID(id), Chain(chain), IsWalkMode(walk), SplineIndex(splineIndex), PointIndex(wpIndex), TimeToNext(msToNext) { }
    bool Empty() const { return Chain == nullptr; }
    void Clear() { Chain = nullptr; }
    uint32 PointID;
    std::vector<SplineChainLink> const* Chain;
    bool IsWalkMode;
    uint8 SplineIndex;
    uint8 PointIndex;
    uint32 TimeToNext;
};

#endif
