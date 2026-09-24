/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WDC5File.hpp"

#include <algorithm>
#include <bit>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>
#include <type_traits>
#include <utility>

namespace WDB
{
    namespace
    {
        constexpr uint32_t WDC5Signature = 0x35434457; // WDC5

        template <typename T>
        bool readPod(std::vector<uint8_t> const& data, size_t& offset, T& value)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            if (offset + sizeof(T) > data.size())
                return false;

            std::memcpy(&value, data.data() + offset, sizeof(T));
            offset += sizeof(T);
            return true;
        }

        bool fail(std::string* error, std::string const& message)
        {
            if (error)
                *error = message;
            return false;
        }
    }


    bool WDC5File::loadGeneric(std::string const& filename, std::string* error)
    {
        std::ifstream stream(filename, std::ios::binary);
        if (!stream)
            return fail(error, "file does not exist: " + filename);

        uint32_t signature = 0;
        uint32_t version = 0;
        char schemaName[128]{};
        uint32_t recordCount = 0;
        uint32_t fieldCount = 0;
        uint32_t recordSize = 0;
        uint32_t stringTableSize = 0;
        uint32_t tableHash = 0;
        uint32_t layoutHash = 0;
        uint32_t minId = 0;
        uint32_t maxId = 0;
        uint32_t locale = 0;
        uint16_t flags = 0;
        int16_t indexField = -1;
        uint32_t totalFieldCount = 0;

        auto read = [&stream](auto& value) -> bool
        {
            return static_cast<bool>(stream.read(reinterpret_cast<char*>(&value), sizeof(value)));
        };

        if (!read(signature) || !read(version) || !stream.read(schemaName, sizeof(schemaName))
            || !read(recordCount) || !read(fieldCount) || !read(recordSize) || !read(stringTableSize)
            || !read(tableHash) || !read(layoutHash) || !read(minId) || !read(maxId) || !read(locale)
            || !read(flags) || !read(indexField) || !read(totalFieldCount))
        {
            return fail(error, "truncated WDC5 header in " + filename);
        }

        (void)recordCount;
        (void)recordSize;
        (void)stringTableSize;
        (void)tableHash;
        (void)minId;
        (void)maxId;
        (void)locale;
        (void)flags;

        if (signature != WDC5Signature)
            return fail(error, "invalid signature (expected WDC5) in " + filename);
        if (version != 5)
            return fail(error, "unsupported WDC version " + std::to_string(version) + " in " + filename);
        if (fieldCount != totalFieldCount)
            return fail(error, "generic WDC5 reader does not support split field counts in " + filename);

        WDC5TableSchema schema;
        schema.filename = nullptr;
        schema.layoutHash = layoutHash;
        schema.indexField = indexField;
        schema.fields.assign(fieldCount, WDC5FieldSchema{1});
        return load(filename, schema, error);
    }

    bool WDC5File::load(std::string const& filename, WDC5TableSchema const& schema, std::string* error)
    {
        m_header = {};
        m_schema = schema;
        m_layoutHash = 0;
        m_schemaName.clear();
        m_fileData.clear();
        m_sections.clear();
        m_columns.clear();
        m_palletValues.clear();
        m_palletArrayValues.clear();
        m_commonValues.clear();
        m_records.clear();
        m_parentIds.clear();
        m_recordIndexById.clear();

        std::ifstream stream(filename, std::ios::binary | std::ios::ate);
        if (!stream)
            return fail(error, "file does not exist: " + filename);

        std::streamsize const fileSize = stream.tellg();
        if (fileSize <= 0)
            return fail(error, "file is empty: " + filename);

        stream.seekg(0, std::ios::beg);
        m_fileData.resize(static_cast<size_t>(fileSize));
        if (!stream.read(reinterpret_cast<char*>(m_fileData.data()), fileSize))
            return fail(error, "unable to read file: " + filename);

        size_t offset = 0;
        if (!readPod(m_fileData, offset, m_header.signature) || !readPod(m_fileData, offset, m_header.version))
            return fail(error, "truncated WDC5 header in " + filename);

        if (offset + sizeof(m_header.schema) > m_fileData.size())
            return fail(error, "truncated WDC5 schema name in " + filename);
        std::memcpy(m_header.schema, m_fileData.data() + offset, sizeof(m_header.schema));
        offset += sizeof(m_header.schema);
        m_header.schema[sizeof(m_header.schema) - 1] = '\0';
        m_schemaName = m_header.schema;

        if (!readPod(m_fileData, offset, m_header.recordCount)
            || !readPod(m_fileData, offset, m_header.fieldCount)
            || !readPod(m_fileData, offset, m_header.recordSize)
            || !readPod(m_fileData, offset, m_header.stringTableSize)
            || !readPod(m_fileData, offset, m_header.tableHash)
            || !readPod(m_fileData, offset, m_header.layoutHash)
            || !readPod(m_fileData, offset, m_header.minId)
            || !readPod(m_fileData, offset, m_header.maxId)
            || !readPod(m_fileData, offset, m_header.locale)
            || !readPod(m_fileData, offset, m_header.flags)
            || !readPod(m_fileData, offset, m_header.indexField)
            || !readPod(m_fileData, offset, m_header.totalFieldCount)
            || !readPod(m_fileData, offset, m_header.packedDataOffset)
            || !readPod(m_fileData, offset, m_header.parentLookupCount)
            || !readPod(m_fileData, offset, m_header.columnMetaSize)
            || !readPod(m_fileData, offset, m_header.commonDataSize)
            || !readPod(m_fileData, offset, m_header.palletDataSize)
            || !readPod(m_fileData, offset, m_header.sectionCount))
        {
            return fail(error, "truncated WDC5 header fields in " + filename);
        }

        if (m_header.signature != WDC5Signature)
            return fail(error, "invalid signature (expected WDC5) in " + filename);
        if (m_header.version != 5)
            return fail(error, "unsupported WDC version " + std::to_string(m_header.version) + " in " + filename);
        if (m_header.flags & 0x1)
            return fail(error, "sparse WDC5 tables are not supported yet: " + filename);
        if (m_header.fieldCount != schema.fields.size() || m_header.totalFieldCount != schema.fields.size())
            return fail(error, "field count mismatch in " + filename);
        if (schema.layoutHash && m_header.layoutHash != schema.layoutHash)
        {
            std::ostringstream out;
            out << "layout hash mismatch in " << filename << ": expected 0x" << std::hex << schema.layoutHash
                << ", got 0x" << m_header.layoutHash;
            return fail(error, out.str());
        }

        m_layoutHash = m_header.layoutHash;

        m_sections.resize(m_header.sectionCount);
        for (SectionHeader& section : m_sections)
        {
            if (!readPod(m_fileData, offset, section.tactKeyLookup)
                || !readPod(m_fileData, offset, section.fileOffset)
                || !readPod(m_fileData, offset, section.recordCount)
                || !readPod(m_fileData, offset, section.stringTableSize)
                || !readPod(m_fileData, offset, section.catalogDataOffset)
                || !readPod(m_fileData, offset, section.idTableSize)
                || !readPod(m_fileData, offset, section.parentLookupDataSize)
                || !readPod(m_fileData, offset, section.catalogDataCount)
                || !readPod(m_fileData, offset, section.copyTableCount))
            {
                return fail(error, "truncated section headers in " + filename);
            }
        }

        // Field entries are currently not required by the regular WDC5 reader,
        // but they are part of the on-disk format and must be skipped.
        size_t const fieldEntriesSize = static_cast<size_t>(m_header.fieldCount) * 4U;
        if (offset + fieldEntriesSize > m_fileData.size())
            return fail(error, "truncated field entries in " + filename);
        offset += fieldEntriesSize;

        if (m_header.columnMetaSize != m_header.totalFieldCount * 24U)
            return fail(error, "unexpected WDC5 column metadata size in " + filename);

        m_columns.resize(m_header.totalFieldCount);
        for (ColumnMeta& column : m_columns)
        {
            uint32_t compression = 0;
            if (!readPod(m_fileData, offset, column.bitOffset)
                || !readPod(m_fileData, offset, column.bitSize)
                || !readPod(m_fileData, offset, column.additionalDataSize)
                || !readPod(m_fileData, offset, compression)
                || !readPod(m_fileData, offset, column.value1)
                || !readPod(m_fileData, offset, column.value2)
                || !readPod(m_fileData, offset, column.value3))
            {
                return fail(error, "truncated WDC5 column metadata in " + filename);
            }

            if (compression > static_cast<uint32_t>(WDC5CompressionType::SignedImmediate))
                return fail(error, "unknown WDC5 compression type in " + filename);
            column.compression = static_cast<WDC5CompressionType>(compression);
        }

        m_palletValues.resize(m_header.totalFieldCount);
        m_palletArrayValues.resize(m_header.totalFieldCount);
        m_commonValues.resize(m_header.totalFieldCount);

        for (uint32_t field = 0; field < m_header.totalFieldCount; ++field)
        {
            ColumnMeta const& column = m_columns[field];
            if (column.compression != WDC5CompressionType::Pallet)
                continue;

            if (column.additionalDataSize % sizeof(uint32_t))
                return fail(error, "unaligned WDC5 pallet data in " + filename);
            auto& values = m_palletValues[field];
            values.resize(column.additionalDataSize / sizeof(uint32_t));
            for (uint32_t& value : values)
                if (!readPod(m_fileData, offset, value))
                    return fail(error, "truncated WDC5 pallet data in " + filename);
        }

        for (uint32_t field = 0; field < m_header.totalFieldCount; ++field)
        {
            ColumnMeta const& column = m_columns[field];
            if (column.compression != WDC5CompressionType::PalletArray)
                continue;

            if (column.additionalDataSize % sizeof(uint32_t))
                return fail(error, "unaligned WDC5 pallet-array data in " + filename);
            auto& values = m_palletArrayValues[field];
            values.resize(column.additionalDataSize / sizeof(uint32_t));
            for (uint32_t& value : values)
                if (!readPod(m_fileData, offset, value))
                    return fail(error, "truncated WDC5 pallet-array data in " + filename);
        }

        for (uint32_t field = 0; field < m_header.totalFieldCount; ++field)
        {
            ColumnMeta const& column = m_columns[field];
            if (column.compression != WDC5CompressionType::CommonData)
                continue;

            if (column.additionalDataSize % 8U)
                return fail(error, "unaligned WDC5 common data in " + filename);
            auto& values = m_commonValues[field];
            for (uint32_t i = 0; i < column.additionalDataSize / 8U; ++i)
            {
                uint32_t id = 0;
                uint32_t value = 0;
                if (!readPod(m_fileData, offset, id) || !readPod(m_fileData, offset, value))
                    return fail(error, "truncated WDC5 common data in " + filename);
                values[id] = value;
            }
        }

        m_records.reserve(m_header.recordCount);
        m_parentIds.reserve(m_header.recordCount);
        m_skippedEncryptedRecords = 0;

        // WDC5 copy-table rows are logical records which reuse another record's
        // payload under a different ID. Keep them until all physical records are
        // indexed, then materialize lightweight RecordRef aliases.
        std::vector<std::pair<uint32_t, uint32_t>> copyRecords;

        for (SectionHeader const& section : m_sections)
        {
            // Retail WDC5 files can contain a mix of readable and encrypted sections.
            // The extractor leaves encrypted section metadata intact, but the records
            // themselves cannot be interpreted without the matching TACT key. Skip
            // those sections and keep all unencrypted rows usable. This is important
            // for Map.db2 in the Forever beta: map 0/1 and the normal login maps are
            // in the unencrypted section while a handful of later maps are encrypted.
            if (section.tactKeyLookup != 0)
            {
                m_skippedEncryptedRecords += section.recordCount;
                continue;
            }

            size_t const recordsEnd = static_cast<size_t>(section.fileOffset)
                + static_cast<size_t>(section.recordCount) * m_header.recordSize;
            size_t const stringsEnd = recordsEnd + section.stringTableSize;
            if (stringsEnd > m_fileData.size())
                return fail(error, "section record/string data is outside file in " + filename);

            // A section may carry an explicit ID table even when indexField is
            // non-negative. In that case the external IDs are authoritative.
            // TactKey.db2 in Forever 69913 is one such table: its only physical
            // field is the 16-byte key payload while the record IDs live in the
            // section ID table. Treating field 0 as the ID indexes the first four
            // key bytes instead and produces essentially random uint32 IDs.
            bool const hasExternalIdTable = section.idTableSize != 0;
            std::vector<uint32_t> externalIds;
            if (hasExternalIdTable)
            {
                if (section.idTableSize != section.recordCount * sizeof(uint32_t))
                    return fail(error, "external WDC5 id table has unexpected size in " + filename);

                size_t idOffset = stringsEnd;
                externalIds.resize(section.recordCount);
                for (uint32_t& id : externalIds)
                {
                    if (!readPod(m_fileData, idOffset, id))
                        return fail(error, "truncated external WDC5 id table in " + filename);
                }
            }
            else if (schema.indexField < 0)
            {
                return fail(error, "WDC5 schema expects external IDs but section has no ID table in " + filename);
            }

            if (section.copyTableCount != 0)
            {
                size_t copyOffset = stringsEnd + static_cast<size_t>(section.idTableSize);
                size_t const copyEnd = copyOffset + static_cast<size_t>(section.copyTableCount) * 8U;
                if (copyEnd > m_fileData.size())
                    return fail(error, "WDC5 copy table is outside file in " + filename);

                for (uint32_t i = 0; i < section.copyTableCount; ++i)
                {
                    uint32_t newId = 0;
                    uint32_t sourceId = 0;
                    if (!readPod(m_fileData, copyOffset, newId) || !readPod(m_fileData, copyOffset, sourceId))
                        return fail(error, "truncated WDC5 copy table in " + filename);
                    copyRecords.emplace_back(newId, sourceId);
                }
            }

            std::vector<uint32_t> parentIds(section.recordCount, 0);
            if (section.parentLookupDataSize != 0)
            {
                // WDC5 relationship/parent data follows the string table, optional
                // external ID table and optional copy table. Layout:
                //   uint32 entryCount, minId, maxId;
                //   repeated { uint32 parentId, uint32 sectionRecordIndex }.
                size_t parentOffset = stringsEnd
                    + static_cast<size_t>(section.idTableSize)
                    + static_cast<size_t>(section.copyTableCount) * 8U;
                size_t const parentEnd = parentOffset + section.parentLookupDataSize;
                if (parentEnd > m_fileData.size())
                    return fail(error, "WDC5 parent lookup data is outside file in " + filename);

                uint32_t entryCount = 0;
                uint32_t minId = 0;
                uint32_t maxId = 0;
                if (!readPod(m_fileData, parentOffset, entryCount)
                    || !readPod(m_fileData, parentOffset, minId)
                    || !readPod(m_fileData, parentOffset, maxId))
                {
                    return fail(error, "truncated WDC5 parent lookup header in " + filename);
                }

                (void)minId;
                (void)maxId;

                if (12U + static_cast<size_t>(entryCount) * 8U > section.parentLookupDataSize)
                    return fail(error, "invalid WDC5 parent lookup size in " + filename);

                for (uint32_t i = 0; i < entryCount; ++i)
                {
                    uint32_t parentId = 0;
                    uint32_t recordIndex = 0;
                    if (!readPod(m_fileData, parentOffset, parentId)
                        || !readPod(m_fileData, parentOffset, recordIndex))
                    {
                        return fail(error, "truncated WDC5 parent lookup entries in " + filename);
                    }

                    if (recordIndex >= section.recordCount)
                        return fail(error, "WDC5 parent lookup record index out of range in " + filename);
                    parentIds[recordIndex] = parentId;
                }
            }

            for (uint32_t record = 0; record < section.recordCount; ++record)
            {
                RecordRef ref;
                ref.offset = section.fileOffset + record * m_header.recordSize;
                if (hasExternalIdTable)
                {
                    ref.externalId = externalIds[record];
                    ref.hasExternalId = true;
                }
                m_records.push_back(ref);
                m_parentIds.push_back(parentIds[record]);
            }
        }

        if (m_records.size() + m_skippedEncryptedRecords != m_header.recordCount)
            return fail(error, "WDC5 record count mismatch in " + filename);

        m_recordIndexById.reserve(m_records.size() + copyRecords.size());
        for (uint32_t recordIndex = 0; recordIndex < m_records.size(); ++recordIndex)
            m_recordIndexById.emplace(getRecordId(recordIndex), recordIndex);

        // Copy records can theoretically reference another copy record. Resolve in
        // passes so chained aliases work as well, while rejecting broken sources.
        while (!copyRecords.empty())
        {
            bool madeProgress = false;
            for (auto itr = copyRecords.begin(); itr != copyRecords.end();)
            {
                auto const source = m_recordIndexById.find(itr->second);
                if (source == m_recordIndexById.end())
                {
                    ++itr;
                    continue;
                }

                uint32_t const sourceIndex = source->second;
                RecordRef copy = m_records[sourceIndex];
                copy.externalId = itr->first;
                copy.hasExternalId = true;

                uint32_t const copyIndex = static_cast<uint32_t>(m_records.size());
                m_records.push_back(copy);
                m_parentIds.push_back(m_parentIds[sourceIndex]);
                m_recordIndexById.emplace(itr->first, copyIndex);

                itr = copyRecords.erase(itr);
                madeProgress = true;
            }

            if (!madeProgress)
                return fail(error, "WDC5 copy table references unknown source record in " + filename);
        }

        return true;
    }

    bool WDC5File::findRecordIndex(uint32_t recordId, uint32_t& recordIndex) const noexcept
    {
        auto const itr = m_recordIndexById.find(recordId);
        if (itr == m_recordIndexById.end())
            return false;

        recordIndex = itr->second;
        return true;
    }

    std::span<uint8_t const> WDC5File::getRawRecord(uint32_t recordIndex) const noexcept
    {
        uint8_t const* data = getRecordData(recordIndex);
        if (!data || m_header.recordSize == 0)
            return {};

        return {data, m_header.recordSize};
    }

    uint8_t const* WDC5File::getRecordData(uint32_t recordIndex) const
    {
        if (recordIndex >= m_records.size())
            return nullptr;
        return m_fileData.data() + m_records[recordIndex].offset;
    }

    bool WDC5File::checkIndex(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex) const
    {
        return recordIndex < m_records.size()
            && field < m_columns.size()
            && field < m_schema.fields.size()
            && arrayIndex < m_schema.fields[field].arraySize;
    }

    uint64_t WDC5File::readPacked(uint8_t const* record, uint32_t bitOffset, uint32_t bitWidth) const
    {
        if (!record || bitWidth == 0 || bitWidth > 64)
            return 0;

        uint64_t value = 0;
        for (uint32_t bit = 0; bit < bitWidth; ++bit)
        {
            uint32_t const sourceBit = bitOffset + bit;
            uint32_t const byteIndex = sourceBit / 8U;
            uint32_t const bitIndex = sourceBit & 7U;
            if (byteIndex >= m_header.recordSize)
                break;
            value |= uint64_t((record[byteIndex] >> bitIndex) & 1U) << bit;
        }
        return value;
    }

    uint32_t WDC5File::getFieldByteOffset(uint32_t field) const
    {
        ColumnMeta const& column = m_columns[field];
        switch (column.compression)
        {
            case WDC5CompressionType::None:
                return column.bitOffset / 8U;
            case WDC5CompressionType::Immediate:
            case WDC5CompressionType::SignedImmediate:
            case WDC5CompressionType::Pallet:
            case WDC5CompressionType::PalletArray:
                return m_header.packedDataOffset + column.value1 / 8U;
            case WDC5CompressionType::CommonData:
                return 0;
        }
        return 0;
    }

    uint32_t WDC5File::getRecordId(uint32_t recordIndex) const
    {
        if (recordIndex >= m_records.size())
            return 0;

        // External IDs are used both by non-inline-ID tables and by copy-table
        // aliases. A copy of an inline-ID row must expose its new alias ID rather
        // than the source row's inline ID.
        if (m_records[recordIndex].hasExternalId)
            return m_records[recordIndex].externalId;

        if (m_schema.indexField < 0)
            return m_records[recordIndex].externalId;
        return getUInt32(recordIndex, static_cast<uint32_t>(m_schema.indexField));
    }

    template <typename T>
    T WDC5File::readValue(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex) const
    {
        if (!checkIndex(recordIndex, field, arrayIndex))
            return T{};

        uint8_t const* record = getRecordData(recordIndex);
        ColumnMeta const& column = m_columns[field];
        uint64_t raw = 0;

        switch (column.compression)
        {
            case WDC5CompressionType::None:
            {
                uint32_t const byteOffset = getFieldByteOffset(field) + static_cast<uint32_t>(sizeof(T)) * arrayIndex;
                if (byteOffset + sizeof(T) > m_header.recordSize)
                    return T{};
                std::memcpy(&raw, record + byteOffset, std::min(sizeof(T), sizeof(raw)));
                break;
            }
            case WDC5CompressionType::Immediate:
            case WDC5CompressionType::SignedImmediate:
            {
                if (arrayIndex != 0 || column.value2 == 0 || column.value2 > 64)
                    return T{};
                uint32_t const bitOffset = m_header.packedDataOffset * 8U + column.value1;
                raw = readPacked(record, bitOffset, column.value2);
                if (column.compression == WDC5CompressionType::SignedImmediate && column.value2 < 64)
                {
                    uint64_t const sign = uint64_t(1) << (column.value2 - 1U);
                    raw = (raw ^ sign) - sign;
                }
                break;
            }
            case WDC5CompressionType::CommonData:
            {
                uint32_t const id = getRecordId(recordIndex);
                auto const found = m_commonValues[field].find(id);
                raw = found != m_commonValues[field].end() ? found->second : column.value1;
                break;
            }
            case WDC5CompressionType::Pallet:
            case WDC5CompressionType::PalletArray:
            {
                uint32_t const bitOffset = m_header.packedDataOffset * 8U + column.value1;
                uint64_t const palletIndex = readPacked(record, bitOffset, column.value2);
                if (column.compression == WDC5CompressionType::Pallet)
                {
                    if (arrayIndex != 0 || palletIndex >= m_palletValues[field].size())
                        return T{};
                    raw = m_palletValues[field][static_cast<size_t>(palletIndex)];
                }
                else
                {
                    uint32_t const arraySize = column.value3;
                    size_t const valueIndex = static_cast<size_t>(palletIndex) * arraySize + arrayIndex;
                    if (!arraySize || valueIndex >= m_palletArrayValues[field].size())
                        return T{};
                    raw = m_palletArrayValues[field][valueIndex];
                }
                break;
            }
        }

        if constexpr (std::is_floating_point_v<T>)
        {
            static_assert(sizeof(T) == sizeof(uint32_t));
            uint32_t const bits = static_cast<uint32_t>(raw);
            return std::bit_cast<T>(bits);
        }
        else
        {
            T value{};
            std::memcpy(&value, &raw, std::min(sizeof(T), sizeof(raw)));
            return value;
        }
    }

    uint8_t WDC5File::getUInt8(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex) const
    {
        return readValue<uint8_t>(recordIndex, field, arrayIndex);
    }

    int8_t WDC5File::getInt8(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex) const
    {
        return readValue<int8_t>(recordIndex, field, arrayIndex);
    }

    uint16_t WDC5File::getUInt16(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex) const
    {
        return readValue<uint16_t>(recordIndex, field, arrayIndex);
    }

    int16_t WDC5File::getInt16(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex) const
    {
        return readValue<int16_t>(recordIndex, field, arrayIndex);
    }

    uint32_t WDC5File::getUInt32(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex) const
    {
        return readValue<uint32_t>(recordIndex, field, arrayIndex);
    }

    int32_t WDC5File::getInt32(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex) const
    {
        return readValue<int32_t>(recordIndex, field, arrayIndex);
    }

    float WDC5File::getFloat(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex) const
    {
        return readValue<float>(recordIndex, field, arrayIndex);
    }

    uint32_t WDC5File::getParentId(uint32_t recordIndex) const noexcept
    {
        return recordIndex < m_parentIds.size() ? m_parentIds[recordIndex] : 0U;
    }
}
