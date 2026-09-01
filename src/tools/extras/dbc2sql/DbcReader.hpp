/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace dbc2sql
{
    enum class ContainerFormat
    {
        Unknown,
        Dbc,        // 'WDBC' magic - classic fixed-width format
        SimpleDb2   // 'WDB2' magic - Cata/Mop's fixed-width DB2 layout (the
                    // only DB2 shape this tool understands; sparse/indexed
                    // DB2 variants used by later formats are out of scope
                    // and are reported as unsupported, not guessed at)
    };

    // Reads a plain .dbc/.db2 file from disk (no MPQ involved - these are
    // meant to already be extracted onto disk). Mirrors the on-disk layout
    // WDB::WDBLoader::load()/loadDb2() (src/world/Storage/WDB) understand,
    // reimplemented standalone here because that file pulls in the world
    // server's logging/crash-handler machinery (via its ASSERT usage) that
    // this tool has no use for and shouldn't need to link against.
    //
    // `format` only ever decides byte width per field ('b'/'X' -> 1 byte,
    // everything else -> 4 bytes) when building the per-field offset table -
    // it does not affect which header fields get read. That means calling
    // load() with an all-'i' "probe" format is always safe and gives back
    // the file's true recordCount()/fieldCount()/recordSize()/stringSize()
    // regardless of whether the probe format is the file's *real* layout.
    class DbcReader
    {
    public:
        bool load(std::string const& path, std::string const& format);

        ContainerFormat containerFormat() const { return m_containerFormat; }
        uint32_t recordCount() const { return m_recordCount; }
        uint32_t fieldCount() const { return m_fieldCount; }
        uint32_t recordSize() const { return m_recordSize; }
        uint32_t stringSize() const { return m_stringSize; }

        uint32_t getUInt32(uint32_t record, uint32_t field) const;
        float getFloat(uint32_t record, uint32_t field) const;
        uint8_t getUInt8(uint32_t record, uint32_t field) const;
        // Returns "" (never nullptr) if the stored offset is out of bounds,
        // rather than asserting/crashing - this tool must survive being
        // pointed at an unexpected/malformed file.
        std::string getString(uint32_t record, uint32_t field) const;

    private:
        unsigned char const* recordPtr(uint32_t record) const;
        unsigned char const* stringTable() const;

        ContainerFormat m_containerFormat = ContainerFormat::Unknown;
        uint32_t m_recordCount = 0;
        uint32_t m_fieldCount = 0;
        uint32_t m_recordSize = 0;
        uint32_t m_stringSize = 0;
        std::vector<uint32_t> m_fieldOffsets;
        std::vector<unsigned char> m_data;
    };
}
