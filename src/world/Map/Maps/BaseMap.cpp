/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "MMapManager.h"
#include "Logging/Logger.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Storage/WDB/WDBStructures.hpp"

BaseMap::BaseMap(uint32_t mapId, MySQLStructure::MapInfo const* mapInfo, WDB::Structures::MapEntry const* mapEntry) :
        _mapEntry(mapEntry), _mapInfo(mapInfo), _mapId(mapId)
{
    // get our name
    if (_mapInfo)
        name = _mapInfo->name;
    else
        name = "Unknown";
}

BaseMap::~BaseMap()
{
    sLogger.info("BaseMap : Close Tempelate from Map {}", this->_mapId);
}

std::string BaseMap::getMapName()
{
    return name;
}

uint32_t BaseMap::getMapId() const
{
    return _mapEntry->id;
}

bool BaseMap::isDungeon() const
{
    if (_mapEntry == nullptr || _mapInfo == nullptr)
        return false;
    return _mapEntry->isDungeon() && (_mapInfo->isDungeon() || _mapInfo->isMultimodeDungeon());
}

bool BaseMap::isRaid() const
{
    if (_mapEntry == nullptr || _mapInfo == nullptr)
        return false;
    return _mapEntry->isRaid() && _mapInfo->isRaid();
}

bool BaseMap::isBattleground() const
{
    if (_mapEntry == nullptr || _mapInfo == nullptr)
        return false;
    return _mapEntry->isBattleground() && _mapInfo->isBattlegroundOrArena();
}

bool BaseMap::isArena() const
{
#if VERSION_STRING > Classic
    if (_mapEntry == nullptr || _mapInfo == nullptr)
        return false;
    return _mapEntry->isArena() && _mapInfo->isBattlegroundOrArena();
#else
    return false;
#endif
}

bool BaseMap::isBattlegroundOrArena() const
{
    if (_mapEntry == nullptr || _mapInfo == nullptr)
        return false;
    return _mapEntry->isBattlegroundOrArena() && _mapInfo->isBattlegroundOrArena();
}

bool BaseMap::isWorldMap() const
{
    if (_mapEntry == nullptr || _mapInfo == nullptr)
        return false;
    return _mapEntry->isWorldMap() && _mapInfo->isWorldMap();
}

bool BaseMap::isInstanceMap() const
{
    if (_mapEntry == nullptr || _mapInfo == nullptr)
        return false;
    return _mapEntry->isInstanceMap() && _mapInfo->isInstanceMap();
}

bool BaseMap::isInstanceableMap() const
{
    if (_mapEntry == nullptr || _mapInfo == nullptr)
        return false;
    return _mapEntry->isInstanceableMap() && _mapInfo->isInstanceableMap();
}

bool BaseMap::getEntrancePos([[maybe_unused]] int32_t& mapid, [[maybe_unused]] float& x, [[maybe_unused]] float& y) const
{
#if VERSION_STRING > Classic
    if (!_mapEntry)
        return false;
    return _mapEntry->getEntrancePos(mapid, x, y);
#else
    return false;
#endif
}
