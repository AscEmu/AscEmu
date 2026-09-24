/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WDBFormat.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace WDB
{
    enum class WDC5CompressionType : uint32_t
    {
        None = 0,
        Immediate = 1,
        CommonData = 2,
        Pallet = 3,
        PalletArray = 4,
        SignedImmediate = 5
    };

    class WDC5File
    {
    public:
        bool load(std::string const& filename, WDC5TableSchema const& schema, std::string* error = nullptr);
        bool loadGeneric(std::string const& filename, std::string* error = nullptr);

        [[nodiscard]] uint32_t getRecordCount() const noexcept { return static_cast<uint32_t>(m_records.size()); }
        [[nodiscard]] uint32_t getTableHash() const noexcept { return m_header.tableHash; }
        [[nodiscard]] uint32_t getRecordSize() const noexcept { return m_header.recordSize; }
        [[nodiscard]] uint32_t getFieldCount() const noexcept { return m_header.fieldCount; }
        [[nodiscard]] uint32_t getTotalFieldCount() const noexcept { return m_header.totalFieldCount; }
        [[nodiscard]] int16_t getIndexField() const noexcept { return m_header.indexField; }
        [[nodiscard]] uint32_t getMinId() const noexcept { return m_header.minId; }
        [[nodiscard]] uint32_t getMaxId() const noexcept { return m_header.maxId; }
        [[nodiscard]] uint32_t getLayoutHash() const noexcept { return m_layoutHash; }
        [[nodiscard]] std::string const& getSchemaName() const noexcept { return m_schemaName; }

        [[nodiscard]] uint32_t getRecordId(uint32_t recordIndex) const;
        [[nodiscard]] uint8_t getUInt8(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex = 0) const;
        [[nodiscard]] int8_t getInt8(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex = 0) const;
        [[nodiscard]] uint16_t getUInt16(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex = 0) const;
        [[nodiscard]] int16_t getInt16(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex = 0) const;
        [[nodiscard]] uint32_t getUInt32(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex = 0) const;
        [[nodiscard]] int32_t getInt32(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex = 0) const;
        [[nodiscard]] float getFloat(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex = 0) const;
        [[nodiscard]] uint32_t getParentId(uint32_t recordIndex) const noexcept;
        [[nodiscard]] uint32_t getSkippedEncryptedRecordCount() const noexcept { return m_skippedEncryptedRecords; }
        [[nodiscard]] bool findRecordIndex(uint32_t recordId, uint32_t& recordIndex) const noexcept;
        [[nodiscard]] std::span<uint8_t const> getRawRecord(uint32_t recordIndex) const noexcept;

    private:
        struct Header
        {
            uint32_t signature{0};
            uint32_t version{0};
            char schema[128]{};
            uint32_t recordCount{0};
            uint32_t fieldCount{0};
            uint32_t recordSize{0};
            uint32_t stringTableSize{0};
            uint32_t tableHash{0};
            uint32_t layoutHash{0};
            uint32_t minId{0};
            uint32_t maxId{0};
            uint32_t locale{0};
            uint16_t flags{0};
            int16_t indexField{-1};
            uint32_t totalFieldCount{0};
            uint32_t packedDataOffset{0};
            uint32_t parentLookupCount{0};
            uint32_t columnMetaSize{0};
            uint32_t commonDataSize{0};
            uint32_t palletDataSize{0};
            uint32_t sectionCount{0};
        };

        struct SectionHeader
        {
            uint64_t tactKeyLookup{0};
            uint32_t fileOffset{0};
            uint32_t recordCount{0};
            uint32_t stringTableSize{0};
            uint32_t catalogDataOffset{0};
            uint32_t idTableSize{0};
            uint32_t parentLookupDataSize{0};
            uint32_t catalogDataCount{0};
            uint32_t copyTableCount{0};
        };

        struct ColumnMeta
        {
            uint16_t bitOffset{0};
            uint16_t bitSize{0};
            uint32_t additionalDataSize{0};
            WDC5CompressionType compression{WDC5CompressionType::None};
            uint32_t value1{0};
            uint32_t value2{0};
            uint32_t value3{0};
        };

        struct RecordRef
        {
            uint32_t offset{0};
            uint32_t externalId{0};
            bool hasExternalId{false};
        };

        template <typename T>
        [[nodiscard]] T readValue(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex) const;

        [[nodiscard]] uint64_t readPacked(uint8_t const* record, uint32_t bitOffset, uint32_t bitWidth) const;
        [[nodiscard]] uint32_t getFieldByteOffset(uint32_t field) const;
        [[nodiscard]] uint8_t const* getRecordData(uint32_t recordIndex) const;
        [[nodiscard]] bool checkIndex(uint32_t recordIndex, uint32_t field, uint32_t arrayIndex) const;

        Header m_header{};
        WDC5TableSchema m_schema{};
        uint32_t m_layoutHash{0};
        std::string m_schemaName;
        std::vector<uint8_t> m_fileData;
        std::vector<SectionHeader> m_sections;
        std::vector<ColumnMeta> m_columns;
        std::vector<std::vector<uint32_t>> m_palletValues;
        std::vector<std::vector<uint32_t>> m_palletArrayValues;
        std::vector<std::unordered_map<uint32_t, uint32_t>> m_commonValues;
        std::vector<RecordRef> m_records;
        std::vector<uint32_t> m_parentIds;
        std::unordered_map<uint32_t, uint32_t> m_recordIndexById;
        uint32_t m_skippedEncryptedRecords{0};
    };
}
