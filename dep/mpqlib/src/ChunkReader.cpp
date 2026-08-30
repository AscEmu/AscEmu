/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/ChunkReader.hpp"

#include <cstring>

namespace mpqlib
{
    namespace
    {
        struct RawChunkHeader
        {
            std::array<char, 4> tag;
            uint32_t size;
        };
    }

    bool ChunkReader::next(Chunk& out)
    {
        if (m_offset + sizeof(RawChunkHeader) > m_reader.size())
            return false;

        // Chunk headers land at a data-dependent (previous chunk's size),
        // not necessarily 4-byte-aligned, offset - copy out rather than
        // viewing them in place with BinaryReader::at.
        RawChunkHeader header;
        std::memcpy(&header, m_reader.subspan(m_offset, sizeof(header)).data(), sizeof(header));

        size_t const payloadOffset = m_offset + sizeof(RawChunkHeader);
        if (payloadOffset + header.size > m_reader.size())
            return false;

        // Chunk tags are stored reversed on disk (e.g. "KNCM" for "MCNK").
        m_tagStorage = { header.tag[3], header.tag[2], header.tag[1], header.tag[0] };

        out.tag = std::string_view(m_tagStorage.data(), m_tagStorage.size());
        out.payload = m_reader.subspan(payloadOffset, header.size);

        m_offset = payloadOffset + header.size;
        return true;
    }
}
