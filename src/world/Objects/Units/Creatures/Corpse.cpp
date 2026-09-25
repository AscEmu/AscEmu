/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Corpse.hpp"

#include <sstream>

#include "Data/Flags.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Map/Management/ObjectFactory.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Strings.hpp"
#include "Utilities/Util.hpp"
#include "Server/World.h"
#include "Version/ObjectLayout.hpp"

using Version::CorpseField;

Corpse::Corpse(uint64_t guid)
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

    m_valuesCount = Version::layouts().corpse.valueCount();
    m_uint32Values = _fields;
    memset(m_uint32Values, 0, (Version::layouts().corpse.valueCount()) * sizeof(uint32_t));
    m_updateMask.SetCount(Version::layouts().corpse.valueCount());

    setOType(TYPE_CORPSE | TYPE_OBJECT);
    setGuid(guid);

    setScale(1);
}

Corpse::~Corpse()
{
}

void Corpse::destroy()
{
    WorldMap* map = getWorldMap();
    if (!map)
    {
        sLogger.warning("Corpse::destroy called without WorldMap guid={}", GetNewGUID().getRawGuid());
        return;
    }

    if (IsInWorld())
        map->getObjectFactory().removeAndDestroy(this, /*recycleGuid=*/true);
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
            case 1: setOType(static_cast<uint16_t>(std::stoul(stringValue))); break;
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

    for (uint8_t i = 0; i < Version::fieldCount(CorpseField::Item); ++i)
        ss << getItem(i) << " ";

    ss << getBytes1() << " " << getBytes2() << " " << getFlags() << " " << getDynamicFlags() << " ";

    ss << "', " << GetInstanceID() << ")";

    CharacterDatabase.execute(ss.str().c_str());
}

void Corpse::deleteFromDB()
{
    std::stringstream ss;
    ss << "DELETE FROM corpses WHERE guid=" << getGuidLow();

    CharacterDatabase.execute(ss.str().c_str());
}

void Corpse::setLoadedFromDB(bool value) { _loadedfromdb = value; }
bool Corpse::getLoadedFromDB(void) { return _loadedfromdb; }

void Corpse::setCorpseState(uint32_t state) { m_state = state; }
uint32_t Corpse::getCorpseState() { return m_state; }

void Corpse::setOwnerNotifyMap(uint64_t guid)
{
    const uint64_t oldOwnerGuid = getOwnerGuid();
    setOwnerGuid(guid);

    if (oldOwnerGuid == guid)
        return;

    // Player corpses keep their area active while they still belong to an owner.
    // Once the owner link is cleared (bones/delink), the corpse should stop being
    // an activator so its cells/grid can unload after the normal delay.
    WorldMap* map = getWorldMap();
    if (!map || !IsInWorld())
        return;

    auto h = map->getSpatialIndex().handleByGuid(GetNewGUID());
    if (!h.id)
        return;

    if (guid == 0)
        map->getVisibilitySystem().setActivatorRole(h, false, 0);
    else if (oldOwnerGuid == 0)
        map->getVisibilitySystem().setActivatorRole(h, true, worldConfig.server.mapCellNumber);
}

void Corpse::generateLoot()
{
    loot.gold = Util::getRandomUInt(50, 150);
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
uint64_t Corpse::getOwnerGuid() const { return getField<uint64_t>(CorpseField::OwnerGuid); }
void Corpse::setOwnerGuid(uint64_t guid) { setField<uint64_t>(CorpseField::OwnerGuid, guid); }

// party

uint32_t Corpse::getDisplayId() const { return getField<uint32_t>(CorpseField::DisplayId); }
void Corpse::setDisplayId(uint32_t id) { setField<uint32_t>(CorpseField::DisplayId, id); }

uint32_t Corpse::getItem(uint8_t slot) const { return getField<uint32_t>(CorpseField::Item, slot); }
void Corpse::setItem(uint8_t slot, uint32_t item) { setField<uint32_t>(CorpseField::Item, item, slot); }

//bytes 1 start
uint32_t Corpse::getBytes1() const { return getField<uint32_t>(CorpseField::CorpseBytes1); }
void Corpse::setBytes1(uint32_t bytes) { setField<uint32_t>(CorpseField::CorpseBytes1, bytes); }

//unk1

uint8_t Corpse::getRace() const { return getField<uint8_t>(CorpseField::CorpseBytes1Race); }
void Corpse::setRace(uint8_t race) { setField<uint8_t>(CorpseField::CorpseBytes1Race, race); }

uint8_t Corpse::getGender() const { return getField<uint8_t>(CorpseField::CorpseBytes1Gender); }
void Corpse::setGender(uint8_t gender) { setField<uint8_t>(CorpseField::CorpseBytes1Gender, gender); }

uint8_t Corpse::getSkinColor() const { return getField<uint8_t>(CorpseField::CorpseBytes1SkinColor); }
void Corpse::setSkinColor(uint8_t color) { setField<uint8_t>(CorpseField::CorpseBytes1SkinColor, color); }
//bytes 1 end

//bytes 2 start
uint32_t Corpse::getBytes2() const { return getField<uint32_t>(CorpseField::CorpseBytes2); }
void Corpse::setBytes2(uint32_t bytes) { setField<uint32_t>(CorpseField::CorpseBytes2, bytes); }

uint8_t Corpse::getFace() const { return getField<uint8_t>(CorpseField::CorpseBytes2Face); }
void Corpse::setFace(uint8_t face) { setField<uint8_t>(CorpseField::CorpseBytes2Face, face); }

uint8_t Corpse::getHairStyle() const { return getField<uint8_t>(CorpseField::CorpseBytes2Face); }
void Corpse::setHairStyle(uint8_t style) { setField<uint8_t>(CorpseField::CorpseBytes2Face, style); }

uint8_t Corpse::getHairColor() const { return getField<uint8_t>(CorpseField::CorpseBytes2Face); }
void Corpse::setHairColor(uint8_t color) { setField<uint8_t>(CorpseField::CorpseBytes2Face, color); }

uint8_t Corpse::getFacialFeatures() const { return getField<uint8_t>(CorpseField::CorpseBytes2Face); }
void Corpse::setFacialFeatures(uint8_t feature) { setField<uint8_t>(CorpseField::CorpseBytes2Face, feature); }
//bytes 2 end

uint32_t Corpse::getFlags() const { return getField<uint32_t>(CorpseField::CorpseFlags); }
void Corpse::setFlags(uint32_t flags) { setField<uint32_t>(CorpseField::CorpseFlags, flags); }

uint32_t Corpse::getDynamicFlags() const { return getField<uint32_t>(CorpseField::DynamicFlags); }
void Corpse::setDynamicFlags(uint32_t flags) { setField<uint32_t>(CorpseField::DynamicFlags, flags); }
