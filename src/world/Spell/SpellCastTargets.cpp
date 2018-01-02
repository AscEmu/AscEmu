/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpellCastTargets.h"
#include "Definitions/SpellCastTargetFlags.h"
#include "Objects/ObjectMgr.h"
#include <Server/WorldSocket.h>

LocationVector SpellCastTargets::source() const
{
    return LocationVector(m_source);
}

LocationVector SpellCastTargets::destination() const
{
    return LocationVector(m_destination);
}

void SpellCastTargets::setSource(LocationVector source)
{
    m_source = LocationVector(source);
}

void SpellCastTargets::setDestination(LocationVector destination)
{
    m_destination = LocationVector(destination);
}

SpellCastTargets::SpellCastTargets()
{
}

SpellCastTargets::SpellCastTargets(
    uint16_t TargetMask,
    uint64_t unitTarget,
    uint64_t itemTarget,
    LocationVector source,
    LocationVector destination) : m_targetMask(TargetMask),
    m_unitTarget(unitTarget),
    m_itemTarget(itemTarget)
{
    setSource(source);
    setDestination(destination);
}

SpellCastTargets::SpellCastTargets(uint64_t unitTarget) : m_targetMask(0x2),
m_unitTarget(unitTarget)
{
}

SpellCastTargets::SpellCastTargets(WorldPacket& data, uint64_t caster)
{
    read(data, caster);
}

SpellCastTargets& SpellCastTargets::operator=(const SpellCastTargets& target)
{
    m_unitTarget = target.m_unitTarget;
    m_itemTarget = target.m_itemTarget;

    setSource(target.source());
    setDestination(target.destination());

    m_strTarget = target.m_strTarget;

    m_targetMask = target.m_targetMask;
    m_targetMaskExtended = target.m_targetMaskExtended;

    unkuint64_1 = target.unkuint64_1;
    unkuint64_2 = target.unkuint64_2;
    return *this;
}

SpellCastTargets::~SpellCastTargets()
{
    m_strTarget.clear();
}

uint32 SpellCastTargets::GetTargetMask() const
{
    return m_targetMask;
}

void SpellCastTargets::reset()
{
    m_source = LocationVector();
    m_destination = LocationVector();
    m_targetMask = 0;
    m_targetMaskExtended = 0;
    m_unitTarget = 0;
    m_itemTarget = 0;
    unkuint64_1 = 0;
    unkuint64_2 = 0;
    m_strTarget = std::string();
}

void SpellCastTargets::read(WorldPacket& data, uint64 caster)
{
    reset();

    data >> m_targetMask;
    data >> m_targetMaskExtended;

    if (m_targetMask == TARGET_FLAG_SELF)
    {
        auto spellId = *(uint32_t*)(data.contents() + 1);
        switch (spellId)
        {
            case 14285: // Arcane Shot (Rank 6)
            case 14286: // Arcane Shot (Rank 7)
            case 14287: // Arcane Shot (Rank 8)
            case 27019: // Arcane Shot (Rank 9)
            case 49044: // Arcane Shot (Rank 10)
            case 49045: // Arcane Shot (Rank 11)
            case 15407: // Mind Flay (Rank 1)
            case 17311: // Mind Flay (Rank 2)
            case 17312: // Mind Flay (Rank 3)
            case 17313: // Mind Flay (Rank 4)
            case 17314: // Mind Flay (Rank 5)
            case 18807: // Mind Flay (Rank 6)
            case 25387: // Mind Flay (Rank 7)
            case 48155: // Mind Flay (Rank 8)
            case 48156: // Mind Flay (Rank 9)
            {
                m_targetMask = TARGET_FLAG_UNIT;
                auto player = objmgr.GetPlayer(static_cast<uint32_t>(caster));
                if (player)
                {
                    m_unitTarget = player->GetTargetGUID();
                }
            }
            break;
            default:
                m_unitTarget = caster;
                break;
        }
        return;
    }

    if (m_targetMask & (TARGET_FLAG_OBJECT | TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2))
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

        if (!(m_targetMask & TARGET_FLAG_DEST_LOCATION))
        {
            setDestination(lv);
        }
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

        if (!(m_targetMask & TARGET_FLAG_SOURCE_LOCATION))
        {
            setSource(lv);
        }
    }

    if (m_targetMask & TARGET_FLAG_STRING)
    {
        data >> m_strTarget;
    }
}

void SpellCastTargets::write(WorldPacket& data) const
{
    data << m_targetMask;
    data << m_targetMaskExtended;

    if (m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 | TARGET_FLAG_OBJECT | TARGET_FLAG_GLYPH))
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
    return GetTargetMask() & TARGET_FLAG_SOURCE_LOCATION;
}

bool SpellCastTargets::hasDestination() const
{
    return GetTargetMask() & TARGET_FLAG_DEST_LOCATION;
}
