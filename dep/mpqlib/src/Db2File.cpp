/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/Db2File.hpp"

#include <algorithm>
#include <cstring>

namespace mpqlib
{
    namespace
    {
        constexpr uint32_t kMagicWdbc = 0x43424457;     // 'WDBC'
        constexpr uint32_t kMagicWdc1 = 0x31434457;     // 'WDC1'
        constexpr uint32_t kMagicWdc3 = 0x33434457;     // 'WDC3'
        constexpr uint32_t kMagicWdc4 = 0x34434457;     // 'WDC4'
        constexpr uint32_t kMagicWdc5 = 0x35434457;     // 'WDC5'

        constexpr uint16_t kFlagSparse = 0x01;
        constexpr uint16_t kFlagNonInlineIds = 0x04;

        constexpr size_t kColumnMetaSize = 24;
        constexpr size_t kPadding = 8;

        // Sequential little-endian reads with a bounds check, the header walk
        // of every format is just a list of these.
        class Cursor
        {
        public:
            Cursor(std::span<const std::byte> data, size_t offset = 0) noexcept : m_data(data), m_offset(offset) {}

            size_t offset() const noexcept { return m_offset; }
            void seek(size_t offset) noexcept { m_offset = offset; }
            bool has(size_t count) const noexcept { return m_offset <= m_data.size() && count <= m_data.size() - m_offset; }

            template<typename T>
            bool read(T& value) noexcept
            {
                if (!has(sizeof(T)))
                    return false;
                std::memcpy(&value, m_data.data() + m_offset, sizeof(T));
                m_offset += sizeof(T);
                return true;
            }

            bool readBytes(void* dest, size_t count) noexcept
            {
                if (!has(count))
                    return false;
                std::memcpy(dest, m_data.data() + m_offset, count);
                m_offset += count;
                return true;
            }

            bool skip(size_t count) noexcept
            {
                if (!has(count))
                    return false;
                m_offset += count;
                return true;
            }

            std::byte const* current() const noexcept { return m_data.data() + m_offset; }

        private:
            std::span<const std::byte> m_data;
            size_t m_offset;
        };

        template<typename T>
        T readAt(std::span<const std::byte> data, size_t offset) noexcept
        {
            T value{};
            std::memcpy(&value, data.data() + offset, sizeof(T));
            return value;
        }
    }

    Db2File::Db2File(Db2File&&) noexcept = default;
    Db2File& Db2File::operator=(Db2File&&) noexcept = default;
    Db2File::~Db2File() = default;

    std::optional<Db2File> Db2File::load(std::vector<std::byte> data, std::string& error)
    {
        if (data.size() < 4)
        {
            error = "file too small";
            return std::nullopt;
        }

        Db2File file;
        uint32_t const magic = readAt<uint32_t>(data, 0);
        bool ok = false;
        switch (magic)
        {
            case kMagicWdbc:
                file.m_format = Format::Wdbc;
                ok = file.loadWdbc(data, error);
                break;
            case kMagicWdc1:
                file.m_format = Format::Wdc1;
                ok = file.loadWdc1(data, error);
                break;
            case kMagicWdc3:
                file.m_format = Format::Wdc3;
                ok = file.loadWdc3(data, error);
                break;
            case kMagicWdc4:
                file.m_format = Format::Wdc4;
                ok = file.loadWdc3(data, error);
                break;
            case kMagicWdc5:
                file.m_format = Format::Wdc5;
                ok = file.loadWdc3(data, error);
                break;
            default:
                error = "unsupported table format '" + std::string(reinterpret_cast<char const*>(data.data()), 4) + "'";
                return std::nullopt;
        }

        if (!ok)
            return std::nullopt;

        return file;
    }

    // --------------------------------------------------------------------------------------------------------------
    // WDBC: fixed 4 byte columns, strings in one block behind the records

