/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
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
 */

#ifndef _DBC_GLOBALS_H
#define _DBC_GLOBALS_H

#include "DBCStorage.hpp"
#include "DBCLoader.hpp"
#include "DBCStructures.hpp"
#include "Log.h"

namespace DBC
{
    namespace
    {
        const uint8 C_TOTAL_LOCALES = 9;
        char const* C_LOCALE_NAMES[C_TOTAL_LOCALES] = {
            "enUS",
            "koKR",
            "frFR",
            "deDE",
            "zhCN",
            "zhTW",
            "esES",
            "esMX",
            "ruRU"
        };

        uint32 g_dbc_file_count = 0;
    }

    typedef std::list<std::string> StoreProblemList;

    static bool LoadDBC_assert_print(uint32 format_string_size, uint32 record_size, const std::string& dbc_filename)
    {
        LOG_ERROR("misc", "Size of '%s' set by format string (%u) is not equal to the size of the C++ structure (%u)", dbc_filename.c_str(), format_string_size, record_size);

        return false;
    }

    template <class T>
    inline void LoadDBC(uint32& available_dbc_locales, StoreProblemList& errors, DBC::DBCStorage<T>& storage, std::string const& dbc_path,
      std::string const& dbc_filename,std::string const* custom_format = NULL, std::string const* custom_index_name = NULL)
    {
        auto tsize = sizeof(T);
        ASSERT(DBC::DBCLoader::GetFormatRecordSize(storage.GetFormat()) == sizeof(T) || LoadDBC_assert_print(DBC::DBCLoader::GetFormatRecordSize(storage.GetFormat()), sizeof(T), dbc_filename));

        ++g_dbc_file_count;
        std::string dbc_file_path = dbc_path + dbc_filename;
        DBC::SQL::SqlDbc* sql = NULL;
        if (custom_format)
        {
            assert(false && "SqlDbc not yet implemented");
        }

        if (storage.Load(dbc_file_path.c_str(), sql))
        {
            for (uint8 i = 0; i < DBC::C_TOTAL_LOCALES; ++i)
            {
                if (!(available_dbc_locales & (1 << i)))
                {
                    continue;
                }

                std::string localised_name(dbc_path);
                localised_name.append(DBC::C_LOCALE_NAMES[i]);
                localised_name.push_back('/');
                localised_name.append(dbc_filename);

                if (!storage.LoadStringsFrom(localised_name.c_str()))
                {
                    /* Mark as not available to speed up next checks */
                    available_dbc_locales &= ~(1 << i);
                }
            }
        }
        else
        {
            /* We failed to load the dbc, so work out if it's incompatible or just doesn't exist */
            if (auto file = fopen(dbc_file_path.c_str(), "rb"))
            {
                std::ostringstream stream;
                stream << dbc_file_path << " exists, and has " << storage.GetFieldCount() << " field(s) (expected " << strlen(storage.GetFormat())
                    << "). Extracted file might be from wrong client version or a database-update has been forgotten.";
                std::string buf = stream.str();
                errors.push_back(buf);
                fclose(file);
            }
            else
            {
                errors.push_back(dbc_file_path);
            }
        }

        delete sql;
    }
}

#endif // _DBC_GLOBALS_H