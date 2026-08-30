/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Blizzard's IMA-ADPCM-derived WAVE compression (mono/stereo), used only for
// embedded sound assets - never for the DBC/ADT/WDT/WMO/M2 content AscEmu's
// tools actually extract, but kept complete for archive-reading correctness.
// A from-scratch C++23 port of AscEmu's original libmpq vendoring (wave.c/
// wave.h, itself adapted from StormLib's wave.cpp); the arithmetic - the
// per-channel predictor/step-index state machine and the two lookup tables -
// is unchanged from that original.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace mpqlib::codecs
{
    // Decompresses one ADPCM WAVE block. `channels` is 1 (mono) or 2
    // (stereo). Returns the number of bytes actually written to `output` -
    // this can be less than output.size() if the input stream runs out
    // early or output fills up mid-sample, matching the original codec's
    // "best effort" behavior (there is no hard failure mode here).
    std::size_t adpcmWaveDecompress(std::span<const std::byte> input, std::span<std::byte> output, int channels);
}
