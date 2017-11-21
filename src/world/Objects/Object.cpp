/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
 */

#include "StdAfx.h"
#include "Units/Unit.h"
#include "Storage/DBC/DBCStores.h"
#include "Management/QuestLogEntry.hpp"
#include "Server/EventableObject.h"
#include "Server/IUpdatable.h"
#include "VMapFactory.h"
#include "MMapFactory.h"
#include "TLSObject.h"
#include "Management/ItemInterface.h"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgrDefines.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Map/MapMgr.h"
#include "Faction.h"
#include "Spell/SpellAuras.h"
#include "Map/WorldCreator.h"
#include "Spell/Definitions/ProcFlags.h"
#include "Spell/Definitions/SpellDamageType.h"
#include "Spell/Definitions/SpellState.h"
#include <Spell/Definitions/AuraInterruptFlags.h>
#include "Spell/Definitions/SpellSchoolConversionTable.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Units/Creatures/CreatureDefines.hpp"

// MIT Start

//////////////////////////////////////////////////////////////////////////////////////////
// Object values
void Object::setByteValue(uint16_t index, uint8_t offset, uint8_t value)
{
    ARCEMU_ASSERT(index < m_valuesCount);

    if (offset > 3)
    {
        LOG_DEBUG("wrong offset %u", offset);
        return;
    }

    if (uint8_t(m_uint32Values[index] >> (offset * 8)) != value)
    {
        m_uint32Values[index] &= ~uint32_t(uint32_t(0xFF) << (offset * 8));
        m_uint32Values[index] |= uint32_t(uint32_t(value) << (offset * 8));
        m_updateMask.SetBit(index);

        updateObject();
    }
}

uint8_t Object::getByteValue(uint16_t index, uint8_t offset) const
{
    ARCEMU_ASSERT(index < m_valuesCount);
    ARCEMU_ASSERT(offset < 4);
    return *(((uint8_t*)&m_uint32Values[index]) + offset);
}

void Object::setByteFlag(uint16_t index, uint8_t offset, uint8_t newFlag)
{
    ARCEMU_ASSERT(index < m_valuesCount);

    if (offset > 3)
    {
        LOG_DEBUG("wrong offset %u", offset);
        return;
    }

    if (!(uint8_t(m_uint32Values[index] >> (offset * 8)) & newFlag))
    {
        m_uint32Values[index] |= uint32_t(uint32_t(newFlag) << (offset * 8));

        m_updateMask.SetBit(index);

        updateObject();
    }
}

void Object::removeByteFlag(uint16_t index, uint8_t offset, uint8_t oldFlag)
{
    ARCEMU_ASSERT(index < m_valuesCount);

    if (offset > 3)
    {
        LOG_DEBUG("wrong offset %u", offset);
        return;
    }

    if (uint8_t(m_uint32Values[index] >> (offset * 8)) & oldFlag)
    {
        m_uint32Values[index] &= ~uint32_t(uint32_t(oldFlag) << (offset * 8));

        m_updateMask.SetBit(index);

        updateObject();
    }
}

bool Object::hasByteFlag(uint16_t index, uint8_t offset, uint8_t flag)
{
    return ((getByteValue(index, offset) & flag) != 0);
}

void Object::setUInt16Value(uint16_t index, uint8_t offset, uint16_t value)
{
    ARCEMU_ASSERT(index < m_valuesCount);

    if (offset > 1)
    {
        LOG_DEBUG("wrong offset %u", offset);
        return;
    }

    if (uint16_t(m_uint32Values[index] >> (offset * 16)) != value)
    {
        m_uint32Values[index] &= ~uint32_t(uint32_t(0xFFFF) << (offset * 16));
        m_uint32Values[index] |= uint32_t(uint32_t(value) << (offset * 16));
        m_updateMask.SetBit(index);

        updateObject();
    }
}

uint16_t Object::getUInt16Value(uint16_t index, uint8_t offset) const
{
    ARCEMU_ASSERT(index < m_valuesCount);
    ARCEMU_ASSERT(offset < 2);
    return *(((uint16_t*)&m_uint32Values[index]) + offset);
}

void Object::setUInt32Value(uint16_t index, uint32_t value)
{
    ARCEMU_ASSERT(index < m_valuesCount);

    if (m_uint32Values[index] != value)
    {
        m_uint32Values[index] = value;
        m_updateMask.SetBit(index);

        updateObject();
    }

    if (IsUnit())
    {
        static_cast<Unit*>(this)->HandleUpdateFieldChange(index);
    }

    if (IsPlayer())
    {
        static_cast<Player*>(this)->HandleUpdateFieldChanged(index);
    }

    // Group update handling
    if (IsPlayer())
    {
        switch (index)
        {
            case UNIT_FIELD_POWER1:
            case UNIT_FIELD_POWER2:
            case UNIT_FIELD_POWER4:
#if VERSION_STRING == WotLK
            case UNIT_FIELD_POWER7:
#endif
                static_cast<Unit*>(this)->SendPowerUpdate(true);
                break;
            default:
                break;
        }
    }
    else if (IsCreature())
    {
        switch (index)
        {
            case UNIT_FIELD_POWER1:
            case UNIT_FIELD_POWER2:
            case UNIT_FIELD_POWER3:
            case UNIT_FIELD_POWER4:
#if VERSION_STRING == WotLK
            case UNIT_FIELD_POWER7:
#endif
                static_cast<Creature*>(this)->SendPowerUpdate(false);
                break;
            default:
                break;
        }
    }
}

uint32_t Object::getUInt32Value(uint16_t index) const
{
    ARCEMU_ASSERT(index < m_valuesCount);
    return m_uint32Values[index];
}

void Object::setUInt64Value(uint16_t index, uint64_t value)
{
    ARCEMU_ASSERT(index + 1 < m_valuesCount);

    uint64_t* _value = reinterpret_cast<uint64_t*>(&m_uint32Values[index]);

    if (*_value != value)
    {
        *_value = value;

        m_updateMask.SetBit(index);
        m_updateMask.SetBit(index + 1);

        updateObject();
    }
}

uint64_t Object::getUInt64Value(uint16_t index) const
{
    ARCEMU_ASSERT(index + 1 < m_valuesCount);
    return *reinterpret_cast<uint64_t*>(&m_uint32Values[index]);
}

void Object::setFloatValue(uint16_t index, float value)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    if (m_floatValues[index] == value)
    {
        return;
    }

    m_floatValues[index] = value;
    m_updateMask.SetBit(index);

    updateObject();
}

float Object::getFloatValue(uint16_t index) const
{
    ARCEMU_ASSERT(index < m_valuesCount);
    return m_floatValues[index];
}

//////////////////////////////////////////////////////////////////////////////////////////
// Object update
void Object::updateObject()
{
    if (IsInWorld() && m_objectUpdated == false)
    {
        m_mapMgr->ObjectUpdated(this);
        m_objectUpdated = true;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Position functions
bool Object::isInRange(LocationVector location, float square_r) const
{
    return getDistanceSq(location) <= square_r;
}

bool Object::isInRange(float x, float y, float z, float square_r) const
{
    return getDistanceSq(x, y, z) <= square_r;
}

float Object::getDistanceSq(LocationVector comp) const
{
    return comp.distanceSquare(m_position);
}

float Object::getDistanceSq(float x, float y, float z) const
{
    return m_position.distanceSquare(x, y, z);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Spell functions
Spell* Object::getCurrentSpell(CurrentSpellType spellType) const
{
    return m_currentSpell[spellType];
}

Spell* Object::getCurrentSpellById(uint32_t spellId) const
{
    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        if (m_currentSpell[i] == nullptr)
            continue;
        if (m_currentSpell[i]->GetSpellInfo()->getId() == spellId)
            return m_currentSpell[i];
    }
    return nullptr;
}

void Object::setCurrentSpell(Spell* curSpell)
{
    ARCEMU_ASSERT(curSpell != nullptr); // curSpell cannot be nullptr

    // Get current spell type
    CurrentSpellType spellType = CURRENT_GENERIC_SPELL;
    if (curSpell->GetSpellInfo()->getAttributes() & (ATTRIBUTES_ON_NEXT_ATTACK | ATTRIBUTES_ON_NEXT_SWING_2))
    {
        // Melee spell
        spellType = CURRENT_MELEE_SPELL;
    }
    else if (curSpell->GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_AUTOREPEAT)
    {
        // Autorepeat spells (Auto shot / Shoot (wand))
        spellType = CURRENT_AUTOREPEAT_SPELL;
    }
    else if (curSpell->GetSpellInfo()->getAttributesEx() & (ATTRIBUTESEX_CHANNELED_1 | ATTRIBUTESEX_CHANNELED_2))
    {
        // Channeled spells
        spellType = CURRENT_CHANNELED_SPELL;
    }

    // We've already set this spell to current spell, ignore
    if (curSpell == m_currentSpell[spellType])
        return;

    // Interrupt spell with same spell type, but ignore delayed spells
    interruptSpellWithSpellType(spellType, false);

    // Handle spelltype specific cases
    switch (spellType)
    {
        case CURRENT_GENERIC_SPELL:
        {
            // Generic spells break channeled spells, but ignore delayed spells
            interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL, false);

            if (m_currentSpell[CURRENT_AUTOREPEAT_SPELL] != nullptr)
            {
                // Generic spells do not break Auto Shot
                if (m_currentSpell[CURRENT_AUTOREPEAT_SPELL]->GetSpellInfo()->getId() != 75)
                    interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
            }
            break;
        }
        case CURRENT_CHANNELED_SPELL:
        {
            // Channeled spells break generic spells, but ignore delayed spells
            interruptSpellWithSpellType(CURRENT_GENERIC_SPELL, false);
            // as well break delayed channeled spells too
            interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);

            // Also break autorepeat spells, unless it's Auto Shot
            if (m_currentSpell[CURRENT_AUTOREPEAT_SPELL] != nullptr && m_currentSpell[CURRENT_AUTOREPEAT_SPELL]->GetSpellInfo()->getId() != 75)
            {
                interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
            }
            break;
        }
        case CURRENT_AUTOREPEAT_SPELL:
        {
            // Other autorepeats than Auto Shot break non-delayed generic and channeled spells
            if (curSpell->GetSpellInfo()->getId() != 75)
            {
                interruptSpellWithSpellType(CURRENT_GENERIC_SPELL, false);
                interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL, false);
            }

            if (IsPlayer())
            {
                static_cast<Player*>(this)->m_FirstCastAutoRepeat = true;
            }
            break;
        }
        default:
            break;
    }

    // If spell is not yet cancelled, force it
    if (m_currentSpell[spellType] != nullptr)
    {
        m_currentSpell[spellType]->finish(false);
    }

    // Set new current spell
    m_currentSpell[spellType] = curSpell;
}

void Object::interruptSpell(uint32_t spellId, bool checkMeleeSpell, bool checkDelayed)
{
    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        if (!checkMeleeSpell && i == CURRENT_MELEE_SPELL)
            continue;

        if (m_currentSpell[i] != nullptr &&
            (spellId == 0 || m_currentSpell[i]->GetSpellInfo()->getId() == spellId))
        {
            interruptSpellWithSpellType(CurrentSpellType(i), checkDelayed);
        }
    }
}

void Object::interruptSpellWithSpellType(CurrentSpellType spellType, bool /*checkDelayed*/)
{
    Spell* curSpell = m_currentSpell[spellType];
    if (curSpell != nullptr)
    {
        if (spellType == CURRENT_AUTOREPEAT_SPELL)
        {
            if (IsPlayer() && IsInWorld())
            {
                // Send server-side cancel message
                static_cast<Player*>(this)->GetSession()->OutPacket(SMSG_CANCEL_AUTO_REPEAT);
            }
        }

        // Cancel spell
        curSpell->cancel();
        m_currentSpell[spellType] = nullptr;
    }
}

bool Object::isCastingNonMeleeSpell(bool /*checkDelayed = true*/, bool skipChanneled /*= false*/, bool skipAutorepeat /*= false*/, bool isAutoshoot /*= false*/) const
{
    // Check from generic spells, ignore finished spells
    if (m_currentSpell[CURRENT_GENERIC_SPELL] != nullptr && m_currentSpell[CURRENT_GENERIC_SPELL]->getState() != SPELL_STATE_FINISHED && m_currentSpell[CURRENT_GENERIC_SPELL]->getCastTimeLeft() > 0 &&
        (!isAutoshoot || !(m_currentSpell[CURRENT_GENERIC_SPELL]->GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_NOT_RESET_AUTO_ATTACKS)))
    {
        return true;
    }

    // If not skipped, check from channeled spells
    if (!skipChanneled && m_currentSpell[CURRENT_CHANNELED_SPELL] != nullptr && m_currentSpell[CURRENT_CHANNELED_SPELL]->getState() != SPELL_STATE_FINISHED &&
        (!isAutoshoot || !(m_currentSpell[CURRENT_CHANNELED_SPELL]->GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_NOT_RESET_AUTO_ATTACKS)))
    {
        return true;
    }

    // If not skipped, check from autorepeat spells
    if (!skipAutorepeat && m_currentSpell[CURRENT_AUTOREPEAT_SPELL] != nullptr)
    {
        return true;
    }
    return false;
}

Spell* Object::findCurrentCastedSpellBySpellId(uint32_t spellId)
{
    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        if (m_currentSpell[i] == nullptr)
            continue;
        if (m_currentSpell[i]->GetSpellInfo()->getId() == spellId)
            return m_currentSpell[i];
    }
    return nullptr;
}

void Object::_UpdateSpells(uint32_t time)
{
    if (m_currentSpell[CURRENT_AUTOREPEAT_SPELL] != nullptr && IsPlayer())
    {
        static_cast<Player*>(this)->updateAutoRepeatSpell();
    }

    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        if (m_currentSpell[i] != nullptr)
        {
            // Remove finished spells from object's current spell array
            if (m_currentSpell[i]->getState() == SPELL_STATE_FINISHED)
            {
                m_currentSpell[i] = nullptr;
            }
            // Update spells with other state
            else
            {
                m_currentSpell[i]->Update(time);
            }
        }
    }
}
// MIT End

