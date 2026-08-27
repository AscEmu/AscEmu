/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/codecs/PkwareExplode.hpp"

#include <array>
#include <cstdint>
#include <utility>

namespace mpqlib::codecs
{
    namespace
    {
        enum class CompressionType : std::uint8_t
        {
            Binary = 0,
            Ascii = 1,
        };

        // Number of bits to skip for a given distance-table slot (indexed by
        // the 6-bit code from kDistCode).
        constexpr std::array<std::uint8_t, 0x40> kDistBits{
            0x02, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
            0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
            0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        };

        constexpr std::array<std::uint8_t, 0x40> kDistCode{
            0x03, 0x0D, 0x05, 0x19, 0x09, 0x11, 0x01, 0x3E, 0x1E, 0x2E, 0x0E, 0x36, 0x16, 0x26, 0x06, 0x3A,
            0x1A, 0x2A, 0x0A, 0x32, 0x12, 0x22, 0x42, 0x02, 0x7C, 0x3C, 0x5C, 0x1C, 0x6C, 0x2C, 0x4C, 0x0C,
            0x74, 0x34, 0x54, 0x14, 0x64, 0x24, 0x44, 0x04, 0x78, 0x38, 0x58, 0x18, 0x68, 0x28, 0x48, 0x08,
            0xF0, 0x70, 0xB0, 0x30, 0xD0, 0x50, 0x90, 0x10, 0xE0, 0x60, 0xA0, 0x20, 0xC0, 0x40, 0x80, 0x00,
        };

        constexpr std::array<std::uint8_t, 0x10> kClenBits{
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        };

        constexpr std::array<std::uint16_t, 0x10> kLenBase{
            0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
            0x0008, 0x000A, 0x000E, 0x0016, 0x0026, 0x0046, 0x0086, 0x0106,
        };

        constexpr std::array<std::uint8_t, 0x10> kSlenBits{
            0x03, 0x02, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x07,
        };

        constexpr std::array<std::uint8_t, 0x10> kLenCode{
            0x05, 0x03, 0x01, 0x06, 0x0A, 0x02, 0x0C, 0x14, 0x04, 0x18, 0x08, 0x30, 0x10, 0x20, 0x40, 0x00,
        };

        // Initial bit-length-per-symbol table for ASCII-mode literals, before
        // generateTablesAscii() folds it down into the offs2c34/2d34/2e34/2eb4
        // lookup tables actually used during decoding.
        constexpr std::array<std::uint8_t, 0x100> kBitsAscii{
            0x0B, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x08, 0x07, 0x0C, 0x0C, 0x07, 0x0C, 0x0C,
            0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
            0x04, 0x0A, 0x08, 0x0C, 0x0A, 0x0C, 0x0A, 0x08, 0x07, 0x07, 0x08, 0x09, 0x07, 0x06, 0x07, 0x08,
            0x07, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07, 0x08, 0x08, 0x0C, 0x0B, 0x07, 0x09, 0x0B,
            0x0C, 0x06, 0x07, 0x06, 0x06, 0x05, 0x07, 0x08, 0x08, 0x06, 0x0B, 0x09, 0x06, 0x07, 0x06, 0x06,
            0x07, 0x0B, 0x06, 0x06, 0x06, 0x07, 0x09, 0x08, 0x09, 0x09, 0x0B, 0x08, 0x0B, 0x09, 0x0C, 0x08,
            0x0C, 0x05, 0x06, 0x06, 0x06, 0x05, 0x06, 0x06, 0x06, 0x05, 0x0B, 0x07, 0x05, 0x06, 0x05, 0x05,
            0x06, 0x0A, 0x05, 0x05, 0x05, 0x05, 0x08, 0x07, 0x08, 0x08, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C,
            0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
            0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
            0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
            0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
            0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
            0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
            0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D,
            0x0D, 0x0D, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
        };

