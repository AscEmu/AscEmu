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
#include "Server/MainServerDefines.h"
#include "Map/MapCell.h"
#include "Corpse.h"
#include "Objects/ObjectMgr.h"
#include "Data/WoWCorpse.h"

// MIT Start
 //////////////////////////////////////////////////////////////////////////////////////////
 // WoWData

uint64_t Corpse::getOwnerGuid() const { return corpseData()->guid; }
void Corpse::setOwnerGuid(uint64_t guid) { write(corpseData()->owner_guid, guid); }

// party

uint32_t Corpse::getDisplayId() const { return corpseData()->display_id; }
void Corpse::setDisplayId(uint32_t id) { write(corpseData()->display_id, id); }

uint32_t Corpse::getItem(uint8_t slot) const { return corpseData()->item[slot]; }
void Corpse::setItem(uint8_t slot, uint32_t item) { write(corpseData()->item[slot], item); }

//bytes 1 start
//unk1

uint8_t Corpse::getRace() const { return corpseData()->corpse_bytes_1.s.race; }
void Corpse::setRace(uint8_t race) { write(corpseData()->corpse_bytes_1.s.race, race); }

//unk2

uint8_t Corpse::getSkinColor() const { return corpseData()->corpse_bytes_1.s.skin_color; }
void Corpse::setSkinColor(uint8_t color) { write(corpseData()->corpse_bytes_1.s.skin_color, color); }
//bytes 1 end

//bytes 2 start
uint8_t Corpse::getFace() const { return corpseData()->corpse_bytes_2.s.face; }
void Corpse::setFace(uint8_t face) { write(corpseData()->corpse_bytes_2.s.face, face); }

uint8_t Corpse::getHairStyle() const { return corpseData()->corpse_bytes_2.s.face; }
void Corpse::setHairStyle(uint8_t style) { write(corpseData()->corpse_bytes_2.s.face, style); }

uint8_t Corpse::getHairColor() const { return corpseData()->corpse_bytes_2.s.face; }
void Corpse::setHairColor(uint8_t color) { write(corpseData()->corpse_bytes_2.s.face, color); }

uint8_t Corpse::getFacialFeatures() const { return corpseData()->corpse_bytes_2.s.face; }
void Corpse::setFacialFeatures(uint8_t feature) { write(corpseData()->corpse_bytes_2.s.face, feature); }
//bytes 2 end

uint32_t Corpse::getFlags() const { return corpseData()->corpse_flags; }
void Corpse::setFlags(uint32_t flags) { write(corpseData()->corpse_flags, flags); }

uint32_t Corpse::getDynamicFlags() const { return corpseData()->dynamic_flags; }
void Corpse::setDynamicFlags(uint32_t flags) { write(corpseData()->dynamic_flags, flags); }

 // MIT End
 // AGPL Start
Corpse::Corpse(uint32 high, uint32 low)
{
    m_objectType |= TYPE_CORPSE;
    m_objectTypeId = TYPEID_CORPSE;

#if VERSION_STRING <= TBC
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_LOWGUID);
#elif VERSION_STRING == WotLK
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_POSITION);
#elif VERSION_STRING == Cata
    m_updateFlag = UPDATEFLAG_POSITION;
#endif

    m_valuesCount = CORPSE_END;

    m_uint32Values = _fields;
    memset(m_uint32Values, 0, (CORPSE_END)*sizeof(uint32));
    m_updateMask.SetCount(CORPSE_END);

    setOType(TYPE_CORPSE | TYPE_OBJECT);

    setGuidLow(low);
    setGuidHigh(high);
    m_wowGuid.Init(getGuid());

    setScale(1);   //always 1

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

    SetOwner(owner->getGuid());
    _loadedfromdb = false;  // can't be created from db ;)
}

void Corpse::SaveToDB()
{
    //save corpse to DB
    std::stringstream ss;
    ss << "DELETE FROM corpses WHERE guid = " << getGuidLow();
    CharacterDatabase.Execute(ss.str().c_str());

    ss.rdbuf()->str("");
    ss << "INSERT INTO corpses (guid, positionx, positiony, positionz, orientation, zoneId, mapId, data, instanceid) VALUES ("
        << getGuidLow()
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

    snprintf(sql, 256, "DELETE FROM corpses WHERE guid=%u", (unsigned int)getGuidLow());
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
    setFlags(CORPSE_FLAG_BONE | CORPSE_FLAG_UNK1);
    SetOwner(0); // remove corpse owner association
    //remove item association
    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if (getItem(i))
            setItem(i, 0);
    }
    DeleteFromDB();
    objmgr.CorpseAddEventDespawn(this);
    SetCorpseState(CORPSE_STATE_BONES);
}

void Corpse::Delink()
{
    setFlags(CORPSE_FLAG_BONE | CORPSE_FLAG_UNK1);
    SetOwner(0);
    SetCorpseState(CORPSE_STATE_BONES);
    DeleteFromDB();
}

void Corpse::SetOwner(uint64 guid)
{
    setOwnerGuid(guid);
    if (guid == 0)
    {
        //notify the MapCell that the Corpse has no more an owner so the MapCell can go idle (if there's nothing else)
        MapCell* cell = GetMapCell();
        if (cell != NULL)
            cell->CorpseGoneIdle(this);
    }
}
