/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

DynamicObject::DynamicObject(uint32_t high, uint32_t low)
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
    setGuid(low, high);

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
    m_factionTemplate = caster->m_factionTemplate;
    m_factionEntry = caster->m_factionEntry;
    m_phase = caster->GetPhase();

    if (spell->g_caster)
        PushToWorld(spell->g_caster->getWorldMap());
    else
        PushToWorld(caster->getWorldMap());

    if (caster->m_dynamicObject != nullptr)
        caster->m_dynamicObject->remove();

    caster->m_dynamicObject = this;

    updateTargets();
}

void DynamicObject::updateTargets()
{
    if (m_aliveDuration == 0)
        return;

    if (m_aliveDuration >= 100)
    {
        float radius = getRadius() * getRadius();

        for (const auto& itr : getInRangeObjectsSet())
        {
            Object* object = itr;
            if (!object || !object->isCreatureOrPlayer() || !static_cast<Unit*>(object)->isAlive())
                continue;

            Unit* target = static_cast<Unit*>(object);

            if (!m_unitCaster->isValidTarget(target, m_spellInfo))
                continue;

            // skip units already hit, their range will be tested later
            if (m_targets.find(target->getGuid()) != m_targets.end())
                continue;

            if (getDistanceSq(target) <= radius)
            {
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

                m_targets.insert(target->getGuid());
            }
        }

        for (auto jtr = m_targets.begin(); jtr != m_targets.end();)
        {
            Unit* target = getWorldMap() ? getWorldMap()->getUnit(*jtr) : nullptr;

            auto jtr2 = jtr;
            ++jtr;

            if (target && getDistanceSq(target) > radius)
            {
                target->removeAllAurasById(m_spellInfo->getId());
                m_targets.erase(jtr2);
            }
        }

        m_aliveDuration -= 100;
    }
    else
    {
        m_aliveDuration = 0;
    }

    if (m_aliveDuration == 0)
        remove();
}

void DynamicObject::onRemoveInRangeObject(Object* pObj)
{
    if (pObj->isCreatureOrPlayer())
        m_targets.erase(pObj->getGuid());

    Object::onRemoveInRangeObject(pObj);
}

void DynamicObject::remove()
{
    if (!IsInWorld())
    {
        delete this;
        return;
    }

    for (auto const targetGuid : m_targets)
    {
        if (Unit* target = m_WorldMap->getUnit(targetGuid))
            target->removeAllAurasById(m_spellInfo->getId());
    }

    sendGameobjectDespawnAnim();

    if (IsInWorld())
        RemoveFromWorld(true);

    if (m_unitCaster && m_spellInfo->getChannelInterruptFlags() != 0)
    {
        m_unitCaster->setChannelObjectGuid(0);
        m_unitCaster->setChannelSpellId(0);
    }

    delete this;
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
void DynamicObject::setRadius(float radius) { write(dynamicObjectData()->radius, radius); }

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

float DynamicObject::getDynamicO() const { return m_position.x; }
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
