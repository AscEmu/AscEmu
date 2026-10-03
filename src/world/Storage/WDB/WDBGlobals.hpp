/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/ClientProtocol.hpp"
#include "WDBContainer.hpp"
#include "WDBLoader.hpp"
#include "WDBStructures.hpp"
#include "WDBTraits.hpp"
#include "Logging/Logger.hpp"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>

namespace WDB
{
    template <typename T>
    class WDBStore : public std::unordered_map<uint32_t, T>
    {
    public:
        WDBStore() noexcept = default;

        [[nodiscard]] T const* lookupEntry(uint32_t id) const noexcept
        {
            auto const it = this->find(id);
            return (it != this->end()) ? &it->second : nullptr;
        }

        [[nodiscard]] std::size_t getNumRows() const noexcept
        {
            return this->size();
        }
    };

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
        LOCALE_itIT = 13,
        LOCALE_ptPT = 14,
        LOCALE_none = 15
    };

    struct LocaleNameStr
    {
        char const* name;
        LocaleConstant locale;
    };

    LocaleNameStr const fullLocaleNameList[] =
    {
        {"enGB", LOCALE_enGB},
        {"enUS", LOCALE_enUS},
        {"deDE", LOCALE_deDE},
        {"esES", LOCALE_esES},
        {"frFR", LOCALE_frFR},
        {"koKR", LOCALE_koKR},
        {"zhCN", LOCALE_zhCN},
        {"zhTW", LOCALE_zhTW},
        {"enCN", LOCALE_enCN},
        {"enTW", LOCALE_enTW},
        {"esMX", LOCALE_esMX},
        {"ruRU", LOCALE_ruRU},
        {"ptBR", LOCALE_ptBR},
        {"itIT", LOCALE_itIT},
        {"ptPT", LOCALE_ptPT},
        {"none", LOCALE_none}
    };

    namespace
    {
        const uint8_t C_TOTAL_LOCALES = 16;
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
            "itIT"
            "ptPT",
            "none"
        };

        uint32_t g_dbc_file_count = 0;
    }

    typedef std::list<std::string> StoreProblemList;

    template <class T>
    void loadWDBFile(uint32_t& /*available_dbc_locales*/, StoreProblemList& _errors, WDB::WDBContainer<T>& _storage, std::string const& _dbcPath,
                     std::string const& _dbcFilename, std::string const* /*_customFormat*/ = nullptr, std::string const* /*custom_index_name*/ = nullptr)
    {
        if (WDB::WDBLoader::hasFormat(_dbcFilename))
        {
            std::string_view format = WDB::WDBLoader::getFormat(_dbcFilename);
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

    template <typename RuntimeEntry, typename MapperF>
    void loadUnifiedWDBStore(StoreProblemList& errors,
                             WDBStore<RuntimeEntry>& storage,
                             const std::string& dbcPath,
                             MapperF&& mapFields)
    {
        using Traits = DbcTraits<RuntimeEntry>;

        // files, layouts and formats follow the expansion of the game data, not of the client
        const WoW::Expansion activeExpansion = WoW::getServerDataExpansion();
        auto const expansionId = static_cast<uint32_t>(activeExpansion);
        auto const expansionName = WoW::getExpansionName(activeExpansion);

        const std::string_view filenameView = Traits::getFilename(activeExpansion);
        const std::string filename(filenameView);

        namespace DbcFs = std::filesystem;
        DbcFs::path baseDbcDir(dbcPath);
        DbcFs::path dbcFilePath = baseDbcDir / filenameView;

        if (!DbcFs::exists(dbcFilePath))
        {
            for (auto const& locale : fullLocaleNameList)
            {
                DbcFs::path localeDir = baseDbcDir / locale.name;
                if (DbcFs::is_directory(localeDir))
                {
                    DbcFs::path fileLocale = localeDir / filenameView;
                    if (DbcFs::exists(fileLocale))
                    {
                        dbcFilePath = fileLocale;
                        break;
                    }
                }
            }
        }

        auto loadRows = [&]<typename T0>(T0/*identity*/)
        {
            using RawT = T0::type;
            if constexpr (!std::is_same_v<RawT, UnsupportedVersion>)
            {
                WDBContainer<RawT> rawStore;

                std::string const formatKey = DbcFs::path(filenameView).replace_extension(".dbc").string();

                // Check if format string exists
                if (!WDBLoader::hasFormat(formatKey)) {
                    std::ostringstream stream;
                    stream << "WDBLoader:: no format found for " << formatKey
                        << " on expansion " << expansionName << " (ID: " << expansionId << ")\n";
                    errors.push_back(stream.str());
                    std::cout << stream.str() << "\n";
                    return;
                }

                std::string_view format = WDBLoader::getFormat(formatKey);
                auto writable = std::make_unique<char[]>(format.size() + 1);
                auto [_, out] = std::ranges::copy(format, writable.get());
                *out = '\0';
                rawStore.setFormat(std::move(writable));

                // Check format record size vs struct size
                size_t const expectedSize = WDBLoader::getFormatRecordSize(rawStore.getFormat());
                size_t const actualSize = sizeof(RawT);

                if (expectedSize != actualSize)
                {
                    std::ostringstream stream;
                    stream << "WDBLoader:: wrong format size for " << filename
                        << " on expansion " << expansionName << " (ID: " << expansionId << ")"
                        << " - Expected: " << expectedSize << " bytes, Struct: " << actualSize << " bytes\n";
                    errors.push_back(stream.str());
                    std::cout << stream.str() << "\n";
                    return;
                }

                // Increment global file counter
                ++g_dbc_file_count;

                const std::string filePathString = dbcFilePath.string();

                // Load DBC file and validate
                if (!rawStore.load(filePathString.c_str()))
                {
                    if (DbcFs::exists(dbcFilePath))
                    {
                        std::ostringstream stream;
                        stream << dbcFilePath << " exists, and has " << rawStore.getFieldCount()
                            << " field(s) (expected " << format.size()
                            << "). Extracted file might be from wrong client version or a database-update has been forgotten.\n";
                        errors.push_back(stream.str());
                        std::cout << stream.str() << "\n";
                    }
                    else
                    {
                        std::cout << filePathString << " does not exist\n";
                        errors.push_back(filePathString);
                    }
                    return;
                }

                uint32_t const numRows = rawStore.getNumRows();
                storage.reserve(storage.size() + numRows);

                for (uint32_t i = 0; i < numRows; ++i)
                {
                    if (auto const* raw = rawStore.lookupEntry(i))
                    {
                        RuntimeEntry entry;
                        mapFields(*raw, entry);
                        if constexpr (requires { raw->id; })
                        {
                            storage[raw->id] = std::move(entry);
                        }
                        else
                        {
                            storage[i] = std::move(entry);
                        }
                    }
                }
            }
            else
            {
                sLogger.debug("WDBStore: File {} is not supported on expansion {} (ID: {}).",
                              filename, expansionName, expansionId);
            }
        };

        switch (activeExpansion)
        {
            case WoW::Expansion::_Classic: loadRows(std::type_identity<typename Traits::classic>{}); break;
            case WoW::Expansion::_TBC: loadRows(std::type_identity<typename Traits::tbc>{}); break;
            case WoW::Expansion::_WotLK: loadRows(std::type_identity<typename Traits::wotlk>{}); break;
            case WoW::Expansion::_Cata: loadRows(std::type_identity<typename Traits::cata>{}); break;
            case WoW::Expansion::_Mop: loadRows(std::type_identity<typename Traits::mop>{}); break;
            case WoW::Expansion::_WoD: loadRows(std::type_identity<typename Traits::wod>{}); break;
            case WoW::Expansion::_Legion: loadRows(std::type_identity<typename Traits::legion>{}); break;
            default:
                errors.push_back("WDBStore: Attempted to load DBC for an unknown or unsupported expansion.");
                break;
        }
    }
}