    bool Db2File::loadWdbc(std::span<const std::byte> file, std::string& error)
    {
        Cursor cursor(file, 4);
        uint32_t stringSize = 0;
        if (!cursor.read(m_recordCount) || !cursor.read(m_fieldCount) || !cursor.read(m_recordSize) || !cursor.read(stringSize))
        {
            error = "truncated WDBC header";
            return false;
        }

        if (m_fieldCount * 4 != m_recordSize)
        {
            error = "WDBC record size does not match the field count";
            return false;
        }

        m_totalFieldCount = m_fieldCount;
        m_idField = 0;
        size_t const dataSize = static_cast<size_t>(m_recordSize) * m_recordCount + stringSize;
        if (!cursor.has(dataSize))
        {
            error = "truncated WDBC data";
            return false;
        }

        m_data.resize(dataSize + kPadding);
        std::memcpy(m_data.data(), cursor.current(), dataSize);
        m_stringBlockOffset = static_cast<size_t>(m_recordSize) * m_recordCount;

        m_columns.resize(m_fieldCount);
        for (uint32_t i = 0; i < m_fieldCount; ++i)
        {
            m_columns[i].bitOffset = static_cast<uint16_t>(i * 32);
            m_columns[i].bitSize = 32;
            m_columns[i].elementSize = 4;
        }

        Section section;
        section.recordCount = m_recordCount;
        section.stringTableSize = stringSize;
        m_sections.push_back(section);
        return true;
    }

    // --------------------------------------------------------------------------------------------------------------
    // shared pieces of the WDC formats

    bool Db2File::readFieldList(std::span<const std::byte> file, size_t& offset, std::string& error)
    {
        Cursor cursor(file, offset);
        m_fieldList.resize(m_fieldCount);
        for (uint32_t i = 0; i < m_fieldCount; ++i)
        {
            if (!cursor.read(m_fieldList[i].first) || !cursor.read(m_fieldList[i].second))
            {
                error = "truncated field list";
                return false;
            }
        }
        offset = cursor.offset();
        return true;
    }

    // column packing descriptions followed by the pallet and common value blocks
    bool Db2File::readColumns(std::span<const std::byte> file, size_t& offset, uint32_t columnMetaSize, std::string& error)
    {
        m_columns.resize(m_totalFieldCount);
        if (columnMetaSize == 0)
        {
            // every column is plain, sized by the field list
            for (uint32_t i = 0; i < m_totalFieldCount; ++i)
            {
                if (i < m_fieldList.size())
                {
                    m_columns[i].bitOffset = static_cast<uint16_t>(m_fieldList[i].second * 8);
                    m_columns[i].elementSize = static_cast<uint32_t>(4 - m_fieldList[i].first / 8);
                }
            }
            return true;
        }

        if (columnMetaSize != m_totalFieldCount * kColumnMetaSize)
        {
            error = "unexpected column meta size";
            return false;
        }

        Cursor cursor(file, offset);
        std::vector<uint32_t> blockSizes(m_totalFieldCount, 0);     // bytes of pallet or common data per column
        for (uint32_t i = 0; i < m_totalFieldCount; ++i)
        {
            Column& column = m_columns[i];
            uint32_t compression = 0;
            uint32_t extra[3] = { 0, 0, 0 };
            if (!cursor.read(column.bitOffset) || !cursor.read(column.bitSize) || !cursor.read(blockSizes[i])
                || !cursor.read(compression) || !cursor.read(extra[0]) || !cursor.read(extra[1]) || !cursor.read(extra[2]))
            {
                error = "truncated column meta";
                return false;
            }

            column.compression = static_cast<Compression>(compression);
            if (i < m_fieldList.size())
                column.elementSize = static_cast<uint32_t>(4 - m_fieldList[i].first / 8);

            switch (column.compression)
            {
                case Compression::None:
                    break;
                case Compression::Immediate:
                case Compression::SignedImmediate:
                    column.packedBitOffset = extra[0];
                    column.packedBitWidth = extra[1];
                    break;
                case Compression::CommonData:
                    column.commonDefault = extra[0];
                    break;
                case Compression::Pallet:
                case Compression::PalletArray:
                    column.packedBitOffset = extra[0];
                    column.packedBitWidth = extra[1];
                    column.arraySize = std::max<uint32_t>(1, extra[2]);
                    break;
                default:
                    error = "unknown column compression " + std::to_string(compression);
                    return false;
            }
        }

        // pallet blocks: the single value columns first, then the array columns, each in column order
        for (Compression kind : { Compression::Pallet, Compression::PalletArray })
        {
            for (uint32_t i = 0; i < m_totalFieldCount; ++i)
            {
                Column& column = m_columns[i];
                if (column.compression != kind)
                    continue;

                column.pallet.resize(blockSizes[i] / 4);
                for (uint32_t& value : column.pallet)
                {
                    if (!cursor.read(value))
                    {
                        error = "truncated pallet data";
                        return false;
                    }
                }
            }
        }

        // common value blocks: pairs of record id and value
        for (uint32_t i = 0; i < m_totalFieldCount; ++i)
        {
            Column& column = m_columns[i];
            if (column.compression != Compression::CommonData)
                continue;

            size_t const pairCount = blockSizes[i] / 8;
            for (size_t pair = 0; pair < pairCount; ++pair)
            {
                uint32_t recordId = 0;
                uint32_t value = 0;
                if (!cursor.read(recordId) || !cursor.read(value))
                {
                    error = "truncated common data";
                    return false;
                }
                column.commonValues[recordId] = value;
            }
        }

        offset = cursor.offset();
        return true;
    }