        constexpr std::array<std::uint16_t, 0x100> kCodeAscii{
            0x0490, 0x0FE0, 0x07E0, 0x0BE0, 0x03E0, 0x0DE0, 0x05E0, 0x09E0,
            0x01E0, 0x00B8, 0x0062, 0x0EE0, 0x06E0, 0x0022, 0x0AE0, 0x02E0,
            0x0CE0, 0x04E0, 0x08E0, 0x00E0, 0x0F60, 0x0760, 0x0B60, 0x0360,
            0x0D60, 0x0560, 0x1240, 0x0960, 0x0160, 0x0E60, 0x0660, 0x0A60,
            0x000F, 0x0250, 0x0038, 0x0260, 0x0050, 0x0C60, 0x0390, 0x00D8,
            0x0042, 0x0002, 0x0058, 0x01B0, 0x007C, 0x0029, 0x003C, 0x0098,
            0x005C, 0x0009, 0x001C, 0x006C, 0x002C, 0x004C, 0x0018, 0x000C,
            0x0074, 0x00E8, 0x0068, 0x0460, 0x0090, 0x0034, 0x00B0, 0x0710,
            0x0860, 0x0031, 0x0054, 0x0011, 0x0021, 0x0017, 0x0014, 0x00A8,
            0x0028, 0x0001, 0x0310, 0x0130, 0x003E, 0x0064, 0x001E, 0x002E,
            0x0024, 0x0510, 0x000E, 0x0036, 0x0016, 0x0044, 0x0030, 0x00C8,
            0x01D0, 0x00D0, 0x0110, 0x0048, 0x0610, 0x0150, 0x0060, 0x0088,
            0x0FA0, 0x0007, 0x0026, 0x0006, 0x003A, 0x001B, 0x001A, 0x002A,
            0x000A, 0x000B, 0x0210, 0x0004, 0x0013, 0x0032, 0x0003, 0x001D,
            0x0012, 0x0190, 0x000D, 0x0015, 0x0005, 0x0019, 0x0008, 0x0078,
            0x00F0, 0x0070, 0x0290, 0x0410, 0x0010, 0x07A0, 0x0BA0, 0x03A0,
            0x0240, 0x1C40, 0x0C40, 0x1440, 0x0440, 0x1840, 0x0840, 0x1040,
            0x0040, 0x1F80, 0x0F80, 0x1780, 0x0780, 0x1B80, 0x0B80, 0x1380,
            0x0380, 0x1D80, 0x0D80, 0x1580, 0x0580, 0x1980, 0x0980, 0x1180,
            0x0180, 0x1E80, 0x0E80, 0x1680, 0x0680, 0x1A80, 0x0A80, 0x1280,
            0x0280, 0x1C80, 0x0C80, 0x1480, 0x0480, 0x1880, 0x0880, 0x1080,
            0x0080, 0x1F00, 0x0F00, 0x1700, 0x0700, 0x1B00, 0x0B00, 0x1300,
            0x0DA0, 0x05A0, 0x09A0, 0x01A0, 0x0EA0, 0x06A0, 0x0AA0, 0x02A0,
            0x0CA0, 0x04A0, 0x08A0, 0x00A0, 0x0F20, 0x0720, 0x0B20, 0x0320,
            0x0D20, 0x0520, 0x0920, 0x0120, 0x0E20, 0x0620, 0x0A20, 0x0220,
            0x0C20, 0x0420, 0x0820, 0x0020, 0x0FC0, 0x07C0, 0x0BC0, 0x03C0,
            0x0DC0, 0x05C0, 0x09C0, 0x01C0, 0x0EC0, 0x06C0, 0x0AC0, 0x02C0,
            0x0CC0, 0x04C0, 0x08C0, 0x00C0, 0x0F40, 0x0740, 0x0B40, 0x0340,
            0x0300, 0x0D40, 0x1D00, 0x0D00, 0x1500, 0x0540, 0x0500, 0x1900,
            0x0900, 0x0940, 0x1100, 0x0100, 0x1E00, 0x0E00, 0x0140, 0x1600,
            0x0600, 0x1A00, 0x0E40, 0x0640, 0x0A40, 0x0A00, 0x1200, 0x0200,
            0x1C00, 0x0C00, 0x1400, 0x0400, 0x1800, 0x0800, 0x1000, 0x0000,
        };

        // Sentinel results from decodeLiteral(): 0x000-0x0FF is a literal
        // byte, 0x100-0x304 is "copy N-0xFE bytes from earlier in the
        // output", 0x305/0x306 mean end-of-stream / stream error.
        constexpr std::uint32_t kEndOfStream = 0x305;
        constexpr std::uint32_t kStreamError = 0x306;

        // The stateful decoder. Holds exactly the tables and bit-stream state
        // the original pkzip_cmp_s struct had, minus the parts that only
        // existed to support callback-driven chunked I/O (the 0x800-byte
        // input staging buffer and the 0x1000/0x2000 circular output window)
        // - mpqlib always decompresses a whole, fully-buffered sector, so
        // decodeDistance's back-references resolve directly against
        // `output_` instead of a rolling 4 KB window of it.
        class Decompressor
        {
        public:
            Decompressor(std::span<const std::byte> input, std::span<std::byte> output) :
                input_(input), output_(output)
            {
            }