Object::Object() : m_position(0, 0, 0, 0), m_spawnLocation(0, 0, 0, 0)
{
    m_mapId = MAPID_NOT_IN_WORLD;
    m_zoneId = 0;

    m_uint32Values = nullptr;
    m_objectUpdated = false;

    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        m_currentSpell[i] = nullptr;
    }
    m_valuesCount = 0;

    m_phase = 1;                //Set the default phase: 00000000 00000000 00000000 00000001

    m_mapMgr = nullptr;
    m_mapCell_x = m_mapCell_y = uint32(-1);

    m_faction = nullptr;
    m_factionDBC = nullptr;

    m_instanceId = INSTANCEID_NOT_IN_WORLD;
    Active = false;
    m_inQueue = false;
    m_loadedFromDB = false;

    m_objectType = TYPE_OBJECT;
    m_objectTypeId = TYPEID_OBJECT;
    m_updateFlag = UPDATEFLAG_NONE;

    m_objectsInRange.clear();
    m_inRangePlayers.clear();
    m_oppFactsInRange.clear();
    m_sameFactsInRange.clear();

    Active = false;
}

Object::~Object()
{
    if (!IsItem())
        ARCEMU_ASSERT(!m_inQueue);

    ARCEMU_ASSERT(!IsInWorld());

    // for linux
    m_instanceId = INSTANCEID_NOT_IN_WORLD;

    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        if (m_currentSpell[i] != nullptr)
        {
            interruptSpellWithSpellType(CurrentSpellType(i));
        }
    }

    //avoid leaving traces in eventmanager. Have to work on the speed. Not all objects ever had events so list iteration can be skipped
    sEventMgr.RemoveEvents(this);
}

::DBC::Structures::AreaTableEntry const* Object::GetArea()
{
    if (!this->IsInWorld()) return nullptr;

    auto map_mgr = this->GetMapMgr();
    if (!map_mgr) return nullptr;

    auto area_flag = map_mgr->GetAreaFlag(this->GetPositionX(), this->GetPositionY(), this->GetPositionZ());
    auto at = MapManagement::AreaManagement::AreaStorage::GetAreaByFlag(area_flag);
    if (!at)
        at = MapManagement::AreaManagement::AreaStorage::GetAreaByMapId(this->GetMapId());

    return at;
}

void Object::_Create(uint32 mapid, float x, float y, float z, float ang)
{
    m_mapId = mapid;
    m_position.ChangeCoords(x, y, z, ang);
    m_spawnLocation.ChangeCoords(x, y, z, ang);
    m_lastMapUpdatePosition.ChangeCoords(x, y, z, ang);
}

uint32 Object::BuildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target)
{
    if (!target)
        return 0;

    uint8 updatetype = UPDATETYPE_CREATE_OBJECT;
    uint16 updateflags = m_updateFlag;

#if VERSION_STRING != Cata
    if (target == this)
    {
        updateflags |= UPDATEFLAG_SELF;
    }

    if (updateflags & UPDATEFLAG_HAS_POSITION)
    {
        if (IsType(TYPE_DYNAMICOBJECT) || IsType(TYPE_CORPSE) || IsType(TYPE_PLAYER))
            updatetype = UPDATETYPE_CREATE_YOURSELF;

        if (target->IsPet() && updateflags & UPDATEFLAG_SELF)
            updatetype = UPDATETYPE_CREATE_YOURSELF;

        if (IsType(TYPE_GAMEOBJECT))
        {
            switch (((GameObject*)this)->GetType())
            {
                case GAMEOBJECT_TYPE_TRAP:
                case GAMEOBJECT_TYPE_DUEL_ARBITER:
                case GAMEOBJECT_TYPE_FLAGSTAND:
                case GAMEOBJECT_TYPE_FLAGDROP:
                    updatetype = UPDATETYPE_CREATE_YOURSELF;
                    break;
                case GAMEOBJECT_TYPE_TRANSPORT:
                    updateflags |= UPDATEFLAG_TRANSPORT;
                    break;
                default:
                    break;
            }
        }

        if (IsType(TYPE_UNIT))
        {
            if (((Unit*)this)->GetTargetGUID() != 0)
                updateflags |= UPDATEFLAG_HAS_TARGET;
        }
    }

    if (IsVehicle())
        updateflags |= UPDATEFLAG_VEHICLE;
#else
    switch (m_objectTypeId)
    {
        case TYPEID_CORPSE:
        case TYPEID_DYNAMICOBJECT:
            updateflags = UPDATEFLAG_HAS_POSITION; // UPDATEFLAG_HAS_STATIONARY_POSITION
            updatetype = 2;
            break;

        case TYPEID_GAMEOBJECT:
            updateflags = UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION;
            //flags = 0x0040 | 0x0200; // UPDATEFLAG_HAS_STATIONARY_POSITION | UPDATEFLAG_ROTATION
            updatetype = 2;
            break;

        case TYPEID_UNIT:
        case TYPEID_PLAYER:
            updateflags = UPDATEFLAG_LIVING;
            updatetype = 2;
            break;
    }

    if (target == this)
    {
        // player creating self
        updateflags |= UPDATEFLAG_SELF;  // UPDATEFLAG_SELF
        updatetype = 2; // UPDATEFLAG_CREATE_OBJECT_SELF
    }

    if (IsUnit())
    {
        if (static_cast< Unit* >(this)->GetTargetGUID())
            updateflags |= UPDATEFLAG_HAS_TARGET; // UPDATEFLAG_HAS_ATTACKING_TARGET
    }
#endif

    // we shouldn't be here, under any circumstances, unless we have a wowguid..
    ARCEMU_ASSERT(m_wowGuid.GetNewGuidLen() > 0);
    // build our actual update
    *data << uint8(updatetype);
    *data << m_wowGuid;
    *data << uint8(m_objectTypeId);

    //\todo remove flags (0) from function call.
    _BuildMovementUpdate(data, updateflags, target);


    // we have dirty data, or are creating for ourself.
    UpdateMask updateMask;
    updateMask.SetCount(m_valuesCount);
    _SetCreateBits(&updateMask, target);

    // this will cache automatically if needed
    _BuildValuesUpdate(data, &updateMask, target);

    // update count: 1 ;)
    return 1;
}


//That is dirty fix it actually creates update of 1 field with
//the given value ignoring existing changes in fields and so on
//useful if we want update this field for certain players
//NOTE: it does not change fields. This is also very fast method
WorldPacket* Object::BuildFieldUpdatePacket(uint32 index, uint32 value)
{
    // uint64 guidfields = GetGUID();
    // uint8 guidmask = 0;
    WorldPacket* packet = new WorldPacket(1500);
    packet->SetOpcode(SMSG_UPDATE_OBJECT);

    *packet << uint32(1);                   //number of update/create blocks
#if VERSION_STRING == TBC
    *packet << uint8(0);                    //unknown removed in 3.1
#endif

    *packet << uint8(UPDATETYPE_VALUES);    // update type == update
    *packet << GetNewGUID();

    uint32 mBlocks = index / 32 + 1;
    *packet << uint8(mBlocks);

    for (uint32 dword_n = mBlocks - 1; dword_n; dword_n--)
        *packet << uint32(0);

    *packet << (((uint32)(1)) << (index % 32));
    *packet << value;

    return packet;
}

void Object::BuildFieldUpdatePacket(Player* Target, uint32 Index, uint32 Value)
{
    ByteBuffer buf(500);
    buf << uint8(UPDATETYPE_VALUES);
    buf << GetNewGUID();

    uint32 mBlocks = Index / 32 + 1;
    buf << uint8(mBlocks);

    for (uint32 dword_n = mBlocks - 1; dword_n; dword_n--)
        buf << uint32(0);

    buf << (((uint32)(1)) << (Index % 32));
    buf << Value;

    Target->PushUpdateData(&buf, 1);
}

void Object::BuildFieldUpdatePacket(ByteBuffer* buf, uint32 Index, uint32 Value)
{
    *buf << uint8(UPDATETYPE_VALUES);
    *buf << GetNewGUID();

    uint32 mBlocks = Index / 32 + 1;
    *buf << uint8(mBlocks);

    for (uint32 dword_n = mBlocks - 1; dword_n; dword_n--)
        *buf << uint32(0);

    *buf << (((uint32)(1)) << (Index % 32));
    *buf << Value;
}

uint32 Object::BuildValuesUpdateBlockForPlayer(ByteBuffer* data, Player* target)
{
    UpdateMask updateMask;
    updateMask.SetCount(m_valuesCount);
    _SetUpdateBits(&updateMask, target);
    for (uint32 x = 0; x < m_valuesCount; ++x)
    {
        if (updateMask.GetBit(x))
        {
            *data << uint8(UPDATETYPE_VALUES);              // update type == update
            ARCEMU_ASSERT(m_wowGuid.GetNewGuidLen() > 0);
            *data << m_wowGuid;

            _BuildValuesUpdate(data, &updateMask, target);
            return 1;
        }
    }

    return 0;
}

