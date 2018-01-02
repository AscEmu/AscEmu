/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "Server/MainServerDefines.h"
#include "Map/MapCell.h"
#include "Corpse.h"
#include "Objects/ObjectMgr.h"

Corpse::Corpse(uint32 high, uint32 low)
{
    m_objectType |= TYPE_CORPSE;
    m_objectTypeId = TYPEID_CORPSE;

#if VERSION_STRING != Cata
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_POSITION);
#else
    m_updateFlag = UPDATEFLAG_POSITION;
#endif

    m_valuesCount = CORPSE_END;

    m_uint32Values = _fields;
    memset(m_uint32Values, 0, (CORPSE_END)*sizeof(uint32));
    m_updateMask.SetCount(CORPSE_END);

    setUInt32Value(OBJECT_FIELD_TYPE, TYPE_CORPSE | TYPE_OBJECT);

    SetLowGUID(low);
    SetHighGUID(high);
    m_wowGuid.Init(GetGUID());

    SetScale(1);   //always 1

    m_time = (time_t)0;

    m_state = CORPSE_STATE_BODY;
    _loadedfromdb = false;

    if (high != 0)
        objmgr.AddCorpse(this);
}

Corpse::~Corpse()
{
    objmgr.RemoveCorpse(this);
    //just in case
}


void Corpse::Create(Player* owner, uint32 mapid, float x, float y, float z, float ang)
{
    Object::_Create(mapid, x, y, z, ang);

    SetOwner(owner->GetGUID());
    _loadedfromdb = false;  // can't be created from db ;)
}

void Corpse::SaveToDB()
{
    //save corpse to DB
    std::stringstream ss;
    ss << "DELETE FROM corpses WHERE guid = " << GetLowGUID();
    CharacterDatabase.Execute(ss.str().c_str());

    ss.rdbuf()->str("");
    ss << "INSERT INTO corpses (guid, positionx, positiony, positionz, orientation, zoneId, mapId, data, instanceid) VALUES ("
        << GetLowGUID()
        << ", '" 
        << GetPositionX() 
        << "', '" << GetPositionY() 
        << "', '" << GetPositionZ() 
        << "', '" << GetOrientation() 
        << "', '" << GetZoneId() 
        << "', '" << GetMapId() 
        << "', '";

    for (uint16 i = 0; i < m_valuesCount; i++)
        ss << getUInt32Value(i) << " ";

    ss << "', " << GetInstanceID() << ")";

    CharacterDatabase.Execute(ss.str().c_str());
}

void Corpse::DeleteFromDB()
{
    //delete corpse from db when its not needed anymore
    char sql[256];

    snprintf(sql, 256, "DELETE FROM corpses WHERE guid=%u", (unsigned int)GetLowGUID());
    CharacterDatabase.Execute(sql);
}

void Corpse::Despawn()
{
    if (this->IsInWorld())
    {
        RemoveFromWorld(false);
    }
}

void Corpse::generateLoot()
{
    loot.gold = Util::getRandomUInt(50, 150); // between 50c and 1.5s, need to fix this!
}

void Corpse::SpawnBones()
{
    setUInt32Value(CORPSE_FIELD_FLAGS, 5);
    SetOwner(0); // remove corpse owner association
    //remove item association
    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if (getUInt32Value(CORPSE_FIELD_ITEM + i))
            setUInt32Value(CORPSE_FIELD_ITEM + i, 0);
    }
    DeleteFromDB();
    objmgr.CorpseAddEventDespawn(this);
    SetCorpseState(CORPSE_STATE_BONES);
}

void Corpse::Delink()
{
    setUInt32Value(CORPSE_FIELD_FLAGS, 5);
    SetOwner(0);
    SetCorpseState(CORPSE_STATE_BONES);
    DeleteFromDB();
}

void Corpse::SetOwner(uint64 guid)
{
    setUInt64Value(CORPSE_FIELD_OWNER, guid);
    if (guid == 0)
    {
        //notify the MapCell that the Corpse has no more an owner so the MapCell can go idle (if there's nothing else)
        MapCell* cell = GetMapCell();
        if (cell != NULL)
            cell->CorpseGoneIdle(this);
    }
}
