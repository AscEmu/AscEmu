/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Definitions/SpellCastTargetFlags.hpp"
#include "SpellCastTargets.hpp"
#include "Objects/Transporter.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/TransporterHandler.hpp"
#include "Server/WorldSocket.h"

void SpellCastTargets::reset()
{
    m_targetMask = 0;
    m_gameObjectTargetGuid = WoWGuid();
    m_unitTargetGuid = WoWGuid();
    m_itemTargetGuid = WoWGuid();
    m_transportSourceGuid = WoWGuid();
    m_transportDestinationGuid = WoWGuid();
    m_source = LocationVector();
    m_destination = LocationVector();
    m_strTarget = std::string();
}

SpellCastTargets::SpellCastTargets(uint64_t unitTarget) : m_targetMask(TARGET_FLAG_UNIT),
m_unitTargetGuid(unitTarget)
{
}

SpellCastTargets& SpellCastTargets::operator=(const SpellCastTargets& target)
{
    m_gameObjectTargetGuid = target.getGameObjectTargetGuid();
    m_unitTargetGuid = target.getUnitTargetGuid();
    m_itemTargetGuid = target.getItemTargetGuid();

    setSource(target.getSource());
    setDestination(target.getDestination());

    m_strTarget = target.m_strTarget;

    m_targetMask = target.getTargetMask();

    m_transportSourceGuid = target.m_transportSourceGuid;
    m_transportDestinationGuid = target.m_transportDestinationGuid;
    return *this;
}

SpellCastTargets::~SpellCastTargets()
{
    m_strTarget.clear();
}

void SpellCastTargets::read(WorldPacket& data)
{
    reset();

    data >> m_targetMask;

    if (m_targetMask == TARGET_FLAG_SELF)
        return;

    if (m_targetMask & (TARGET_FLAG_OBJECT | TARGET_FLAG_OPEN_LOCK))
    {
        data >> m_gameObjectTargetGuid;
    }

    if (m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 | TARGET_FLAG_UNK17))
    {
        data >> m_unitTargetGuid;
    }

    if (m_targetMask & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM))
    {
        data >> m_itemTargetGuid;
    }

    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
    {
        data >> m_transportSourceGuid;
        data >> m_source.x;
        data >> m_source.y;
        data >> m_source.z;
    }

    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        data >> m_transportDestinationGuid;
        data >> m_destination.x;
        data >> m_destination.y;
        data >> m_destination.z;

        if (auto transporter = sTransportHandler.getTransporter(m_transportDestinationGuid.getGuidLow()))
            transporter->calculatePassengerPosition(m_destination.x, m_destination.y, m_destination.z);
    }

    if (m_targetMask & TARGET_FLAG_STRING)
    {
        data >> m_strTarget;
    }
}

void SpellCastTargets::write(WorldPacket& data) const
{
    data << m_targetMask;

    if (m_targetMask & (TARGET_FLAG_OBJECT | TARGET_FLAG_OPEN_LOCK))
    {
        FastGUIDPack(data, m_gameObjectTargetGuid);
    }

    if (m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 | TARGET_FLAG_UNK17))
    {
        FastGUIDPack(data, m_unitTargetGuid);
    }

    if (m_targetMask & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM))
    {
        FastGUIDPack(data, m_itemTargetGuid);
    }

    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
    {
        data << m_transportSourceGuid;
        data << m_source;
    }

    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        data << m_transportDestinationGuid;
        data << m_destination;
    }

    if (m_targetMask & TARGET_FLAG_STRING)
    {
        data << m_strTarget.c_str();
    }
}

bool SpellCastTargets::isEmpty() const
{
    return m_gameObjectTargetGuid.isEmpty() && m_unitTargetGuid.isEmpty() && m_itemTargetGuid.isEmpty() &&
        m_transportSourceGuid.isEmpty() && m_transportDestinationGuid.isEmpty() && !hasSource() && !hasDestination();
}

bool SpellCastTargets::hasSource() const
{
    return (getTargetMask() & TARGET_FLAG_SOURCE_LOCATION) != 0;
}

bool SpellCastTargets::hasDestination() const
{
    return (getTargetMask() & TARGET_FLAG_DEST_LOCATION) != 0;
}

bool SpellCastTargets::isTradeItem() const
{
    return (getTargetMask() & TARGET_FLAG_TRADE_ITEM) != 0;
}

uint32_t SpellCastTargets::getTargetMask() const
{
    return m_targetMask;
}

void SpellCastTargets::setTargetMask(uint32_t mask)
{
    m_targetMask = mask;
}

void SpellCastTargets::addTargetMask(uint32_t mask)
{
    setTargetMask(getTargetMask() | mask);
}

void SpellCastTargets::setStringTarget(const std::string& str)
{
    m_strTarget = str;
    addTargetMask(TARGET_FLAG_STRING);
}

std::string SpellCastTargets::getStringTarget() const
{
    return m_strTarget;
}

uint64_t SpellCastTargets::getGameObjectTargetGuid() const
{
    return m_gameObjectTargetGuid.getRawGuid();
}

uint64_t SpellCastTargets::getUnitTargetGuid() const
{
    return m_unitTargetGuid.getRawGuid();
}

uint64_t SpellCastTargets::getItemTargetGuid() const
{
    return m_itemTargetGuid.getRawGuid();
}

uint64_t SpellCastTargets::getTransportSourceGuid() const
{
    return m_transportSourceGuid.getRawGuid();
}

LocationVector SpellCastTargets::getSource() const
{
    return m_source;
}

uint64_t SpellCastTargets::getTransportDestinationGuid() const
{
    return m_transportDestinationGuid;
}

LocationVector SpellCastTargets::getDestination() const
{
    return m_destination;
}

void SpellCastTargets::setGameObjectTarget(uint64_t guid)
{
    m_gameObjectTargetGuid = guid;
    addTargetMask(TARGET_FLAG_OBJECT);
}

void SpellCastTargets::setUnitTarget(uint64_t guid)
{
    m_unitTargetGuid = guid;
    addTargetMask(TARGET_FLAG_UNIT);
}

void SpellCastTargets::setItemTarget(uint64_t guid)
{
    m_itemTargetGuid = guid;
    addTargetMask(TARGET_FLAG_ITEM);
}

void SpellCastTargets::setTransportSourceGuid(uint64_t guid)
{
    m_transportDestinationGuid = guid;
    addTargetMask(TARGET_FLAG_SOURCE_LOCATION);
}

void SpellCastTargets::setSource(LocationVector source)
{
    m_source = LocationVector(source);
    addTargetMask(TARGET_FLAG_SOURCE_LOCATION);
}

void SpellCastTargets::setTransportDestinationGuid(uint64_t guid)
{
    m_transportDestinationGuid = guid;
    addTargetMask(TARGET_FLAG_DEST_LOCATION);
}

void SpellCastTargets::setDestination(LocationVector destination)
{
    m_destination = LocationVector(destination);
    addTargetMask(TARGET_FLAG_DEST_LOCATION);
}