uint32 Object::BuildValuesUpdateBlockForPlayer(ByteBuffer* buf, UpdateMask* mask)
{
    // returns: update count
    // update type == update
    *buf << uint8(UPDATETYPE_VALUES);

    ARCEMU_ASSERT(m_wowGuid.GetNewGuidLen() > 0);
    *buf << m_wowGuid;

    _BuildValuesUpdate(buf, mask, nullptr);

    // 1 update.
    return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Build the Movement Data portion of the update packet Fills the data with this object's movement/speed info
///\todo rewrite this stuff, document unknown fields and flags
uint32 TimeStamp();

#if VERSION_STRING < WotLK
void Object::_BuildMovementUpdate(ByteBuffer* data, uint16 flags, Player* target)
{
    uint32 flags2 = 0;

    ByteBuffer* splinebuf = (m_objectTypeId == TYPEID_UNIT) ? target->GetAndRemoveSplinePacket(GetGUID()) : 0;

    if (splinebuf != NULL)
    {
        flags2 |= MOVEFLAG_SPLINE_ENABLED | MOVEFLAG_MOVE_FORWARD;	   //1=move forward
        if (IsCreature())
        {
            if (static_cast<Unit*>(this)->GetAIInterface()->HasWalkMode(WALKMODE_WALK))
                flags2 |= MOVEFLAG_WALK;
        }
    }

    uint16 moveflags2 = 0;      // mostly seem to be used by vehicles to control what kind of movement is allowed

    *data << uint8(flags);

    Player* pThis = NULL;
    MovementInfo* moveinfo = NULL;
    if (IsPlayer())
    {
        pThis = static_cast< Player* >(this);
        if (pThis->GetSession())
            moveinfo = pThis->GetSession()->GetMovementInfo();
    }
    Creature* uThis = NULL;
    if (IsCreature())
        uThis = static_cast< Creature* >(this);

    if (flags & UPDATEFLAG_LIVING)  //0x20
    {
        /*if (pThis && pThis->obj_movement_info.transporter_info.guid != 0)
        flags2 |= MOVEFLAG_TRANSPORT; //0x200
        else if (uThis != NULL && obj_movement_info.transporter_info.guid != 0 && uThis->obj_movement_info.transporter_info.guid != 0)
        flags2 |= MOVEFLAG_TRANSPORT; //0x200*/

        // Zyres: If a unit has this flag, add it to the update packet, otherwise not.
        if (pThis && pThis->HasUnitMovementFlag(MOVEFLAG_TRANSPORT))
            flags2 |= MOVEFLAG_TRANSPORT;
        else if (uThis && uThis->HasUnitMovementFlag(MOVEFLAG_TRANSPORT))
            flags2 |= MOVEFLAG_TRANSPORT;

        if ((pThis != NULL) && pThis->isRooted())
            flags2 |= MOVEFLAG_ROOTED;
        else if ((uThis != NULL) && uThis->isRooted())
            flags2 |= MOVEFLAG_ROOTED;

        if (uThis != NULL)
        {
            // Don't know what this is, but I've only seen it applied to spirit healers. maybe some sort of invisibility flag? :/
            switch (GetEntry())
            {
                case 6491:      // Spirit Healer
                case 13116:     // Alliance Spirit Guide
                case 13117:     // Horde Spirit Guide
                {
                    flags2 |= MOVEFLAG_WATER_WALK;      //0x10000000
                }
                break;
            }

            if (uThis->GetAIInterface()->IsFlying())
                flags2 |= MOVEFLAG_DISABLEGRAVITY;        //0x400 Zack : Teribus the Cursed had flag 400 instead of 800 and he is flying all the time
            if (uThis->GetAIInterface()->onGameobject)
                flags2 |= MOVEFLAG_ROOTED;
            if (uThis->GetCreatureProperties()->extra_a9_flags)
            {
                //do not send shit we can't honor
#define UNKNOWN_FLAGS2 (0x00002000 | 0x04000000 | 0x08000000)
                uint32 inherit = uThis->GetCreatureProperties()->extra_a9_flags & UNKNOWN_FLAGS2;
                flags2 |= inherit;
            }
        }

        *data << uint32(flags2);

        *data << uint8(moveflags2);

        *data <<Util::getMSTime(); // this appears to be time in ms but can be any thing. Maybe packet serializer ?

                              // this stuff:
                              //   0x01 -> Enable Swimming?
                              //   0x04 -> ??
                              //   0x10 -> disables movement compensation and causes players to jump around all the place
                              //   0x40 -> disables movement compensation and causes players to jump around all the place

                              //Send position data, every living thing has these
        *data << float(m_position.x);
        *data << float(m_position.y);
        *data << float(m_position.z);
        *data << float(m_position.o);

        if (flags2 & MOVEFLAG_TRANSPORT) //0x0200
        {
            *data << WoWGuid(obj_movement_info.transporter_info.guid);
            *data << float(GetTransPositionX());
            *data << float(GetTransPositionY());
            *data << float(GetTransPositionZ());
            *data << float(GetTransPositionO());
            *data << uint32(GetTransTime());
            *data << uint8(GetTransSeat());
        }

        if ((flags2 & (MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || (moveflags2 & MOVEFLAG2_ALLOW_PITCHING))   // 0x2000000+0x0200000 flying/swimming, || sflags & SMOVE_FLAG_ENABLE_PITCH
        {
            if (pThis && moveinfo)
                *data << moveinfo->pitch;
            else
                *data << float(0); //pitch
        }

        if (pThis && moveinfo)
            *data << moveinfo->fall_time;
        else
            *data << uint32(0); //last fall time

        if (flags2 & MOVEFLAG_REDIRECTED)   // 0x00001000
        {
            if (moveinfo != NULL)
            {
                *data << moveinfo->redirectVelocity;
                *data << moveinfo->redirectSin;
                *data << moveinfo->redirectCos;
                *data << moveinfo->redirect2DSpeed;
            }
            else
            {
                *data << float(0);
                *data << float(1.0);
                *data << float(0);
                *data << float(0);
            }
        }

        if (Unit* unit = static_cast<Unit*>(this))
        {
            *data << unit->getSpeedForType(TYPE_WALK);
            *data << unit->getSpeedForType(TYPE_RUN);
            *data << unit->getSpeedForType(TYPE_RUN_BACK);
            *data << unit->getSpeedForType(TYPE_SWIM);
            *data << unit->getSpeedForType(TYPE_SWIM_BACK);
            *data << unit->getSpeedForType(TYPE_FLY);
            *data << unit->getSpeedForType(TYPE_FLY_BACK);
            *data << unit->getSpeedForType(TYPE_TURN_RATE);
        }
        else                                //\todo Zyres: this is ridiculous... only units have these types, but this function is a mess so don't breake anything.
        {
            *data << float(2.5f);
            *data << float(7.0f);
            *data << float(4.5f);
            *data << float(4.722222f);
            *data << float(2.5f);
            *data << float(7.0f);
            *data << float(4.5f);
            *data << float(3.141594f);

        }

        if (flags2 & MOVEFLAG_SPLINE_ENABLED)   //VLack: On Mangos this is a nice spline movement code, but we never had such... Also, at this point we haven't got this flag, that's for sure, but fail just in case...
        {
            if (splinebuf != NULL)
            {
                data->append(*splinebuf);
                //delete splinebuf;
            }
            else
                *data << float(0.0f);
        }
    }
    else        // No UPDATEFLAG_LIVING
    {
        if (flags & UPDATEFLAG_POSITION)        //0x0100
        {
            *data << uint8(0);                  //some say it is like parent guid ?
            *data << float(m_position.x);
            *data << float(m_position.y);
            *data << float(m_position.z);
            *data << float(m_position.x);
            *data << float(m_position.y);
            *data << float(m_position.z);
            *data << float(m_position.o);

            if (IsCorpse())
                *data << float(m_position.o);   //VLack: repeat the orientation!
            else
                *data << float(0);
        }
        else if (flags & UPDATEFLAG_HAS_POSITION)  //0x40
        {

            if (flags & UPDATEFLAG_TRANSPORT && m_uint32Values[GAMEOBJECT_TYPE_ID] == GAMEOBJECT_TYPE_MO_TRANSPORT)
            {
                *data << float(0);
                *data << float(0);
                *data << float(0);
            }
            else
            {
                *data << float(m_position.x);
                *data << float(m_position.y);
                *data << float(m_position.z);
            }
            *data << float(m_position.o);
        }
    }


    if (flags & UPDATEFLAG_LOWGUID)     //0x08
        *data << GetLowGUID();

    if (flags & UPDATEFLAG_HIGHGUID)    //0x10
        *data << GetHighGUID();

    if (flags & UPDATEFLAG_HAS_TARGET)  //0x04
    {
        if (IsUnit())
            FastGUIDPack(*data, static_cast<Unit*>(this)->GetTargetGUID());	//some compressed GUID
        else
            *data << uint64(0);
    }

    if (flags & UPDATEFLAG_TRANSPORT)   //0x2
    {
        *data <<Util::getMSTime();
    }

    if (flags & UPDATEFLAG_ROTATION)   //0x0200
    {
        if (IsGameObject())
            *data << static_cast< GameObject* >(this)->GetRotation();
    }
}
#endif

#if VERSION_STRING == WotLK
void Object::_BuildMovementUpdate(ByteBuffer* data, uint16 flags, Player* target)
{
    uint32 flags2 = 0;

    ByteBuffer* splinebuf = (m_objectTypeId == TYPEID_UNIT) ? target->GetAndRemoveSplinePacket(GetGUID()) : 0;

    if (splinebuf != nullptr)
    {
        flags2 |= MOVEFLAG_SPLINE_ENABLED | MOVEFLAG_MOVE_FORWARD;	   //1=move forward
        if (IsCreature())
        {
            if (static_cast<Unit*>(this)->GetAIInterface()->hasWalkMode(WALKMODE_WALK))
                flags2 |= MOVEFLAG_WALK;
        }
    }

    uint16 moveflags2 = 0;      // mostly seem to be used by vehicles to control what kind of movement is allowed

    if (IsVehicle())
    {
        Unit* u = static_cast< Unit* >(this);
        if (u->GetVehicleComponent() != nullptr)
            moveflags2 |= u->GetVehicleComponent()->GetMoveFlags2();

        if (IsCreature())
        {
            if (static_cast< Unit* >(this)->HasAuraWithName(SPELL_AURA_ENABLE_FLIGHT))
                flags2 |= (MOVEFLAG_DISABLEGRAVITY | MOVEFLAG_FLYING);
        }

    }

    *data << uint16(flags);

    Player* pThis = nullptr;
    MovementInfo* moveinfo = nullptr;
    if (IsPlayer())
    {
        pThis = static_cast< Player* >(this);
        if (pThis->GetSession())
            moveinfo = pThis->GetSession()->GetMovementInfo();
    }
    Creature* uThis = nullptr;
    if (IsCreature())
        uThis = static_cast< Creature* >(this);

    if (flags & UPDATEFLAG_LIVING)  //0x20
    {
        /*if (pThis && pThis->obj_movement_info.transporter_info.guid != 0)
            flags2 |= MOVEFLAG_TRANSPORT; //0x200
        else if (uThis != NULL && obj_movement_info.transporter_info.guid != 0 && uThis->obj_movement_info.transporter_info.guid != 0)
            flags2 |= MOVEFLAG_TRANSPORT; //0x200*/

        // Zyres: If a unit has this flag, add it to the update packet, otherwise not.
        if (pThis && pThis->HasUnitMovementFlag(MOVEFLAG_TRANSPORT))
            flags2 |= MOVEFLAG_TRANSPORT;
        else if (uThis && uThis->HasUnitMovementFlag(MOVEFLAG_TRANSPORT))
            flags2 |= MOVEFLAG_TRANSPORT;

        if ((pThis != nullptr) && pThis->isRooted())
            flags2 |= MOVEFLAG_ROOTED;
        else if ((uThis != nullptr) && uThis->isRooted())
            flags2 |= MOVEFLAG_ROOTED;

        if (uThis != nullptr)
        {
            // Don't know what this is, but I've only seen it applied to spirit healers. maybe some sort of invisibility flag? :/
            switch (GetEntry())
            {
                case 6491:      // Spirit Healer
                case 13116:     // Alliance Spirit Guide
                case 13117:     // Horde Spirit Guide
                {
                    flags2 |= MOVEFLAG_WATER_WALK;      //0x10000000
                }
                break;
            }

            if (uThis->GetAIInterface()->isFlying())
                flags2 |= MOVEFLAG_DISABLEGRAVITY;        //0x400 Zack : Teribus the Cursed had flag 400 instead of 800 and he is flying all the time
            if (uThis->GetAIInterface()->onGameobject)
                flags2 |= MOVEFLAG_ROOTED;
            if (uThis->GetCreatureProperties()->extra_a9_flags)
            {
                //do not send shit we can't honor
#define UNKNOWN_FLAGS2 (0x00002000 | 0x04000000 | 0x08000000)
                uint32 inherit = uThis->GetCreatureProperties()->extra_a9_flags & UNKNOWN_FLAGS2;
                flags2 |= inherit;
            }
        }

        *data << uint32(flags2);

        *data << uint16(moveflags2);

        *data <<Util::getMSTime(); // this appears to be time in ms but can be any thing. Maybe packet serializer ?

        // this stuff:
        //   0x01 -> Enable Swimming?
        //   0x04 -> ??
        //   0x10 -> disables movement compensation and causes players to jump around all the place
        //   0x40 -> disables movement compensation and causes players to jump around all the place

        //Send position data, every living thing has these
        *data << float(m_position.x);
        *data << float(m_position.y);
        *data << float(m_position.z);
        *data << float(m_position.o);

        if (flags2 & MOVEFLAG_TRANSPORT) //0x0200
        {
            *data << WoWGuid(obj_movement_info.transporter_info.guid);
            *data << float(GetTransPositionX());
            *data << float(GetTransPositionY());
            *data << float(GetTransPositionZ());
            *data << float(GetTransPositionO());
            *data << uint32(GetTransTime());
            *data << uint8(GetTransSeat());
        }

        if ((flags2 & (MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || (moveflags2 & MOVEFLAG2_ALLOW_PITCHING))   // 0x2000000+0x0200000 flying/swimming, || sflags & SMOVE_FLAG_ENABLE_PITCH
        {
            if (pThis && moveinfo)
                *data << moveinfo->pitch;
            else
                *data << float(0); //pitch
        }

        if (pThis && moveinfo)
            *data << moveinfo->fall_time;
        else
            *data << uint32(0); //last fall time

        if (flags2 & MOVEFLAG_REDIRECTED)   // 0x00001000
        {
            if (moveinfo != nullptr)
            {
                *data << moveinfo->redirectVelocity;
                *data << moveinfo->redirectSin;
                *data << moveinfo->redirectCos;
                *data << moveinfo->redirect2DSpeed;
            }
            else
            {
                *data << float(0);
                *data << float(1.0);
                *data << float(0);
                *data << float(0);
            }
        }

        if (Unit* unit = static_cast<Unit*>(this))
        {
            *data << unit->getSpeedForType(TYPE_WALK);
            *data << unit->getSpeedForType(TYPE_RUN);
            *data << unit->getSpeedForType(TYPE_RUN_BACK);
            *data << unit->getSpeedForType(TYPE_SWIM);
            *data << unit->getSpeedForType(TYPE_SWIM_BACK);
            *data << unit->getSpeedForType(TYPE_FLY);
            *data << unit->getSpeedForType(TYPE_FLY_BACK);
            *data << unit->getSpeedForType(TYPE_TURN_RATE);
            *data << unit->getSpeedForType(TYPE_PITCH_RATE);
        }
        else                                //\todo Zyres: this is ridiculous... only units have these types, but this function is a mess so don't breake anything.
        {
            *data << float(2.5f);
            *data << float(7.0f);
            *data << float(4.5f);
            *data << float(4.722222f);
            *data << float(2.5f);
            *data << float(7.0f);
            *data << float(4.5f);
            *data << float(3.141594f);
            *data << float(3.14f);
        }

        if (flags2 & MOVEFLAG_SPLINE_ENABLED)   //VLack: On Mangos this is a nice spline movement code, but we never had such... Also, at this point we haven't got this flag, that's for sure, but fail just in case...
        {
            if (splinebuf != nullptr)
            {
                data->append(*splinebuf);
                //delete splinebuf;
            }
            else
                *data << float(0.0f);
        }
    }
    else        // No UPDATEFLAG_LIVING
    {
        if (flags & UPDATEFLAG_POSITION)        //0x0100
        {
            *data << uint8(0);                  //some say it is like parent guid ?
            *data << float(m_position.x);
            *data << float(m_position.y);
            *data << float(m_position.z);
            *data << float(m_position.x);
            *data << float(m_position.y);
            *data << float(m_position.z);
            *data << float(m_position.o);

            if (IsCorpse())
                *data << float(m_position.o);   //VLack: repeat the orientation!
            else
                *data << float(0);
        }
        else if (flags & UPDATEFLAG_HAS_POSITION)  //0x40
        {
            if (flags & UPDATEFLAG_TRANSPORT && m_uint32Values[GAMEOBJECT_BYTES_1] == GAMEOBJECT_TYPE_MO_TRANSPORT)
            {
                *data << float(0);
                *data << float(0);
                *data << float(0);
            }
            else
            {
                *data << float(m_position.x);
                *data << float(m_position.y);
                *data << float(m_position.z);
            }
            *data << float(m_position.o);
        }
    }


    if (flags & UPDATEFLAG_LOWGUID)     //0x08
        *data << GetLowGUID();

    if (flags & UPDATEFLAG_HIGHGUID)    //0x10
        *data << GetHighGUID();

    if (flags & UPDATEFLAG_HAS_TARGET)  //0x04
    {
        if (IsUnit())
            FastGUIDPack(*data, static_cast<Unit*>(this)->GetTargetGUID());	//some compressed GUID
        else
            *data << uint64(0);
    }

    if (flags & UPDATEFLAG_TRANSPORT)   //0x2
    {
        *data <<Util::getMSTime();
    }

    if (flags & UPDATEFLAG_VEHICLE)
    {
        uint32 vehicleid = 0;

        if (IsCreature())
            vehicleid = static_cast< Creature* >(this)->GetCreatureProperties()->vehicleid;
        else
            if (IsPlayer())
                vehicleid = static_cast< Player* >(this)->mountvehicleid;

        *data << uint32(vehicleid);
        *data << float(GetOrientation());
    }

    if (flags & UPDATEFLAG_ROTATION)   //0x0200
    {
        if (IsGameObject())
            *data << static_cast< GameObject* >(this)->GetRotation();
    }
}
#endif

#if VERSION_STRING == Cata
void Object::_BuildMovementUpdate(ByteBuffer* data, uint16 updateFlags, Player* /*target*/)
{
    ObjectGuid Guid = GetGUID();

    data->writeBit(false);
    data->writeBit(false);
    data->writeBit(updateFlags & UPDATEFLAG_ROTATION);
    data->writeBit(updateFlags & UPDATEFLAG_ANIM_KITS);
    data->writeBit(updateFlags & UPDATEFLAG_HAS_TARGET);
    data->writeBit(updateFlags & UPDATEFLAG_SELF);
    data->writeBit(updateFlags & UPDATEFLAG_VEHICLE);
    data->writeBit(updateFlags & UPDATEFLAG_LIVING);
    data->writeBits(0, 24);
    data->writeBit(false);
    data->writeBit(updateFlags & UPDATEFLAG_POSITION);
    data->writeBit(updateFlags & UPDATEFLAG_HAS_POSITION);
    data->writeBit(updateFlags & UPDATEFLAG_TRANSPORT_ARR);
    data->writeBit(false);
    data->writeBit(updateFlags & UPDATEFLAG_TRANSPORT);

    bool hasTransport = false;
    bool isSplineEnabled = false;
    bool hasPitch = false;
    bool hasFallData = false;
    bool hasFallDirection = false;
    bool hasElevation = false;
    bool hasOrientation = !IsType(TYPE_ITEM);
    bool hasTimeStamp = true;
    bool hasTransportTime2 = false;
    bool hasTransportTime3 = false;

    if (IsType(TYPE_UNIT))
    {
        Unit* unit = (Unit*)this;
        hasTransport = !unit->movement_info.getTransportGuid().IsEmpty();
        isSplineEnabled = false; // unit->IsSplineEnabled();

        if (GetTypeId() == TYPEID_PLAYER)
        {
            hasPitch = unit->movement_info.getMovementStatusInfo().hasPitch;
            hasFallData = unit->movement_info.getMovementStatusInfo().hasFallData;
            hasFallDirection = unit->movement_info.getMovementStatusInfo().hasFallDirection;
            hasElevation = unit->movement_info.getMovementStatusInfo().hasSplineElevation;
            hasTransportTime2 = unit->movement_info.getMovementStatusInfo().hasTransportTime2;
            hasTransportTime3 = unit->movement_info.getMovementStatusInfo().hasTransportTime3;
        }
        else
        {
            hasPitch = unit->movement_info.hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) ||
                unit->movement_info.hasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING);
            hasFallData = unit->movement_info.hasMovementFlag2(MOVEFLAG2_INTERPOLATED_TURN);
            hasFallDirection = unit->movement_info.hasMovementFlag(MOVEFLAG_FALLING);
            hasElevation = unit->movement_info.hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION);
        }
    }

    if (updateFlags & UPDATEFLAG_LIVING)
    {
        Unit* unit = (Unit*)this;

        data->writeBit(!unit->movement_info.getMovementFlags());
        data->writeBit(!hasOrientation);

        data->writeBit(Guid[7]);
        data->writeBit(Guid[3]);
        data->writeBit(Guid[2]);

        if (unit->movement_info.getMovementFlags())
            data->writeBits(unit->movement_info.getMovementFlags(), 30);

        data->writeBit(false);
        data->writeBit(!hasPitch);
        data->writeBit(isSplineEnabled);
        data->writeBit(hasFallData);
        data->writeBit(!hasElevation);
        data->writeBit(Guid[5]);
        data->writeBit(hasTransport);
        data->writeBit(!hasTimeStamp);

        if (hasTransport)
        {
            ObjectGuid tGuid = unit->movement_info.getTransportGuid();

            data->writeBit(tGuid[1]);
            data->writeBit(hasTransportTime2);
            data->writeBit(tGuid[4]);
            data->writeBit(tGuid[0]);
            data->writeBit(tGuid[6]);
            data->writeBit(hasTransportTime3);
            data->writeBit(tGuid[7]);
            data->writeBit(tGuid[5]);
            data->writeBit(tGuid[3]);
            data->writeBit(tGuid[2]);
        }

        data->writeBit(Guid[4]);

        if (isSplineEnabled)
        {
            //Movement::PacketBuilder::WriteCreateBits(*unit->movespline, *data);
        }

        data->writeBit(Guid[6]);

        if (hasFallData)
            data->writeBit(hasFallDirection);

        data->writeBit(Guid[0]);
        data->writeBit(Guid[1]);

        data->writeBit(false);
        data->writeBit(!unit->movement_info.getMovementFlags2());

        if (unit->movement_info.getMovementFlags2())
            data->writeBits(unit->movement_info.getMovementFlags2(), 12);

    }

    if (updateFlags & UPDATEFLAG_POSITION)
    {
        ObjectGuid transGuid;

        data->writeBit(transGuid[5]);
        data->writeBit(hasTransportTime3);
        data->writeBit(transGuid[0]);
        data->writeBit(transGuid[3]);
        data->writeBit(transGuid[6]);
        data->writeBit(transGuid[1]);
        data->writeBit(transGuid[4]);
        data->writeBit(transGuid[2]);
        data->writeBit(hasTransportTime2);
        data->writeBit(transGuid[7]);
    }

    if (updateFlags & UPDATEFLAG_HAS_TARGET)
    {
        if (IsUnit())
        {
            ObjectGuid victimGuid = static_cast<Unit*>(this)->GetTargetGUID();

            data->writeBit(victimGuid[2]);
            data->writeBit(victimGuid[7]);
            data->writeBit(victimGuid[0]);
            data->writeBit(victimGuid[4]);
            data->writeBit(victimGuid[5]);
            data->writeBit(victimGuid[6]);
            data->writeBit(victimGuid[1]);
            data->writeBit(victimGuid[3]);
        }
        else
            data->writeBits(0, 8);
    }

    if (updateFlags & UPDATEFLAG_ANIM_KITS)
    {
        data->writeBit(true);
        data->writeBit(true);
        data->writeBit(true);
    }

    data->flushBits();

    if (updateFlags & UPDATEFLAG_LIVING)
    {
        Unit* unit = (Unit*)this;

        data->WriteByteSeq(Guid[4]);

        *data << unit->getSpeedForType(TYPE_RUN_BACK);

        if (hasFallData)
        {
            if (hasFallDirection)
            {
                *data << float(unit->movement_info.getJumpInfo().cosAngle);
                *data << float(unit->movement_info.getJumpInfo().xyspeed);
                *data << float(unit->movement_info.getJumpInfo().sinAngle);
            }

            *data << uint32_t(unit->movement_info.fetFallTime());
            *data << float(unit->movement_info.getJumpInfo().velocity);
        }

        *data << unit->getSpeedForType(TYPE_SWIM_BACK);

        if (hasElevation)
            *data << float(unit->movement_info.getSplineElevation());

        if (isSplineEnabled)
        {
            //Movement::PacketBuilder::WriteCreateBytes(*unit->movespline, *data);
        }

        *data << float(unit->GetPositionZ());
        data->WriteByteSeq(Guid[5]);

        if (hasTransport)
        {
            ObjectGuid tGuid = unit->movement_info.getTransportGuid();

            data->WriteByteSeq(tGuid[5]);
            data->WriteByteSeq(tGuid[7]);

            *data << uint32(unit->movement_info.getTransportTime());
            *data << float(normalizeOrientation(unit->movement_info.getTransportPosition()->o));

            if (hasTransportTime2)
                *data << uint32_t(unit->movement_info.getTransportTime2());

            *data << float(unit->movement_info.getTransportPosition()->y);
            *data << float(unit->movement_info.getTransportPosition()->x);

            data->WriteByteSeq(tGuid[3]);

            *data << float(unit->movement_info.getTransportPosition()->z);

            data->WriteByteSeq(tGuid[0]);

            if (hasTransportTime3)
                *data << uint32_t(unit->movement_info.fetFallTime());

            *data << int8_t(unit->movement_info.getTransportSeat());

            data->WriteByteSeq(tGuid[1]);
            data->WriteByteSeq(tGuid[6]);
            data->WriteByteSeq(tGuid[2]);
            data->WriteByteSeq(tGuid[4]);
        }

        *data << float(unit->GetPositionX());
        *data << float(unit->getSpeedForType(TYPE_PITCH_RATE));

        data->WriteByteSeq(Guid[3]);
        data->WriteByteSeq(Guid[0]);

        *data << float(unit->getSpeedForType(TYPE_SWIM));
        *data << float(unit->GetPositionY());

        data->WriteByteSeq(Guid[7]);
        data->WriteByteSeq(Guid[1]);
        data->WriteByteSeq(Guid[2]);

        *data << float(unit->getSpeedForType(TYPE_WALK));

        *data << uint32_t(Util::getMSTime());

        *data << float(unit->getSpeedForType(TYPE_FLY_BACK));

        data->WriteByteSeq(Guid[6]);

        *data << float(unit->getSpeedForType(TYPE_TURN_RATE));

        if (hasOrientation)
            *data << float(normalizeOrientation(unit->GetOrientation()));

        *data << unit->getSpeedForType(TYPE_RUN);

        if (hasPitch)
            *data << float(unit->movement_info.getPitch());

        *data << float(unit->getSpeedForType(TYPE_FLY));
    }

    if(updateFlags & UPDATEFLAG_VEHICLE)
    {
        uint32_t vehicleid = 0;

        if (IsCreature())
        {
            vehicleid = static_cast<Creature*>(this)->GetCreatureProperties()->vehicleid;
        }
        else
        {
            if (IsPlayer())
                vehicleid = static_cast<Player*>(this)->mountvehicleid;
        }

        *data << float(normalizeOrientation(((Object*)this)->GetOrientation()));
        *data << uint32_t(vehicleid);
    }

    if (updateFlags & UPDATEFLAG_POSITION)
    {
        ObjectGuid transGuid;

        data->WriteByteSeq(transGuid[0]);
        data->WriteByteSeq(transGuid[5]);

        if (hasTransportTime3)
            *data << uint32_t(0);

        data->WriteByteSeq(transGuid[3]);

        *data << float(0.0f);               // x offset

        data->WriteByteSeq(transGuid[4]);
        data->WriteByteSeq(transGuid[6]);
        data->WriteByteSeq(transGuid[1]);

        *data << uint32_t(0);               // transport time
        *data << float(0.0f);               // y offset

        data->WriteByteSeq(transGuid[2]);
        data->WriteByteSeq(transGuid[7]);

        *data << float(0.0f);               // z offset
        *data << int8_t(-1);                // transport seat
        *data << float(0.0f);               // o offset

        if (hasTransportTime2)
            *data << uint32_t(0);
    }

    if (updateFlags & UPDATEFLAG_ROTATION)
    {
        if (IsGameObject())
            *data << int64_t(static_cast<GameObject*>(this)->GetRotation());
    }

    if (updateFlags & UPDATEFLAG_TRANSPORT_ARR)
    {
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << uint8_t(0);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
    }

    if (updateFlags & UPDATEFLAG_HAS_POSITION)
    {
        *data << float(normalizeOrientation(((Object*)this)->GetOrientation()));
        *data << float(((Object*)this)->GetPositionX());
        *data << float(((Object*)this)->GetPositionY());
        *data << float(((Object*)this)->GetPositionZ());
    }

    if (updateFlags & UPDATEFLAG_HAS_TARGET)
    {
        if (IsUnit())
        {
            ObjectGuid victimGuid = static_cast<Unit*>(this)->GetTargetGUID();

            data->WriteByteSeq(victimGuid[4]);
            data->WriteByteSeq(victimGuid[0]);
            data->WriteByteSeq(victimGuid[3]);
            data->WriteByteSeq(victimGuid[5]);
            data->WriteByteSeq(victimGuid[7]);
            data->WriteByteSeq(victimGuid[6]);
            data->WriteByteSeq(victimGuid[2]);
            data->WriteByteSeq(victimGuid[1]);
        }
        else
        {
            for (uint8_t i = 0; i < 8; ++i)
            {
                *data << uint8_t(0);
            }
        }
    }

    if (updateFlags & UPDATEFLAG_TRANSPORT)
        *data << uint32_t(Util::getMSTime());
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
/// Creates an update block with the values of this object as determined by the updateMask.
//////////////////////////////////////////////////////////////////////////////////////////
void Object::_BuildValuesUpdate(ByteBuffer* data, UpdateMask* updateMask, Player* target)
{
    bool activate_quest_object = false;
    bool reset = false;
    uint32 oldflags = 0;

    if (updateMask->GetBit(OBJECT_FIELD_GUID) && target)	   // We're creating.
    {
        if (IsCreature())
        {
            Creature* pThis = static_cast< Creature* >(this);
            if (pThis->IsTagged() && (pThis->loot.gold || pThis->loot.items.size()))
            {
                // Let's see if we're the tagger or not.
                oldflags = m_uint32Values[UNIT_DYNAMIC_FLAGS];
                uint32 Flags = m_uint32Values[UNIT_DYNAMIC_FLAGS];
                uint32 oldFlags = 0;

                if (pThis->GetTaggerGUID() == target->GetGUID())
                {
                    // Our target is our tagger.
                    oldFlags = U_DYN_FLAG_TAGGED_BY_OTHER;

                    if (Flags & U_DYN_FLAG_TAGGED_BY_OTHER)
                        Flags &= ~oldFlags;

                    if (!(Flags & U_DYN_FLAG_LOOTABLE) && pThis->HasLootForPlayer(target))
                        Flags |= U_DYN_FLAG_LOOTABLE;
                }
                else
                {
                    // Target is not the tagger.
                    oldFlags = U_DYN_FLAG_LOOTABLE;

                    if (!(Flags & U_DYN_FLAG_TAGGED_BY_OTHER))
                        Flags |= U_DYN_FLAG_TAGGED_BY_OTHER;

                    if (Flags & U_DYN_FLAG_LOOTABLE)
                        Flags &= ~oldFlags;
                }

                m_uint32Values[UNIT_DYNAMIC_FLAGS] = Flags;

                updateMask->SetBit(UNIT_DYNAMIC_FLAGS);

                reset = true;
            }
        }

        if (target && IsGameObject())
        {
            GameObject* go = static_cast<GameObject*>(this);
            QuestLogEntry* qle;
            GameObjectProperties const* gameobject_info;
            GameObject_QuestGiver* go_quest_giver = nullptr;
            if (go->GetType() == GAMEOBJECT_TYPE_QUESTGIVER)
                go_quest_giver = static_cast<GameObject_QuestGiver*>(go);

            if (go_quest_giver != nullptr && go_quest_giver->HasQuests())
            {
                std::list<QuestRelation*>::iterator itr;
                for (itr = go_quest_giver->QuestsBegin(); itr != go_quest_giver->QuestsEnd(); ++itr)
                {
                    QuestRelation* qr = (*itr);
                    if (qr != nullptr)
                    {
                        QuestProperties const* qst = qr->qst;
                        if (qst != nullptr)
                        {
                            if ((qr->type & QUESTGIVER_QUEST_START && !target->HasQuest(qst->id))
                                || (qr->type & QUESTGIVER_QUEST_END && target->HasQuest(qst->id))
                               )
                            {
                                activate_quest_object = true;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                gameobject_info = go->GetGameObjectProperties();
                if (gameobject_info && (gameobject_info->goMap.size() || gameobject_info->itemMap.size()))
                {
                    for (GameObjectGOMap::const_iterator itr = gameobject_info->goMap.begin(); itr != gameobject_info->goMap.end(); ++itr)
                    {
                        qle = target->GetQuestLogForEntry(itr->first->id);
                        if (qle != nullptr)
                        {
                            if (qle->GetQuest()->count_required_mob == 0)
                                continue;
                            for (uint8 i = 0; i < 4; ++i)
                            {
                                if (qle->GetQuest()->required_mob_or_go[i] == static_cast<int32>(go->GetEntry()) && qle->GetMobCount(i) < qle->GetQuest()->required_mob_or_go_count[i])
                                {
                                    activate_quest_object = true;
                                    break;
                                }
                            }
                            if (activate_quest_object)
                                break;
                        }
                    }

                    if (!activate_quest_object)
                    {
                        for (GameObjectItemMap::const_iterator itr = gameobject_info->itemMap.begin();
                             itr != go->GetGameObjectProperties()->itemMap.end();
                             ++itr)
                        {
                            for (std::map<uint32, uint32>::const_iterator it2 = itr->second.begin();
                                 it2 != itr->second.end();
                                 ++it2)
                            {
                                if ((qle = target->GetQuestLogForEntry(itr->first->id)) != nullptr)
                                {
                                    if (target->GetItemInterface()->GetItemCount(it2->first) < it2->second)
                                    {
                                        activate_quest_object = true;
                                        break;
                                    }
                                }
                            }
                            if (activate_quest_object)
                                break;
                        }
                    }
                }
            }
        }
    }


    if (activate_quest_object)
    {
        oldflags = m_uint32Values[GAMEOBJECT_DYNAMIC];
        if (!updateMask->GetBit(GAMEOBJECT_DYNAMIC))
            updateMask->SetBit(GAMEOBJECT_DYNAMIC);
        m_uint32Values[GAMEOBJECT_DYNAMIC] = 1 | 8;     // 8 to show sparkles
        reset = true;
    }

    ARCEMU_ASSERT(updateMask && updateMask->GetCount() == m_valuesCount);
    uint32 bc;
    uint32 values_count;
    if (m_valuesCount > (2 * 0x20))    //if number of blocks > 2->  unit and player+item container
    {
        bc = updateMask->GetUpdateBlockCount();
        values_count = std::min<uint32>(bc * 32, m_valuesCount);

    }
    else
    {
        bc = updateMask->GetBlockCount();
        values_count = m_valuesCount;
    }

    *data << (uint8)bc;
    data->append(updateMask->GetMask(), bc * 4);

    for (uint32 index = 0; index < values_count; index++)
    {
        if (updateMask->GetBit(index))
        {
            *data << m_uint32Values[index];
        }
    }

    if (reset)
    {
        switch (GetTypeId())
        {
            case TYPEID_UNIT:
                m_uint32Values[UNIT_DYNAMIC_FLAGS] = oldflags;
                break;
            case TYPEID_GAMEOBJECT:
                m_uint32Values[GAMEOBJECT_DYNAMIC] = oldflags;
                break;
        }
    }
}

// This is not called!
void Unit::BuildHeartBeatMsg(WorldPacket* data)
{
    data->Initialize(MSG_MOVE_HEARTBEAT, 32);
    *data << GetGUID();
    BuildMovementPacket(data);
}

bool Object::SetPosition(const LocationVector & v, bool allowPorting /* = false */)
{
    bool updateMap = false, result = true;

    if (m_position.x != v.x || m_position.y != v.y)
        updateMap = true;

    m_position = const_cast<LocationVector &>(v);

    if (!allowPorting && v.z < -500)
    {
        m_position.z = 500;
        LOG_ERROR("setPosition: fell through map; height ported");

        result = false;
    }

    if (IsInWorld() && updateMap)
    {
        m_mapMgr->ChangeObjectLocation(this);
    }

    return result;
}

bool Object::SetPosition(float newX, float newY, float newZ, float newOrientation, bool allowPorting)
{
    bool updateMap = false, result = true;

    ARCEMU_ASSERT(!std::isnan(newX) && !std::isnan(newY) && !std::isnan(newOrientation));

    //It's a good idea to push through EVERY transport position change, no matter how small they are. By: VLack aka. VLsoft
    if (IsGameObject() && static_cast< GameObject* >(this)->GetGameObjectProperties()->type == GAMEOBJECT_TYPE_MO_TRANSPORT)
        updateMap = true;

    //if (m_position.x != newX || m_position.y != newY)
    //updateMap = true;
    if (m_lastMapUpdatePosition.Distance2DSq(newX, newY) > 4.0f)		/* 2.0f */
        updateMap = true;

    m_position.ChangeCoords(newX, newY, newZ, newOrientation);

    if (!allowPorting && newZ < -500)
    {
        m_position.z = 500;
        LOG_ERROR("setPosition: fell through map; height ported");

        result = false;
    }

    if (IsInWorld() && updateMap)
    {
        m_lastMapUpdatePosition.ChangeCoords(newX, newY, newZ, newOrientation);
        m_mapMgr->ChangeObjectLocation(this);

        if (IsPlayer() && static_cast< Player* >(this)->GetGroup() && static_cast< Player* >(this)->m_last_group_position.Distance2DSq(m_position) > 25.0f)       // distance of 5.0
        {
            static_cast< Player* >(this)->AddGroupUpdateFlag(GROUP_UPDATE_FLAG_POSITION);
        }
    }

    if (IsUnit())
    {
        Unit* u = static_cast< Unit* >(this);
        if (u->GetVehicleComponent() != nullptr)
            u->GetVehicleComponent()->MovePassengers(newX, newY, newZ, newOrientation);
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Fill the object's Update Values from a space delimitated list of values.
//////////////////////////////////////////////////////////////////////////////////////////
void Object::LoadValues(const char* data)
{
    // thread-safe ;) strtok is not.
    std::string ndata = data;
    std::string::size_type last_pos = 0, pos = 0;
    uint32 index = 0;
    uint32 val;
    do
    {
        // prevent overflow
        if (index >= m_valuesCount)
        {
            break;
        }
        pos = ndata.find(" ", last_pos);
        val = atol(ndata.substr(last_pos, (pos - last_pos)).c_str());
        if (m_uint32Values[index] == 0)
            m_uint32Values[index] = val;
        last_pos = pos + 1;
        ++index;
    }
    while (pos != std::string::npos);
}

void Object::_SetUpdateBits(UpdateMask* updateMask, Player* /*target*/) const
{
    *updateMask = m_updateMask;
}


void Object::_SetCreateBits(UpdateMask* updateMask, Player* /*target*/) const
{

    for (uint32 i = 0; i < m_valuesCount; ++i)
        if (m_uint32Values[i] != 0)
            updateMask->SetBit(i);
}

void Object::AddToWorld()
{
    MapMgr* mapMgr = sInstanceMgr.GetInstance(this);
    if (mapMgr == nullptr)
    {
        LOG_ERROR("AddToWorld() failed for Object with GUID " I64FMT " MapId %u InstanceId %u", GetGUID(), GetMapId(), GetInstanceID());
        return;
    }

    if (IsPlayer())
    {
        Player* plr = static_cast< Player* >(this);
        if (mapMgr->pInstance != nullptr && !plr->isGMFlagSet())
        {
            // Player limit?
            if (mapMgr->GetMapInfo()->playerlimit && mapMgr->GetPlayerCount() >= mapMgr->GetMapInfo()->playerlimit)
                return;
            Group* group = plr->GetGroup();
            // Player in group?
            if (group == nullptr && mapMgr->pInstance->m_creatorGuid == 0)
                return;
            // If set: Owns player the instance?
            if (mapMgr->pInstance->m_creatorGuid != 0 && mapMgr->pInstance->m_creatorGuid != plr->GetLowGUID())
                return;

            if (group != nullptr)
            {
                // Is instance empty or owns our group the instance?
                if (mapMgr->pInstance->m_creatorGroup != 0 && mapMgr->pInstance->m_creatorGroup != group->GetID())
                {
                    // Player not in group or another group is already playing this instance.
                    sChatHandler.SystemMessage(plr->GetSession(), "Another group is already inside this instance of the dungeon.");
                    if (plr->GetSession()->GetPermissionCount() > 0)
                        sChatHandler.BlueSystemMessage(plr->GetSession(), "Enable your GameMaster flag to ignore this rule.");
                    return;
                }
                else if (mapMgr->pInstance->m_creatorGroup == 0)
                    // Players group now "owns" the instance.
                    mapMgr->pInstance->m_creatorGroup = group->GetID();
            }
        }
    }

    m_mapMgr = mapMgr;
    m_inQueue = true;

    // correct incorrect instance id's
    m_instanceId = m_mapMgr->GetInstanceID();
    m_mapId = m_mapMgr->GetMapId();
    mapMgr->AddObject(this);
}

void Object::AddToWorld(MapMgr* pMapMgr)
{
    if (!pMapMgr || (pMapMgr->GetMapInfo()->playerlimit && this->IsPlayer() && pMapMgr->GetPlayerCount() >= pMapMgr->GetMapInfo()->playerlimit))
        return; //instance add failed

    m_mapMgr = pMapMgr;
    m_inQueue = true;

    pMapMgr->AddObject(this);

    // correct incorrect instance id's
    m_instanceId = pMapMgr->GetInstanceID();
    m_mapId = m_mapMgr->GetMapId();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Unlike addtoworld it pushes it directly ignoring add pool this can
/// only be called from the thread of mapmgr!
//////////////////////////////////////////////////////////////////////////////////////////
void Object::PushToWorld(MapMgr* mgr)
{
    ARCEMU_ASSERT(t_currentMapContext.get() == mgr);

    if (mgr == nullptr)
    {
        LOG_ERROR("Invalid push to world of Object " I64FMT, GetGUID());
        return; //instance add failed
    }

    m_mapId = mgr->GetMapId();
    //there's no need to set the InstanceId before calling PushToWorld() because it's already set here.
    m_instanceId = mgr->GetInstanceID();

    if (IsPlayer())
    {
        static_cast<Player*>(this)->m_cache->SetInt32Value(CACHE_MAPID, m_mapId);
        static_cast<Player*>(this)->m_cache->SetInt32Value(CACHE_INSTANCEID, m_instanceId);
    }

    m_mapMgr = mgr;
    OnPrePushToWorld();

    mgr->PushObject(this);

    // correct incorrect instance id's
    m_inQueue = false;

    event_Relocate();

    // call virtual function to handle stuff.. :P
    OnPushToWorld();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Remove object from world
//////////////////////////////////////////////////////////////////////////////////////////
void Object::RemoveFromWorld(bool free_guid)
{
    ARCEMU_ASSERT(m_mapMgr != NULL);

    OnPreRemoveFromWorld();

    MapMgr* m = m_mapMgr;
    m_mapMgr = nullptr;

    m->RemoveObject(this, free_guid);

    OnRemoveFromWorld();

    std::set<Spell*>::iterator itr, itr2;
    Spell* sp;
    for (itr = m_pendingSpells.begin(); itr != m_pendingSpells.end();)
    {
        itr2 = itr++;
        sp = *itr2;
        //if the spell being deleted is the same being casted, Spell::cancel will take care of deleting the spell
        //if it's not the same removing us from world. Otherwise finish() will delete the spell once all SpellEffects are handled.
        bool foundSpell = false;
        for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
        {
            if (sp == m_currentSpell[i])
            {
                interruptSpellWithSpellType(CurrentSpellType(i));
                foundSpell = true;
                break;
            }
        }
        if (!foundSpell)
            delete sp;
    }
    //shouldnt need to clear, spell destructor will erase
    //m_pendingSpells.clear();

    m_instanceId = INSTANCEID_NOT_IN_WORLD;
    m_mapId = MAPID_NOT_IN_WORLD;
    //m_inQueue is set to true when AddToWorld() is called. AddToWorld() queues the Object to be pushed, but if it's not pushed and RemoveFromWorld()
    //is called, m_inQueue will still be true even if the Object is no more inworld, nor queued.
    m_inQueue = false;

    // update our event holder
    event_Relocate();
}

uint32 Object::GetModPUInt32Value(const uint32 index, const int32 value)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    int32 basevalue = (int32)m_uint32Values[index];
    return ((basevalue * value) / 100);
}

void Object::ModUnsigned32Value(uint32 index, int32 mod)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    if (mod == 0)
        return;

    m_uint32Values[index] += mod;
    if ((int32)m_uint32Values[index] < 0)
        m_uint32Values[index] = 0;

    m_updateMask.SetBit(index);

    updateObject();

    if (IsPlayer())
    {
        switch (index)
        {
            case UNIT_FIELD_POWER1:
            case UNIT_FIELD_POWER2:
            case UNIT_FIELD_POWER4:
#if VERSION_STRING == WotLK
            case UNIT_FIELD_POWER7:
#endif
                static_cast< Unit* >(this)->SendPowerUpdate(true);
                break;
            default:
                break;
        }
    }
    else if (IsCreature())
    {
        switch (index)
        {
            case UNIT_FIELD_POWER1:
            case UNIT_FIELD_POWER2:
            case UNIT_FIELD_POWER3:
            case UNIT_FIELD_POWER4:
#if VERSION_STRING == WotLK
            case UNIT_FIELD_POWER7:
#endif
                static_cast<Creature*>(this)->SendPowerUpdate(false);
                break;
            default:
                break;
        }
    }
}

void Object::ModSignedInt32Value(uint32 index, int32 value)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    if (value == 0)
        return;

    m_uint32Values[index] += value;

    m_updateMask.SetBit(index);

    updateObject();
}

void Object::ModFloatValue(const uint32 index, const float value)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    m_floatValues[index] += value;

    m_updateMask.SetBit(index);

    updateObject();
}

void Object::ModFloatValueByPCT(const uint32 index, int32 byPct)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    if (byPct > 0)
        m_floatValues[index] *= 1.0f + byPct / 100.0f;
    else
        m_floatValues[index] /= 1.0f - byPct / 100.0f;

    m_updateMask.SetBit(index);

    updateObject();
}


void Object::SetFlag(const uint16 index, uint32 newFlag)
{
    setUInt32Value(index, getUInt32Value(index) | newFlag);
}


void Object::RemoveFlag(const uint16 index, uint32 oldFlag)
{
    setUInt32Value(index, getUInt32Value(index) & ~oldFlag);
}


float Object::CalcDistance(Object* Ob)
{
    ARCEMU_ASSERT(Ob != NULL);
    return CalcDistance(this->GetPositionX(), this->GetPositionY(), this->GetPositionZ(), Ob->GetPositionX(), Ob->GetPositionY(), Ob->GetPositionZ());
}

float Object::CalcDistance(float ObX, float ObY, float ObZ)
{
    return CalcDistance(this->GetPositionX(), this->GetPositionY(), this->GetPositionZ(), ObX, ObY, ObZ);
}

float Object::CalcDistance(Object* Oa, Object* Ob)
{
    ARCEMU_ASSERT(Oa != NULL);
    ARCEMU_ASSERT(Ob != NULL);
    return CalcDistance(Oa->GetPositionX(), Oa->GetPositionY(), Oa->GetPositionZ(), Ob->GetPositionX(), Ob->GetPositionY(), Ob->GetPositionZ());
}

float Object::CalcDistance(Object* Oa, float ObX, float ObY, float ObZ)
{
    ARCEMU_ASSERT(Oa != NULL);
    return CalcDistance(Oa->GetPositionX(), Oa->GetPositionY(), Oa->GetPositionZ(), ObX, ObY, ObZ);
}

float Object::CalcDistance(float OaX, float OaY, float OaZ, float ObX, float ObY, float ObZ)
{
    float xdest = OaX - ObX;
    float ydest = OaY - ObY;
    float zdest = OaZ - ObZ;
    return sqrtf(zdest * zdest + ydest * ydest + xdest * xdest);
}

bool Object::IsWithinDistInMap(Object* obj, const float dist2compare) const
{
    ARCEMU_ASSERT(obj != NULL);
    float xdest = this->GetPositionX() - obj->GetPositionX();
    float ydest = this->GetPositionY() - obj->GetPositionY();
    float zdest = this->GetPositionZ() - obj->GetPositionZ();
    return sqrtf(zdest * zdest + ydest * ydest + xdest * xdest) <= dist2compare;
}

bool Object::IsWithinLOSInMap(Object* obj)
{
    ARCEMU_ASSERT(obj != NULL);
    if (!IsInMap(obj)) return false;
    LocationVector location;
    location = obj->GetPosition();
    return IsWithinLOS(location);
}

bool Object::IsWithinLOS(LocationVector location)
{
    LocationVector location2;
    location2 = GetPosition();

    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
        return mgr->isInLineOfSight(GetMapId(), location2.x, location2.y, location2.z + 2.0f, location.x, location.y, location.z + 2.0f);
    }
    else
    {
        return true;
    }
}

float Object::calcAngle(float Position1X, float Position1Y, float Position2X, float Position2Y)
{
    float dx = Position2X - Position1X;
    float dy = Position2Y - Position1Y;
    double angle = 0.0f;

    // Calculate angle
    if (dx == 0.0)
    {
        if (dy == 0.0)
            angle = 0.0;
        else if (dy > 0.0)
            angle = M_PI * 0.5 /* / 2 */;
        else
            angle = M_PI * 3.0 * 0.5/* / 2 */;
    }
    else if (dy == 0.0)
    {
        if (dx > 0.0)
            angle = 0.0;
        else
            angle = M_PI;
    }
    else
    {
        if (dx < 0.0)
            angle = atanf(dy / dx) + M_PI;
        else if (dy < 0.0)
            angle = atanf(dy / dx) + (2 * M_PI);
        else
            angle = atanf(dy / dx);
    }

    // Convert to degrees
    angle = angle * float(180 / M_PI);

    // Return
    return float(angle);
}

float Object::calcRadAngle(float Position1X, float Position1Y, float Position2X, float Position2Y)
{
    double dx = double(Position2X - Position1X);
    double dy = double(Position2Y - Position1Y);
    double angle = 0.0;

    // Calculate angle
    if (dx == 0.0)
    {
        if (dy == 0.0)
            angle = 0.0;
        else if (dy > 0.0)
            angle = M_PI * 0.5/*/ 2.0*/;
        else
            angle = M_PI * 3.0 * 0.5/*/ 2.0*/;
    }
    else if (dy == 0.0)
    {
        if (dx > 0.0)
            angle = 0.0;
        else
            angle = M_PI;
    }
    else
    {
        if (dx < 0.0)
            angle = atan(dy / dx) + M_PI;
        else if (dy < 0.0)
            angle = atan(dy / dx) + (2 * M_PI);
        else
            angle = atan(dy / dx);
    }

    // Return
    return float(angle);
}

float Object::getEasyAngle(float angle)
{
    while (angle < 0)
    {
        angle = angle + 360;
    }
    while (angle >= 360)
    {
        angle = angle - 360;
    }
    return angle;
}

bool Object::inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y)
{
    float angle = calcAngle(Position1X, Position1Y, Position2X, Position2Y);
    float lborder = getEasyAngle((Orientation - (FOV * 0.5f/*/2*/)));
    float rborder = getEasyAngle((Orientation + (FOV * 0.5f/*/2*/)));
    //LOG_DEBUG("Orientation: %f Angle: %f LeftBorder: %f RightBorder %f",Orientation,angle,lborder,rborder);
    if (((angle >= lborder) && (angle <= rborder)) || ((lborder > rborder) && ((angle < rborder) || (angle > lborder))))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Object::isInFront(Object* target)
{
    // check if we facing something (is the object within a 180 degree slice of our positive y axis)

    double x = target->GetPositionX() - m_position.x;
    double y = target->GetPositionY() - m_position.y;

    double angle = atan2(y, x);
    angle = (angle >= 0.0) ? angle : 2.0 * M_PI + angle;
    angle -= m_position.o;

    while (angle > M_PI)
        angle -= 2.0 * M_PI;

    while (angle < -M_PI)
        angle += 2.0 * M_PI;

    // replace M_PI in the two lines below to reduce or increase angle

    double left = -1.0 * (M_PI / 2.0);
    double right = (M_PI / 2.0);

    return((angle >= left) && (angle <= right));
}

bool Object::isInBack(Object* target)
{
    // check if we are behind something (is the object within a 180 degree slice of our negative y axis)

    double x = m_position.x - target->GetPositionX();
    double y = m_position.y - target->GetPositionY();

    double angle = atan2(y, x);
    angle = (angle >= 0.0) ? angle : 2.0 * M_PI + angle;

    // if we are a creature and have a UNIT_FIELD_TARGET then we are always facing them
    if (IsCreature() && static_cast<Creature*>(this)->GetTargetGUID() != 0)
    {
        Unit* pTarget = static_cast<Creature*>(this)->GetAIInterface()->getNextTarget();
        if (pTarget != nullptr)
            angle -= double(Object::calcRadAngle(target->m_position.x, target->m_position.y, pTarget->m_position.x, pTarget->m_position.y));
        else
            angle -= target->GetOrientation();
    }
    else
        angle -= target->GetOrientation();

    while (angle > M_PI)
        angle -= 2.0 * M_PI;

    while (angle < -M_PI)
        angle += 2.0 * M_PI;

    // replace M_H_PI in the two lines below to reduce or increase angle

    double left = -1.0 * (M_H_PI / 2.0);
    double right = (M_H_PI / 2.0);

    return((angle <= left) && (angle >= right));
}

bool Object::isInArc(Object* target, float angle) // angle in degrees
{
    return inArc(GetPositionX(), GetPositionY(), angle, GetOrientation(), target->GetPositionX(), target->GetPositionY());
}

bool Object::HasInArc(float degrees, Object* target)
{
    return isInArc(target, degrees);
}

bool Object::isInRange(Object* target, float range)
{
    if (!this->IsInWorld() || !target) return false;
    float dist = CalcDistance(target);
    return(dist <= range);
}

void Object::_setFaction()
{
    DBC::Structures::FactionTemplateEntry const* faction_template = nullptr;

    if (IsUnit())
    {
        faction_template = sFactionTemplateStore.LookupEntry(static_cast<Unit*>(this)->GetFaction());
        if (faction_template == nullptr)
            LOG_ERROR("Unit does not have a valid faction. Faction: %u set to Entry: %u", static_cast<Unit*>(this)->GetFaction(), GetEntry());
    }
    else if (IsGameObject())
    {
        uint32 go_faction_id = static_cast<GameObject*>(this)->GetFaction();
        faction_template = sFactionTemplateStore.LookupEntry(go_faction_id);
        if (go_faction_id != 0)         // faction = 0 means it has no faction.
        {
            if (faction_template == nullptr)
            {
                LOG_ERROR("GameObject does not have a valid faction. Faction: %u set to Entry: %u", static_cast<GameObject*>(this)->GetFaction(), GetEntry());
            }
        }
    }

    //this solution looks a bit off, but our db is not perfect and this prevents some crashes.
    m_faction = faction_template;
    if (m_faction == nullptr)
    {
        m_faction = sFactionTemplateStore.LookupEntry(0);
        m_factionDBC = sFactionStore.LookupEntry(0);
    }
    else
    {
        m_factionDBC = sFactionStore.LookupEntry(m_faction->Faction);
    }
}

uint32 Object::_getFaction()
{
    return m_faction->Faction;
}

void Object::UpdateOppFactionSet()
{
    m_oppFactsInRange.clear();

    for (std::set< Object* >::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
    {
        Object* i = *itr;

        if (i->IsUnit() || i->IsGameObject())
        {
            if (isHostile(this, i))
            {
                if (!i->IsInRangeOppFactSet(this))
                    i->m_oppFactsInRange.insert(this);
                if (!IsInRangeOppFactSet(i))
                    m_oppFactsInRange.insert(i);

            }
            else
            {
                if (i->IsInRangeOppFactSet(this))
                    i->m_oppFactsInRange.erase(this);
                if (IsInRangeOppFactSet(i))
                    m_oppFactsInRange.erase(i);
            }
        }
    }
}

void Object::UpdateSameFactionSet()
{
    m_sameFactsInRange.clear();


    for (std::set< Object* >::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
    {
        Object* i = *itr;

        if (i->IsUnit() || i->IsGameObject())
        {
            if (isFriendly(this, i))
            {
                if (!i->IsInRangeSameFactSet(this))
                    i->m_sameFactsInRange.insert(this);

                if (!IsInRangeOppFactSet(i))
                    m_sameFactsInRange.insert(i);

            }
            else
            {
                if (i->IsInRangeSameFactSet(this))
                    i->m_sameFactsInRange.erase(this);

                if (IsInRangeSameFactSet(i))
                    m_sameFactsInRange.erase(i);
            }
        }
    }
}

void Object::EventSetUInt32Value(uint16 index, uint32 value)
{
    setUInt32Value(index, value);
}

void Object::DealDamage(Unit* /*pVictim*/, uint32 /*damage*/, uint32 /*targetEvent*/, uint32 /*unitEvent*/, uint32 /*spellId*/, bool /*no_remove_auras*/)
{}

void Object::SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, bool allowProc, bool static_damage, bool /*no_remove_auras*/)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //Unacceptable Cases Processing
    if (pVictim == nullptr || !pVictim->isAlive())
        return;

    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellID);
    if (spellInfo == nullptr)
        return;

    if (this->IsPlayer() && !static_cast< Player* >(this)->canCast(spellInfo))
        return;

    //////////////////////////////////////////////////////////////////////////////////////////
    //Variables Initialization
    float res = static_cast< float >(damage);
    bool critical = false;

    uint32 aproc = PROC_ON_ANY_HOSTILE_ACTION; /*| PROC_ON_SPELL_HIT;*/
    uint32 vproc = PROC_ON_ANY_HOSTILE_ACTION | PROC_ON_ANY_DAMAGE_VICTIM; /*| PROC_ON_SPELL_HIT_VICTIM;*/

    //A school damage is not necessarily magic
    switch (spellInfo->getSpell_Dmg_Type())
    {
        case SPELL_DMG_TYPE_RANGED:
        {
            aproc |= PROC_ON_RANGED_ATTACK;
            vproc |= PROC_ON_RANGED_ATTACK_VICTIM;
        }
        break;

        case SPELL_DMG_TYPE_MELEE:
        {
            aproc |= PROC_ON_MELEE_ATTACK;
            vproc |= PROC_ON_MELEE_ATTACK_VICTIM;
        }
        break;

        case SPELL_DMG_TYPE_MAGIC:
        {
            aproc |= PROC_ON_SPELL_HIT;
            vproc |= PROC_ON_SPELL_HIT_VICTIM;
        }
        break;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Spell Damage Bonus Calculations
    //by stats
    if (IsUnit() && !static_damage)
    {
        Unit* caster = static_cast< Unit* >(this);

        caster->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_START_ATTACK);

        res += static_cast< float >(caster->GetSpellDmgBonus(pVictim, spellInfo, damage, false));

        if (res < 0.0f)
            res = 0.0f;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Post +SpellDamage Bonus Modifications
    if (res > 0.0f && !(spellInfo->getAttributesExB() & ATTRIBUTESEXB_CANT_CRIT))
    {
        critical = this->IsCriticalDamageForSpell(pVictim, spellInfo);

        //////////////////////////////////////////////////////////////////////////////////////////
        //Spell Critical Hit
        if (critical)
        {
            res = this->GetCriticalDamageBonusForSpell(pVictim, spellInfo, res);

            switch (spellInfo->getSpell_Dmg_Type())
            {
                case SPELL_DMG_TYPE_RANGED:
                {
                    aproc |= PROC_ON_RANGED_CRIT_ATTACK;
                    vproc |= PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
                }
                break;

                case SPELL_DMG_TYPE_MELEE:
                {
                    aproc |= PROC_ON_CRIT_ATTACK;
                    vproc |= PROC_ON_CRIT_HIT_VICTIM;
                }
                break;

                case SPELL_DMG_TYPE_MAGIC:
                {
                    aproc |= PROC_ON_SPELL_CRIT_HIT;
                    vproc |= PROC_ON_SPELL_CRIT_HIT_VICTIM;
                }
                break;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Post Roll Calculations

    //damage reduction
    if (this->IsUnit())
        res += static_cast< Unit* >(this)->CalcSpellDamageReduction(pVictim, spellInfo, res);

    //absorption
    uint32 ress = static_cast< uint32 >(res);
    uint32 abs_dmg = pVictim->AbsorbDamage(spellInfo->getSchool(), &ress);
    uint32 ms_abs_dmg = pVictim->ManaShieldAbsorb(ress);
    if (ms_abs_dmg)
    {
        if (ms_abs_dmg > ress)
            ress = 0;
        else
            ress -= ms_abs_dmg;

        abs_dmg += ms_abs_dmg;
    }

    if (abs_dmg)
        vproc |= PROC_ON_ABSORB;

    // Incanter's Absorption
    if (pVictim->IsPlayer())
    {
        uint32 incanterSAbsorption[] =
        {
            //SPELL_HASH_INCANTER_S_ABSORPTION
            44394,
            44395,
            44396,
            44413,
            0
        };

        if (pVictim->hasAurasWithId(incanterSAbsorption))
        {
            float pctmod = 0.0f;
            Player* pl = static_cast<Player*>(pVictim);
            if (pl->HasAura(44394))
                pctmod = 0.05f;
            else if (pl->HasAura(44395))
                pctmod = 0.10f;
            else if (pl->HasAura(44396))
                pctmod = 0.15f;

            uint32 hp = static_cast<uint32>(0.05f * pl->getUInt32Value(UNIT_FIELD_MAXHEALTH));
            uint32 spellpower = static_cast<uint32>(pctmod * pl->GetPosDamageDoneMod(SCHOOL_NORMAL));

            if (spellpower > hp)
                spellpower = hp;

            SpellInfo* entry = sSpellCustomizations.GetSpellInfo(44413);
            if (!entry)
                return;

            Spell* sp = sSpellFactoryMgr.NewSpell(pl, entry, true, nullptr);
            sp->GetSpellInfo()->setEffectBasePoints(spellpower, 0);
            SpellCastTargets targets;
            targets.m_unitTarget = pl->GetGUID();
            sp->prepare(&targets);
        }
    }

    res = static_cast< float >(ress);
    dealdamage dmg;
    dmg.school_type = spellInfo->getSchool();
    dmg.full_damage = ress;
    dmg.resisted_damage = 0;

    if (res <= 0)
        dmg.resisted_damage = dmg.full_damage;

    //resistance reducing
    if (res > 0 && this->IsUnit())
    {
        static_cast< Unit* >(this)->CalculateResistanceReduction(pVictim, &dmg, spellInfo, 0);
        if ((int32)dmg.resisted_damage > dmg.full_damage)
            res = 0;
        else
            res = static_cast< float >(dmg.full_damage - dmg.resisted_damage);
    }

    //special states
    if (pVictim->IsPlayer() && static_cast< Player* >(pVictim)->GodModeCheat == true)
    {
        res = static_cast< float >(dmg.full_damage);
        dmg.resisted_damage = dmg.full_damage;
    }

    // Paladin: Blessing of Sacrifice, and Warlock: Soul Link
    if (pVictim->m_damageSplitTarget)
    {
        res = (float)pVictim->DoDamageSplitTarget((uint32)res, spellInfo->getSchool(), false);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Data Sending ProcHandling
    SendSpellNonMeleeDamageLog(this, pVictim, spellID, static_cast< int32 >(res), static_cast< uint8 >(spellInfo->getSchool()), abs_dmg, dmg.resisted_damage, false, 0, critical, IsPlayer());
    DealDamage(pVictim, static_cast< int32 >(res), 2, 0, spellID);

    if (IsUnit())
    {
        int32 dmg2 = static_cast< int32 >(res);

        pVictim->HandleProc(vproc, static_cast< Unit* >(this), spellInfo, !allowProc, dmg2, abs_dmg);
        pVictim->m_procCounter = 0;
        static_cast< Unit* >(this)->HandleProc(aproc, pVictim, spellInfo, !allowProc, dmg2, abs_dmg);
        static_cast< Unit* >(this)->m_procCounter = 0;
    }
    if (this->IsPlayer())
    {
        static_cast< Player* >(this)->m_casted_amount[spellInfo->getSchool()] = (uint32)res;
    }

    if (!(dmg.full_damage == 0 && abs_dmg))
    {
        //Only pushback the victim current spell if it's not fully absorbed
        if (pVictim->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr && pVictim->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getCastTimeLeft() > 0)
            pVictim->getCurrentSpell(CURRENT_CHANNELED_SPELL)->AddTime(spellInfo->getSchool());
        else if (pVictim->getCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr && pVictim->getCurrentSpell(CURRENT_GENERIC_SPELL)->getCastTimeLeft() > 0)
            pVictim->getCurrentSpell(CURRENT_GENERIC_SPELL)->AddTime(spellInfo->getSchool());
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Post Damage Processing
    if ((int32)dmg.resisted_damage == dmg.full_damage && !abs_dmg)
    {
        //Magic Absorption
        if (pVictim->IsPlayer())
        {
            if (static_cast< Player* >(pVictim)->m_RegenManaOnSpellResist)
            {
                Player* pl = static_cast< Player* >(pVictim);

                uint32 maxmana = pl->GetMaxPower(POWER_TYPE_MANA);
                uint32 amount = static_cast< uint32 >(maxmana * pl->m_RegenManaOnSpellResist);

                pVictim->Energize(pVictim, 29442, amount, POWER_TYPE_MANA);
            }
            // we still stay in combat dude
            static_cast< Player* >(pVictim)->CombatStatusHandler_ResetPvPTimeout();
        }
        if (IsPlayer())
            static_cast< Player* >(this)->CombatStatusHandler_ResetPvPTimeout();
    }
    if (spellInfo->getSchool() == SCHOOL_SHADOW)
    {
        if (pVictim->isAlive() && this->IsUnit())
        {
            //Shadow Word:Death
            if (spellID == 32379 || spellID == 32996 || spellID == 48157 || spellID == 48158)
            {
                uint32 damage2 = static_cast< uint32 >(res + abs_dmg);
                uint32 absorbed = static_cast< Unit* >(this)->AbsorbDamage(spellInfo->getSchool(), &damage2);
                DealDamage(static_cast< Unit* >(this), damage2, 2, 0, spellID);
                SendSpellNonMeleeDamageLog(this, this, spellID, damage2, static_cast< uint8 >(spellInfo->getSchool()), absorbed, 0, false, 0, false, IsPlayer());
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// SpellLog packets just to keep the code cleaner and better to read
//////////////////////////////////////////////////////////////////////////////////////////
void Object::SendSpellLog(Object* Caster, Object* Target, uint32 Ability, uint8 SpellLogType)
{
    if (Caster == nullptr || Target == nullptr || Ability == 0)
        return;


    WorldPacket data(SMSG_SPELLLOGMISS, 26);

    data << uint32(Ability);            // spellid
    data << Caster->GetGUID();          // caster / player
    data << uint8(1);                   // unknown but I think they are const
    data << uint32(1);                  // unknown but I think they are const
    data << Target->GetGUID();          // target
    data << uint8(SpellLogType);        // spelllogtype

    Caster->SendMessageToSet(&data, true);
}

void Object::SendSpellNonMeleeDamageLog(Object* Caster, Object* Target, uint32 SpellID, uint32 Damage, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToset)
{
    if (!Caster || !Target || !SpellID)
        return;

    uint32 Overkill = 0;

    if (Damage > Target->getUInt32Value(UNIT_FIELD_HEALTH))
        Overkill = Damage - Target->getUInt32Value(UNIT_FIELD_HEALTH);

    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, 48);

    data << Target->GetNewGUID();
    data << Caster->GetNewGUID();
    data << uint32(SpellID);                        // SpellID / AbilityID
    data << uint32(Damage);                         // All Damage
    data << uint32(Overkill);                       // Overkill
    data << uint8(g_spellSchoolConversionTable[School]);        // School
    data << uint32(AbsorbedDamage);                 // Absorbed Damage
    data << uint32(ResistedDamage);                 // Resisted Damage
    data << uint8(PhysicalDamage);                  // Physical Damage (true/false)
    data << uint8(0);                               // unknown or it binds with Physical Damage
    data << uint32(BlockedDamage);                  // Physical Damage (true/false)

    // unknown const
    if (CriticalHit)
        data << uint8(7);
    else
        data << uint8(5);

    data << uint32(0);

    Caster->SendMessageToSet(&data, bToset);
}

void Object::SendAttackerStateUpdate(Object* Caster, Object* Target, dealdamage* Dmg, uint32 Damage, uint32 Abs, uint32 BlockedDamage, uint32 HitStatus, uint32 VState)
{
    if (!Caster || !Target || !Dmg)
        return;

    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, 108);

    uint32 Overkill = 0;

    if (Damage > Target->getUInt32Value(UNIT_FIELD_MAXHEALTH))
        Overkill = Damage - Target->getUInt32Value(UNIT_FIELD_HEALTH);

    data << uint32(HitStatus);
    data << Caster->GetNewGUID();
    data << Target->GetNewGUID();

    data << uint32(Damage);                 // Realdamage
    data << uint32(Overkill);               // Overkill
    data << uint8(1);                       // Damage type counter / swing type

    data << uint32(g_spellSchoolConversionTable[Dmg->school_type]);         // Damage school
    data << float(Dmg->full_damage);        // Damage float
    data << uint32(Dmg->full_damage);       // Damage amount

    if (HitStatus & HITSTATUS_ABSORBED)
    {
        data << uint32(Abs);                // Damage absorbed
    }

    if (HitStatus & HITSTATUS_RESIST)
    {
        data << uint32(Dmg->resisted_damage);   // Damage resisted
    }

    data << uint8(VState);
    data << uint32(0);          // can be 0,1000 or -1
    data << uint32(0);

    if (HitStatus & HITSTATUS_BLOCK)
    {
        data << uint32(BlockedDamage);  // Damage amount blocked
    }


    if (HitStatus & HITSTATUS_RAGE_GAIN)
    {
        data << uint32(0);              // unknown
    }

    if (HitStatus & HITSTATUS_UNK_00)
    {
        data << uint32(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);

        data << float(0);       // Found in loop
        data << float(0);       // Found in loop
        data << uint32(0);
    }

    SendMessageToSet(&data, Caster->IsPlayer());
}

int32 Object::event_GetInstanceID()
{
    // \return -1 for non-inworld.. so we get our shit moved to the right thread
    // \return default value is -1, if it's something else then we are/will be soon InWorld.
    return m_instanceId;
}

void Object::EventSpellDamage(uint64 Victim, uint32 SpellID, uint32 Damage)
{
    if (!IsInWorld())
        return;

    Unit* pUnit = GetMapMgr()->GetUnit(Victim);
    if (pUnit == nullptr)
        return;

    SpellNonMeleeDamageLog(pUnit, SpellID, Damage, true);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Object has an active state
//////////////////////////////////////////////////////////////////////////////////////////
bool Object::CanActivate()
{
    switch (m_objectTypeId)
    {
        case TYPEID_UNIT:
        {
            if (!IsPet())
                return true;
        }
        break;

        case TYPEID_GAMEOBJECT:
        {
            if (static_cast<GameObject*>(this)->GetType() != GAMEOBJECT_TYPE_TRAP)
                return true;
        }
        break;
    }

    return false;
}

void Object::Activate(MapMgr* mgr)
{
    switch (m_objectTypeId)
    {
        case TYPEID_UNIT:
            mgr->activeCreatures.insert(static_cast<Creature*>(this));
            break;

        case TYPEID_GAMEOBJECT:
            mgr->activeGameObjects.insert(static_cast<GameObject*>(this));
            break;
    }
    // Objects are active so set to true.
    Active = true;
}

void Object::Deactivate(MapMgr* mgr)
{
    if (mgr == nullptr)
        return;

    switch (m_objectTypeId)
    {
        case TYPEID_UNIT:
            // check iterator
            if (mgr->creature_iterator != mgr->activeCreatures.end() && (*mgr->creature_iterator)->GetGUID() == GetGUID())
                ++mgr->creature_iterator;
            mgr->activeCreatures.erase(static_cast<Creature*>(this));
            break;

        case TYPEID_GAMEOBJECT:
            mgr->activeGameObjects.erase(static_cast<GameObject*>(this));
            break;
    }
    Active = false;
}

void Object::SetZoneId(uint32 newZone)
{
    m_zoneId = newZone;

    if (IsPlayer())
    {
        static_cast<Player*>(this)->m_cache->SetUInt32Value(CACHE_PLAYER_ZONEID, newZone);
        if (static_cast<Player*>(this)->GetGroup() != nullptr)
            static_cast<Player*>(this)->AddGroupUpdateFlag(GROUP_UPDATE_FLAG_ZONE);
    }
}

void Object::PlaySoundToSet(uint32 sound_entry)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << sound_entry;

    SendMessageToSet(&data, true);
}

bool Object::IsInBg()
{
    MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(GetMapId());

    if (pMapinfo != nullptr)
    {
        return (pMapinfo->type == INSTANCE_BATTLEGROUND);
    }

    return false;
}

uint32 Object::GetTeam()
{

    switch (m_factionDBC->ID)
    {
        // Human
        case 1:
            // Dwarf
        case 3:
            // Nightelf
        case 4:
            // Gnome
        case 8:
            // Draenei
        case 927:
            return TEAM_ALLIANCE;

            // Orc
        case 2:
            // Undead
        case 5:
            // Tauren
        case 6:
            // Troll
        case 9:
            // Bloodelf
        case 914:
            return TEAM_HORDE;
    }

    return static_cast< uint32 >(-1);
}

Transporter* Object::GetTransport() const
{
#if VERSION_STRING != Cata
    return objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(obj_movement_info.transporter_info.guid));
#else
    return nullptr;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Manipulates the phase value, see "enum PHASECOMMANDS" in
/// Object.h for a longer explanation!
//////////////////////////////////////////////////////////////////////////////////////////
void Object::Phase(uint8 command, uint32 newphase)
{
    switch (command)
    {
        case PHASE_SET:
            m_phase = newphase;
            break;
        case PHASE_ADD:
            m_phase |= newphase;
            break;
        case PHASE_DEL:
            m_phase &= ~newphase;
            break;
        case PHASE_RESET:
            m_phase = 1;
            break;
        default:
            ARCEMU_ASSERT(false);
    }

    return;
}

void Object::AddInRangeObject(Object* pObj)
{

    ARCEMU_ASSERT(pObj != NULL);

    if (pObj == this)
        LOG_ERROR("We are in range of ourselves!");

    if (pObj->IsPlayer())
        m_inRangePlayers.insert(pObj);

    m_objectsInRange.insert(pObj);
}

#if VERSION_STRING == Cata
void Object::OutPacketToSet(uint32 Opcode, uint16 Len, const void* Data, bool self)
#else
void Object::OutPacketToSet(uint16 Opcode, uint16 Len, const void* Data, bool /*self*/)
#endif
{
    if (!IsInWorld())
        return;

    // We are on Object level, which means we can't send it to ourselves so we only send to Players inrange
    for (std::set< Object* >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr)
    {
        Object* o = *itr;

        o->OutPacket(Opcode, Len, Data);
    }
}

void Object::SendMessageToSet(WorldPacket* data, bool /*bToSelf*/, bool /*myteam_only*/)
{
    if (!IsInWorld())
        return;

    uint32 myphase = GetPhase();
    for (std::set< Object* >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr)
    {
        Object* o = *itr;
        if ((o->GetPhase() & myphase) != 0)
            o->SendPacket(data);
    }
}

void Object::SendCreatureChatMessageInRange(Creature* creature, uint32_t textId)
{
    uint32 myphase = GetPhase();
    for (std::set<Object*>::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr)
    {
        Object* object = *itr;
        if ((object->GetPhase() & myphase) != 0)
        {
            if (object->IsPlayer())
            {
                Player* player = static_cast<Player*>(object);
                uint32_t sessionLanguage = player->GetSession()->language;

                std::string message;
                MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(textId);
                if (npcScriptText == nullptr)
                {
                    LOG_ERROR("Invalid textId: %u. This text is send by a script but not in table npc_script_text!", textId);
                    return;
                }

                MySQLStructure::LocalesNpcScriptText const* lnpct = (sessionLanguage > 0) ? sMySQLStore.getLocalizedNpcScriptText(textId, sessionLanguage) : nullptr;
                if (lnpct != nullptr)
                {
                    message = lnpct->text;
                }
                else
                {
                    message = npcScriptText->text;
                }

                std::string creatureName;

                MySQLStructure::LocalesCreature const* lcn = (sessionLanguage > 0) ? sMySQLStore.getLocalizedCreature(creature->GetEntry(), sessionLanguage) : nullptr;
                if (lcn != nullptr)
                {
                    creatureName = lcn->name;
                }
                else
                {
                    creatureName = creature->GetCreatureProperties()->Name;
                }

                size_t creatureNameLength = creatureName.length() + 1;
                size_t messageLength = message.length() + 1;

                if (npcScriptText->emote != 0)
                {
                    creature->EventAddEmote((EmoteType)npcScriptText->emote, npcScriptText->duration);
                }

                if (npcScriptText->sound != 0)
                {
                    creature->PlaySoundToSet(npcScriptText->sound);
                }

                WorldPacket data(SMSG_MESSAGECHAT, 35 + creatureNameLength + messageLength);
                data << uint8_t(npcScriptText->type);
                data << uint32_t(npcScriptText->language);
                data << uint64_t(GetGUID());
                data << uint32_t(0);
                data << uint32_t(creatureNameLength);
                data << creatureName;
                data << uint64_t(0);
                data << uint32_t(messageLength);
                data << message;
                data << uint8_t(0);
                player->SendPacket(&data);
            }
        }
    }
}

void Object::SendMonsterSayMessageInRange(Creature* creature, MySQLStructure::NpcMonsterSay* npcMonsterSay, int randChoice, uint32_t event)
{
    uint32 myphase = GetPhase();
    for (std::set<Object*>::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr)
    {
        Object* object = *itr;
        if ((object->GetPhase() & myphase) != 0)
        {
            if (object->IsPlayer())
            {
                Player* player = static_cast<Player*>(object);
                uint32_t sessionLanguage = player->GetSession()->language;

                //////////////////////////////////////////////////////////////////////////////////////////////
                // get text (normal or localized)
                const char* text = npcMonsterSay->texts[randChoice];
                MySQLStructure::LocalesNPCMonstersay const* lmsay = (sessionLanguage > 0) ? sMySQLStore.getLocalizedMonsterSay(GetEntry(), sessionLanguage, event) : nullptr;
                if (lmsay != nullptr)
                {
                    switch (randChoice)
                    {
                        case 0:
                            if (lmsay->text0 != nullptr)
                                text = lmsay->text0;
                            break;
                        case 1:
                            if (lmsay->text1 != nullptr)
                                text = lmsay->text1;
                            break;
                        case 2:
                            if (lmsay->text2 != nullptr)
                                text = lmsay->text2;
                            break;
                        case 3:
                            if (lmsay->text3 != nullptr)
                                text = lmsay->text3;
                            break;
                        case 4:
                            if (lmsay->text4 != nullptr)
                                text = lmsay->text4;
                            break;
                        default:
                            text = npcMonsterSay->texts[randChoice];
                    }
                }
                else
                {
                    text = npcMonsterSay->texts[randChoice];
                }

                // replace text with content
                std::string newText = text;
#if VERSION_STRING != Cata
                static const char* races[NUM_RACES] = { "None", "Human", "Orc", "Dwarf", "Night Elf", "Undead", "Tauren", "Gnome", "Troll", "None", "Blood Elf", "Draenei" };
#else
                static const char* races[NUM_RACES] = { "None", "Human", "Orc", "Dwarf", "Night Elf", "Undead", "Tauren", "Gnome", "Troll", "Goblin", "Blood Elf", "Draenei", "None", "None", "None", "None", "None", "None", "None", "None", "None", "None", "Worgen" };
#endif
                static const char* classes[MAX_PLAYER_CLASSES] = { "None", "Warrior", "Paladin", "Hunter", "Rogue", "Priest", "Death Knight", "Shaman", "Mage", "Warlock", "None", "Druid" };
                char* test = strstr((char*)text, "$R");
                if (test == nullptr)
                    test = strstr((char*)text, "$r");
                if (test != nullptr)
                {
                    uint64 targetGUID = creature->GetTargetGUID();
                    Unit* CurrentTarget = GetMapMgr()->GetUnit(targetGUID);
                    if (CurrentTarget)
                    {
                        ptrdiff_t testOfs = test - text;
                        newText.replace(testOfs, 2, races[CurrentTarget->getRace()]);
                    }
                }
                test = strstr((char*)text, "$N");
                if (test == nullptr)
                    test = strstr((char*)text, "$n");
                if (test != nullptr)
                {
                    uint64 targetGUID = creature->GetTargetGUID();
                    Unit* CurrentTarget = GetMapMgr()->GetUnit(targetGUID);
                    if (CurrentTarget && CurrentTarget->IsPlayer())
                    {
                        ptrdiff_t testOfs = test - text;
                        newText.replace(testOfs, 2, static_cast<Player*>(CurrentTarget)->GetName());
                    }
                }
                test = strstr((char*)text, "$C");
                if (test == nullptr)
                    test = strstr((char*)text, "$c");
                if (test != nullptr)
                {
                    uint64 targetGUID = creature->GetTargetGUID();
                    Unit* CurrentTarget = GetMapMgr()->GetUnit(targetGUID);
                    if (CurrentTarget)
                    {
                        ptrdiff_t testOfs = test - text;
                        newText.replace(testOfs, 2, classes[CurrentTarget->getClass()]);
                    }
                }
                test = strstr((char*)text, "$G");
                if (test == nullptr)
                    test = strstr((char*)text, "$g");
                if (test != nullptr)
                {
                    uint64 targetGUID = creature->GetTargetGUID();
                    Unit* CurrentTarget = GetMapMgr()->GetUnit(targetGUID);
                    if (CurrentTarget)
                    {
                        char* g0 = test + 2;
                        char* g1 = strchr(g0, ':');
                        if (g1)
                        {
                            char* gEnd = strchr(g1, ';');
                            if (gEnd)
                            {
                                *g1 = 0x00;
                                ++g1;
                                *gEnd = 0x00;
                                ++gEnd;
                                *test = 0x00;
                                newText = text;
                                newText += (CurrentTarget->getGender() == 0) ? g0 : g1;
                                newText += gEnd;
                            }
                        }
                    }
                }

                ////////////////////////////////////////////////////////////////////////////////////////////

                std::string creatureName;

                MySQLStructure::LocalesCreature const* lcn = (sessionLanguage > 0) ? sMySQLStore.getLocalizedCreature(creature->GetEntry(), sessionLanguage) : nullptr;
                if (lcn != nullptr)
                {
                    creatureName = lcn->name;
                }
                else
                {
                    creatureName = creature->GetCreatureProperties()->Name;
                }

                size_t creatureNameLength = creatureName.length() + 1;
                size_t messageLength = newText.length() + 1;

                WorldPacket data(SMSG_MESSAGECHAT, 35 + creatureNameLength + messageLength);
                data << uint8_t(npcMonsterSay->type);
                data << uint32_t(npcMonsterSay->language);
                data << uint64_t(GetGUID());
                data << uint32_t(0);
                data << uint32_t(creatureNameLength);
                data << creatureName;
                data << uint64_t(0);
                data << uint32_t(messageLength);
                data << newText;
                data << uint8_t(0);
                player->SendPacket(&data);
            }
        }
    }
}

void Object::RemoveInRangeObject(Object* pObj)
{
    ARCEMU_ASSERT(pObj != NULL);

    if (pObj->IsPlayer())
    {
        ARCEMU_ASSERT(m_inRangePlayers.erase(pObj) == 1);
    }

    ARCEMU_ASSERT(m_objectsInRange.erase(pObj) == 1);

    OnRemoveInRangeObject(pObj);
}

void Object::RemoveSelfFromInrangeSets()
{
    std::set< Object* >::iterator itr;

    for (itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
    {
        Object* o = *itr;
        ARCEMU_ASSERT(o != NULL);

        o->RemoveInRangeObject(this);
    }
}

void Object::OnRemoveInRangeObject(Object* /*pObj*/)
{
    // This method will remain empty for now, don't remove it! -dfighter
}

Object* Object::GetMapMgrObject(const uint64 & guid)
{
    if (!IsInWorld())
        return nullptr;

    return GetMapMgr()->_GetObject(guid);
}

Pet* Object::GetMapMgrPet(const uint64 & guid)
{
    if (!IsInWorld())
        return nullptr;

    return GetMapMgr()->GetPet(GET_LOWGUID_PART(guid));
}

Unit* Object::GetMapMgrUnit(const uint64 & guid)
{
    if (!IsInWorld())
        return nullptr;

    return GetMapMgr()->GetUnit(guid);
}

Player* Object::GetMapMgrPlayer(const uint64 & guid)
{
    if (!IsInWorld())
        return nullptr;

    return GetMapMgr()->GetPlayer(GET_LOWGUID_PART(guid));
}

Creature* Object::GetMapMgrCreature(const uint64 & guid)
{
    if (!IsInWorld())
        return nullptr;

    return GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
}

GameObject* Object::GetMapMgrGameObject(const uint64 & guid)
{
    if (!IsInWorld())
        return nullptr;

    return GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
}

DynamicObject* Object::GetMapMgrDynamicObject(const uint64 & guid)
{
    if (!IsInWorld())
        return nullptr;

    return GetMapMgr()->GetDynamicObject(GET_LOWGUID_PART(guid));
}

Object* Object::GetPlayerOwner()
{
    return nullptr;
}

MapCell* Object::GetMapCell() const
{
    ARCEMU_ASSERT(m_mapMgr != NULL);
    return m_mapMgr->GetCell(m_mapCell_x, m_mapCell_y);
}

void Object::SetMapCell(MapCell* cell)
{
    if (cell == nullptr)
    {
        //mapcell coordinates are uint16, so using uint32(-1) will always make GetMapCell() return NULL.
        m_mapCell_x = m_mapCell_y = uint32(-1);
    }
    else
    {
        m_mapCell_x = cell->GetPositionX();
        m_mapCell_y = cell->GetPositionY();
    }
}

void Object::SendAIReaction(uint32 reaction)
{
    WorldPacket data(SMSG_AI_REACTION, 12);
    data << uint64(GetGUID());
    data << uint32(reaction);
    SendMessageToSet(&data, false);
}

void Object::SendDestroyObject()
{
    WorldPacket data(SMSG_DESTROY_OBJECT, 9);
    data << uint64(GetGUID());
    data << uint8(0);
    SendMessageToSet(&data, false);
}

bool Object::GetPoint(float angle, float rad, float & outx, float & outy, float & outz, bool sloppypath)
{
    if (!IsInWorld())
        return false;
    outx = GetPositionX() + rad * cos(angle);
    outy = GetPositionY() + rad * sin(angle);
    outz = GetMapMgr()->GetLandHeight(outx, outy, GetPositionZ() + 2);
    float waterz;
    uint32 watertype;
    GetMapMgr()->GetLiquidInfo(outx, outy, GetPositionZ() + 2, waterz, watertype);
    outz = std::max(waterz, outz);

    MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();
    dtNavMesh* nav = const_cast<dtNavMesh*>(mmap->GetNavMesh(GetMapId()));
    dtNavMeshQuery* nav_query = const_cast<dtNavMeshQuery*>(mmap->GetNavMeshQuery(GetMapId(), GetInstanceID()));
    //NavMeshData* nav = CollideInterface.GetNavMesh(GetMapId());

    if (nav != nullptr)
    {
        //if we can path there, go for it
        if (!IsUnit() || !sloppypath || !static_cast<Unit*>(this)->GetAIInterface()->CanCreatePath(outx, outy, outz))
        {
            //raycast nav mesh to see if this place is valid
            float start[3] = { GetPositionY(), GetPositionZ() + 0.5f, GetPositionX() };
            float end[3] = { outy, outz + 0.5f, outx };
            float extents[3] = { 3, 5, 3 };
            dtQueryFilter filter;
            filter.setIncludeFlags(NAV_GROUND | NAV_WATER | NAV_SLIME | NAV_MAGMA);

            dtPolyRef startref;
            nav_query->findNearestPoly(start, extents, &filter, &startref, nullptr);

            float point;
            float hitNormal[3];
            float result[3];
            int numvisited;
            dtPolyRef visited[MAX_PATH_LENGTH];

            dtStatus rayresult = nav_query->raycast(startref, start, end, &filter, &point, hitNormal, visited, &numvisited, MAX_PATH_LENGTH);

            if (point <= 1.0f)
            {
                if (numvisited == 0 || rayresult == DT_FAILURE)
                {
                    result[0] = start[0];
                    result[1] = start[1];
                    result[2] = start[2];
                }
                else
                {
                    result[0] = start[0] + ((end[0] - start[0]) * point);
                    result[1] = start[1] + ((end[1] - start[1]) * point);
                    result[2] = start[2] + ((end[2] - start[2]) * point);
                    nav_query->getPolyHeight(visited[numvisited - 1], result, &result[1]);
                }

                //copy end back to function floats
                outy = result[0];
                outz = result[1];
                outx = result[2];
            }
        }
    }
    else //test against vmap if mmap isn't available
    {
        float testx, testy, testz;

        VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
        bool isHittingObject = mgr->getObjectHitPos(GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ() + 2, outx, outy, outz + 2, testx, testy, testz, -0.5f);

        if (isHittingObject)
        {
            //hit something
            outx = testx;
            outy = testy;
            outz = testz;
        }

        outz = GetMapMgr()->GetLandHeight(outx, outy, outz + 2);
    }

    return true;
}
