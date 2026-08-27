/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/FileLoader.hpp"

#include <cstdio>

namespace
{
    constexpr u_map_fcc kMverMagic = { { 'R', 'E', 'V', 'M' } };
}

FileLoader::FileLoader() : m_version(nullptr)
{
}

FileLoader::~FileLoader()
{
    free();
}

bool FileLoader::loadFile(mpqlib::MpqPatchChain& mpq, std::string const& fileName, bool log)
{
    free();

    auto reader = mpqlib::BinaryReader::fromMpq(mpq, fileName);
    if (!reader)
    {
        if (log)
            printf("No such file %s\n", fileName.c_str());
        return false;
    }

    m_reader = std::move(reader);

    if (prepareLoadedData())
        return true;

    printf("Error loading %s", fileName.c_str());
    free();
    return false;
}

bool FileLoader::prepareLoadedData()
{
    if (!m_reader || m_reader->size() < sizeof(file_MVER))
        return false;

    // The public interface hands out a mutable file_MVER* (ADT_file/WDT_file
    // overlay further writable-looking chunk structs past it) even though
    // this accessor is logically const.
    m_version = const_cast<file_MVER*>(&m_reader->at<file_MVER>(0));
    if (m_version->fcc != kMverMagic.fcc)
        return false;
    if (m_version->ver != FILE_FORMAT_VERSION)
        return false;
    return true;
}

void FileLoader::free()
{
    m_reader.reset();
    m_version = nullptr;
}

uint8_t* FileLoader::getData() const
{
    if (!m_reader)
        return nullptr;

    return const_cast<uint8_t*>(reinterpret_cast<uint8_t const*>(m_reader->bytes().data()));
}

uint32_t FileLoader::getDataSize() const
{
    return m_reader ? static_cast<uint32_t>(m_reader->size()) : 0;
}