    // --------------------------------------------------------------------------------------------------------------
    // WDC1 (7.3): one section, the id list, copy table and column blocks behind the records

    bool Db2File::loadWdc1(std::span<const std::byte> file, std::string& error)
    {
        Cursor cursor(file, 4);
        uint32_t stringTableSize = 0, minId = 0, maxId = 0, locale = 0, copyTableSize = 0, lookupColumnCount = 0;
        uint32_t offsetMapOffset = 0, idListSize = 0, columnMetaSize = 0, commonDataSize = 0, palletDataSize = 0, relationshipDataSize = 0;
        if (!cursor.read(m_recordCount) || !cursor.read(m_fieldCount) || !cursor.read(m_recordSize) || !cursor.read(stringTableSize)
            || !cursor.read(m_tableHash) || !cursor.read(m_layoutHash) || !cursor.read(minId) || !cursor.read(maxId) || !cursor.read(locale)
            || !cursor.read(copyTableSize) || !cursor.read(m_flags) || !cursor.read(m_idField) || !cursor.read(m_totalFieldCount)
            || !cursor.read(m_packedDataOffset) || !cursor.read(lookupColumnCount) || !cursor.read(offsetMapOffset) || !cursor.read(idListSize)
            || !cursor.read(columnMetaSize) || !cursor.read(commonDataSize) || !cursor.read(palletDataSize) || !cursor.read(relationshipDataSize))
        {
            error = "truncated WDC1 header";
            return false;
        }

        if (m_flags & kFlagSparse)
        {
            error = "offset map (sparse) tables are not supported";
            return false;
        }

        size_t offset = cursor.offset();
        if (!readFieldList(file, offset, error))
            return false;

        size_t const recordBlockSize = static_cast<size_t>(m_recordSize) * m_recordCount;
        cursor.seek(offset);
        if (!cursor.has(recordBlockSize + stringTableSize))
        {
            error = "truncated WDC1 records";
            return false;
        }

        m_data.resize(recordBlockSize + stringTableSize + kPadding);
        std::memcpy(m_data.data(), cursor.current(), recordBlockSize + stringTableSize);
        m_stringBlockOffset = recordBlockSize;
        cursor.skip(recordBlockSize + stringTableSize);

        if (m_flags & kFlagNonInlineIds)
        {
            m_ids.resize(idListSize / 4);
            if (!cursor.readBytes(m_ids.data(), idListSize))
            {
                error = "truncated WDC1 id list";
                return false;
            }
        }
        else if (!cursor.skip(idListSize))
        {
            error = "truncated WDC1 id list";
            return false;
        }

        m_copies.resize(copyTableSize / sizeof(Copy));
        if (!cursor.readBytes(m_copies.data(), m_copies.size() * sizeof(Copy)))
        {
            error = "truncated WDC1 copy table";
            return false;
        }

        offset = cursor.offset();
        if (!readColumns(file, offset, columnMetaSize, error))
            return false;

        // the relationship (parent lookup) block is not needed by any reader of this class

        Section section;
        section.recordCount = m_recordCount;
        section.stringTableSize = stringTableSize;
        m_sections.push_back(section);
        m_relativeStrings = false;
        return true;
    }