            std::optional<std::size_t> run()
            {
                if (input_.size() <= 4)
                    return std::nullopt;

                const auto header0 = std::to_integer<std::uint8_t>(input_[0]);
                const auto header1 = std::to_integer<std::uint8_t>(input_[1]);
                const auto header2 = std::to_integer<std::uint8_t>(input_[2]);

                if (header0 != std::to_underlying(CompressionType::Binary) &&
                    header0 != std::to_underlying(CompressionType::Ascii))
                    return std::nullopt;

                cmpType_ = static_cast<CompressionType>(header0);
                dsizeBits_ = header1;

                if (dsizeBits_ < 4 || dsizeBits_ > 6)
                    return std::nullopt;

                dsizeMask_ = static_cast<std::uint32_t>(0xFFFFu >> (0x10 - dsizeBits_));
                bitBuf_ = header2;
                extraBits_ = 0;
                inPos_ = 3;

                if (cmpType_ == CompressionType::Ascii)
                {
                    bitsAsc_ = kBitsAscii;
                    generateTablesAscii();
                }

                slenBits_ = kSlenBits;
                generateTablesDecode(slenBits_, kLenCode, pos2_);

                clenBits_ = kClenBits;
                lenBase_ = kLenBase;
                distBits_ = kDistBits;
                generateTablesDecode(distBits_, kDistCode, pos1_);

                if (expand() != kStreamError)
                    return outPos_;

                return std::nullopt;
            }

        private:
            bool skipBit(std::uint32_t bits)
            {
                if (bits <= extraBits_)
                {
                    extraBits_ -= bits;
                    bitBuf_ >>= bits;
                    return true;
                }

                bitBuf_ >>= extraBits_;

                if (inPos_ >= input_.size())
                    return false;

                bitBuf_ |= (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(input_[inPos_++])) << 8);
                bitBuf_ >>= (bits - extraBits_);
                extraBits_ = (extraBits_ - bits) + 8;
                return true;
            }

            template <std::size_t N>
            static void generateTablesDecode(const std::array<std::uint8_t, N>& bits, const std::array<std::uint8_t, N>& code, std::array<std::uint8_t, 0x100>& out)
            {
                for (std::size_t i = N; i-- > 0;)
                {
                    std::uint32_t idx1 = code[i];
                    const std::uint32_t idx2 = 1u << bits[i];

                    do
                    {
                        out[idx1] = static_cast<std::uint8_t>(i);
                        idx1 += idx2;
                    } while (idx1 < 0x100);
                }
            }

            void generateTablesAscii()
            {
                for (int count = 0xFF; count >= 0; --count)
                {
                    const auto index = static_cast<std::size_t>(count);
                    std::uint8_t bitsTmp = bitsAsc_[index];
                    const std::uint16_t codeAsc = kCodeAscii[index];

                    if (bitsTmp <= 8)
                    {
                        const std::uint32_t add = (1u << bitsTmp);
                        std::uint32_t acc = codeAsc;

                        do
                        {
                            offs2c34_[acc] = static_cast<std::uint8_t>(count);
                            acc += add;
                        } while (acc < 0x100);

                        continue;
                    }

                    std::uint32_t acc = codeAsc & 0xFF;

                    if (acc == 0)
                    {
                        bitsTmp -= 8;
                        bitsAsc_[index] = bitsTmp;
                        const std::uint32_t add = (1u << bitsTmp);
                        acc = codeAsc >> 8;

                        do
                        {
                            offs2eb4_[acc] = static_cast<std::uint8_t>(count);
                            acc += add;
                        } while (acc < 0x100);

                        continue;
                    }

                    offs2c34_[acc] = 0xFF;

                    if (codeAsc & 0x3F)
                    {
                        bitsTmp -= 4;
                        bitsAsc_[index] = bitsTmp;
                        const std::uint32_t add = (1u << bitsTmp);
                        acc = codeAsc >> 4;

                        do
                        {
                            offs2d34_[acc] = static_cast<std::uint8_t>(count);
                            acc += add;
                        } while (acc < 0x100);
                    }
                    else
                    {
                        bitsTmp -= 6;
                        bitsAsc_[index] = bitsTmp;
                        const std::uint32_t add = (1u << bitsTmp);
                        acc = codeAsc >> 6;

                        do
                        {
                            offs2e34_[acc] = static_cast<std::uint8_t>(count);
                            acc += add;
                        } while (acc < 0x80);
                    }
                }
            }

