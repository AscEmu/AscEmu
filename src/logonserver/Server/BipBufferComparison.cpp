/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BipBufferComparison.hpp"

#include "Network/BipBuffer.hpp"
#include "Network/CircularBuffer.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace
{
    template <typename Buffer>
    bool writeBytes(Buffer& buffer, std::vector<uint8_t> const& bytes)
    {
        return buffer.Write(bytes.data(), bytes.size());
    }

    template <typename Buffer>
    bool readBytes(Buffer& buffer, size_t count, std::vector<uint8_t>& out)
    {
        out.assign(count, 0);
        return buffer.Read(out.data(), count);
    }

    bool expectEqual(std::vector<uint8_t> const& a, std::vector<uint8_t> const& b, char const* label)
    {
        if (a == b)
            return true;

        std::printf("[BipBufferComparison] FAILED: %s\n", label);
        return false;
    }

    bool runFunctionalComparison()
    {
        CircularBuffer oldBuffer;
        AscEmu::BipBuffer newBuffer;

        oldBuffer.Allocate(16);
        newBuffer.Allocate(16);

        {
            std::vector<uint8_t> input{ 1, 2, 3, 4, 5, 6, 7, 8 };

            if (writeBytes(oldBuffer, input) != writeBytes(newBuffer, input))
                return false;

            if (oldBuffer.GetSize() != newBuffer.GetSize())
                return false;

            if (oldBuffer.GetContiguiousBytes() != newBuffer.GetContiguiousBytes())
                return false;
        }

        {
            std::vector<uint8_t> oldOut;
            std::vector<uint8_t> newOut;

            if (readBytes(oldBuffer, 5, oldOut) != readBytes(newBuffer, 5, newOut))
                return false;

            if (!expectEqual(oldOut, newOut, "read first 5 bytes"))
                return false;
        }

        {
            std::vector<uint8_t> input{ 9, 10, 11, 12, 13, 14, 15 };

            if (writeBytes(oldBuffer, input) != writeBytes(newBuffer, input))
                return false;

            if (oldBuffer.GetSize() != newBuffer.GetSize())
                return false;
        }

        {
            std::vector<uint8_t> oldOut;
            std::vector<uint8_t> newOut;

            const size_t bytesToRead = oldBuffer.GetSize();

            if (readBytes(oldBuffer, bytesToRead, oldOut) != readBytes(newBuffer, bytesToRead, newOut))
                return false;

            if (!expectEqual(oldOut, newOut, "read remaining bytes"))
                return false;
        }

        return oldBuffer.GetSize() == 0 && newBuffer.GetSize() == 0;
    }

    bool runDirectWriteComparison()
    {
        CircularBuffer oldBuffer;
        AscEmu::BipBuffer newBuffer;

        oldBuffer.Allocate(32);
        newBuffer.Allocate(32);

        auto directWrite = [](auto& buffer, std::vector<uint8_t> const& data)
            {
                const size_t space = buffer.GetSpace();

                if (space < data.size())
                    return false;

                void* target = buffer.GetBuffer();

                if (target == nullptr)
                    return false;

                std::memcpy(target, data.data(), data.size());
                buffer.IncrementWritten(data.size());
                return true;
            };

        {
            std::vector<uint8_t> input{ 10, 20, 30, 40, 50, 60 };

            if (directWrite(oldBuffer, input) != directWrite(newBuffer, input))
                return false;
        }

        {
            std::vector<uint8_t> oldOut;
            std::vector<uint8_t> newOut;

            if (readBytes(oldBuffer, 3, oldOut) != readBytes(newBuffer, 3, newOut))
                return false;

            if (!expectEqual(oldOut, newOut, "direct write read 3 bytes"))
                return false;
        }

        {
            std::vector<uint8_t> input{ 70, 80, 90, 100, 110, 120, 130, 140 };

            if (directWrite(oldBuffer, input) != directWrite(newBuffer, input))
                return false;
        }

        {
            std::vector<uint8_t> oldOut;
            std::vector<uint8_t> newOut;

            const size_t bytesToRead = oldBuffer.GetSize();

            if (readBytes(oldBuffer, bytesToRead, oldOut) != readBytes(newBuffer, bytesToRead, newOut))
                return false;

            if (!expectEqual(oldOut, newOut, "direct write remaining bytes"))
                return false;
        }

        return true;
    }

    bool runRandomComparison()
    {
        CircularBuffer oldBuffer;
        AscEmu::BipBuffer newBuffer;

        oldBuffer.Allocate(256);
        newBuffer.Allocate(256);

        std::mt19937 rng{ 0xA5CE };
        std::uniform_int_distribution<int> opDist(0, 2);
        std::uniform_int_distribution<int> sizeDist(1, 64);
        uint8_t nextValue = 1;

        for (size_t i = 0; i < 10000; ++i)
        {
            const int op = opDist(rng);
            const size_t count = static_cast<size_t>(sizeDist(rng));

            if (op == 0)
            {
                std::vector<uint8_t> input(count);

                for (auto& value : input)
                    value = nextValue++;

                const bool oldResult = writeBytes(oldBuffer, input);
                const bool newResult = writeBytes(newBuffer, input);

                if (oldResult != newResult)
                {
                    std::printf("[BipBufferComparison] FAILED: Write result mismatch at iteration %zu\n", i);
                    return false;
                }
            }
            else if (op == 1)
            {
                if (oldBuffer.GetSize() == 0)
                    continue;

                const size_t readable = std::min(count, oldBuffer.GetSize());

                std::vector<uint8_t> oldOut;
                std::vector<uint8_t> newOut;

                const bool oldResult = readBytes(oldBuffer, readable, oldOut);
                const bool newResult = readBytes(newBuffer, readable, newOut);

                if (oldResult != newResult)
                {
                    std::printf("[BipBufferComparison] FAILED: Read result mismatch at iteration %zu\n", i);
                    return false;
                }

                if (!expectEqual(oldOut, newOut, "random read bytes"))
                    return false;
            }
            else
            {
                const size_t beforeOld = oldBuffer.GetSize();
                const size_t beforeNew = newBuffer.GetSize();

                if (beforeOld != beforeNew)
                {
                    std::printf("[BipBufferComparison] FAILED: size mismatch before remove at iteration %zu\n", i);
                    return false;
                }

                oldBuffer.Remove(count);
                newBuffer.Remove(count);

                if (oldBuffer.GetSize() != newBuffer.GetSize())
                {
                    std::printf("[BipBufferComparison] FAILED: size mismatch after remove at iteration %zu\n", i);
                    return false;
                }
            }

            if (oldBuffer.GetSize() != newBuffer.GetSize())
            {
                std::printf("[BipBufferComparison] FAILED: size mismatch at iteration %zu\n", i);
                return false;
            }

            if (oldBuffer.GetSpace() != newBuffer.GetSpace())
            {
                std::printf("[BipBufferComparison] FAILED: contiguous write space mismatch at iteration %zu\n", i);
                return false;
            }

            if (oldBuffer.GetContiguiousBytes() != newBuffer.GetContiguiousBytes())
            {
                std::printf("[BipBufferComparison] FAILED: contiguous readable bytes mismatch at iteration %zu\n", i);
                return false;
            }
        }

        return true;
    }

    template <typename Buffer>
    uint64_t runMicroBenchmark(char const* name)
    {
        constexpr size_t Capacity = 64 * 1024;
        constexpr size_t Iterations = 500000;
        constexpr size_t ChunkSize = 128;

        Buffer buffer;
        buffer.Allocate(Capacity);

        std::array<uint8_t, ChunkSize> input{};
        std::array<uint8_t, ChunkSize> output{};

        for (size_t i = 0; i < input.size(); ++i)
            input[i] = static_cast<uint8_t>(i);

        const auto start = std::chrono::steady_clock::now();

        for (size_t i = 0; i < Iterations; ++i)
        {
            if (!buffer.Write(input.data(), input.size()))
            {
                std::printf("[BipBufferComparison] Benchmark write failed for %s\n", name);
                return 0;
            }

            if (!buffer.Read(output.data(), output.size()))
            {
                std::printf("[BipBufferComparison] Benchmark read failed for %s\n", name);
                return 0;
            }
        }

        const auto end = std::chrono::steady_clock::now();

        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
            );
    }

    void runBenchmarkComparison()
    {
        const uint64_t oldMicros = runMicroBenchmark<CircularBuffer>("CircularBuffer");
        const uint64_t newMicros = runMicroBenchmark<AscEmu::BipBuffer>("BipBuffer");

        std::printf("[BipBufferComparison] CircularBuffer: %llu us\n",
            static_cast<unsigned long long>(oldMicros));

        std::printf("[BipBufferComparison] BipBuffer:      %llu us\n",
            static_cast<unsigned long long>(newMicros));

        if (oldMicros != 0 && newMicros != 0)
        {
            const double ratio = static_cast<double>(oldMicros) / static_cast<double>(newMicros);

            std::printf("[BipBufferComparison] Ratio old/new: %.3f\n", ratio);
        }
    }
}

void runBipBufferComparison()
{
    std::printf("[BipBufferComparison] Starting tests...\n");

    const bool functionalOk = runFunctionalComparison();
    const bool directWriteOk = runDirectWriteComparison();
    const bool randomOk = runRandomComparison();

    std::printf("[BipBufferComparison] Functional:   %s\n", functionalOk ? "OK" : "FAILED");
    std::printf("[BipBufferComparison] Direct write: %s\n", directWriteOk ? "OK" : "FAILED");
    std::printf("[BipBufferComparison] Random:       %s\n", randomOk ? "OK" : "FAILED");

    if (functionalOk && directWriteOk && randomOk)
        runBenchmarkComparison();
}

