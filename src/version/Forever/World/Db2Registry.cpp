/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "version/Forever/World/Db2Registry.hpp"

#include "Logging/Logger.hpp"
#include "Server/World.h"
#include "version/Forever/BuildProfile.hpp"
#include "world/Storage/ForeverTactKeyBridge.hpp"
#include "Storage/WDB/WDC5File.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace AscEmu::Version::Forever::Db2
{
    namespace
    {
        constexpr uint32_t Wdc5Signature = 0x35434457U; // "WDC5"
        constexpr uint32_t Wdc5Version = 5U;
        constexpr uint32_t TactKeyTableHash = 0xDF2F53CFU;
        constexpr size_t TactKeySize = 16U;

        struct HeaderIdentity
        {
            uint32_t signature{0};
            uint32_t version{0};
            std::array<char, 128> schema{};
            uint32_t recordCount{0};
            uint32_t fieldCount{0};
            uint32_t recordSize{0};
            uint32_t stringTableSize{0};
            uint32_t tableHash{0};
            uint32_t layoutHash{0};
        };

        using TactKeyBytes = std::array<uint8_t, TactKeySize>;

        struct TableEntry
        {
            std::filesystem::path path;
            std::string name;
            std::unique_ptr<WDB::WDC5File> file;
            bool loadAttempted{false};
        };

        class Registry
        {
        public:
            bool get(uint32_t tableHash, uint32_t recordId, RecordView& out)
            {
                std::scoped_lock lock(m_mutex);
                ensureIndexed();

                if (tableHash == TactKeyTableHash)
                {
                    ensureTactKeySqlLoaded();
                    auto const sqlItr = m_tactKeySqlRecords.find(recordId);
                    if (sqlItr != m_tactKeySqlRecords.end())
                    {
                        out.data = std::span<uint8_t const>(sqlItr->second.data(), sqlItr->second.size());
                        out.tableName = "tact_key(SQL)";
                        out.layoutHash = 0xCBA490FCU;
                        return true;
                    }
                }

                auto const tableItr = m_tables.find(tableHash);
                if (tableItr == m_tables.end())
                    return false;

                TableEntry& table = tableItr->second;
                if (!table.loadAttempted)
                {
                    table.loadAttempted = true;
                    table.file = std::make_unique<WDB::WDC5File>();

                    std::string error;
                    if (!table.file->loadGeneric(table.path.string(), &error))
                    {
                        sLogger.failure("Failed to load {} DB2 table.", table.name);
                        table.file.reset();
                        return false;
                    }

                    sLogger.info("Loaded {} DB2 table.", table.name);
                }

                if (!table.file)
                    return false;

                // Preserve the resolved table identity even on an ID miss so the
                // caller can distinguish "table not found/load failed" from
                // "table loaded, requested record ID is not in our index".
                out.tableName = table.name;
                out.layoutHash = table.file->getLayoutHash();

                uint32_t recordIndex = 0;
                if (!table.file->findRecordIndex(recordId, recordIndex))
                {
                    if (m_missingIdDiagnostics.emplace(tableHash).second)
                    {
                        uint32_t indexedMin = UINT32_MAX;
                        uint32_t indexedMax = 0;
                        std::ostringstream samples;
                        constexpr uint32_t MaxSamples = 16;
                        uint32_t const recordCount = table.file->getRecordCount();
                        for (uint32_t index = 0; index < recordCount; ++index)
                        {
                            uint32_t const id = table.file->getRecordId(index);
                            indexedMin = std::min(indexedMin, id);
                            indexedMax = std::max(indexedMax, id);
                            if (index < MaxSamples)
                            {
                                if (index != 0)
                                    samples << ", ";
                                samples << id;
                            }
                        }

                        std::ostringstream candidateFields;
                        uint32_t candidateCount = 0;
                        uint32_t const scanRecords = std::min<uint32_t>(recordCount, 4096U);
                        for (uint32_t field = 0; field < table.file->getFieldCount(); ++field)
                        {
                            bool matched = false;
                            for (uint32_t index = 0; index < scanRecords; ++index)
                            {
                                if (table.file->getUInt32(index, field) == recordId)
                                {
                                    matched = true;
                                    break;
                                }
                            }

                            if (!matched)
                                continue;

                            if (candidateCount++ != 0)
                                candidateFields << ", ";
                            candidateFields << field;
                        }

                        sLogger.warning("Forever DB2 ID miss diagnostic: table='{}' hash=0x{:08X} requestedId={} layout=0x{:08X} " "records={} recordSize={} fieldCount={}/{} headerIndexField={} headerIdRange=[{},{}] " "indexedIdRange=[{},{}] sampleIds=[{}] requestedIdSeenInField(s)=[{}].", table.name, tableHash, recordId, table.file->getLayoutHash(), recordCount, table.file->getRecordSize(), table.file->getFieldCount(), table.file->getTotalFieldCount(), table.file->getIndexField(), table.file->getMinId(), table.file->getMaxId(), indexedMin == UINT32_MAX ? 0U : indexedMin, indexedMax, samples.str(), candidateFields.str());
                    }
                    return false;
                }

                std::span<uint8_t const> const data = table.file->getRawRecord(recordIndex);
                if (data.empty())
                    return false;

                out.data = data;
                return true;
            }

        private:

            void ensureTactKeySqlLoaded()
            {
                if (m_tactKeySqlLoaded)
                    return;

                m_tactKeySqlLoaded = true;

                // Trinity loads TactKey.db2 records from its hotfix `tact_key`
                // table into the same logical DB2 store. Keep the Forever DB2
                // query path generic and layer SQL records over the client DB2
                // here. For duplicate IDs the newest row not newer than the
                // active Forever build wins; VerifiedBuild=0 is the generic base.
                auto const records = AscEmu::World::Storage::loadForeverTactKeys(AscEmu::Version::Forever::Build);

                for (auto const& record : records)
                {
                    m_tactKeySqlRecords[record.id] = record.key;
                }

                sLogger.info("Loaded TactKey SQL data.");
            }

            static bool readIdentity(std::filesystem::path const& path, HeaderIdentity& identity)
            {
                std::ifstream file(path, std::ios::binary);
                if (!file)
                    return false;

                // Read only the fixed WDC5 prefix through layoutHash. Do not
                // instantiate/load every DB2 merely to build the hash index.
                file.read(reinterpret_cast<char*>(&identity.signature), sizeof(identity.signature));
                file.read(reinterpret_cast<char*>(&identity.version), sizeof(identity.version));
                file.read(identity.schema.data(), static_cast<std::streamsize>(identity.schema.size()));
                file.read(reinterpret_cast<char*>(&identity.recordCount), sizeof(identity.recordCount));
                file.read(reinterpret_cast<char*>(&identity.fieldCount), sizeof(identity.fieldCount));
                file.read(reinterpret_cast<char*>(&identity.recordSize), sizeof(identity.recordSize));
                file.read(reinterpret_cast<char*>(&identity.stringTableSize), sizeof(identity.stringTableSize));
                file.read(reinterpret_cast<char*>(&identity.tableHash), sizeof(identity.tableHash));
                file.read(reinterpret_cast<char*>(&identity.layoutHash), sizeof(identity.layoutHash));
                return file.good();
            }

            void ensureIndexed()
            {
                if (m_indexed)
                    return;
                m_indexed = true;

                std::filesystem::path const dbcPath = std::filesystem::path(::World::getInstance().settings.server.dataDir) / "dbc";
                std::error_code ec;
                if (!std::filesystem::is_directory(dbcPath, ec))
                {
                    sLogger.failure("Forever DB2: '{}' is not a readable DB2 directory.", dbcPath.string());
                    return;
                }

                uint32_t indexed = 0;
                for (std::filesystem::directory_iterator itr(dbcPath, ec), end; itr != end && !ec; itr.increment(ec))
                {
                    if (!itr->is_regular_file(ec))
                        continue;

                    std::filesystem::path const& path = itr->path();
                    if (path.extension() != ".db2" && path.extension() != ".DB2")
                        continue;

                    HeaderIdentity identity;
                    if (!readIdentity(path, identity) || identity.signature != Wdc5Signature || identity.version != Wdc5Version || identity.tableHash == 0)
                    {
                        continue;
                    }

                    TableEntry entry;
                    entry.path = path;
                    entry.name = path.filename().string();

                    auto [pos, inserted] = m_tables.emplace(identity.tableHash, std::move(entry));
                    if (inserted)
                    {
                        ++indexed;
                    }
                    else
                    {
                        sLogger.warning("Forever DB2: duplicate table hash 0x{:08X}: keeping '{}', ignoring '{}'.", identity.tableHash, pos->second.path.string(), path.string());
                    }
                }

                if (ec)
                    sLogger.failure("Forever DB2: directory scan of '{}' stopped: {}.", dbcPath.string(), ec.message());

                sLogger.info("Indexed {} Forever DB2 tables.", indexed);
            }

            std::mutex m_mutex;
            bool m_indexed{false};
            bool m_tactKeySqlLoaded{false};
            std::unordered_map<uint32_t, TableEntry> m_tables;
            std::unordered_map<uint32_t, TactKeyBytes> m_tactKeySqlRecords;
            std::unordered_set<uint32_t> m_missingIdDiagnostics;
        };

        Registry& registry()
        {
            static Registry instance;
            return instance;
        }
    }

    bool getRecord(uint32_t tableHash, uint32_t recordId, RecordView& out)
    {
        out = {};
        return registry().get(tableHash, recordId, out);
    }
}