    // --------------------------------------------------------------------------------------------------------------
    // WDC3 (8.1 to 9.x), WDC4 (10.1) and WDC5 (10.2 and later): sections with own records and strings

    bool Db2File::loadWdc3(std::span<const std::byte> file, std::string& error)
    {
        Cursor cursor(file, 4);
        if (m_format == Format::Wdc5)
        {
            // version and a 128 byte schema name in front of the common header
            uint32_t version = 0;
            if (!cursor.read(version) || !cursor.skip(128))
            {
                error = "truncated WDC5 header";
                return false;
            }
        }

        uint32_t stringTableSize = 0, minId = 0, maxId = 0, locale = 0, lookupColumnCount = 0;
        uint32_t columnMetaSize = 0, commonDataSize = 0, palletDataSize = 0, sectionCount = 0;
        if (!cursor.read(m_recordCount) || !cursor.read(m_fieldCount) || !cursor.read(m_recordSize) || !cursor.read(stringTableSize)
            || !cursor.read(m_tableHash) || !cursor.read(m_layoutHash) || !cursor.read(minId) || !cursor.read(maxId) || !cursor.read(locale)
            || !cursor.read(m_flags) || !cursor.read(m_idField) || !cursor.read(m_totalFieldCount) || !cursor.read(m_packedDataOffset)
            || !cursor.read(lookupColumnCount) || !cursor.read(columnMetaSize) || !cursor.read(commonDataSize) || !cursor.read(palletDataSize)
            || !cursor.read(sectionCount))
        {
            error = "truncated WDC header";
            return false;
        }

        if (m_flags & kFlagSparse)
        {
            error = "offset map (sparse) tables are not supported";
            return false;
        }

        m_sections.resize(sectionCount);
        uint32_t recordsSeen = 0;
        for (Section& section : m_sections)
        {
            if (!cursor.read(section.tactKey) || !cursor.read(section.fileOffset) || !cursor.read(section.recordCount)
                || !cursor.read(section.stringTableSize) || !cursor.read(section.offsetRecordsEnd) || !cursor.read(section.idListSize)
                || !cursor.read(section.relationshipDataSize) || !cursor.read(section.offsetMapIdCount) || !cursor.read(section.copyTableCount))
            {
                error = "truncated section headers";
                return false;
            }
            section.firstRecord = recordsSeen;
            recordsSeen += section.recordCount;
            // without the key the client would decrypt with, the section's bytes are zeros or garbage
            section.readable = section.tactKey == 0;
        }

        size_t offset = cursor.offset();
        if (!readFieldList(file, offset, error))
            return false;
        if (!readColumns(file, offset, columnMetaSize, error))
            return false;

        // 10.1 and later list the record ids of every encrypted section behind the column blocks
        if (m_format == Format::Wdc4 || m_format == Format::Wdc5)
        {
            cursor.seek(offset);
            for (Section const& section : m_sections)
            {
                if (section.tactKey == 0)
                    continue;

                uint32_t encryptedIdCount = 0;
                if (!cursor.read(encryptedIdCount) || !cursor.skip(static_cast<size_t>(encryptedIdCount) * 4))
                {
                    error = "truncated encrypted id lists";
                    return false;
                }
            }
        }

        // one buffer: the records of every section, then the strings of every section
        size_t const recordBlockSize = static_cast<size_t>(m_recordSize) * m_recordCount;
        size_t stringBlockSize = 0;
        for (Section const& section : m_sections)
            stringBlockSize += section.stringTableSize;

        m_data.assign(recordBlockSize + stringBlockSize + kPadding, 0);
        m_stringBlockOffset = recordBlockSize;
        if (m_flags & kFlagNonInlineIds)
            m_ids.assign(m_recordCount, 0);

        size_t recordWrite = 0;
        size_t stringWrite = m_stringBlockOffset;
        for (Section& section : m_sections)
        {
            size_t const sectionRecords = static_cast<size_t>(m_recordSize) * section.recordCount;
            if (section.readable)
            {
                cursor.seek(section.fileOffset);
                if (!cursor.readBytes(m_data.data() + recordWrite, sectionRecords) || !cursor.readBytes(m_data.data() + stringWrite, section.stringTableSize))
                {
                    error = "truncated section records";
                    return false;
                }

                if (m_flags & kFlagNonInlineIds)
                {
                    if (section.idListSize != section.recordCount * 4 || !cursor.readBytes(m_ids.data() + section.firstRecord, section.idListSize))
                    {
                        error = "unexpected id list size";
                        return false;
                    }
                }
                else if (!cursor.skip(section.idListSize))
                {
                    error = "truncated id list";
                    return false;
                }

                size_t const copyStart = m_copies.size();
                m_copies.resize(copyStart + section.copyTableCount);
                if (!cursor.readBytes(m_copies.data() + copyStart, static_cast<size_t>(section.copyTableCount) * sizeof(Copy)))
                {
                    error = "truncated copy table";
                    return false;
                }
                // offset map, relationship data and offset map ids follow - none of them is needed here
            }
            recordWrite += sectionRecords;
            stringWrite += section.stringTableSize;
        }

        m_relativeStrings = true;
        return true;
    }

