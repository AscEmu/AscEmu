/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// shared
#include <LocationVector.h>
#include <WorldPacket.h>

#include <cstdint>
#include <string>

class SpellCastTargets
{
    LocationVector m_source = LocationVector();
    LocationVector m_destination = LocationVector();

    void reset();
public:
    uint16_t m_targetMask = 0;
    uint16_t m_targetMaskExtended = 0;
    uint64_t m_unitTarget = 0;
    uint64_t m_itemTarget = 0;

    uint64_t unkuint64_1 = 0;
    uint64_t unkuint64_2 = 0;
    std::string m_strTarget = std::string();

    void read(WorldPacket & data, uint64 caster);
    void write(WorldPacket & data) const;

    LocationVector source() const;
    LocationVector destination() const;

    void setSource(LocationVector source);
    void setDestination(LocationVector destination);

    SpellCastTargets();
    SpellCastTargets(uint16_t TargetMask, uint64_t unitTarget, uint64_t itemTarget, LocationVector source, LocationVector destination);
    SpellCastTargets(uint64_t unitTarget);
    SpellCastTargets(WorldPacket& data, uint64_t caster);
    SpellCastTargets& operator=(const SpellCastTargets& target);
    ~SpellCastTargets();

    uint32 GetTargetMask() const;
    bool hasSource() const;

    bool hasDestination() const;
};
