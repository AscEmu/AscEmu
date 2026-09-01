/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "DbcReader.hpp"

#include <cstring>
#include <fstream>

namespace dbc2sql
{
    namespace
    {
        constexpr uint32_t kWdbcMagic = 0x43424457;  // 'WDBC'
        constexpr uint32_t kWdb2Magic = 0x32424457;  // 'WDB2'

        bool readU32(std::ifstream& file, uint32_t& out)
        {
            file.read(reinterpret_cast<char*>(&out), sizeof(out));
            return static_cast<bool>(file);
        }
    }

    bool DbcReader::load(std::string const& path, std::string const& format)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            return false;

        uint32_t magic = 0;
        if (!readU32(file, magic))
            return false;

        if (magic == kWdbcMagic)
            m_containerFormat = ContainerFormat::Dbc;
        else if (magic == kWdb2Magic)
            m_containerFormat = ContainerFormat::SimpleDb2;
        else
            return false;

        if (!readU32(file, m_recordCount) || !readU32(file, m_fieldCount) ||
            !readU32(file, m_recordSize) || !readU32(file, m_stringSize))
            return false;

        if (m_containerFormat == ContainerFormat::SimpleDb2)
        {
            // tableHash, build, 3 unknowns, locale, 1 more unknown - 7 more
            // uint32 fields we don't need, but must skip to reach the data.
            uint32_t discard = 0;
            for (int i = 0; i < 7; ++i)
            {
                if (!readU32(file, discard))
                    return false;
            }
        }

        if (m_fieldCount == 0 || format.size() < m_fieldCount)
            return false;

        m_fieldOffsets.assign(m_fieldCount, 0);
        for (uint32_t i = 1; i < m_fieldCount; ++i)
        {
            char const widthChar = format[i - 1];
            uint32_t const width = (widthChar == 'b' || widthChar == 'X') ? 1u : 4u;
            m_fieldOffsets[i] = m_fieldOffsets[i - 1] + width;
        }

        size_t const dataSize = static_cast<size_t>(m_recordSize) * m_recordCount + m_stringSize;
        m_data.assign(dataSize, 0);

        if (dataSize != 0)
        {
            file.read(reinterpret_cast<char*>(m_data.data()), static_cast<std::streamsize>(dataSize));
            if (!file)
                return false;
        }

        return true;
    }

    unsigned char const* DbcReader::recordPtr(uint32_t record) const
    {
        return m_data.data() + static_cast<size_t>(record) * m_recordSize;
    }

    unsigned char const* DbcReader::stringTable() const
    {
        return m_data.data() + static_cast<size_t>(m_recordSize) * m_recordCount;
    }

    uint32_t DbcReader::getUInt32(uint32_t record, uint32_t field) const
    {
        uint32_t value = 0;
        std::memcpy(&value, recordPtr(record) + m_fieldOffsets[field], sizeof(value));
        return value;
    }

    float DbcReader::getFloat(uint32_t record, uint32_t field) const
    {
        float value = 0.0f;
        std::memcpy(&value, recordPtr(record) + m_fieldOffsets[field], sizeof(value));
        return value;
    }

    uint8_t DbcReader::getUInt8(uint32_t record, uint32_t field) const
    {
        return *(recordPtr(record) + m_fieldOffsets[field]);
    }

    std::string DbcReader::getString(uint32_t record, uint32_t field) const
    {
        uint32_t const stringOffset = getUInt32(record, field);
        if (stringOffset >= m_stringSize)
            return {};

        char const* str = reinterpret_cast<char const*>(stringTable() + stringOffset);
        // Bound the scan to the remaining string-table size in case of a
        // malformed/non-null-terminated block near the very end of the file.
        size_t const maxLen = m_stringSize - stringOffset;
        size_t len = 0;
        while (len < maxLen && str[len] != '\0')
            ++len;

        return std::string(str, len);
    }
}
