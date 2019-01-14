/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Objects/DynamicObject.h"
#include "Map/MapMgr.h"
#include "Faction.h"
#include "Spell/SpellMgr.h"
#include "Spell/SpellAuras.h"
#include "Spell/Spell.Legacy.h"
#include "Spell/Definitions/ProcFlags.h"
#include "Spell/Definitions/SpellIsFlags.h"
#include "Objects/ObjectMgr.h"
#include "Data/WoWDynamicObject.h"

// MIT Start

 //////////////////////////////////////////////////////////////////////////////////////////
 // WoWData

uint64_t DynamicObject::getCasterGuid() const { return dynamicObjectData()->caster_guid; }
void DynamicObject::setCasterGuid(uint64_t guid) { write(dynamicObjectData()->caster_guid, guid); }

//bytes start
uint8_t DynamicObject::getDynamicType() const { return dynamicObjectData()->dynamicobject_bytes.s.type; }
void DynamicObject::setDynamicType(uint8_t type) { write(dynamicObjectData()->dynamicobject_bytes.s.type, type); }
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

 //////////////////////////////////////////////////////////////////////////////////////////
 // Misc
void DynamicObject::Create(Unit* caster, Spell* spell, LocationVector lv, uint32 duration, float radius, uint32 type)
{
    Create(caster, spell, lv.x, lv.y, lv.z, duration, radius, type);
}

// MIT End

DynamicObject::DynamicObject(uint32 high, uint32 low)
{
    m_objectType |= TYPE_DYNAMICOBJECT;
    m_objectTypeId = TYPEID_DYNAMICOBJECT;

#if VERSION_STRING <= TBC
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_LOWGUID);
#elif VERSION_STRING == WotLK
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_POSITION);
#elif VERSION_STRING == Cata
    m_updateFlag = UPDATEFLAG_POSITION;
#endif

    m_valuesCount = DYNAMICOBJECT_END;
    m_uint32Values = _fields;
    memset(m_uint32Values, 0, (DYNAMICOBJECT_END)*sizeof(uint32));
    m_updateMask.SetCount(DYNAMICOBJECT_END);
    setOType(TYPE_DYNAMICOBJECT | TYPE_OBJECT);
    m_uint32Values[OBJECT_FIELD_GUID] = low;
    m_uint32Values[OBJECT_FIELD_GUID + 1] = high;
    m_wowGuid.Init(getGuid());
    setScale(1.0f);


    m_parentSpell = nullptr;
    m_aliveDuration = 0;
    u_caster = nullptr;
    m_spellProto = nullptr;
    p_caster = nullptr;
}

DynamicObject::~DynamicObject()
{
    if (u_caster != nullptr && u_caster->dynObj == this)
        u_caster->dynObj = nullptr;
}

void DynamicObject::Create(Unit* caster, Spell* pSpell, float x, float y, float z, uint32 duration, float radius, uint32 type)
{
    Object::_Create(caster->GetMapId(), x, y, z, 0);
    if (pSpell->g_caster)
    {
        m_parentSpell = pSpell;
    }
    if (pSpell->p_caster == nullptr)
    {
        // try to find player caster here
        if (caster->isPlayer())
            p_caster = static_cast< Player* >(caster);
    }
    else
        p_caster = pSpell->p_caster;

    m_spellProto = pSpell->getSpellInfo();
    setEntry(m_spellProto->getId());
    setScale(1.0f);

    setCasterGuid(caster->getGuid());

    setDynamicType(static_cast<uint8_t>(type));

    setSpellId(m_spellProto->getId());
    setRadius(radius);

#if VERSION_STRING > Classic
    setCastTime(Util::getMSTime());
#endif

    setDynamicX(x);
    setDynamicY(y);
    setDynamicZ(z);
    setDynamicO(0.f);

    m_aliveDuration = duration;
    u_caster = caster;
    m_factionTemplate = caster->m_factionTemplate;
    m_factionEntry = caster->m_factionEntry;
    m_phase = caster->GetPhase();

    if (pSpell->g_caster)
        PushToWorld(pSpell->g_caster->GetMapMgr());
    else
        PushToWorld(caster->GetMapMgr());

    if (caster->dynObj != nullptr)
    {
        //expires
        caster->dynObj->Remove();
    }
    caster->dynObj = this;

    //sEventMgr.AddEvent(this, &DynamicObject::UpdateTargets, EVENT_DYNAMICOBJECT_UPDATE, 100, 0,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    UpdateTargets();
}

