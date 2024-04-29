/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
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
#include "Database/Field.hpp"
#include "Database/Database.h"
#include "WoWGuid.h"
#include "WorldPacket.h"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"

namespace Arcemu
{
    EquipmentSetMgr::~EquipmentSetMgr()
    {
        for (EquipmentSetStorage::iterator itr = EquipmentSets.begin(); itr != EquipmentSets.end(); ++itr)
            delete itr->second;

        EquipmentSets.clear();
    }

    EquipmentSet* EquipmentSetMgr::GetEquipmentSet(uint32 id)
    {
        EquipmentSetStorage::iterator itr;

        itr = EquipmentSets.find(id);

        if (itr != EquipmentSets.end())
            return itr->second;
        else
            return NULL;
    }

    bool EquipmentSetMgr::AddEquipmentSet(uint32 setGUID, EquipmentSet* set)
    {
        std::pair< EquipmentSetStorage::iterator, bool > retval;

        retval = EquipmentSets.emplace(std::pair< uint32, EquipmentSet* >(setGUID, set));

        return retval.second;
    }

    bool EquipmentSetMgr::DeleteEquipmentSet(uint32 setGUID)
    {
        EquipmentSetStorage::iterator itr;

        itr = EquipmentSets.find(setGUID);

        if (itr != EquipmentSets.end())
        {
            EquipmentSet* set = itr->second;

            EquipmentSets.erase(itr);
            delete set;
            set = NULL;

            return true;
        }
        else
            return false;
    }

    bool EquipmentSetMgr::LoadfromDB(QueryResult* result)
    {
        if (result == NULL)
            return false;

        uint32 setcount = 0;

        do
        {
            if (setcount >= 10)
            {
                sLogger.failure("There were more than 10 equipment sets for GUID: {}", ownerGUID);
                return true;
            }

            Field* fields = result->Fetch();

            EquipmentSet* set = new EquipmentSet();
            set->SetGUID = fields[1].GetUInt32();
            set->SetID = fields[2].GetUInt32();
            set->SetName = fields[3].GetString();
            set->IconName = fields[4].GetString();

            for (uint32 i = 0; i < set->ItemGUID.size(); ++i)
                set->ItemGUID[i] = fields[5 + i].GetUInt32();

            EquipmentSets.emplace(std::pair< uint32, EquipmentSet* >(set->SetGUID, set));
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
            EquipmentSet* set = itr->second;

            std::stringstream ss;

            ss << "INSERT INTO equipmentsets VALUES('";
            ss << ownerGUID << "','";
            ss << set->SetGUID << "','";
            ss << set->SetID << "','";
            ss << CharacterDatabase.EscapeString(set->SetName) << "','";
            ss << set->IconName << "'";

            for (uint32 j = 0; j < set->ItemGUID.size(); ++j)
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
        data << uint32(EquipmentSets.size());

        for (EquipmentSetStorage::iterator itr = EquipmentSets.begin(); itr != EquipmentSets.end(); ++itr)
        {
            EquipmentSet* set = itr->second;

            data << WoWGuid(uint64(set->SetGUID));
            data << uint32(set->SetID);
            data << std::string(set->SetName);
            data << std::string(set->IconName);

            for (uint32 i = 0; i < set->ItemGUID.size(); ++i)
            {
                data << WoWGuid(uint64(WoWGuid::createItemGuid(set->ItemGUID[i])));
            }
        }
    }
}
