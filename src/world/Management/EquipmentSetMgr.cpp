/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

#include "Management/EquipmentSetMgr.h"

#include <sstream>

#include "Database/Field.hpp"
#include "Database/Database.h"
#include "WoWGuid.h"
#include "WorldPacket.h"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Utilities/Util.hpp"

namespace Arcemu
{
    EquipmentSet::EquipmentSet(Field const* fields)
    {
        SetGUID = fields[1].asUint32();
        SetID = fields[2].asUint32();
        SetName = fields[3].asCString();
        IconName = fields[4].asCString();
        for (uint32_t i = 0; i < ItemGUID.size(); ++i)
            ItemGUID[i] = fields[5 + i].asUint32();
    }

    EquipmentSetMgr::~EquipmentSetMgr() = default;

    EquipmentSet* EquipmentSetMgr::GetEquipmentSet(uint32_t id)
    {
        EquipmentSetStorage::iterator itr;

        itr = EquipmentSets.find(id);

        if (itr != EquipmentSets.end())
            return itr->second.get();
        else
            return NULL;
    }

    bool EquipmentSetMgr::AddEquipmentSet(uint32_t setGUID, std::unique_ptr<EquipmentSet> set)
    {
        const auto retval = EquipmentSets.emplace(setGUID, std::move(set));
        return retval.second;
    }

    bool EquipmentSetMgr::DeleteEquipmentSet(uint32_t setGUID)
    {
        auto itr = EquipmentSets.find(setGUID);
        if (itr != EquipmentSets.end())
        {
            EquipmentSets.erase(itr);
            return true;
        }
        else
            return false;
    }

    bool EquipmentSetMgr::LoadfromDB(QueryResult* result)
    {
        if (result == NULL)
            return false;

        uint32_t setcount = 0;

        do
        {
            if (setcount >= 10)
            {
                sLogger.failure("There were more than 10 equipment sets for GUID: {}", ownerGUID);
                return true;
            }

            Field* fields = result->Fetch();

            EquipmentSets.try_emplace(fields[1].asUint32(), Util::LazyInstanceCreator([fields] {
                return std::make_unique<EquipmentSet>(fields);
            }));

            setcount++;
        }
        while (result->NextRow());

        return true;
    }

    bool EquipmentSetMgr::SavetoDB(QueryBuffer* buf)
    {
        if (buf == NULL)
            return false;

        std::stringstream ds;
        ds << "DELETE FROM equipmentsets WHERE ownerguid = ";
        ds << ownerGUID;

        buf->AddQueryNA(ds.str().c_str());

        for (EquipmentSetStorage::iterator itr = EquipmentSets.begin(); itr != EquipmentSets.end(); ++itr)
        {
            const auto& set = itr->second;

            std::stringstream ss;

            ss << "INSERT INTO equipmentsets VALUES('";
            ss << ownerGUID << "','";
            ss << set->SetGUID << "','";
            ss << set->SetID << "','";
            ss << CharacterDatabase.EscapeString(set->SetName) << "','";
            ss << set->IconName << "'";

            for (uint32_t j = 0; j < set->ItemGUID.size(); ++j)
            {
                ss << ",'";
                ss << set->ItemGUID[j];
                ss << "'";
            }

            ss << ")";

            buf->AddQueryNA(ss.str().c_str());
        }

        return true;
    }

    void EquipmentSetMgr::FillEquipmentSetListPacket(WorldPacket& data)
    {
        data << uint32_t(EquipmentSets.size());

        for (EquipmentSetStorage::iterator itr = EquipmentSets.begin(); itr != EquipmentSets.end(); ++itr)
        {
            const auto& set = itr->second;

            data << WoWGuid(uint64_t(set->SetGUID));
            data << uint32_t(set->SetID);
            data << std::string(set->SetName);
            data << std::string(set->IconName);

            for (uint32_t i = 0; i < set->ItemGUID.size(); ++i)
            {
                data << WoWGuid(uint64_t(WoWGuid::createItemGuid(set->ItemGUID[i])));
            }
        }
    }
}
