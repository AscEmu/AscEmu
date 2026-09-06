/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Objects/DynamicObject.hpp"

#include "GameObject.h"
#include "Data/Flags.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Data/WoWDynamicObject.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Objects/Units/Players/Player.hpp"

DynamicObject::DynamicObject(uint64_t guid)
{
    m_objectType |= TYPE_DYNAMICOBJECT;
    m_objectTypeId = TYPEID_DYNAMICOBJECT;

#if VERSION_STRING == Classic
    m_updateFlag = (UPDATEFLAG_ALL | UPDATEFLAG_HAS_POSITION);
#endif
#if VERSION_STRING == TBC
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION);
#endif
#if VERSION_STRING == WotLK
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_POSITION);
#endif
#if VERSION_STRING == Cata
    m_updateFlag = UPDATEFLAG_POSITION;
#endif
#if VERSION_STRING == Mop
    m_updateFlag = UPDATEFLAG_HAS_POSITION;
#endif

    m_valuesCount = getSizeOfStructure(WoWDynamicObject);
    m_uint32Values = _fields;
    memset(m_uint32Values, 0, (getSizeOfStructure(WoWDynamicObject)) * sizeof(uint32_t));
    m_updateMask.SetCount(getSizeOfStructure(WoWDynamicObject));

    setOType(TYPE_DYNAMICOBJECT | TYPE_OBJECT);
    setGuid(guid);

    setScale(1.0f);
}

DynamicObject::~DynamicObject()
{
    if (m_unitCaster && m_unitCaster->m_dynamicObject == this)
        m_unitCaster->m_dynamicObject = nullptr;
}

void DynamicObject::create(Unit* caster, Spell* spell, LocationVector lv, uint32_t duration, float radius, uint32_t type)
{
    Object::_Create(caster->GetMapId(), lv.x, lv.y, lv.z, lv.o);

    if (spell->g_caster)
        m_parentSpell = spell;

    if (spell->p_caster == nullptr)
    {
        if (caster->isPlayer())
            m_playerCaster = static_cast<Player*>(caster);
    }
    else
    {
        m_playerCaster = spell->p_caster;
    }

    m_spellInfo = spell->getSpellInfo();

    setEntry(m_spellInfo->getId());
    setScale(1.0f);

    setCasterGuid(caster->getGuid());

    setDynamicType(static_cast<uint8_t>(type));

    setSpellId(m_spellInfo->getId());
    setRadius(radius);

#if VERSION_STRING > Classic
    setCastTime(Util::getMSTime());
#endif

    setDynamicX(lv.x);
    setDynamicY(lv.y);
    setDynamicZ(lv.z);
    setDynamicO(lv.o);

    m_aliveDuration = duration;
    m_unitCaster = caster;
    m_factionTemplate = caster->getServersideFactionTemplate();
    m_factionEntry = caster->getServersideFactionEntry();
    m_phase = caster->GetPhase();

    if (spell->g_caster)
        spell->g_caster->getWorldMap()->getObjectFactory().attachToWorld(this);
    else
        caster->getWorldMap()->getObjectFactory().attachToWorld(this);    

    if (caster->m_dynamicObject != nullptr)
        caster->m_dynamicObject->remove();

    caster->m_dynamicObject = this;

    getWorldMap()->refreshDynamicObjectTargets(this);
}

void DynamicObject::considerTarget(Unit* target)
{
    if (!target || !IsInWorld() || m_aliveDuration == 0 || !m_unitCaster || !m_spellInfo)
        return;

    const uint64_t targetGuid = target->getGuid();
    const bool alreadyTargeted = m_targets.find(targetGuid) != m_targets.end();

    const bool validTarget =
        target->IsInWorld() &&
        target->getWorldMap() == getWorldMap() &&
        target->isAlive() &&
        m_unitCaster->isValidAttackableTarget(target, m_spellInfo) &&
        getDistanceSq(target) <= getRadius() * getRadius();

    if (!validTarget)
    {
        if (alreadyTargeted)
            removeTarget(target);
        return;
    }

    if (alreadyTargeted)
        return;

    auto aura = sSpellMgr.newAura(m_spellInfo, m_aliveDuration, m_unitCaster, target, true);
    for (uint8_t i = 0; i < 3; ++i)
    {
        if (m_spellInfo->getEffect(i) == SPELL_EFFECT_PERSISTENT_AREA_AURA)
        {
            aura->addAuraEffect(static_cast<AuraEffect>(m_spellInfo->getEffectApplyAuraName(i)),
                m_spellInfo->getEffectBasePoints(i) + 1, m_spellInfo->getEffectMiscValue(i), 1.0f, false, i);
        }
    }

    target->addAura(std::move(aura));
    m_targets.insert(targetGuid);
    target->addDynamicObjectTarget(getGuid());
}

