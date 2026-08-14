/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/EquipmentSetMgr.h"

#include <sstream>

#include "Database/Field.hpp"
#include "Database/Database.hpp"
#include "WoWGuid.hpp"
#include "Network/WorldPacket.hpp"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Utilities/Util.hpp"


EquipmentSet::EquipmentSet(Field const* fields)
{
    setGuid = fields[1].asUint32();
    setId = fields[2].asUint32();
    setName = fields[3].asCString();
    iconName = fields[4].asCString();
    for (uint32_t i = 0; i < itemGuid.size(); ++i)
        itemGuid[i] = fields[5 + i].asUint32();
}

EquipmentSetMgr::~EquipmentSetMgr() = default;

EquipmentSet* EquipmentSetMgr::getEquipmentSet(uint32_t setGuid)
{
    auto itr = m_equipmentSets.find(setGuid);
    if (itr != m_equipmentSets.end())
        return itr->second.get();

    return nullptr;
}

bool EquipmentSetMgr::addEquipmentSet(uint32_t setGuid, std::unique_ptr<EquipmentSet> set)
{
    const auto retval = m_equipmentSets.emplace(setGuid, std::move(set));
    return retval.second;
}

bool EquipmentSetMgr::deleteEquipmentSet(uint32_t setGuid)
{
    auto itr = m_equipmentSets.find(setGuid);
    if (itr != m_equipmentSets.end())
    {
        m_equipmentSets.erase(itr);
        return true;
    }

    return false;
}

bool EquipmentSetMgr::loadFromDB(QueryResult* result)
{
    if (result == nullptr)
        return false;

    uint32_t setcount = 0;

    do
    {
        if (setcount >= 10)
        {
            sLogger.failure("There were more than 10 equipment sets for GUID: {}", m_ownerGuid);
            return true;
        }

        Field* fields = result->fetch();

        m_equipmentSets.try_emplace(fields[1].asUint32(), Util::LazyInstanceCreator([fields] {
            return std::make_unique<EquipmentSet>(fields);
            }));

        setcount++;
    } while (result->nextRow());

    return true;
}

bool EquipmentSetMgr::saveToDB(QueryBuffer* bufffer)
{
    if (bufffer == nullptr)
        return false;

    std::stringstream ds;
    ds << "DELETE FROM equipmentsets WHERE ownerguid = ";
    ds << m_ownerGuid;

    bufffer->addQueryNA(ds.str().c_str());

    for (EquipmentSetStorage::iterator itr = m_equipmentSets.begin(); itr != m_equipmentSets.end(); ++itr)
    {
        const auto& set = itr->second;

        std::stringstream ss;

        ss << "INSERT INTO equipmentsets VALUES('";
        ss << m_ownerGuid << "','";
        ss << set->setGuid << "','";
        ss << set->setId << "','";
        ss << CharacterDatabase.escapeString(set->setName) << "','";
        ss << set->iconName << "'";

        for (uint32_t j = 0; j < set->itemGuid.size(); ++j)
        {
            ss << ",'";
            ss << set->itemGuid[j];
            ss << "'";
        }

        ss << ")";

        bufffer->addQueryNA(ss.str().c_str());
    }

    return true;
}

const EquipmentSetStorage& EquipmentSetMgr::getEquipmentSets() const noexcept
{
    return m_equipmentSets;
}