    // --------------------------------------------------------------------------------------------------------------
    // record access

    bool Db2File::isReadable(uint32_t index) const noexcept
    {
        if (index >= m_recordCount)
            return false;
        for (Section const& section : m_sections)
        {
            if (index >= section.firstRecord && index < section.firstRecord + section.recordCount)
                return section.readable;
        }
        return false;
    }

    uint8_t const* Db2File::recordData(uint32_t index) const noexcept
    {
        return m_data.data() + static_cast<size_t>(index) * m_recordSize;
    }

    uint64_t Db2File::packedValue(uint8_t const* record, uint32_t bitOffset, uint32_t bitWidth) const noexcept
    {
        if (bitWidth == 0 || bitWidth > 64)
            return 0;
        uint64_t raw = 0;
        std::memcpy(&raw, record + m_packedDataOffset + bitOffset / 8, sizeof(raw));
        uint32_t const shift = bitOffset & 7;
        raw >>= shift;
        if (bitWidth < 64)
            raw &= (uint64_t(1) << bitWidth) - 1;
        return raw;
    }

    uint32_t Db2File::recordId(uint32_t index) const
    {
        if (m_flags & kFlagNonInlineIds)
            return index < m_ids.size() ? m_ids[index] : 0;
        if (m_idField < 0)
            return 0;
        return static_cast<uint32_t>(rawValue(index, static_cast<uint32_t>(m_idField), 0));
    }

