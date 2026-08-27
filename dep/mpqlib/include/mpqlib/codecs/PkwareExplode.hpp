/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// PKWARE Data Compression Library "explode" (DCL implode decompression).
// A from-scratch C++23 port of AscEmu's original libmpq vendoring
// (explode.c/explode.h, itself adapted from StormLib's pklib.cpp) - the
// bit-level algorithm (literal/length/distance decoding, the six lookup
// tables, the binary/ASCII literal modes) is unchanged from that original,
// battle-tested implementation. What changed is everything around it: no
// malloc, no read/write callback indirection (the original streamed through
// a fixed 0x800-byte input chunk and a 0x1000/0x2000 circular output window
// because it was designed for callback-fed I/O; mpqlib always has the whole
// sector in memory already, so decompression reads/writes std::span directly
// with a single unbounded-lookback view of the output produced so far).

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace mpqlib::codecs
{
    // Decompresses one PKWARE DCL-imploded block. `output` must already be
    // sized to the block's known unpacked size. Returns the number of bytes
    // written (equal to output.size() on a clean decode), or std::nullopt if
    // the input is malformed (bad dictionary size, unknown compression mode,
    // or the literal/distance stream runs out before the output is filled).
    std::optional<std::size_t> pkwareExplode(std::span<const std::byte> input, std::span<std::byte> output);
}
