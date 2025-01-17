/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <vector>

#include "Movement/Spline/SplineChain.h"
#include "Movement/MovementGenerator.h"
#include "Movement/MovementDefines.h"

class Unit;

class SERVER_DECL SplineChainMovementGenerator : public MovementGenerator
{
public:
    explicit SplineChainMovementGenerator(uint32_t id, std::vector<SplineChainLink> const& chain, bool walk = false);
    explicit SplineChainMovementGenerator(SplineChainResumeInfo const& info);

    void initialize(Unit*) override;
    void reset(Unit*) override;
    bool update(Unit*, uint32_t) override;
    void deactivate(Unit*) override;
    void finalize(Unit*, bool, bool) override;
    MovementGeneratorType getMovementGeneratorType() const override;

    // Builds info that can later be used to resume this spline chain movement at the current position
    static void getResumeInfo(SplineChainResumeInfo& info, Unit const* owner, Optional<uint32_t> id = {});
    // Leaving the object method public for people that know what they're doing to use
    // But really, 99% of the time you should be using the static one instead
    SplineChainResumeInfo getResumeInfo(Unit const* owner) const;
    uint32_t getId() const { return _id; }

private:
    void sendSplineFor(Unit* owner, uint32_t index, uint32_t& duration);
    uint32_t sendPathSpline(Unit* owner, float velocity, MovementMgr::PointsArray const& path) const;

    uint32_t const _id;
    std::vector<SplineChainLink> const& _chain;
    uint8_t const _chainSize;
    bool const _walk;
    uint8_t _nextIndex;
    uint8_t _nextFirstWP; // only used for resuming
    uint32_t _msToNext;
};
