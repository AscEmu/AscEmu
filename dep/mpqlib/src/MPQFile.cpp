/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/MPQFile.hpp"

#include <cstdio>
#include <cstring>

MPQFile::MPQFile(mpqlib::MpqPatchChain& mpq, const char* filename, bool warnNoExist) :
    m_eof(false),
    m_pointer(0),
    m_reader(std::vector<std::byte>{})
{
    auto reader = mpqlib::BinaryReader::fromMpq(mpq, filename);
    if (!reader)
    {
        if (warnNoExist)
            fprintf(stderr, "Can't open %s!\n", filename);
        m_eof = true;
        return;
    }

    if (reader->size() <= 1)
    {
        fprintf(stderr, "Can't open %s, size = %u!\n", filename, static_cast<uint32_t>(reader->size()));
        m_eof = true;
        return;
    }

    m_reader = std::move(*reader);
}

MPQFile::~MPQFile() = default;

size_t MPQFile::read(void* dest, size_t bytes)
{
    if (m_eof)
        return 0;

    auto const data = m_reader.bytes();
    size_t readPos = m_pointer + bytes;
    if (readPos > data.size())
    {
        bytes = data.size() - m_pointer;
        m_eof = true;
    }

    memcpy(dest, data.data() + m_pointer, bytes);
    m_pointer = readPos;

    return bytes;
}

void MPQFile::seek(int offset)
{
    m_pointer = static_cast<size_t>(offset);
    m_eof = (m_pointer >= m_reader.size());
}

void MPQFile::seekRelative(int offset)
{
    m_pointer += static_cast<size_t>(offset);
    m_eof = (m_pointer >= m_reader.size());
}

void MPQFile::close()
{
    m_reader = mpqlib::BinaryReader(std::vector<std::byte>{});
    m_pointer = 0;
    m_eof = true;
}

size_t MPQFile::getSize() const
{
    return m_reader.size();
}

size_t MPQFile::getPos() const
{
    return m_pointer;
}

const char* MPQFile::getBuffer() const
{
    return reinterpret_cast<const char*>(m_reader.bytes().data());
}

const char* MPQFile::getPointer() const
{
    return reinterpret_cast<const char*>(m_reader.bytes().data()) + m_pointer;
}
