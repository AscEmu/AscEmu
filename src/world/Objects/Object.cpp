/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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
#include "MMapFactory.h"

Object::Object() : m_position(0, 0, 0, 0), m_spawnLocation(0, 0, 0, 0)
{
    m_mapId = MAPID_NOT_IN_WORLD;
    m_zoneId = 0;

    m_uint32Values = 0;
    m_objectUpdated = false;

    m_currentSpell = NULL;
    m_valuesCount = 0;

    //official Values
    m_walkSpeed = 2.5f;
    m_runSpeed = 7.0f;
    m_base_runSpeed = m_runSpeed;
    m_base_walkSpeed = m_walkSpeed;

    m_flySpeed = 7.0f;
    m_backFlySpeed = 4.5f;

    m_backWalkSpeed = 4.5f;
    m_swimSpeed = 4.722222f;
    m_backSwimSpeed = 2.5f;
    m_turnRate = M_PI_FLOAT;
    m_pitchRate = 3.14f;

    m_phase = 1;                //Set the default phase: 00000000 00000000 00000000 00000001

    m_mapMgr = 0;
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

    if (m_currentSpell)
    {
        m_currentSpell->cancel();
        m_currentSpell = NULL;
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

    switch (m_objectTypeId)
    {
        case TYPEID_CORPSE:
        case TYPEID_DYNAMICOBJECT:
            updateflags = UPDATEFLAG_HAS_POSITION;
            updatetype = 2;
            break;

        case TYPEID_GAMEOBJECT:
            updateflags = UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION;
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
        updateflags |= UPDATEFLAG_SELF;
        updatetype = 2;
    }

    if (IsUnit())
    {
        if (static_cast< Unit* >(this)->GetTargetGUID())
            updateflags |= UPDATEFLAG_HAS_ATTACKING_TARGET;
    }

    // we shouldn't be here, under any circumstances, unless we have a wowguid..
    ASSERT(m_wowGuid.GetNewGuidLen());
    // build our actual update
    *data << uint8(updatetype);
    *data << m_wowGuid;
    *data << uint8(m_objectTypeId);

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

    *packet << (uint32)1;//number of update/create blocks
    //	*packet << (uint8)0;//unknown //VLack: removed for 3.1

    *packet << (uint8)UPDATETYPE_VALUES;		// update type == update
    *packet << GetNewGUID();

    uint32 mBlocks = index / 32 + 1;
    *packet << (uint8)mBlocks;

    for (uint32 dword_n = mBlocks - 1; dword_n; dword_n--)
        *packet << (uint32)0;

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
    buf << (uint8)mBlocks;

    for (uint32 dword_n = mBlocks - 1; dword_n; dword_n--)
        buf << (uint32)0;

    buf << (((uint32)(1)) << (Index % 32));
    buf << Value;

    Target->PushUpdateData(&buf, 1);
}

void Object::BuildFieldUpdatePacket(ByteBuffer* buf, uint32 Index, uint32 Value)
{
    *buf << uint8(UPDATETYPE_VALUES);
    *buf << GetNewGUID();

    uint32 mBlocks = Index / 32 + 1;
    *buf << (uint8)mBlocks;

    for (uint32 dword_n = mBlocks - 1; dword_n; dword_n--)
        *buf << (uint32)0;

    *buf << (((uint32)(1)) << (Index % 32));
    *buf << Value;
}

uint32 Object::BuildValuesUpdateBlockForPlayer(ByteBuffer* data, Player* target)
{
    uint32 valuesCount = m_valuesCount;
    if (GetTypeId() == TYPEID_PLAYER && target != this)
        valuesCount = PLAYER_FIELD_INV_SLOT_HEAD;

    UpdateMask updateMask;
    updateMask.SetCount(valuesCount);
    _SetUpdateBits(&updateMask, target);

    for (uint32 x = 0; x < valuesCount; ++x)
    {
        if (updateMask.GetBit(x))
        {
            *data << (uint8)UPDATETYPE_VALUES;		// update type == update
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
    *buf << (uint8)UPDATETYPE_VALUES;

    ARCEMU_ASSERT(m_wowGuid.GetNewGuidLen() > 0);
    *buf << m_wowGuid;

    _BuildValuesUpdate(buf, mask, 0);

    // 1 update.
    return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Build the Movement Data portion of the update packet Fills the data with this object's movement/speed info
///\todo rewrite this stuff, document unknown fields and flags
uint32 TimeStamp();

void Object::_BuildMovementUpdate(ByteBuffer* data, uint16 updateFlags, Player* target)
{
    ObjectGuid Guid = GetGUID();

    data->writeBit(false);
    data->writeBit(false);
    data->writeBit(updateFlags & UPDATEFLAG_ROTATION);
    data->writeBit(updateFlags & UPDATEFLAG_ANIM_KITS);               // AnimKits
    data->writeBit(updateFlags & UPDATEFLAG_HAS_ATTACKING_TARGET);
    data->writeBit(updateFlags & UPDATEFLAG_SELF);
    data->writeBit(updateFlags & UPDATEFLAG_VEHICLE);
    data->writeBit(updateFlags & UPDATEFLAG_LIVING);
    data->writeBits(0, 24);                                     // Byte Counter
    data->writeBit(false);
    data->writeBit(updateFlags & UPDATEFLAG_POSITION);                // flags & UPDATEFLAG_HAS_POSITION Game Object Position
    data->writeBit(updateFlags & UPDATEFLAG_HAS_POSITION);            // Stationary Position
    data->writeBit(updateFlags & UPDATEFLAG_TRANSPORT_ARR);
    data->writeBit(false);
    data->writeBit(updateFlags & UPDATEFLAG_TRANSPORT);

    bool hasTransport = false,
        isSplineEnabled = false,
        hasPitch = false,
        hasFallData = false,
        hasFallDirection = false,
        hasElevation = false,
        hasOrientation = !IsType(TYPE_ITEM),
        hasTimeStamp = true,
        hasTransportTime2 = false,
        hasTransportTime3 = false;

    if (IsType(TYPE_UNIT))
    {
        Unit const* unit = (Unit const*)this;
        hasTransport = !unit->movement_info.GetTransportGuid().IsEmpty();
        isSplineEnabled = false; // unit->IsSplineEnabled();

        if (GetTypeId() == TYPEID_PLAYER)
        {
            // use flags received from client as they are more correct
            hasPitch = unit->movement_info.GetStatusInfo().hasPitch;
            hasFallData = unit->movement_info.GetStatusInfo().hasFallData;
            hasFallDirection = unit->movement_info.GetStatusInfo().hasFallDirection;
            hasElevation = unit->movement_info.GetStatusInfo().hasSplineElevation;
            hasTransportTime2 = unit->movement_info.GetStatusInfo().hasTransportTime2;
            hasTransportTime3 = unit->movement_info.GetStatusInfo().hasTransportTime3;
        }
        else
        {
            hasPitch = unit->movement_info.HasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) ||
                unit->movement_info.HasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING);
            hasFallData = unit->movement_info.HasMovementFlag2(MOVEFLAG2_INTERP_TURNING);
            hasFallDirection = unit->movement_info.HasMovementFlag(MOVEFLAG_FALLING);
            hasElevation = unit->movement_info.HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION);
        }
    }

    if (updateFlags & UPDATEFLAG_LIVING)
    {
        Unit const* unit = (Unit const*)this;

        data->writeBit(!unit->movement_info.GetMovementFlags());
        data->writeBit(!hasOrientation);


        data->writeBit(Guid[7]);
        data->writeBit(Guid[3]);
        data->writeBit(Guid[2]);

        if (unit->movement_info.GetMovementFlags())
            data->writeBits(unit->movement_info.GetMovementFlags(), 30);

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
            ObjectGuid tGuid = unit->movement_info.GetTransportGuid();

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

        data->writeBit(false);       // unknown 4.3.3
        data->writeBit(!unit->movement_info.GetMovementFlags2());

        if (unit->movement_info.GetMovementFlags2())
            data->writeBits(unit->movement_info.GetMovementFlags2(), 12);

    }

    // used only with GO's, placeholder
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

    if (updateFlags & UPDATEFLAG_HAS_ATTACKING_TARGET)
    {
        if (IsUnit())
        {
            ObjectGuid victimGuid = static_cast< Unit* >(this)->GetTargetGUID();

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
        data->writeBit(true); // hasAnimKit0
        data->writeBit(true); // hasAnimKit1
        data->writeBit(true); // hasAnimKit2
    }

    data->flushBits();


    if (updateFlags & UPDATEFLAG_LIVING)
    {
        Unit const* unit = (Unit const*)this;

        data->WriteByteSeq(Guid[4]);

        *data << m_backWalkSpeed;

        if (hasFallData)
        {
            if (hasFallDirection)
            {
                *data << float(unit->movement_info.GetJumpInfo().cosAngle);
                *data << float(unit->movement_info.GetJumpInfo().xyspeed);
                *data << float(unit->movement_info.GetJumpInfo().sinAngle);
            }

            *data << uint32(unit->movement_info.GetFallTime());
            *data << float(unit->movement_info.GetJumpInfo().velocity);
        }

        *data << m_backSwimSpeed;

        if (hasElevation)
            *data << float(unit->movement_info.GetSplineElevation());

        if (isSplineEnabled)
        {
            //Movement::PacketBuilder::WriteCreateBytes(*unit->movespline, *data);
        }

        *data << float(unit->GetPositionZ());
        data->WriteByteSeq(Guid[5]);

        if (hasTransport)
        {
            ObjectGuid tGuid = unit->movement_info.GetTransportGuid();

            data->WriteByteSeq(tGuid[5]);
            data->WriteByteSeq(tGuid[7]);
            *data << uint32(unit->movement_info.GetTransportTime());
            *data << float(NormalizeOrientation(unit->movement_info.GetTransportPos()->o));

            if (hasTransportTime2)
                *data << uint32(unit->movement_info.GetTransportTime2());

            *data << float(unit->movement_info.GetTransportPos()->y);
            *data << float(unit->movement_info.GetTransportPos()->x);
            data->WriteByteSeq(tGuid[3]);
            *data << float(unit->movement_info.GetTransportPos()->z);
            data->WriteByteSeq(tGuid[0]);
            
            if (hasTransportTime3)
                *data << uint32(unit->movement_info.GetFallTime());

            *data << int8(unit->movement_info.GetTransportSeat());
            data->WriteByteSeq(tGuid[1]);
            data->WriteByteSeq(tGuid[6]);
            data->WriteByteSeq(tGuid[2]);
            data->WriteByteSeq(tGuid[4]);
        }

        *data << float(unit->GetPositionX());
        *data << float(unit->m_pitchRate);
        data->WriteByteSeq(Guid[3]);
        data->WriteByteSeq(Guid[0]);
        *data << float(unit->m_swimSpeed);
        *data << float(unit->GetPositionY());
        data->WriteByteSeq(Guid[7]);
        data->WriteByteSeq(Guid[1]);
        data->WriteByteSeq(Guid[2]);
        *data << float(unit->m_walkSpeed);

        *data << uint32(getMSTime());

        *data << float(unit->m_backFlySpeed);
        data->WriteByteSeq(Guid[6]);
        *data << float(unit->m_turnRate);

        if (hasOrientation)
            *data << float(NormalizeOrientation(unit->GetOrientation()));

        *data << m_runSpeed;

        if (hasPitch)
            *data << float(unit->movement_info.GetPitch());

        *data << float(unit->m_flySpeed);

    }

    if (updateFlags & UPDATEFLAG_VEHICLE)
    {
        uint32 vehicleid = 0;

        if (IsCreature())
            vehicleid = static_cast< Creature* >(this)->GetCreatureProperties()->vehicleid;
        else
            if (IsPlayer())
                vehicleid = static_cast< Player* >(this)->mountvehicleid;

        *data << float(NormalizeOrientation(((Object*)this)->GetOrientation()));
        *data << uint32(vehicleid);
    }

    // used only with GO's, placeholder
    if (updateFlags & UPDATEFLAG_POSITION)
    {
        ObjectGuid transGuid;

        data->WriteByteSeq(transGuid[0]);
        data->WriteByteSeq(transGuid[5]);
        if (hasTransportTime3)
            *data << uint32(0);

        data->WriteByteSeq(transGuid[3]);
        *data << float(0.0f);   // x offset
        data->WriteByteSeq(transGuid[4]);
        data->WriteByteSeq(transGuid[6]);
        data->WriteByteSeq(transGuid[1]);
        *data << uint32(0);     // transport time
        *data << float(0.0f);   // y offset
        data->WriteByteSeq(transGuid[2]);
        data->WriteByteSeq(transGuid[7]);
        *data << float(0.0f);   // z offset
        *data << int8(-1);      // transport seat
        *data << float(0.0f);   // o offset

        if (hasTransportTime2)
            *data << uint32(0);
    }

    if (updateFlags & UPDATEFLAG_ROTATION)
    {
        if (IsGameObject())
            *data << int64(static_cast< GameObject* >(this)->GetRotation());
    }

    if (updateFlags & UPDATEFLAG_TRANSPORT_ARR)
    {
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << float(0.0f);
        *data << uint8(0);
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

    if (updateFlags & UPDATEFLAG_HAS_POSITION) // UPDATEFLAG_HAS_STATIONARY_POSITION
    {
        *data << float(NormalizeOrientation(((Object*)this)->GetOrientation()));
        *data << float(((Object*)this)->GetPositionX());
        *data << float(((Object*)this)->GetPositionY());
        *data << float(((Object*)this)->GetPositionZ());
    }

    if (updateFlags & UPDATEFLAG_HAS_ATTACKING_TARGET) // UPDATEFLAG_HAS_TARGET
    {
        if (IsUnit())
        {
            ObjectGuid victimGuid = static_cast< Unit* >(this)->GetTargetGUID();

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
            for (uint8 i = 0; i < 8; ++i)
                *data << uint8(0);
    }

    if (updateFlags & UPDATEFLAG_TRANSPORT)
        *data << uint32(getMSTime());
}

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
                    if (qr != NULL)
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
                        if (qle != NULL)
                        {
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
                                if ((qle = target->GetQuestLogForEntry(itr->first->id)) != 0)
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

    uint32 valuesCount = values_count;
    if (GetTypeId() == TYPEID_PLAYER && target != this)
        valuesCount = PLAYER_FIELD_INV_SLOT_HEAD;

    for (uint32 index = 0; index < valuesCount; index++)
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

// cNot called - \todo danko call it on near teleport
void Unit::SendHeartBeatMsg()
{
    movement_info.UpdateTime(getMSTime());
    WorldPacket data(MSG_MOVE_HEARTBEAT, 64);
    data << GetGUID();
    data << movement_info;
    SendMessageToSet(&data, true);
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

    ARCEMU_ASSERT(!isnan(newX) && !isnan(newY) && !isnan(newOrientation));

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
        if (u->GetVehicleComponent() != NULL)
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

void Object::_SetUpdateBits(UpdateMask* updateMask, Player* target) const
{
    *updateMask = m_updateMask;
}


void Object::_SetCreateBits(UpdateMask* updateMask, Player* target) const
{
    uint32 valuesCount = m_valuesCount;
    if (GetTypeId() == TYPEID_PLAYER && target != this)
        valuesCount = PLAYER_FIELD_INV_SLOT_HEAD;

    for (uint32 i = 0; i < valuesCount; ++i)
        if (m_uint32Values[i] != 0)
            updateMask->SetBit(i);
}

void Object::AddToWorld()
{
    MapMgr* mapMgr = sInstanceMgr.GetInstance(this);
    if (mapMgr == NULL)
    {
        LOG_ERROR("AddToWorld() failed for Object with GUID " I64FMT " MapId %u InstanceId %u", GetGUID(), GetMapId(), GetInstanceID());
        return;
    }

    if (IsPlayer())
    {
        Player* plr = static_cast< Player* >(this);
        if (mapMgr->pInstance != NULL && !plr->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
        {
            // Player limit?
            if (mapMgr->GetMapInfo()->playerlimit && mapMgr->GetPlayerCount() >= mapMgr->GetMapInfo()->playerlimit)
                return;
            Group* group = plr->GetGroup();
            // Player in group?
            if (group == NULL && mapMgr->pInstance->m_creatorGuid == 0)
                return;
            // If set: Owns player the instance?
            if (mapMgr->pInstance->m_creatorGuid != 0 && mapMgr->pInstance->m_creatorGuid != plr->GetLowGUID())
                return;

            if (group != NULL)
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

    if (mgr == NULL)
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
    m_mapMgr = NULL;

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
        if (sp == m_currentSpell)
            sp->cancel();
        else
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

void Object::SetUInt16Value(uint16 index, uint8 offset, uint16 value)
{
    ASSERT(index < m_valuesCount);

    if (offset > 2)
    {
        Log.Debug("Object", "Object::SetUInt16Value: wrong offset %u", offset);
        return;
    }

    if (uint16(m_uint32Values[index] >> (offset * 16)) != value)
    {
        m_uint32Values[index] &= ~uint32(uint32(0xFFFF) << (offset * 16));
        m_uint32Values[index] |= uint32(uint32(value) << (offset * 16));
        m_updateMask.SetBit(index);

        if (!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Set uint32 property
//////////////////////////////////////////////////////////////////////////////////////////
void Object::SetUInt32Value(const uint32 index, const uint32 value)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    /// Save updating when val isn't changing.
    if (m_uint32Values[index] == value)
        return;

    m_uint32Values[index] = value;

    if (IsInWorld())
    {
        m_updateMask.SetBit(index);

        if (!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }

    if(IsUnit())
	{
		static_cast<Unit*>(this)->HandleUpdateFieldChange(index);
	}

    /// Group update handling
    if (IsPlayer())
    {
        static_cast<Player*>(this)->HandleUpdateFieldChanged(index);

        switch (index)
        {
            case UNIT_FIELD_POWER1:
            case UNIT_FIELD_POWER2:
            case UNIT_FIELD_POWER4:
            //case UNIT_FIELD_POWER7:
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
            //case UNIT_FIELD_POWER7:
                static_cast<Creature*>(this)->SendPowerUpdate(false);
                break;
            default:
                break;
        }
    }
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

    if (IsInWorld())
    {
        m_updateMask.SetBit(index);

        if (!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }

    if (IsPlayer())
    {
        switch (index)
        {
            case UNIT_FIELD_POWER1:
            case UNIT_FIELD_POWER2:
            case UNIT_FIELD_POWER4:
            //case UNIT_FIELD_POWER7:
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
            //case UNIT_FIELD_POWER7:
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
    if (IsInWorld())
    {
        m_updateMask.SetBit(index);

        if (!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}

void Object::ModFloatValue(const uint32 index, const float value)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    m_floatValues[index] += value;

    if (IsInWorld())
    {
        m_updateMask.SetBit(index);

        if (!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}

void Object::ModFloatValueByPCT(const uint32 index, int32 byPct)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    if (byPct > 0)
        m_floatValues[index] *= 1.0f + byPct / 100.0f;
    else
        m_floatValues[index] /= 1.0f - byPct / 100.0f;


    if (IsInWorld())
    {
        m_updateMask.SetBit(index);

        if (!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Set uint64 property
//////////////////////////////////////////////////////////////////////////////////////////
void Object::SetUInt64Value(const uint32 index, const uint64 value)
{
    ARCEMU_ASSERT(index + 1 < m_valuesCount);

    uint64* p = reinterpret_cast< uint64* >(&m_uint32Values[index]);

    if (*p == value)
        return;
    else
        *p = value;

    if (IsInWorld())
    {
        m_updateMask.SetBit(index);
        m_updateMask.SetBit(index + 1);

        if (!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Set float property
//////////////////////////////////////////////////////////////////////////////////////////
void Object::SetFloatValue(const uint32 index, const float value)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    if (m_floatValues[index] == value)
        return;

    m_floatValues[index] = value;

    if (IsInWorld())
    {
        m_updateMask.SetBit(index);

        if (!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}


void Object::SetFlag(const uint32 index, uint32 newFlag)
{
    SetUInt32Value(index, GetUInt32Value(index) | newFlag);
}


void Object::RemoveFlag(const uint32 index, uint32 oldFlag)
{
    SetUInt32Value(index, GetUInt32Value(index) & ~oldFlag);
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

    if (sWorld.Collision)
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
        if (pTarget != NULL)
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

void Object::EventSetUInt32Value(uint32 index, uint32 value)
{
    SetUInt32Value(index, value);
}

void Object::DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras)
{}

void Object::SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, bool allowProc, bool static_damage, bool no_remove_auras)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //Unacceptable Cases Processing
    if (pVictim == NULL || !pVictim->isAlive())
        return;

    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellID);
    if (spellInfo == NULL)
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
    switch (spellInfo->Spell_Dmg_Type)
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
    if (res > 0.0f && !(spellInfo->AttributesExB & ATTRIBUTESEXB_CANT_CRIT))
    {
        critical = this->IsCriticalDamageForSpell(pVictim, spellInfo);

        //////////////////////////////////////////////////////////////////////////////////////////
        //Spell Critical Hit
        if (critical)
        {
            res = this->GetCriticalDamageBonusForSpell(pVictim, spellInfo, res);

            switch (spellInfo->Spell_Dmg_Type)
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
    uint32 abs_dmg = pVictim->AbsorbDamage(spellInfo->School, &ress);
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
    if (pVictim->IsPlayer() && pVictim->HasAurasWithNameHash(SPELL_HASH_INCANTER_S_ABSORPTION))
    {
        float pctmod = 0.0f;
        Player* pl = static_cast< Player* >(pVictim);
        if (pl->HasAura(44394))
            pctmod = 0.05f;
        else if (pl->HasAura(44395))
            pctmod = 0.10f;
        else if (pl->HasAura(44396))
            pctmod = 0.15f;

        uint32 hp = static_cast< uint32 >(0.05f * pl->GetUInt32Value(UNIT_FIELD_MAXHEALTH));
        uint32 spellpower = static_cast< uint32 >(pctmod * pl->GetPosDamageDoneMod(SCHOOL_NORMAL));

        if (spellpower > hp)
            spellpower = hp;

        SpellInfo* entry = sSpellCustomizations.GetSpellInfo(44413);
        if (!entry)
            return;

        Spell* sp = sSpellFactoryMgr.NewSpell(pl, entry, true, NULL);
        sp->GetSpellInfo()->EffectBasePoints[0] = spellpower;
        SpellCastTargets targets;
        targets.m_unitTarget = pl->GetGUID();
        sp->prepare(&targets);
    }

    res = static_cast< float >(ress);
    dealdamage dmg;
    dmg.school_type = spellInfo->School;
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
        res = (float)pVictim->DoDamageSplitTarget((uint32)res, spellInfo->School, false);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Data Sending ProcHandling
    SendSpellNonMeleeDamageLog(this, pVictim, spellID, static_cast< int32 >(res), static_cast< uint8 >(spellInfo->School), abs_dmg, dmg.resisted_damage, false, 0, critical, IsPlayer());
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
        static_cast< Player* >(this)->m_casted_amount[spellInfo->School] = (uint32)res;
    }

    if (!(dmg.full_damage == 0 && abs_dmg))
    {
        //Only pushback the victim current spell if it's not fully absorbed
        if (pVictim->GetCurrentSpell())
            pVictim->GetCurrentSpell()->AddTime(spellInfo->School);
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
    if (spellInfo->School == SCHOOL_SHADOW)
    {
        if (pVictim->isAlive() && this->IsUnit())
        {
            //Shadow Word:Death
            if (spellID == 32379 || spellID == 32996 || spellID == 48157 || spellID == 48158)
            {
                uint32 damage2 = static_cast< uint32 >(res + abs_dmg);
                uint32 absorbed = static_cast< Unit* >(this)->AbsorbDamage(spellInfo->School, &damage2);
                DealDamage(static_cast< Unit* >(this), damage2, 2, 0, spellID);
                SendSpellNonMeleeDamageLog(this, this, spellID, damage2, static_cast< uint8 >(spellInfo->School), absorbed, 0, false, 0, false, IsPlayer());
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// SpellLog packets just to keep the code cleaner and better to read
//////////////////////////////////////////////////////////////////////////////////////////
void Object::SendSpellLog(Object* Caster, Object* Target, uint32 Ability, uint8 SpellLogType)
{
    if (Caster == NULL || Target == NULL || Ability == 0)
        return;

    WorldPacket data(SMSG_SPELLLOGMISS, 26);
    data << uint32(Ability);            // spellid
    data << Caster->GetGUID();          // caster / player
    data << uint8(0);                   // unknown but I think they are const
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

    if (Damage > Target->GetUInt32Value(UNIT_FIELD_HEALTH))
        Overkill = Damage - Target->GetUInt32Value(UNIT_FIELD_HEALTH);

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
        data << uint32(39); // 1 + 4 + 32 + 2
    else
        data << uint32(37); // 1 + 4 + 32

    data << uint8(0);

    Caster->SendMessageToSet(&data, bToset);
}

void Object::SendAttackerStateUpdate(Object* Caster, Object* Target, dealdamage* Dmg, uint32 Damage, uint32 Abs, uint32 BlockedDamage, uint32 HitStatus, uint32 VState)
{
    if (!Caster || !Target || !Dmg)
        return;

    uint32 Overkill = 0;

    if (Damage > Target->GetUInt32Value(UNIT_FIELD_MAXHEALTH))
        Overkill = Damage - Target->GetUInt32Value(UNIT_FIELD_HEALTH);

    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, 64 * 12);
    data << uint32(HitStatus);
    data.appendPackGUID(Caster->GetGUID());
    data.appendPackGUID(Target->GetGUID());
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

    if (HitStatus & HITSTATUS_UNK)
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

        for (uint8 i = 0; i < 5; ++i)
        {
            data << float(0);
            data << float(0);
        }
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
    if (pUnit == NULL)
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
    if (mgr == NULL)
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

void Object::SetByte(uint32 index, uint32 index1, uint8 value)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    // save updating when val isn't changing.

    uint8* v = &((uint8*)m_uint32Values)[index * 4 + index1];

    if (*v == value)
        return;

    *v = value;

    if (IsInWorld())
    {
        m_updateMask.SetBit(index);

        if (!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }

}

void Object::SetByteFlag(uint16 index, uint8 offset, uint8 newFlag)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    ARCEMU_ASSERT(offset < 4);

    offset <<= 3;

    if (!(uint8(m_uint32Values[index] >> offset) & newFlag))
    {
        m_uint32Values[index] |= uint32(uint32(newFlag) << offset);

        if (IsInWorld())
        {
            m_updateMask.SetBit(index);

            if (!m_objectUpdated)
            {
                m_mapMgr->ObjectUpdated(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::RemoveByteFlag(uint16 index, uint8 offset, uint8 oldFlag)
{
    ARCEMU_ASSERT(index < m_valuesCount);
    ARCEMU_ASSERT(offset < 4);

    offset <<= 3;

    if (uint8(m_uint32Values[index] >> offset) & oldFlag)
    {
        m_uint32Values[index] &= ~uint32(uint32(oldFlag) << offset);

        if (IsInWorld())
        {
            m_updateMask.SetBit(index);

            if (!m_objectUpdated)
            {
                m_mapMgr->ObjectUpdated(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetZoneId(uint32 newZone)
{
    m_zoneId = newZone;

    if (IsPlayer())
    {
        static_cast<Player*>(this)->m_cache->SetUInt32Value(CACHE_PLAYER_ZONEID, newZone);
        if (static_cast<Player*>(this)->GetGroup() != NULL)
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
    MapInfo const* pMapinfo = sMySQLStore.GetWorldMapInfo(GetMapId());

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

//Transporter* Object::GetTransport() const
//{
//    return nullptr; /*objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(obj_movement_info.transporter_info.guid));*/
//}

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

void Object::OutPacketToSet(uint32 Opcode, uint16 Len, const void* Data, bool self)
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

void Object::SendMessageToSet(WorldPacket* data, bool bToSelf, bool myteam_only)
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

void Object::OnRemoveInRangeObject(Object* pObj)
{
    // This method will remain empty for now, don't remove it! -dfighter
}

Object* Object::GetMapMgrObject(const uint64 & guid)
{
    if (!IsInWorld())
        return NULL;

    return GetMapMgr()->_GetObject(guid);
}

Pet* Object::GetMapMgrPet(const uint64 & guid)
{
    if (!IsInWorld())
        return NULL;

    return GetMapMgr()->GetPet(GET_LOWGUID_PART(guid));
}

Unit* Object::GetMapMgrUnit(const uint64 & guid)
{
    if (!IsInWorld())
        return NULL;

    return GetMapMgr()->GetUnit(guid);
}

Player* Object::GetMapMgrPlayer(const uint64 & guid)
{
    if (!IsInWorld())
        return NULL;

    return GetMapMgr()->GetPlayer(GET_LOWGUID_PART(guid));
}

Creature* Object::GetMapMgrCreature(const uint64 & guid)
{
    if (!IsInWorld())
        return NULL;

    return GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
}

GameObject* Object::GetMapMgrGameObject(const uint64 & guid)
{
    if (!IsInWorld())
        return NULL;

    return GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
}

DynamicObject* Object::GetMapMgrDynamicObject(const uint64 & guid)
{
    if (!IsInWorld())
        return NULL;

    return GetMapMgr()->GetDynamicObject(GET_LOWGUID_PART(guid));
}

Object* Object::GetPlayerOwner()
{
    return NULL;
}

MapCell* Object::GetMapCell() const
{
    ARCEMU_ASSERT(m_mapMgr != NULL);
    return m_mapMgr->GetCell(m_mapCell_x, m_mapCell_y);
}

void Object::SetMapCell(MapCell* cell)
{
    if (cell == NULL)
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
            nav_query->findNearestPoly(start, extents, &filter, &startref, NULL);

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
