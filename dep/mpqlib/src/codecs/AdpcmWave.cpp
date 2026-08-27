/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/codecs/AdpcmWave.hpp"

#include <array>
#include <bit>
#include <cstring>

namespace mpqlib::codecs
{
    namespace
    {
        // Per-nibble step-index adjustment, indexed by the low 5 bits of a
        // coded sample byte.
        constexpr std::array<std::int32_t, 32> kStepAdjust{
            -1, 0, -1, 4, -1, 2, -1, 6, -1, 1, -1, 5, -1, 3, -1, 7,
            -1, 1, -1, 5, -1, 3, -1, 7, -1, 2, -1, 4, -1, 6, -1, 8,
        };

        // Step size per predictor step-index (0..88).
        constexpr std::array<std::uint32_t, 89> kStepTable{
            0x00000007, 0x00000008, 0x00000009, 0x0000000A, 0x0000000B, 0x0000000C, 0x0000000D, 0x0000000E,
            0x00000010, 0x00000011, 0x00000013, 0x00000015, 0x00000017, 0x00000019, 0x0000001C, 0x0000001F,
            0x00000022, 0x00000025, 0x00000029, 0x0000002D, 0x00000032, 0x00000037, 0x0000003C, 0x00000042,
            0x00000049, 0x00000050, 0x00000058, 0x00000061, 0x0000006B, 0x00000076, 0x00000082, 0x0000008F,
            0x0000009D, 0x000000AD, 0x000000BE, 0x000000D1, 0x000000E6, 0x000000FD, 0x00000117, 0x00000133,
            0x00000151, 0x00000173, 0x00000198, 0x000001C1, 0x000001EE, 0x00000220, 0x00000256, 0x00000292,
            0x000002D4, 0x0000031C, 0x0000036C, 0x000003C3, 0x00000424, 0x0000048E, 0x00000502, 0x00000583,
            0x00000610, 0x000006AB, 0x00000756, 0x00000812, 0x000008E0, 0x000009C3, 0x00000ABD, 0x00000BD0,
            0x00000CFF, 0x00000E4C, 0x00000FBA, 0x0000114C, 0x00001307, 0x000014EE, 0x00001706, 0x00001954,
            0x00001BDC, 0x00001EA5, 0x000021B6, 0x00002515, 0x000028CA, 0x00002CDF, 0x0000315B, 0x0000364B,
            0x00003BB9, 0x000041B2, 0x00004844, 0x00004F7E, 0x00005771, 0x0000602F, 0x000069CE, 0x00007462,
            0x00007FFF,
        };

        std::int16_t readLE16(std::span<const std::byte> buf, std::size_t pos)
        {
            std::uint16_t value;
            std::memcpy(&value, buf.data() + pos, sizeof(value));
            if constexpr (std::endian::native == std::endian::big)
                value = static_cast<std::uint16_t>((value >> 8) | (value << 8));
            return static_cast<std::int16_t>(value);
        }

        void writeLE16(std::span<std::byte> buf, std::size_t pos, std::uint16_t value)
        {
            if constexpr (std::endian::native == std::endian::big)
                value = static_cast<std::uint16_t>((value >> 8) | (value << 8));
            std::memcpy(buf.data() + pos, &value, sizeof(value));
        }
    }

    std::size_t adpcmWaveDecompress(std::span<const std::byte> input, std::span<std::byte> output, int channels)
    {
        if (input.size() < 2)
            return 0;

        // The second header byte is a fixed right-shift amount applied to
        // every step-size lookup for the rest of the stream (read once from
        // the untouched start of the input, not from the advancing cursor -
        // that asymmetry is intentional and present in the original codec).
        const auto shift = static_cast<unsigned>(std::to_integer<std::uint8_t>(input[1]));

        std::size_t inPos = 2;
        std::size_t outPos = 0;
        std::array<std::int32_t, 2> stepIndex{0x2C, 0x2C};
        std::array<std::int32_t, 2> predictor{};

        for (int channel = 0; channel < channels; ++channel)
        {
            if (inPos + 2 > input.size())
                return outPos;

            const std::int16_t sample = readLE16(input, inPos);
            inPos += 2;
            predictor[static_cast<std::size_t>(channel)] = sample;

            if (output.size() - outPos < 2)
                return outPos;

            writeLE16(output, outPos, static_cast<std::uint16_t>(sample));
            outPos += 2;
        }

        auto index = static_cast<std::size_t>(channels - 1);

        while (inPos < input.size())
        {
            const auto oneByte = std::to_integer<std::uint8_t>(input[inPos++]);

            if (channels == 2)
                index = (index == 0) ? 1 : 0;

            if (oneByte & 0x80)
            {
                switch (oneByte & 0x7F)
                {
                    case 0:
                        if (stepIndex[index] != 0)
                            --stepIndex[index];

                        if (output.size() - outPos < 2)
                            continue;

                        writeLE16(output, outPos, static_cast<std::uint16_t>(predictor[index]));
                        outPos += 2;
                        continue;

                    case 1:
                        stepIndex[index] += 8;
                        if (stepIndex[index] > 0x58)
                            stepIndex[index] = 0x58;
                        if (channels == 2)
                            index = (index == 0) ? 1 : 0;
                        continue;

                    case 2:
                        continue;

                    default:
                        stepIndex[index] -= 8;
                        if (stepIndex[index] < 0)
                            stepIndex[index] = 0;
                        if (channels != 2)
                            continue;
                        index = (index == 0) ? 1 : 0;
                        continue;
                }
            }
            else
            {
                const std::uint32_t stepSize = kStepTable[static_cast<std::size_t>(stepIndex[index])];
                std::uint32_t delta = stepSize >> shift;
                std::int32_t predicted = predictor[index];

                if (oneByte & 0x01) delta += (stepSize >> 0);
                if (oneByte & 0x02) delta += (stepSize >> 1);
                if (oneByte & 0x04) delta += (stepSize >> 2);
                if (oneByte & 0x08) delta += (stepSize >> 3);
                if (oneByte & 0x10) delta += (stepSize >> 4);
                if (oneByte & 0x20) delta += (stepSize >> 5);

                if (oneByte & 0x40)
                {
                    predicted -= static_cast<std::int32_t>(delta);
                    if (predicted <= static_cast<std::int32_t>(0xFFFF8000))
                        predicted = static_cast<std::int32_t>(0xFFFF8000);
                }
                else
                {
                    predicted += static_cast<std::int32_t>(delta);
                    if (predicted >= 0x7FFF)
                        predicted = 0x7FFF;
                }

                predictor[index] = predicted;

                if (output.size() - outPos < 2)
                    break;

                writeLE16(output, outPos, static_cast<std::uint16_t>(predicted));
                outPos += 2;

                std::int32_t newStep = stepIndex[index] + kStepAdjust[oneByte & 0x1F];
                stepIndex[index] = newStep < 0 ? 0 : (newStep > 0x58 ? 0x58 : newStep);
            }
        }

        return outPos;
    }
}
