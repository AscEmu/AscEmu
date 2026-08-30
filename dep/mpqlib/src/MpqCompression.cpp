/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/MpqCompression.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <span>
#include <vector>

#include <bzlib.h>
#include <zlib.h>

#include "LzmaDec.h"
#include "mpqlib/codecs/AdpcmWave.hpp"
#include "mpqlib/codecs/HuffmanCodec.hpp"
#include "mpqlib/codecs/PkwareExplode.hpp"

namespace mpqlib
{
    namespace
    {
        // Per-sector compression method byte(s), as verified against StormLib's
        // SCompDecompress2 (SCompression.cpp) - the dispatcher every real Cata/Mop
        // archive actually uses, since all of them are MPQ format version >= 2.
        // Unlike the older bitmask-OR dispatch (format version 1 only), this is a
        // fixed table of exact byte values, including two-codec combinations that
        // are NOT simple ORs of independently-applicable codecs.
        constexpr uint8_t kHuffman = 0x01;
        constexpr uint8_t kZlib = 0x02;
        constexpr uint8_t kPkware = 0x08;
        constexpr uint8_t kBzip2 = 0x10;
        constexpr uint8_t kLzma = 0x12;
        constexpr uint8_t kSparse = 0x20;
        constexpr uint8_t kAdpcmMono = 0x40;
        constexpr uint8_t kAdpcmStereo = 0x80;

        std::span<const std::byte> asBytes(const uint8_t* p, uint32_t size)
        {
            return { reinterpret_cast<const std::byte*>(p), size };
        }

        std::span<std::byte> asBytes(uint8_t* p, uint32_t size)
        {
            return { reinterpret_cast<std::byte*>(p), size };
        }

        int32_t decompressZlib(const uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t outSize)
        {
            z_stream z{};
            z.next_in = const_cast<Bytef*>(in);
            z.avail_in = inSize;
            z.next_out = out;
            z.avail_out = outSize;

            if (inflateInit(&z) != Z_OK)
                return -1;

            const int result = inflate(&z, Z_FINISH);
            const auto written = static_cast<int32_t>(z.total_out);
            inflateEnd(&z);

            return result == Z_STREAM_END ? written : -1;
        }

        int32_t decompressBzip2(const uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t outSize)
        {
            bz_stream strm{};
            strm.next_in = const_cast<char*>(reinterpret_cast<const char*>(in));
            strm.avail_in = inSize;
            strm.next_out = reinterpret_cast<char*>(out);
            strm.avail_out = outSize;

            if (BZ2_bzDecompressInit(&strm, 0, 0) != BZ_OK)
                return -1;

            int result;
            do
            {
                result = BZ2_bzDecompress(&strm);
            } while (result == BZ_OK && strm.avail_in > 0 && strm.avail_out > 0);

            const auto written = static_cast<int32_t>(strm.total_out_lo32);
            BZ2_bzDecompressEnd(&strm);

            return result == BZ_STREAM_END ? written : -1;
        }

        int32_t decompressPkware(const uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t outSize)
        {
            auto result = codecs::pkwareExplode(asBytes(in, inSize), asBytes(out, outSize));
            return result ? static_cast<int32_t>(*result) : -1;
        }

        int32_t decompressHuffmanThenAdpcm(const uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t outSize, int channels)
        {
            std::vector<uint8_t> huffmanOut(outSize);
            codecs::HuffmanInputStream stream(asBytes(in, inSize));
            codecs::HuffmanTree tree;
            const std::size_t huffmanBytes = tree.decompress(stream, asBytes(huffmanOut.data(), outSize));

            if (huffmanBytes == 0)
                return -1;

            const std::size_t written = codecs::adpcmWaveDecompress(
                asBytes(huffmanOut.data(), static_cast<uint32_t>(huffmanBytes)), asBytes(out, outSize), channels);

            return static_cast<int32_t>(written);
        }

        void* lzmaAlloc(void*, size_t size) { return std::malloc(size); }
        void lzmaFree(void*, void* address) { if (address) std::free(address); }

        // LZMA sector layout (see StormLib's Decompress_LZMA): 1 byte "useFilter"
        // (always 0 for MPQ data), 5-byte LZMA properties, 8-byte uncompressed size
        // (only the low 4 bytes are ever non-zero), then the raw LZMA stream.
        constexpr size_t kLzmaHeaderSize = 1 + LZMA_PROPS_SIZE + 8;

