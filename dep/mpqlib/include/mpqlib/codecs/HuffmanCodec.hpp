/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Blizzard's adaptive Huffman compression, used only in combination with
// ADPCM for embedded sound assets - never for the DBC/ADT/WDT/WMO/M2 content
// AscEmu's tools actually extract. Kept complete for archive-reading
// correctness, but this specific codec has no reachable path in any real
// extraction AscEmu performs today.
//
// A structure-preserving C++23 port of AscEmu's original libmpq vendoring
// (huffman.c/huffman.h, itself adapted from StormLib's huffman.cpp). That
// original is a disassembly-derived, self-modifying intrusive doubly-linked
// list where a pointer field's *sign bit* (checked via a cast to a
// pointer-sized integer) doubles as a tag distinguishing "real tree item
// pointer" from "encoded sentinel" - a technique that only has one honest
// translation into typed C++: keep the exact same tagged-pointer scheme,
// give the tagging operation a name instead of a macro, and leave the
// control flow untouched function-by-function. Several of the tree
// mutation functions are given their original disassembly-address names
// (call1500E740/call1500E820) rather than invented descriptive ones,
// because the original port's own comments ("usually NULL?", "whats
// that?") make clear even its author wasn't fully certain of every
// branch's intent - a confident-sounding name here would be dishonest.
// This codec could not be exercised against real data during this port
// (see above: nothing in AscEmu's actual extraction ever reaches it), so
// treat it as unverified if it is ever wired up for real use.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace mpqlib::codecs
{
    enum class HuffmanMode : std::uint32_t
    {
        Decompress = 0,
    };

    // Bit-level reader over an in-memory buffer, matching the original
    // huffman_input_stream_s: get1Bit() consumes one bit; get7Bits() and
    // get8Bits() ensure enough bits are buffered first, but only get8Bits()
    // (and get1Bit()) actually consume - get7Bits() is a peek, matching the
    // original (the caller decides how many of those 7 bits to consume once
    // it knows how far it walked the tree).
    class HuffmanInputStream
    {
    public:
        // The original codec's caller (extract.c's libmpq__decompress_huffman
        // wrapper) manually preloaded the first 32 bits before decoding a
        // single symbol; replicated here in the constructor via refill32()
        // so get8Bits()'s very first call sees bits_==32, not 0.
        explicit HuffmanInputStream(std::span<const std::byte> data) : data_(data) { refill32(); }

        std::uint32_t get1Bit();
        std::uint32_t get7Bits();
        std::uint32_t get8Bits();
        void consumeBits(std::uint32_t count);

    private:
        void refill32();
        void refill16();

        std::span<const std::byte> data_;
        std::size_t pos_ = 0;
        std::uint32_t bitBuf_ = 0;
        std::uint32_t bits_ = 0;
    };

    class HuffmanTree
    {
    public:
        HuffmanTree();

        // Reads one 8-bit compression-type selector from `is`, (re)builds
        // the tree for it, then decodes into `output`. Returns the number
        // of bytes written (0 if output is empty).
        std::size_t decompress(HuffmanInputStream& is, std::span<std::byte> output);

    private:
        struct Item
        {
            Item* next = nullptr;
            Item* prev = nullptr;
            std::uint32_t dcmpByte = 0;
            std::uint32_t byteValue = 0;
            Item* parent = nullptr;
            Item* child = nullptr;
        };

        // Cached fast path for symbols whose Huffman code is 7 bits or
        // fewer: resolved once, reused until the tree is next rebuilt
        // (tracked via `generation` vs. the tree's own `rebuildCount_`).
        struct QuickDecode
        {
            std::uint32_t generation = 0;
            std::uint32_t bits = 0;
            union
            {
                std::uint32_t dcmpByte;
                Item* item;
            };
        };

        enum class InsertMode
        {
            SwitchItems,
            InsertItem,
        };

        // Ports of the original PTR_INT/PTR_NOT macros - see HuffmanCodec.cpp
        // for why the tagged-pointer scheme itself is preserved as-is. Members
        // (not free functions) purely because Item is a private nested type.
        static std::intptr_t asInt(const Item* p);
        static Item* bitNot(const Item* p);

        void treeInit();
        void buildTree(std::uint32_t compressionType);
        void insertItem(Item* item, InsertMode mode, Item* relativeTo);
        void removeItem(Item* item);
        Item* previousItem(Item* item, std::intptr_t fallbackOffset);
        Item* call1500E740();
        void call1500E820(Item* item);

        std::array<Item, 0x203> items_{};
        std::uint32_t itemsUsed_ = 0;

        bool cmp0_ = false;
        std::uint32_t rebuildCount_ = 1;

        Item* item3050_ = nullptr;
        Item* item3054_ = nullptr;
        Item* item3058_ = nullptr;
        Item* item305C_ = nullptr;
        Item* first_ = nullptr;
        Item* last_ = nullptr;

        std::array<Item*, 0x102> itemByByte_{};
        std::array<QuickDecode, 0x80> quickDecode_{};
    };
}
