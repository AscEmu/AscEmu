/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Sector decompression. PKWARE DCL implode (codecs/PkwareExplode), Huffman
// (codecs/HuffmanCodec), and ADPCM wave (codecs/AdpcmWave) are from-scratch
// C++23 ports of AscEmu's original libmpq vendoring - see those headers for
// how faithfully each preserves the original bit-level algorithm. zlib/bzip2
// go through the zlib/bzip2 deps AscEmu already links elsewhere. LZMA
// (vendored, public-domain LZMA SDK decoder - see vendor/lzma) and Sparse
// (ported from StormLib's sparse.cpp) round out the codec set: both are used
// by real Cata/Mop archives, notably the Data/Cache/patch-*.MPQ hotfix
// archives, and were the missing piece causing silent per-file extraction
// gaps before they were added.

#pragma once

#include <cstdint>
#include <vector>

namespace mpqlib
{
    // Per-file compression flags, as stored in the MPQ block table (matches
    // Blizzard's on-disk format; see MpqArchive.hpp's BlockFlags).
    enum class SectorCompression
    {
        None,
        Multi,      // sector begins with a byte describing one or more chained codecs (zlib/bzip2/pkware/adpcm/huffman)
        Imploded    // legacy single-codec PKWARE DCL implode, no leading flag byte
    };

    // Decompresses one sector. Returns the number of bytes written to `out`
    // (which must already be sized to the sector's known unpacked size), or
    // -1 on failure.
    int32_t decompressSector(const uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t outSize, SectorCompression compression);
}