        int32_t decompressLzma(const uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t outSize)
        {
            if (inSize <= kLzmaHeaderSize || in[0] != 0)
                return -1;

            ISzAlloc alloc{ lzmaAlloc, lzmaFree };
            SizeT destLen = outSize;
            SizeT srcLen = inSize - static_cast<uint32_t>(kLzmaHeaderSize);
            ELzmaStatus status;

            SRes result = LzmaDecode(out, &destLen, in + kLzmaHeaderSize, &srcLen,
                in + 1, LZMA_PROPS_SIZE, LZMA_FINISH_END, &status, &alloc);

            if (result != SZ_OK)
                return -1;

            return static_cast<int32_t>(destLen);
        }

        // Blizzard's "Sparse" (zero-run-length) compression. Ported from
        // StormLib's DecompressSparse (src/sparse/sparse.cpp): a 4-byte
        // big-endian uncompressed size, followed by markers - high bit set means
        // a literal run of ((marker & 0x7F) + 1) bytes follows; clear means a
        // zero-run of ((marker & 0x7F) + 3) bytes.
        int32_t decompressSparse(const uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t outSize)
        {
            if (inSize < 4)
                return -1;

            uint32_t expectedSize = (uint32_t(in[0]) << 24) | (uint32_t(in[1]) << 16) | (uint32_t(in[2]) << 8) | uint32_t(in[3]);
            if (expectedSize > outSize)
                return -1;

            const uint8_t* inCursor = in + 4;
            const uint8_t* inEnd = in + inSize;
            uint8_t* outCursor = out;
            uint32_t remaining = expectedSize;

            while (inCursor < inEnd && remaining > 0)
            {
                uint8_t marker = *inCursor++;

                if (marker & 0x80)
                {
                    uint32_t chunkSize = std::min<uint32_t>((marker & 0x7F) + 1, remaining);
                    if (inCursor + chunkSize > inEnd)
                        return -1;
                    std::memcpy(outCursor, inCursor, chunkSize);
                    inCursor += chunkSize;
                    outCursor += chunkSize;
                    remaining -= chunkSize;
                }
                else
                {
                    uint32_t chunkSize = std::min<uint32_t>((marker & 0x7F) + 3, remaining);
                    std::memset(outCursor, 0, chunkSize);
                    outCursor += chunkSize;
                    remaining -= chunkSize;
                }
            }

            if (remaining != 0)
                return -1;

            return static_cast<int32_t>(expectedSize);
        }

        // Faithful port of StormLib's SCompDecompress2: a single exact-match
        // method byte (not a bitmask-OR of independently chained codecs), since
        // that is what every real Cata/Mop archive (format version >= 2) uses for
        // per-sector decompression and for compressed v4 hash/block tables alike.
        int32_t decompressMulti(const uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t outSize)
        {
            if (inSize < 1)
                return -1;

            uint8_t method = in[0];
            const uint8_t* payload = in + 1;
            uint32_t payloadSize = inSize - 1;

            switch (method)
            {
                case kZlib:
                    return decompressZlib(payload, payloadSize, out, outSize);

                case kPkware:
                    return decompressPkware(payload, payloadSize, out, outSize);

                case kBzip2:
                    return decompressBzip2(payload, payloadSize, out, outSize);

                case kLzma:
                    return decompressLzma(payload, payloadSize, out, outSize);

                case kSparse:
                    return decompressSparse(payload, payloadSize, out, outSize);

                case kSparse | kZlib:
                {
                    std::vector<uint8_t> temp(outSize);
                    int32_t tb = decompressZlib(payload, payloadSize, temp.data(), outSize);
                    if (tb < 0)
                        return -1;
                    return decompressSparse(temp.data(), static_cast<uint32_t>(tb), out, outSize);
                }

                case kSparse | kBzip2:
                {
                    std::vector<uint8_t> temp(outSize);
                    int32_t tb = decompressBzip2(payload, payloadSize, temp.data(), outSize);
                    if (tb < 0)
                        return -1;
                    return decompressSparse(temp.data(), static_cast<uint32_t>(tb), out, outSize);
                }

                case kAdpcmMono | kHuffman:
                    return decompressHuffmanThenAdpcm(payload, payloadSize, out, outSize, 1);

                case kAdpcmStereo | kHuffman:
                    return decompressHuffmanThenAdpcm(payload, payloadSize, out, outSize, 2);

                default:
                    return -1;
            }
        }
    }

    int32_t decompressSector(const uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t outSize, SectorCompression compression)
    {
        switch (compression)
        {
            case SectorCompression::None:
                if (inSize < outSize)
                    return -1;
                std::copy(in, in + outSize, out);
                return static_cast<int32_t>(outSize);

            case SectorCompression::Multi:
                return decompressMulti(in, inSize, out, outSize);

            case SectorCompression::Imploded:
                return decompressPkware(in, inSize, out, outSize);
        }

        return -1;
    }
}