void DynamicObject::removeTarget(Unit* target)
{
    if (!target || !m_spellInfo)
        return;

    const uint64_t targetGuid = target->getGuid();
    if (m_targets.erase(targetGuid) == 0)
        return;

    target->removeAllAurasById(m_spellInfo->getId());
    target->removeDynamicObjectTarget(getGuid());
}

void DynamicObject::refreshCurrentTargets()
{
    if (!IsInWorld() || m_targets.empty())
        return;

    // considerTarget() can erase from m_targets, so iterate over a snapshot.
    const std::vector<uint64_t> targets(m_targets.begin(), m_targets.end());
    for (uint64_t targetGuid : targets)
    {
        if (Unit* target = getWorldMapUnit(targetGuid))
            considerTarget(target);
        else
            m_targets.erase(targetGuid);
    }
}

void DynamicObject::updateLifetime(uint32_t diff)
{
    if (m_aliveDuration == 0 || diff == 0)
        return;

    if (diff >= m_aliveDuration)
        m_aliveDuration = 0;
    else
        m_aliveDuration -= diff;

    if (m_aliveDuration == 0)
        remove();
}

void DynamicObject::remove()
{
    if (!IsInWorld())
    {
        return;
    }

    for (auto const targetGuid : m_targets)
    {
        if (Unit* target = getWorldMapUnit(targetGuid))
        {
            target->removeAllAurasById(m_spellInfo->getId());
            target->removeDynamicObjectTarget(getGuid());
        }
    }
    m_targets.clear();

    sendGameobjectDespawnAnim();

    if (IsInWorld())
        destroy();

    if (m_unitCaster && m_spellInfo->getChannelInterruptFlags() != 0)
    {
        m_unitCaster->setChannelObjectGuid(0);
        m_unitCaster->setChannelSpellId(0);
    }
}

 //////////////////////////////////////////////////////////////////////////////////////////
 // WoWData

uint64_t DynamicObject::getCasterGuid() const { return dynamicObjectData()->caster_guid; }
void DynamicObject::setCasterGuid(uint64_t guid) { write(dynamicObjectData()->caster_guid, guid); }

//bytes start
uint8_t DynamicObject::getDynamicType() const { return dynamicObjectData()->dynamicobject_bytes.s.type; }
#if VERSION_STRING < Cata
void DynamicObject::setDynamicType(uint8_t type) { write(dynamicObjectData()->dynamicobject_bytes.s.type, type); }
#else
void DynamicObject::setDynamicType(uint8_t type) { write(dynamicObjectData()->dynamicobject_bytes.s.type, static_cast<uint32_t>(type)); }
#endif
//bytes end

uint32_t DynamicObject::getSpellId() const { return dynamicObjectData()->spell_id; }
void DynamicObject::setSpellId(uint32_t id) { write(dynamicObjectData()->spell_id, id); }

float DynamicObject::getRadius() const { return dynamicObjectData()->radius; }
void DynamicObject::setRadius(float radius)
{
    write(dynamicObjectData()->radius, radius);

    if (IsInWorld() && getWorldMap())
        getWorldMap()->refreshDynamicObjectTargets(this);
}

// Position set for classic and TBC
float DynamicObject::getDynamicX() const { return m_position.x; }
void DynamicObject::setDynamicX(float x)
{
    m_position.x = x;
#if VERSION_STRING <= TBC
    write(dynamicObjectData()->x, x);
#endif
}

float DynamicObject::getDynamicY() const { return m_position.y; }
void DynamicObject::setDynamicY(float y)
{
    m_position.y = y;
#if VERSION_STRING <= TBC
    write(dynamicObjectData()->y, y);
#endif
}

float DynamicObject::getDynamicZ() const { return m_position.z; }
void DynamicObject::setDynamicZ(float z)
{
    m_position.z = z;
#if VERSION_STRING <= TBC
    write(dynamicObjectData()->z, z);
#endif
}

float DynamicObject::getDynamicO() const { return m_position.o; }
void DynamicObject::setDynamicO(float o)
{
    m_position.o = o;
#if VERSION_STRING <= TBC
    write(dynamicObjectData()->o, o);
#endif
}

#if VERSION_STRING > Classic
uint32_t DynamicObject::getCastTime() const { return dynamicObjectData()->cast_time; }
void DynamicObject::setCastTime(uint32_t time) { write(dynamicObjectData()->cast_time, time); }
#endif
