/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Definitions/SpellCastTargetFlags.h"
#include "SpellCastTargets.h"

#include "Objects/ObjectMgr.h"
#include <Server/WorldSocket.h>

void SpellCastTargets::reset()
{
    m_targetMask = 0;
    m_gameObjectTarget = 0;
    m_unitTarget = 0;
    m_itemTarget = 0;
    unkuint64_1 = 0;
    unkuint64_2 = 0;
    m_source = LocationVector();
    m_destination = LocationVector();
    m_strTarget = std::string();
}

SpellCastTargets::SpellCastTargets(uint64_t unitTarget) : m_targetMask(TARGET_FLAG_UNIT),
m_unitTarget(unitTarget)
{
}

SpellCastTargets::SpellCastTargets(WorldPacket& data, uint64_t caster)
{
    read(data, caster);
}

SpellCastTargets& SpellCastTargets::operator=(const SpellCastTargets& target)
{
    m_gameObjectTarget = target.getGameObjectTarget();
    m_unitTarget = target.getUnitTarget();
    m_itemTarget = target.getItemTarget();

    setSource(target.getSource());
    setDestination(target.getDestination());

    m_strTarget = target.m_strTarget;

    m_targetMask = target.getTargetMask();

    unkuint64_1 = target.unkuint64_1;
    unkuint64_2 = target.unkuint64_2;
    return *this;
}

SpellCastTargets::~SpellCastTargets()
{
    m_strTarget.clear();
}

void SpellCastTargets::read(WorldPacket& data, uint64_t caster)
{
    reset();

    data >> m_targetMask;

    if (m_targetMask == TARGET_FLAG_SELF)
    {
        m_unitTarget = caster;
        return;
    }

    if (m_targetMask & (TARGET_FLAG_OBJECT | TARGET_FLAG_OBJECT_CASTER))
    {
        WoWGuid guid;
        data >> guid;
        m_gameObjectTarget = guid.GetOldGuid();
    }

    if (m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 | TARGET_FLAG_UNK17))
    {
        WoWGuid guid;
        data >> guid;
        m_unitTarget = guid.GetOldGuid();
    }

    if (m_targetMask & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM))
    {
        WoWGuid guid;
        data >> guid;
        m_itemTarget = guid.GetOldGuid();
    }

    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
    {
        WoWGuid sourceGuid;

        data >> sourceGuid;
        unkuint64_1 = sourceGuid.GetOldGuid();

        LocationVector lv;
        data >> lv.x;
        data >> lv.y;
        data >> lv.z;

        setSource(lv);
    }

    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        WoWGuid destinationGuid;
        data >> destinationGuid;
        unkuint64_2 = destinationGuid.GetOldGuid();

        LocationVector lv;
        data >> lv.x;
        data >> lv.y;
        data >> lv.z;

        setDestination(lv);
    }

    if (m_targetMask & TARGET_FLAG_STRING)
    {
        data >> m_strTarget;
    }
}

void SpellCastTargets::write(WorldPacket& data) const
{
    data << m_targetMask;

    if (m_targetMask & (TARGET_FLAG_OBJECT | TARGET_FLAG_OBJECT_CASTER))
    {
        FastGUIDPack(data, m_gameObjectTarget);
    }

    if (m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 | TARGET_FLAG_UNK17))
    {
        FastGUIDPack(data, m_unitTarget);
    }

    if (m_targetMask & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM))
    {
        FastGUIDPack(data, m_itemTarget);
    }

    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
    {
        data << WoWGuid(unkuint64_1);
        data << m_source;
    }

    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        data << WoWGuid(unkuint64_2);
        data << m_destination;
    }

    if (m_targetMask & TARGET_FLAG_STRING)
    {
        data << m_strTarget.c_str();
    }
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

uint64_t SpellCastTargets::getGameObjectTarget() const
{
    return m_gameObjectTarget;
}

uint64_t SpellCastTargets::getUnitTarget() const
{
    return m_unitTarget;
}

uint64_t SpellCastTargets::getItemTarget() const
{
    return m_itemTarget;
}

LocationVector SpellCastTargets::getSource() const
{
    return LocationVector(m_source);
}

LocationVector SpellCastTargets::getDestination() const
{
    return LocationVector(m_destination);
}

void SpellCastTargets::setGameObjectTarget(uint64_t guid)
{
    m_gameObjectTarget = guid;
    addTargetMask(TARGET_FLAG_OBJECT);
}

void SpellCastTargets::setUnitTarget(uint64_t guid)
{
    m_unitTarget = guid;
    addTargetMask(TARGET_FLAG_UNIT);
}

void SpellCastTargets::setItemTarget(uint64_t guid)
{
    m_itemTarget = guid;
    addTargetMask(TARGET_FLAG_ITEM);
}

void SpellCastTargets::setSource(LocationVector source)
{
    m_source = LocationVector(source);
    addTargetMask(TARGET_FLAG_SOURCE_LOCATION);
}

void SpellCastTargets::setDestination(LocationVector destination)
{
    m_destination = LocationVector(destination);
    addTargetMask(TARGET_FLAG_DEST_LOCATION);
}
