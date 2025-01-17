/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Map/Cells/MapCell.hpp"
#include "Corpse.hpp"

#include <sstream>

#include "Data/Flags.hpp"
#include "Management/ObjectMgr.hpp"
#include "Data/WoWCorpse.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Strings.hpp"
#include "Utilities/Util.hpp"

Corpse::Corpse(uint32_t high, uint32_t low)
{
    m_objectType |= TYPE_CORPSE;
    m_objectTypeId = TYPEID_CORPSE;

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

    m_valuesCount = getSizeOfStructure(WoWCorpse);
    m_uint32Values = _fields;
    memset(m_uint32Values, 0, (getSizeOfStructure(WoWCorpse)) * sizeof(uint32_t));
    m_updateMask.SetCount(getSizeOfStructure(WoWCorpse));

    setOType(TYPE_CORPSE | TYPE_OBJECT);
    setGuid(low, high);

    setScale(1);
}

Corpse::~Corpse()
{
}

void Corpse::create(Player* owner, uint32_t mapid, LocationVector lv)
{
    Object::_Create(mapid, lv.x, lv.y, lv.z, lv.o);

    setOwnerNotifyMap(owner->getGuid());
}

void Corpse::setCorpseDataFromDbString(std::string dbString)
{
    std::string seperator = " ";
    auto dataVector = AscEmu::Util::Strings::split(dbString, seperator);

    uint8_t countPosition = 0;
    uint8_t itemOffset = 6;
    for (auto stringValue : dataVector)
    {
        switch (countPosition)
        {
            case 0: setGuid(std::stoull(stringValue)); break;
            case 1: setOType(std::stoul(stringValue)); break;
            case 2: setEntry(std::stoul(stringValue)); break;
            case 3: setScale(std::stof(stringValue)); break;

            case 4: setOwnerGuid(std::stoull(stringValue)); break;
            case 5: setDisplayId(std::stoul(stringValue)); break;

            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
                setItem(countPosition - itemOffset, std::stoul(stringValue));
                break;

            case 25: setBytes1(std::stoul(stringValue)); break;
            case 26: setBytes2(std::stoul(stringValue)); break;
            case 27: setFlags(std::stoul(stringValue)); break;
            case 28: setDynamicFlags(std::stoul(stringValue)); break;
        }
        ++countPosition;
    }
}

void Corpse::saveToDB()
{
    std::stringstream ss;
    ss.rdbuf()->str("");
    ss << "REPLACE INTO corpses (guid, positionx, positiony, positionz, orientation, zoneId, mapId, data, instanceid) VALUES ("
        << getGuidLow()
        << ", '"
        << GetPositionX()
        << "', '" << GetPositionY()
        << "', '" << GetPositionZ()
        << "', '" << GetOrientation()
        << "', '" << getZoneId()
        << "', '" << GetMapId()

        << "', '";
    ss << getGuid() << " " << getOType() << " " << getEntry() << " " << getScale() << " ";
    ss << getOwnerGuid() << " " << getDisplayId() << " ";

    for (uint8_t i = 0; i < WOWCORPSE_ITEM_COUNT; ++i)
        ss << getItem(i) << " ";

    ss << getBytes1() << " " << getBytes2() << " " << getFlags() << " " << getDynamicFlags() << " ";

    ss << "', " << GetInstanceID() << ")";

    CharacterDatabase.Execute(ss.str().c_str());
}

void Corpse::deleteFromDB()
{
    std::stringstream ss;
    ss << "DELETE FROM corpses WHERE guid=" << getGuidLow();

    CharacterDatabase.Execute(ss.str().c_str());
}

void Corpse::setLoadedFromDB(bool value) { _loadedfromdb = value; }
bool Corpse::getLoadedFromDB(void) { return _loadedfromdb; }

void Corpse::setCorpseState(uint32_t state) { m_state = state; }
uint32_t Corpse::getCorpseState() { return m_state; }

void Corpse::setOwnerNotifyMap(uint64_t guid)
{
    setOwnerGuid(guid);

    if (guid == 0)
    {
        if (MapCell* mapCell = GetMapCell())
            mapCell->corpseGoneIdle(this);
    }
}

void Corpse::generateLoot()
{
    loot.gold = Util::getRandomUInt(50, 150);
}

void Corpse::despawn()
{
    if (this->IsInWorld())
        RemoveFromWorld(false);
}

void Corpse::spawnBones()
{
    setFlags(CORPSE_FLAG_BONE | CORPSE_FLAG_UNK1);
    setOwnerNotifyMap(0);

    for (uint8_t i = 0; i < EQUIPMENT_SLOT_END; ++i)
        if (getItem(i))
            setItem(i, 0);

    deleteFromDB();
    setCorpseState(CORPSE_STATE_BONES);
}

void Corpse::delink()
{
    setFlags(CORPSE_FLAG_BONE | CORPSE_FLAG_UNK1);
    setOwnerNotifyMap(0);

    setCorpseState(CORPSE_STATE_BONES);
    deleteFromDB();
}

void Corpse::resetDeathClock() { m_time = time(nullptr); }
time_t Corpse::getDeathClock() { return m_time; }

 //////////////////////////////////////////////////////////////////////////////////////////
 // WoWData
uint64_t Corpse::getOwnerGuid() const { return corpseData()->owner_guid; }
void Corpse::setOwnerGuid(uint64_t guid) { write(corpseData()->owner_guid, guid); }

// party

uint32_t Corpse::getDisplayId() const { return corpseData()->display_id; }
void Corpse::setDisplayId(uint32_t id) { write(corpseData()->display_id, id); }

uint32_t Corpse::getItem(uint8_t slot) const { return corpseData()->item[slot]; }
void Corpse::setItem(uint8_t slot, uint32_t item) { write(corpseData()->item[slot], item); }

//bytes 1 start
uint32_t Corpse::getBytes1() const { return corpseData()->corpse_bytes_1.raw; }
void Corpse::setBytes1(uint32_t bytes) { write(corpseData()->corpse_bytes_1.raw, bytes); }

//unk1

uint8_t Corpse::getRace() const { return corpseData()->corpse_bytes_1.s.race; }
void Corpse::setRace(uint8_t race) { write(corpseData()->corpse_bytes_1.s.race, race); }

uint8_t Corpse::getGender() const { return corpseData()->corpse_bytes_1.s.gender; }
void Corpse::setGender(uint8_t gender) { write(corpseData()->corpse_bytes_1.s.gender, gender); }

uint8_t Corpse::getSkinColor() const { return corpseData()->corpse_bytes_1.s.skin_color; }
void Corpse::setSkinColor(uint8_t color) { write(corpseData()->corpse_bytes_1.s.skin_color, color); }
//bytes 1 end

//bytes 2 start
uint32_t Corpse::getBytes2() const { return corpseData()->corpse_bytes_2.raw; }
void Corpse::setBytes2(uint32_t bytes) { write(corpseData()->corpse_bytes_2.raw, bytes); }

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
