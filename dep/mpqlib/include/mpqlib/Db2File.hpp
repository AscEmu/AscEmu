/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Reader for the client database tables of the CASC clients: the WDBC tables
// 6.x still ships as .dbc, and the WDC1 (7.3), WDC3 (8.1 to 9.x), WDC4 (10.1)
// and WDC5 (10.2 and later) .db2 layouts. The caller hands over the complete
// file (CascStorage reads it) and addresses columns by their index in the
// table's field list - the files carry sizes and packing of every column but
// no names, so which index means what is a per-client-version fact the caller
// owns (see map_extractor's MapDb2Layouts.hpp).
//
// Built from the documented on-disk layout (wowdev.wiki/DB2): a header per
// format, then the field list, the column packing descriptions with their
// pallet and common-value blocks, and the record sections. Packed columns are
// decoded on access; the record and string blocks of all sections are kept in
// one buffer in the order the format defines relative string offsets against
// (every section's records first, then every section's strings).
//
// Not covered: the offset-map ("sparse") tables, whose records can only be
// walked with the column types known, and WDB2/WDB5/WDB6 (6.x .db2 files and
// the early 7.x builds) - no table the tools read uses those.

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mpqlib
{
    class Db2File
    {
    public:
        enum class Format : uint8_t
        {
            Wdbc,
            Wdc1,
            Wdc3,
            Wdc4,
            Wdc5
        };

        struct Copy
        {
            uint32_t newId;
            uint32_t sourceId;
        };

        class Record
        {
        public:
            uint32_t id() const;
            uint32_t getUInt32(uint32_t field, uint32_t arrayIndex = 0) const;
            int32_t getInt32(uint32_t field, uint32_t arrayIndex = 0) const;
            uint64_t getUInt64(uint32_t field, uint32_t arrayIndex = 0) const;
            float getFloat(uint32_t field, uint32_t arrayIndex = 0) const;
            std::string_view getString(uint32_t field, uint32_t arrayIndex = 0) const;

        private:
            friend class Db2File;
            Record(Db2File const& file, uint32_t index) noexcept : m_file(file), m_index(index) {}

            Db2File const& m_file;
            uint32_t m_index;
        };

        // Parses the whole file. On failure the returned optional is empty and
        // error names the reason.
        static std::optional<Db2File> load(std::vector<std::byte> data, std::string& error);

        Db2File(Db2File const&) = delete;
        Db2File& operator=(Db2File const&) = delete;
        Db2File(Db2File&&) noexcept;
        Db2File& operator=(Db2File&&) noexcept;
        ~Db2File();

        Format format() const noexcept { return m_format; }
        uint32_t tableHash() const noexcept { return m_tableHash; }
        uint32_t layoutHash() const noexcept { return m_layoutHash; }
        // Columns of the table as the field list counts them (an array column is one field).
        uint32_t fieldCount() const noexcept { return m_totalFieldCount; }

        // Records in file order, including the ones of sections this reader
        // could not decrypt - those answer false from isReadable().
        uint32_t recordCount() const noexcept { return m_recordCount; }
        bool isReadable(uint32_t index) const noexcept;
        Record record(uint32_t index) const noexcept { return Record(*this, index); }

        // Rows that duplicate another row under a new id.
        std::span<const Copy> copies() const noexcept { return m_copies; }

    private:
        enum class Compression : uint32_t
        {
            None = 0,
            Immediate = 1,
            CommonData = 2,
            Pallet = 3,
            PalletArray = 4,
            SignedImmediate = 5
        };

        struct Column
        {
            Compression compression = Compression::None;
            uint16_t bitOffset = 0;         // of the plain column inside the record
            uint16_t bitSize = 0;
            uint32_t elementSize = 4;       // bytes of one plain element, from the field list
            uint32_t packedBitOffset = 0;   // of a packed column inside the packed part
            uint32_t packedBitWidth = 0;
            uint32_t arraySize = 1;         // pallet arrays
            uint32_t commonDefault = 0;
            std::vector<uint32_t> pallet;
            std::unordered_map<uint32_t, uint32_t> commonValues;
        };

        struct Section
        {
            uint64_t tactKey = 0;
            uint32_t fileOffset = 0;
            uint32_t recordCount = 0;
            uint32_t stringTableSize = 0;
            uint32_t offsetRecordsEnd = 0;
            uint32_t idListSize = 0;
            uint32_t relationshipDataSize = 0;
            uint32_t offsetMapIdCount = 0;
            uint32_t copyTableCount = 0;
            uint32_t firstRecord = 0;       // index of its first record in the combined block
            bool readable = true;
        };

        Db2File() = default;

        bool loadWdbc(std::span<const std::byte> file, std::string& error);
        bool loadWdc1(std::span<const std::byte> file, std::string& error);
        bool loadWdc3(std::span<const std::byte> file, std::string& error);
        bool readColumns(std::span<const std::byte> file, size_t& cursor, uint32_t columnMetaSize, std::string& error);
        bool readFieldList(std::span<const std::byte> file, size_t& cursor, std::string& error);

        uint8_t const* recordData(uint32_t index) const noexcept;
        uint64_t rawValue(uint32_t index, uint32_t field, uint32_t arrayIndex) const;
        uint64_t packedValue(uint8_t const* record, uint32_t bitOffset, uint32_t bitWidth) const noexcept;
        uint32_t recordId(uint32_t index) const;

        Format m_format = Format::Wdbc;
        uint32_t m_tableHash = 0;
        uint32_t m_layoutHash = 0;
        uint32_t m_recordCount = 0;
        uint32_t m_recordSize = 0;
        uint32_t m_fieldCount = 0;
        uint32_t m_totalFieldCount = 0;
        uint32_t m_packedDataOffset = 0;
        uint16_t m_flags = 0;
        int16_t m_idField = -1;
        bool m_relativeStrings = false;

        // all record blocks, then all string blocks, then 8 bytes of padding for the packed reads
        std::vector<uint8_t> m_data;
        size_t m_stringBlockOffset = 0;
        std::vector<std::pair<int16_t, uint16_t>> m_fieldList;  // unused bits, byte offset
        std::vector<Column> m_columns;
        std::vector<Section> m_sections;
        std::vector<uint32_t> m_ids;                            // non-inline ids, one per record
        std::vector<Copy> m_copies;
    };
}
