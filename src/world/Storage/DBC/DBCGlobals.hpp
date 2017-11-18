/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "DBCStorage.hpp"
#include "DBCLoader.hpp"
#include "DBCStructures.hpp"
#include "Log.hpp"
#include <iostream>

namespace DBC
{
    enum LocaleConstant
    {
        LOCALE_enUS = 0,
        LOCALE_koKR = 1,
        LOCALE_frFR = 2,
        LOCALE_deDE = 3,
        LOCALE_zhCN = 4,
        LOCALE_zhTW = 5,
        LOCALE_esES = 6,
        LOCALE_esMX = 7,
        LOCALE_ruRU = 8
    };

    struct LocaleNameStr
    {
        char const* name;
        LocaleConstant locale;
    };

    LocaleNameStr const fullLocaleNameList[] =
    {
        { "enUS", LOCALE_enUS },
        { "enGB", LOCALE_enUS },
        { "koKR", LOCALE_koKR },
        { "frFR", LOCALE_frFR },
        { "deDE", LOCALE_deDE },
        { "zhCN", LOCALE_zhCN },
        { "zhTW", LOCALE_zhTW },
        { "esES", LOCALE_esES },
        { "esMX", LOCALE_esMX },
        { "ruRU", LOCALE_ruRU },
        { nullptr, LOCALE_enUS }
    };

    namespace
    {
        const uint8 C_TOTAL_LOCALES = 9;
        char const* C_LOCALE_NAMES[C_TOTAL_LOCALES] =
        {
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
    void LoadDBC(uint32& available_dbc_locales, StoreProblemList& errors, DBC::DBCStorage<T>& storage, std::string const& dbc_path,
      std::string const& dbc_filename,std::string const* custom_format = NULL, std::string const* /*custom_index_name*/ = NULL)
    {
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
                    // Mark as not available to speed up next checks
                    available_dbc_locales &= ~(1 << i);
                }
            }
        }
        else
        {
            // We failed to load the dbc, so work out if it's incompatible or just doesn't exist
            if (auto file = fopen(dbc_file_path.c_str(), "rb"))
            {
                std::ostringstream stream;
                stream << dbc_file_path << " exists, and has " << storage.GetFieldCount() << " field(s) (expected " << strlen(storage.GetFormat())
                    << "). Extracted file might be from wrong client version or a database-update has been forgotten.";
                std::string buf = stream.str();
                errors.push_back(buf);

                std::cout << stream.str() << std::endl;

                fclose(file);
            }
            else
            {
                std::cout << dbc_file_path << " does not exist" << std::endl;

                errors.push_back(dbc_file_path);
            }
        }

        delete sql;
    }
}
