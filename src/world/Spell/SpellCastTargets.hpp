/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LocationVector.hpp"
#include "WoWGuid.hpp"
#include "AEVersion.hpp"

#include <cstdint>
#include <string>

class WorldPacket;

class SpellCastTargets
{
private:
    uint32_t m_targetMask = 0;

    WoWGuid m_gameObjectTargetGuid = WoWGuid();
    WoWGuid m_unitTargetGuid = WoWGuid();
    WoWGuid m_itemTargetGuid = WoWGuid();
    WoWGuid m_transportSourceGuid = WoWGuid();
    WoWGuid m_transportDestinationGuid = WoWGuid();

    LocationVector m_source = LocationVector();
    LocationVector m_destination = LocationVector();

    std::string m_strTarget = std::string();

    void reset();

public:
    SpellCastTargets() = default;
    SpellCastTargets(uint64_t unitTarget);
    SpellCastTargets& operator=(const SpellCastTargets& target);
    ~SpellCastTargets();

#if VERSION_STRING < Mop
    void read(WorldPacket& data);
    void write(WorldPacket& data) const;
#endif

    bool isEmpty() const;
    bool hasSource() const;
    bool hasDestination() const;
    bool isTradeItem() const;

    uint32_t getTargetMask() const;
    void setTargetMask(uint32_t mask);
    void addTargetMask(uint32_t mask);

    void setStringTarget(const std::string& str);
    std::string getStringTarget() const;

    uint64_t getGameObjectTargetGuid() const;
    uint64_t getUnitTargetGuid() const;
    uint64_t getItemTargetGuid() const;

    uint64_t getTransportSourceGuid() const;
    LocationVector getSource() const;

    uint64_t getTransportDestinationGuid() const;
    LocationVector getDestination() const;

    void setGameObjectTarget(uint64_t guid);
    void setUnitTarget(uint64_t guid);
    void setItemTarget(uint64_t guid);
    void setTransportSourceGuid(uint64_t guid);
    void setSource(LocationVector source);
    void setTransportDestinationGuid(uint64_t guid);
    void setDestination(LocationVector destination);
};