    uint64_t Db2File::rawValue(uint32_t index, uint32_t field, uint32_t arrayIndex) const
    {
        if (field >= m_columns.size() || !isReadable(index))
            return 0;

        Column const& column = m_columns[field];
        uint8_t const* record = recordData(index);
        switch (column.compression)
        {
            case Compression::None:
            {
                size_t const offset = column.bitOffset / 8 + static_cast<size_t>(column.elementSize) * arrayIndex;
                if (offset + column.elementSize > m_recordSize)
                    return 0;
                uint64_t value = 0;
                std::memcpy(&value, record + offset, std::min<size_t>(column.elementSize, sizeof(value)));
                return value;
            }
            case Compression::Immediate:
                return packedValue(record, column.packedBitOffset, column.packedBitWidth);
            case Compression::SignedImmediate:
            {
                uint64_t value = packedValue(record, column.packedBitOffset, column.packedBitWidth);
                if (column.packedBitWidth > 0 && column.packedBitWidth < 64)
                {
                    uint64_t const sign = uint64_t(1) << (column.packedBitWidth - 1);
                    value = (value ^ sign) - sign;
                }
                return value;
            }
            case Compression::CommonData:
            {
                auto const it = column.commonValues.find(recordId(index));
                return it != column.commonValues.end() ? it->second : column.commonDefault;
            }
            case Compression::Pallet:
            {
                uint64_t const palletIndex = packedValue(record, column.packedBitOffset, column.packedBitWidth);
                return palletIndex < column.pallet.size() ? column.pallet[static_cast<size_t>(palletIndex)] : 0;
            }
            case Compression::PalletArray:
            {
                uint64_t const palletIndex = packedValue(record, column.packedBitOffset, column.packedBitWidth) * column.arraySize + arrayIndex;
                return palletIndex < column.pallet.size() ? column.pallet[static_cast<size_t>(palletIndex)] : 0;
            }
        }
        return 0;
    }

    uint32_t Db2File::Record::id() const
    {
        return m_file.recordId(m_index);
    }

    uint32_t Db2File::Record::getUInt32(uint32_t field, uint32_t arrayIndex) const
    {
        return static_cast<uint32_t>(m_file.rawValue(m_index, field, arrayIndex));
    }

    int32_t Db2File::Record::getInt32(uint32_t field, uint32_t arrayIndex) const
    {
        return static_cast<int32_t>(static_cast<uint32_t>(m_file.rawValue(m_index, field, arrayIndex)));
    }

    uint64_t Db2File::Record::getUInt64(uint32_t field, uint32_t arrayIndex) const
    {
        return m_file.rawValue(m_index, field, arrayIndex);
    }

    float Db2File::Record::getFloat(uint32_t field, uint32_t arrayIndex) const
    {
        uint32_t const bits = static_cast<uint32_t>(m_file.rawValue(m_index, field, arrayIndex));
        float value = 0.0f;
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }

    std::string_view Db2File::Record::getString(uint32_t field, uint32_t arrayIndex) const
    {
        if (field >= m_file.m_columns.size() || !m_file.isReadable(m_index))
            return {};

        uint32_t const value = static_cast<uint32_t>(m_file.rawValue(m_index, field, arrayIndex));
        size_t position = 0;
        if (m_file.m_relativeStrings)
        {
            // measured from the string field's own place in the record
            Column const& column = m_file.m_columns[field];
            size_t const fieldPosition = static_cast<size_t>(m_index) * m_file.m_recordSize + column.bitOffset / 8 + static_cast<size_t>(4) * arrayIndex;
            position = fieldPosition + value;
        }
        else
        {
            position = m_file.m_stringBlockOffset + value;
        }

        if (value == 0 || position >= m_file.m_data.size() - kPadding)
            return {};

        char const* text = reinterpret_cast<char const*>(m_file.m_data.data() + position);
        size_t const maxLength = m_file.m_data.size() - kPadding - position;
        size_t length = 0;
        while (length < maxLength && text[length] != '\0')
            ++length;
        return std::string_view(text, length);
    }
}
