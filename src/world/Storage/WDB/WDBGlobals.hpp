/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WDBContainer.hpp"
#include "WDBLoader.hpp"
#include <cstring>
#include <filesystem>
#include <iostream>

#include "AEVersion.hpp"

namespace WDB
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
    void loadWDBFile(uint32_t& /*available_dbc_locales*/, StoreProblemList& _errors, WDB::WDBContainer<T>& _storage, std::string const& _dbcPath,
      std::string const& _dbcFilename,std::string const* _customFormat = nullptr, std::string const* /*custom_index_name*/ = nullptr)
    {
        if (WDB::WDBLoader::hasFormat(_dbcFilename))
        {
            std::string format = WDB::WDBLoader::getFormat(_dbcFilename);
            auto writable = std::make_unique<char[]>(format.size() + 1);
            std::copy(format.begin(), format.end(), writable.get());
            writable[format.size()] = '\0'; // don't forget the terminating 0


            _storage.setFormat(std::move(writable));
        }

        if (WDB::WDBLoader::getFormatRecordSize(_storage.getFormat()) == NULL)
        {
            std::ostringstream stream;
            stream << "DBCLoader:: no format found for " << _dbcFilename << " and version:  " << VERSION_STRING << "\n";
            std::string buf = stream.str();
            _errors.push_back(buf);

            std::cout << stream.str() << "\n";
            return;
        }

        if (WDB::WDBLoader::getFormatRecordSize(_storage.getFormat()) != sizeof(T))
        {
            std::ostringstream stream;
            stream << "WDBLoader:: wrong format for " << _dbcFilename << " and version:  " << VERSION_STRING << "\n";
            std::string buf = stream.str();
            _errors.push_back(buf);

            std::cout << stream.str() << "\n";
            return;
        }

        std::string dbc_file_path = _dbcPath + _dbcFilename;

        // find first available locale
        for (auto locales : fullLocaleNameList)
        {
            if (std::filesystem::is_directory(_dbcPath + locales.name + "/"))
            {
                dbc_file_path = _dbcPath + locales.name + "/" + _dbcFilename;
                break;
            }
        }

        ++g_dbc_file_count;

        if (!_storage.load(dbc_file_path.c_str()))
        {
            // We failed to load the dbc, so work out if it's incompatible or just doesn't exist
            if (auto file = fopen(dbc_file_path.c_str(), "rb"))
            {
                std::ostringstream stream;
                stream << dbc_file_path << " exists, and has " << _storage.getFieldCount() << " field(s) (expected " << strlen(_storage.getFormat())
                    << "). Extracted file might be from wrong client version or a database-update has been forgotten.";
                std::string buf = stream.str();
                _errors.push_back(buf);

                std::cout << stream.str() << "\n";

                fclose(file);
            }
            else
            {
                std::cout << dbc_file_path << " does not exist" << "\n";

                _errors.push_back(dbc_file_path);
            }
        }
    }
}
