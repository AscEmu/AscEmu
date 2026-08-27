/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include "mpqlib/BinaryReader.hpp"
#include "mpqlib/MpqPatchChain.hpp"

// Reads a single file out of an mpqlib::MpqPatchChain into an in-memory
// buffer with a small stream-like interface (read/seek/close) on top -
// filenames are not case sensitive.
class MPQFile
{
public:
    MPQFile(mpqlib::MpqPatchChain& mpq, const char* filename, bool warnNoExist = true);
    ~MPQFile();

    MPQFile(const MPQFile&) = delete;
    MPQFile& operator=(const MPQFile&) = delete;

    size_t read(void* dest, size_t bytes);
    void seek(int offset);
    void seekRelative(int offset);
    void close();

    size_t getSize() const;
    size_t getPos() const;
    const char* getBuffer() const;
    const char* getPointer() const;

    // Lets callers walk the raw bytes with their own tools (e.g. a
    // mpqlib::ChunkReader for a fourCC-tagged format) without duplicating
    // the load-from-mpq step this class already did.
    mpqlib::BinaryReader const& reader() const { return m_reader; }

    // Checked as the condition of every chunk-parsing while loop across the
    // ADT/WDT/WMO readers - kept inline for that reason.
    bool isEof() const { return m_eof; }

private:
    bool m_eof;
    size_t m_pointer;
    mpqlib::BinaryReader m_reader;
};

// A tiny one-liner called on every chunk header read across the ADT/WDT/WMO
// parsers - kept inline for that reason.
inline void flipFourCC(char* fourCC)
{
    std::swap(fourCC[0], fourCC[3]);
    std::swap(fourCC[1], fourCC[2]);
}
