/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "MoveSplineInitArgs.h"
#include <G3D/Vector3.h>

struct SERVER_DECL SplineChainLink
{
    SplineChainLink(MovementNew::PointsArray const& points, uint32_t expectedDuration, uint32_t msToNext, float velocity) : Points(points), ExpectedDuration(expectedDuration), TimeToNext(msToNext), Velocity(velocity) { }
    template <typename iteratorType> SplineChainLink(iteratorType begin, iteratorType end, uint32_t expectedDuration, uint32_t msToNext, float velocity) : Points(begin, end), ExpectedDuration(expectedDuration), TimeToNext(msToNext), Velocity(velocity) { }
    SplineChainLink(uint32_t expectedDuration, uint32_t msToNext, float velocity) : Points(), ExpectedDuration(expectedDuration), TimeToNext(msToNext), Velocity(velocity) { }
    MovementNew::PointsArray Points;
    uint32_t ExpectedDuration;
    uint32_t TimeToNext;
    float Velocity;
};

struct SERVER_DECL SplineChainResumeInfo
{
    SplineChainResumeInfo() : PointID(0), Chain(nullptr), IsWalkMode(false), SplineIndex(0), PointIndex(0), TimeToNext(0) { }
    SplineChainResumeInfo(uint32_t id, std::vector<SplineChainLink> const* chain, bool walk, uint8_t splineIndex, uint8_t wpIndex, uint32_t msToNext) :
        PointID(id), Chain(chain), IsWalkMode(walk), SplineIndex(splineIndex), PointIndex(wpIndex), TimeToNext(msToNext) { }
    bool Empty() const { return Chain == nullptr; }
    void Clear() { Chain = nullptr; }
    uint32_t PointID;
    std::vector<SplineChainLink> const* Chain;
    bool IsWalkMode;
    uint8_t SplineIndex;
    uint8_t PointIndex;
    uint32_t TimeToNext;
};
