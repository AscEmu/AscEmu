/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Corpse.hpp"

#include <sstream>

#include "Data/Flags.hpp"
#include "Management/ObjectMgr.hpp"
#include "Data/WoWCorpse.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Map/Management/ObjectFactory.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Strings.hpp"
#include "Utilities/Util.hpp"
#include "Server/World.h"

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
#elif defined(AE_FOREVER)
// Copied from MoP as a temporary baseline. Replace with dedicated Forever values once verified.
    m_updateFlag = UPDATEFLAG_HAS_POSITION;
#endif

    m_valuesCount = getSizeOfStructure(WoWCorpse);
    m_uint32Values = _fields;
    memset(m_uint32Values, 0, (getSizeOfStructure(WoWCorpse)) * sizeof(uint32_t));
    m_updateMask.SetCount(getSizeOfStructure(WoWCorpse));

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

    for (uint8_t i = 0; i < WOWCORPSE_ITEM_COUNT; ++i)
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
#if defined(AE_FOREVER)
namespace
{
    WoWGuid makeForeverCorpseReferenceGuid(Corpse const* owner, uint64_t legacyGuid)
    {
        if (legacyGuid == 0)
            return WoWGuid::createModernEmpty();
        return WoWGuid::createModernFromLegacy(legacyGuid, worldConfig.battleNetComm.realmId, static_cast<uint16_t>(owner->GetMapId()), 0, 0);
    }
}
#endif

uint64_t Corpse::getOwnerGuid() const
{
#if defined(AE_FOREVER)
    return m_foreverCorpseFields.owner.toLegacyRaw();
#else
    return corpseData()->owner_guid;
#endif
}
void Corpse::setOwnerGuid(uint64_t guid)
{
#if defined(AE_FOREVER)
    const WoWGuid value = makeForeverCorpseReferenceGuid(this, guid);
    if (m_foreverCorpseFields.owner.getModernHigh() == value.getModernHigh() && m_foreverCorpseFields.owner.getModernLow() == value.getModernLow())
        return;
    m_foreverCorpseFields.owner = value;
    m_foreverCorpseFields.markChanged(AscEmu::Version::Forever::Fields::CorpseData::OwnerBit);
    updateObject();
#else
    write(corpseData()->owner_guid, guid);
#endif
}

// party

uint32_t Corpse::getDisplayId() const
{
#if defined(AE_FOREVER)
    return m_foreverCorpseFields.displayId;
#else
    return corpseData()->display_id;
#endif
}
void Corpse::setDisplayId(uint32_t id)
{
#if defined(AE_FOREVER)
    if (m_foreverCorpseFields.displayId == id)
        return;
    m_foreverCorpseFields.displayId = id;
    m_foreverCorpseFields.markChanged(AscEmu::Version::Forever::Fields::CorpseData::DisplayIdBit);
    updateObject();
#else
    write(corpseData()->display_id, id);
#endif
}

uint32_t Corpse::getItem(uint8_t slot) const
{
#if defined(AE_FOREVER)
    return slot < m_foreverCorpseFields.items.size() ? m_foreverCorpseFields.items[slot] : 0;
#else
    return corpseData()->item[slot];
#endif
}
void Corpse::setItem(uint8_t slot, uint32_t item)
{
#if defined(AE_FOREVER)
    if (slot >= m_foreverCorpseFields.items.size() || m_foreverCorpseFields.items[slot] == item)
        return;
    m_foreverCorpseFields.items[slot] = item;
    m_foreverCorpseFields.markArrayChanged(AscEmu::Version::Forever::Fields::CorpseData::ItemsGroupBit, AscEmu::Version::Forever::Fields::CorpseData::ItemsFirstBit + slot);
    updateObject();
#else
    write(corpseData()->item[slot], item);
#endif
}

//bytes 1 start
uint32_t Corpse::getBytes1() const { return corpseData()->corpse_bytes_1.raw; }
void Corpse::setBytes1(uint32_t bytes) { write(corpseData()->corpse_bytes_1.raw, bytes); }

//unk1

uint8_t Corpse::getRace() const
{
#if defined(AE_FOREVER)
    return m_foreverCorpseFields.raceId;
#else
    return corpseData()->corpse_bytes_1.s.race;
#endif
}
void Corpse::setRace(uint8_t race)
{
#if defined(AE_FOREVER)
    if (m_foreverCorpseFields.raceId == race)
        return;
    m_foreverCorpseFields.raceId = race;
    m_foreverCorpseFields.markChanged(AscEmu::Version::Forever::Fields::CorpseData::RaceIdBit);
    updateObject();
#else
    write(corpseData()->corpse_bytes_1.s.race, race);
#endif
}

uint8_t Corpse::getGender() const
{
#if defined(AE_FOREVER)
    return m_foreverCorpseFields.sex;
#else
    return corpseData()->corpse_bytes_1.s.gender;
#endif
}
void Corpse::setGender(uint8_t gender)
{
#if defined(AE_FOREVER)
    if (m_foreverCorpseFields.sex == gender)
        return;
    m_foreverCorpseFields.sex = gender;
    m_foreverCorpseFields.markChanged(AscEmu::Version::Forever::Fields::CorpseData::SexBit);
    updateObject();
#else
    write(corpseData()->corpse_bytes_1.s.gender, gender);
#endif
}

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

uint32_t Corpse::getFlags() const
{
#if defined(AE_FOREVER)
    return m_foreverCorpseFields.flags;
#else
    return corpseData()->corpse_flags;
#endif
}
void Corpse::setFlags(uint32_t flags)
{
#if defined(AE_FOREVER)
    if (m_foreverCorpseFields.flags == flags)
        return;
    m_foreverCorpseFields.flags = flags;
    m_foreverCorpseFields.markChanged(AscEmu::Version::Forever::Fields::CorpseData::FlagsBit);
    updateObject();
#else
    write(corpseData()->corpse_flags, flags);
#endif
}

uint32_t Corpse::getDynamicFlags() const
{
#if defined(AE_FOREVER)
    return m_foreverCorpseFields.dynamicFlags;
#else
    return corpseData()->dynamic_flags;
#endif
}
void Corpse::setDynamicFlags(uint32_t flags)
{
#if defined(AE_FOREVER)
    if (m_foreverCorpseFields.dynamicFlags == flags)
        return;
    m_foreverCorpseFields.dynamicFlags = flags;
    m_foreverCorpseFields.markChanged(AscEmu::Version::Forever::Fields::CorpseData::DynamicFlagsBit);
    updateObject();
#else
    write(corpseData()->dynamic_flags, flags);
#endif
}