void DynamicObject::addToInRangeObjects(Object* pObj)
{
    Object::addToInRangeObjects(pObj);
}

void DynamicObject::onRemoveInRangeObject(Object* pObj)
{
    if (pObj->isCreatureOrPlayer())
    {
        targets.erase(pObj->getGuid());
    }
    Object::onRemoveInRangeObject(pObj);
}

void DynamicObject::UpdateTargets()
{
    if (m_aliveDuration == 0)
        return;

    if (m_aliveDuration >= 100)
    {
        Unit* target;
        Aura* pAura;

        float radius = getRadius() * getRadius();

        // Looking for targets in the Object set
        for (const auto& itr : getInRangeObjectsSet())
        {
            Object* o = itr;
            if (!o || !o->isCreatureOrPlayer() || !static_cast< Unit* >(o)->isAlive())
                continue;

            target = static_cast<Unit*>(o);

            if (!isAttackable(u_caster, target, !(m_spellProto->custom_c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
                continue;

            // skip units already hit, their range will be tested later
            if (targets.find(target->getGuid()) != targets.end())
                continue;

            if (getDistanceSq(target) <= radius)
            {
                pAura = sSpellMgr.newAura(m_spellProto, m_aliveDuration, u_caster, target, true);
                for (uint8 i = 0; i < 3; ++i)
                {
                    if (m_spellProto->getEffect(i) == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                    {
                        pAura->AddMod(m_spellProto->getEffectApplyAuraName(i),
                                      m_spellProto->getEffectBasePoints(i) + 1, m_spellProto->getEffectMiscValue(i), i);
                    }
                }
                target->AddAura(pAura);
                if (p_caster)
                {
                    p_caster->HandleProc(PROC_ON_CAST_SPECIFIC_SPELL | PROC_ON_CAST_SPELL, target, m_spellProto);
                    p_caster->m_procCounter = 0;
                }

                // add to target list
                targets.insert(target->getGuid());
            }
        }


        // loop the targets, check the range of all of them
        DynamicObjectList::iterator jtr = targets.begin();
        DynamicObjectList::iterator jtr2;
        DynamicObjectList::iterator jend = targets.end();

        while (jtr != jend)
        {
            target = GetMapMgr() ? GetMapMgr()->GetUnit(*jtr) : nullptr;
            jtr2 = jtr;
            ++jtr;

            if ((target != nullptr) && (getDistanceSq(target) > radius))
            {
                target->RemoveAura(m_spellProto->getId());
                targets.erase(jtr2);
            }
        }

        m_aliveDuration -= 100;
    }
    else
    {
        m_aliveDuration = 0;
    }

    if (m_aliveDuration == 0)
    {
        Remove();
    }
}

void DynamicObject::Remove()
{
    // remove aura from all targets
    Unit* target;

    if (!IsInWorld())
    {
        delete this;
        return;
    }

    for (std::set< uint64 >::iterator itr = targets.begin(); itr != targets.end(); ++itr)
    {

        uint64 TargetGUID = *itr;

        target = m_mapMgr->GetUnit(TargetGUID);

        if (target != nullptr)
            target->RemoveAura(m_spellProto->getId());
    }

    WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);

    data << getGuid();
    SendMessageToSet(&data, false);

    if (IsInWorld())
        RemoveFromWorld(true);

    if (u_caster != nullptr && m_spellProto->getChannelInterruptFlags() != 0)
    {
        u_caster->setChannelObjectGuid(0);
        u_caster->setChannelSpellId(0);
    }

    delete this;
}
