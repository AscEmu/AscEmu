/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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

 // MIT Start
void DynamicObject::Create(Unit* caster, Spell* spell, LocationVector lv, uint32 duration, float radius, uint32 type)
{
    Create(caster, spell, lv.x, lv.y, lv.z, duration, radius, type);
}
// MIT End

DynamicObject::DynamicObject(uint32 high, uint32 low)
{
    m_objectType |= TYPE_DYNAMICOBJECT;
    m_objectTypeId = TYPEID_DYNAMICOBJECT;

#if VERSION_STRING != Cata
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_POSITION);
#else
    m_updateFlag = UPDATEFLAG_POSITION;
#endif

    m_valuesCount = DYNAMICOBJECT_END;
    m_uint32Values = _fields;
    memset(m_uint32Values, 0, (DYNAMICOBJECT_END)*sizeof(uint32));
    m_updateMask.SetCount(DYNAMICOBJECT_END);
    m_uint32Values[OBJECT_FIELD_TYPE] = TYPE_DYNAMICOBJECT | TYPE_OBJECT;
    m_uint32Values[OBJECT_FIELD_GUID] = low;
    m_uint32Values[OBJECT_FIELD_GUID + 1] = high;
    m_wowGuid.Init(GetGUID());
    SetScale(1);


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
        if (caster->IsPlayer())
            p_caster = static_cast< Player* >(caster);
    }
    else
        p_caster = pSpell->p_caster;

    m_spellProto = pSpell->GetSpellInfo();
    SetEntry(m_spellProto->getId());
    setFloatValue(OBJECT_FIELD_SCALE_X, 1);
    setUInt64Value(DYNAMICOBJECT_CASTER, caster->GetGUID());
    setByteFlag(DYNAMICOBJECT_BYTES, 0, type);
    setUInt32Value(DYNAMICOBJECT_SPELLID, m_spellProto->getId());
    setFloatValue(DYNAMICOBJECT_RADIUS, radius);
    setUInt32Value(DYNAMICOBJECT_CASTTIME,Util::getMSTime());
    m_position.x = x; //m_floatValues[DYNAMICOBJECT_POS_X]  = x;
    m_position.y = y; //m_floatValues[DYNAMICOBJECT_POS_Y]  = y;
    m_position.z = z; //m_floatValues[DYNAMICOBJECT_POS_Z]  = z;


    m_aliveDuration = duration;
    u_caster = caster;
    m_faction = caster->m_faction;
    m_factionDBC = caster->m_factionDBC;
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

void DynamicObject::AddInRangeObject(Object* pObj)
{
    Object::AddInRangeObject(pObj);
}

void DynamicObject::OnRemoveInRangeObject(Object* pObj)
{
    if (pObj->IsUnit())
    {
        targets.erase(pObj->GetGUID());
    }
    Object::OnRemoveInRangeObject(pObj);
}

void DynamicObject::UpdateTargets()
{
    if (m_aliveDuration == 0)
        return;

    if (m_aliveDuration >= 100)
    {
        Unit* target;
        Aura* pAura;

        float radius = getFloatValue(DYNAMICOBJECT_RADIUS) * getFloatValue(DYNAMICOBJECT_RADIUS);

        // Looking for targets in the Object set
        for (std::set< Object* >::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
        {
            Object* o = *itr;

            if (!o->IsUnit() || !static_cast< Unit* >(o)->isAlive())
                continue;

            target = static_cast< Unit* >(o);

            if (!isAttackable(u_caster, target, !(m_spellProto->custom_c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
                continue;

            // skip units already hit, their range will be tested later
            if (targets.find(target->GetGUID()) != targets.end())
                continue;

            if (getDistanceSq(target) <= radius)
            {
                pAura = sSpellFactoryMgr.NewAura(m_spellProto, m_aliveDuration, u_caster, target, true);
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
                targets.insert(target->GetGUID());
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

    data << GetGUID();
    SendMessageToSet(&data, false);

    if (IsInWorld())
        RemoveFromWorld(true);

    if (u_caster != nullptr && m_spellProto->getChannelInterruptFlags() != 0)
    {
        u_caster->SetChannelSpellTargetGUID(0);
        u_caster->SetChannelSpellId(0);
    }

    delete this;
}