            // Returns 0x000-0x0FF for a literal byte, 0x100-0x304 for
            // "repeat N-0xFE bytes back", or kEndOfStream/kStreamError.
            std::uint32_t decodeLiteral()
            {
                std::uint32_t value;

                if (bitBuf_ & 1)
                {
                    if (!skipBit(1))
                        return kStreamError;

                    value = pos2_[bitBuf_ & 0xFF];

                    if (!skipBit(slenBits_[value]))
                        return kStreamError;

                    if (const std::uint32_t bits = clenBits_[value]; bits != 0)
                    {
                        const std::uint32_t val2 = bitBuf_ & ((1u << bits) - 1);

                        if (!skipBit(bits) && (value + val2) != 0x10E)
                            return kStreamError;

                        value = lenBase_[value] + val2;
                    }

                    return value + 0x100;
                }

                if (!skipBit(1))
                    return kStreamError;

                if (cmpType_ == CompressionType::Binary)
                {
                    value = bitBuf_ & 0xFF;
                    return skipBit(8) ? value : kStreamError;
                }

                if (bitBuf_ & 0xFF)
                {
                    value = offs2c34_[bitBuf_ & 0xFF];

                    if (value == 0xFF)
                    {
                        if (bitBuf_ & 0x3F)
                        {
                            if (!skipBit(4))
                                return kStreamError;
                            value = offs2d34_[bitBuf_ & 0xFF];
                        }
                        else
                        {
                            if (!skipBit(6))
                                return kStreamError;
                            value = offs2e34_[bitBuf_ & 0x7F];
                        }
                    }
                }
                else
                {
                    if (!skipBit(8))
                        return kStreamError;
                    value = offs2eb4_[bitBuf_ & 0xFF];
                }

                return skipBit(bitsAsc_[value]) ? value : kStreamError;
            }

            // Number of bytes to move back for a repeat of the given length,
            // or 0 on a malformed/truncated stream.
            std::uint32_t decodeDistance(std::uint32_t length)
            {
                std::uint32_t pos = pos1_[bitBuf_ & 0xFF];
                const std::uint32_t skip = distBits_[pos];

                if (!skipBit(skip))
                    return 0;

                if (length == 2)
                {
                    pos = (pos << 2) | (bitBuf_ & 0x03);
                    if (!skipBit(2))
                        return 0;
                }
                else
                {
                    pos = (pos << dsizeBits_) | (bitBuf_ & dsizeMask_);
                    if (!skipBit(dsizeBits_))
                        return 0;
                }

                return pos + 1;
            }

            std::uint32_t expand()
            {
                std::uint32_t result = 0;

                for (;;)
                {
                    const std::uint32_t oneByte = decodeLiteral();
                    result = oneByte;

                    if (oneByte >= kEndOfStream)
                        break;

                    if (oneByte >= 0x100)
                    {
                        const std::uint32_t copyLength = oneByte - 0xFE;
                        const std::uint32_t moveBack = decodeDistance(copyLength);

                        if (moveBack == 0 || moveBack > outPos_ || copyLength > output_.size() - outPos_)
                        {
                            result = kStreamError;
                            break;
                        }

                        const std::size_t sourcePos = outPos_ - moveBack;

                        for (std::uint32_t i = 0; i < copyLength; ++i)
                            output_[outPos_ + i] = output_[sourcePos + i];

                        outPos_ += copyLength;
                    }
                    else
                    {
                        if (outPos_ >= output_.size())
                        {
                            result = kStreamError;
                            break;
                        }

                        output_[outPos_++] = static_cast<std::byte>(oneByte);
                    }
                }

                return result;
            }

            std::span<const std::byte> input_;
            std::span<std::byte> output_;
            std::size_t inPos_ = 0;
            std::size_t outPos_ = 0;

            CompressionType cmpType_ = CompressionType::Binary;
            std::uint32_t dsizeBits_ = 0;
            std::uint32_t dsizeMask_ = 0;
            std::uint32_t bitBuf_ = 0;
            std::uint32_t extraBits_ = 0;

            std::array<std::uint8_t, 0x100> pos1_{};
            std::array<std::uint8_t, 0x100> pos2_{};
            std::array<std::uint8_t, 0x100> offs2c34_{};
            std::array<std::uint8_t, 0x100> offs2d34_{};
            std::array<std::uint8_t, 0x80> offs2e34_{};
            std::array<std::uint8_t, 0x100> offs2eb4_{};
            std::array<std::uint8_t, 0x100> bitsAsc_{};
            std::array<std::uint8_t, 0x40> distBits_{};
            std::array<std::uint8_t, 0x10> slenBits_{};
            std::array<std::uint8_t, 0x10> clenBits_{};
            std::array<std::uint16_t, 0x10> lenBase_{};
        };
    }

    std::optional<std::size_t> pkwareExplode(std::span<const std::byte> input, std::span<std::byte> output)
    {
        return Decompressor(input, output).run();
    }
}
