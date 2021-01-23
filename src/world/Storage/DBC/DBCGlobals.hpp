/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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
        LOCALE_enGB = 0,
        LOCALE_enUS = 1,
        LOCALE_deDE = 2,
        LOCALE_esES = 3,
        LOCALE_frFR = 4,
        LOCALE_koKR = 5,
        LOCALE_zhCN = 6,
        LOCALE_zhTW = 7,
        LOCALE_enCN = 8,
        LOCALE_enTW = 9,
        LOCALE_esMX = 10,
        LOCALE_ruRU = 11,
        LOCALE_ptBR = 12,
        LOCALE_ptPT = 13,
        LOCALE_itIT = 14,
    };

    struct LocaleNameStr
    {
        char const* name;
        LocaleConstant locale;
    };

    LocaleNameStr const fullLocaleNameList[] =
    {
           { "enGB", LOCALE_enGB },
           { "enUS", LOCALE_enUS },
           { "deDE", LOCALE_deDE },
           { "esES", LOCALE_esES },
           { "frFR", LOCALE_frFR },
           { "koKR", LOCALE_koKR },
           { "zhCN", LOCALE_zhCN },
           { "zhTW", LOCALE_zhTW },
           { "enCN", LOCALE_enCN },
           { "enTW", LOCALE_enTW },
           { "esMX", LOCALE_esMX },
           { "ruRU", LOCALE_ruRU },
           { "ptBR", LOCALE_ptBR },
           { "ptPT", LOCALE_ptPT },
           { "itIT", LOCALE_itIT },
    };

    namespace
    {
        const uint8_t C_TOTAL_LOCALES = 15;
        char const* C_LOCALE_NAMES[C_TOTAL_LOCALES] =
        {
            "enGB",
            "enUS",
            "deDE",
            "esES",
            "frFR",
            "koKR",
            "zhCN",
            "zhTW",
            "enCN",
            "enTW",
            "esMX",
            "ruRU",
            "ptBR",
            "ptPT",
            "itIT"
        };

        uint32_t g_dbc_file_count = 0;
    }

    typedef std::list<std::string> StoreProblemList;

    template <class T>
    void LoadDBC(uint32_t& /*available_dbc_locales*/, StoreProblemList& errors, DBC::DBCStorage<T>& storage, std::string const& dbc_path,
      std::string const& dbc_filename,std::string const* custom_format = NULL, std::string const* /*custom_index_name*/ = NULL)
    {
        ASSERT(DBC::DBCLoader::GetFormatRecordSize(storage.GetFormat()) == sizeof(T));

        std::string dbc_file_path = dbc_path + dbc_filename;

        // find first available locale
        for (auto locales : fullLocaleNameList)
        {
            if (fs::is_directory(dbc_path + locales.name + "/"))
            {
                dbc_file_path = dbc_path + locales.name + "/" + dbc_filename;
                break;
            }
        }

        ++g_dbc_file_count;
        DBC::SQL::SqlDbc* sql = NULL;
        if (custom_format)
        {
            assert(false && "SqlDbc not yet implemented");
        }

        if (!storage.Load(dbc_file_path.c_str(), sql))
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
