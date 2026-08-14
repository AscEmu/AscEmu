/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <map>
#include <memory>
#include <string>

#include "Objects/ItemDefines.hpp"

class Field;
class WorldPacket;
class QueryBuffer;
class QueryResult;


struct EquipmentSet
{
    EquipmentSet() = default;
    EquipmentSet(Field const* fields);

    uint32_t setGuid = 0;
    uint32_t setId = 0;
    std::string setName;
    std::string iconName;
    std::array<uint32_t, EQUIPMENT_SLOT_END> itemGuid = {};
};

typedef std::map<uint32_t, std::unique_ptr<EquipmentSet>> EquipmentSetStorage;


class EquipmentSetMgr
{
public:
    EquipmentSetMgr() { m_ownerGuid = 1; }
    EquipmentSetMgr(uint32_t ownerGuid) { this->m_ownerGuid = ownerGuid; }
    ~EquipmentSetMgr();

    EquipmentSet* getEquipmentSet(uint32_t setGuid);
    bool addEquipmentSet(uint32_t setGuid, std::unique_ptr<EquipmentSet> set);
    bool deleteEquipmentSet(uint32_t setGuid);

    const EquipmentSetStorage& getEquipmentSets() const noexcept;

    bool loadFromDB(QueryResult* result);
    bool saveToDB(QueryBuffer* bufffer);

private:
    EquipmentSetMgr(EquipmentSetMgr& /*other*/) {}
    EquipmentSetMgr& operator=(EquipmentSetMgr& /*other*/) { return *this; }

    uint32_t m_ownerGuid = 0;

    EquipmentSetStorage m_equipmentSets;
};

