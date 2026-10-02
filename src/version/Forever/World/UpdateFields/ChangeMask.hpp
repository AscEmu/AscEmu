/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once


#include "Network/ByteBuffer.hpp"

#include <bitset>
#include <cstddef>
#include <cstdint>

namespace AscEmu::Version::Forever::UpdateFields
{
    template <std::size_t N>
    uint32_t getChangeBlock(std::bitset<N> const& changes, std::size_t block)
    {
        uint32_t value = 0;
        const std::size_t firstBit = block * 32U;
        const std::size_t endBit = firstBit + 32U < N ? firstBit + 32U : N;
        for (std::size_t bit = firstBit; bit < endBit; ++bit)
            if (changes.test(bit))
                value |= uint32_t(1) << (bit - firstBit);
        return value;
    }

    template <std::size_t N>
    void writeChangeMask(ByteBuffer& data, std::bitset<N> const& changes, bool flush = true)
    {
        constexpr std::size_t BlockCount = (N + 31U) / 32U;
        if constexpr (BlockCount == 1)
        {
            data.writeBits(getChangeBlock(changes, 0), N);
        }
        else
        {
            uint32_t blocksMask = 0;
            for (std::size_t block = 0; block < BlockCount; ++block)
                if (getChangeBlock(changes, block) != 0)
                    blocksMask |= uint32_t(1) << block;

            data.writeBits(blocksMask, BlockCount);
            for (std::size_t block = 0; block < BlockCount; ++block)
                if ((blocksMask & (uint32_t(1) << block)) != 0)
                    data.writeBits(getChangeBlock(changes, block), 32);
        }

        if (flush)
            data.flushBits();
    }
}
