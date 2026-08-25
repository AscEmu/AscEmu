/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/MpqCompression.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "LzmaDec.h"

extern "C"
{
    // Implemented in dep/libmpq/libmpq/extract.c (vendored, unmodified codec dispatch).
    int32_t libmpq__decompress_zlib(uint8_t* in_buf, uint32_t in_size, uint8_t* out_buf, uint32_t out_size);
    int32_t libmpq__decompress_pkzip(uint8_t* in_buf, uint32_t in_size, uint8_t* out_buf, uint32_t out_size);
    int32_t libmpq__decompress_bzip2(uint8_t* in_buf, uint32_t in_size, uint8_t* out_buf, uint32_t out_size);
    int32_t libmpq__decompress_huffman(uint8_t* in_buf, uint32_t in_size, uint8_t* out_buf, uint32_t out_size);
    int32_t libmpq__decompress_wave_mono(uint8_t* in_buf, uint32_t in_size, uint8_t* out_buf, uint32_t out_size);
    int32_t libmpq__decompress_wave_stereo(uint8_t* in_buf, uint32_t in_size, uint8_t* out_buf, uint32_t out_size);
}

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
                    return libmpq__decompress_zlib(const_cast<uint8_t*>(payload), payloadSize, out, outSize);

                case kPkware:
                    return libmpq__decompress_pkzip(const_cast<uint8_t*>(payload), payloadSize, out, outSize);

                case kBzip2:
                    return libmpq__decompress_bzip2(const_cast<uint8_t*>(payload), payloadSize, out, outSize);

                case kLzma:
                    return decompressLzma(payload, payloadSize, out, outSize);

                case kSparse:
                    return decompressSparse(payload, payloadSize, out, outSize);

                case kSparse | kZlib:
                {
                    std::vector<uint8_t> temp(outSize);
                    int32_t tb = libmpq__decompress_zlib(const_cast<uint8_t*>(payload), payloadSize, temp.data(), outSize);
                    if (tb < 0)
                        return -1;
                    return decompressSparse(temp.data(), static_cast<uint32_t>(tb), out, outSize);
                }

                case kSparse | kBzip2:
                {
                    std::vector<uint8_t> temp(outSize);
                    int32_t tb = libmpq__decompress_bzip2(const_cast<uint8_t*>(payload), payloadSize, temp.data(), outSize);
                    if (tb < 0)
                        return -1;
                    return decompressSparse(temp.data(), static_cast<uint32_t>(tb), out, outSize);
                }

                case kAdpcmMono | kHuffman:
                {
                    std::vector<uint8_t> temp(outSize);
                    int32_t tb = libmpq__decompress_huffman(const_cast<uint8_t*>(payload), payloadSize, temp.data(), outSize);
                    if (tb < 0)
                        return -1;
                    return libmpq__decompress_wave_mono(temp.data(), static_cast<uint32_t>(tb), out, outSize);
                }

                case kAdpcmStereo | kHuffman:
                {
                    std::vector<uint8_t> temp(outSize);
                    int32_t tb = libmpq__decompress_huffman(const_cast<uint8_t*>(payload), payloadSize, temp.data(), outSize);
                    if (tb < 0)
                        return -1;
                    return libmpq__decompress_wave_stereo(temp.data(), static_cast<uint32_t>(tb), out, outSize);
                }

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
                return libmpq__decompress_pkzip(const_cast<uint8_t*>(in), inSize, out, outSize);
        }

        return -1;
    }
}
