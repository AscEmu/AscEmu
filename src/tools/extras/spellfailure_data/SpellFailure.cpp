/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Reverse-engineers the server's SpellCastResult/PetTameFailure enums (the
// numeric codes the client expects for "why did my spell/tame fail")
// directly from a client wow.exe: WoW clients embed these enumerator names
// as plain ASCII debug strings, packed back-to-back in the binary in the
// same order the real client enum declares them. Walking that string table
// and recording each name against a running index reconstructs the whole
// enum without any official documentation - see
// src/world/Spell/Definitions/SpellFailure.hpp for where the output of a
// run of this tool against each client version ends up.

#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    constexpr std::string_view kLicenseHeader =
        "/*\n"
        "Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>\n"
        "This file is released under the MIT license. See README-MIT for more information.\n"
        "*/";

    constexpr char const* kDefaultExecutable = "wow.exe";
    constexpr char const* kOutputFile = "SpellFailure.hpp";

    // At this exact index, WotLK's string table doesn't have the next
    // expected "SPELL_FAILED_*" name adjacent to the previous one - the real
    // client enum has SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW here, but its
    // debug string isn't laid out where the rest of the sequence predicts,
    // so the backward search below naturally skips past it to the next
    // available name instead. This patch re-inserts it at the index it
    // belongs at without disturbing that already-correct skip.
    //
    // Confirmed (by comparing src/world/Spell/Definitions/SpellFailure.hpp's
    // hand-verified per-version enums) the same entry exists in Cata at
    // index 176 and Mop at index 185 - different indices, since the enum
    // grew between versions - so this patch is WotLK-specific and won't
    // fire there. If a Cata/Mop run hits an analogous gap, the gap warning
    // in extractSequence() below will flag it instead of silently
    // mis-numbering everything after it.
    constexpr int kCantDoIndex = 173;
    constexpr char const* kCantDoName = "SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW";

    std::vector<char> readFile(std::string const& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file)
            return {};

        std::streamsize const size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(static_cast<size_t>(size));
        if (size > 0 && !file.read(buffer.data(), size))
            return {};

        return buffer;
    }

    using Entries = std::vector<std::pair<std::string, int>>;

    // Walks backward from startAnchor (already known to exist in `data`),
    // collecting one name per step, until it reaches or passes endAnchor.
    // `specialCase`, when set, re-inserts a known name at a known index
    // without consuming a search step (see kCantDoIndex's doc comment).
    // Prints a warning whenever a step has to travel unusually far to find
    // the next name - a sign a name is missing at that point.
    bool extractSequence(std::string_view data, std::string_view startAnchor, std::string_view endAnchor,
        std::string_view prefix, int firstIndex, Entries& outEntries,
        std::optional<std::pair<int, std::string_view>> specialCase = std::nullopt)
    {
        size_t const startPos = data.find(startAnchor);
        size_t const endPos = data.find(endAnchor);
        if (startPos == std::string_view::npos || endPos == std::string_view::npos)
        {
            std::cout << std::format("Error: couldn't find anchor strings ('{}'/'{}') in the given file.\n", startAnchor, endAnchor);
            return false;
        }

        size_t pos = startPos;
        int index = firstIndex;
        double runningMeanGap = 0.0;
        size_t gapSamples = 0;

        while (true)
        {
            if (specialCase && index == specialCase->first)
            {
                outEntries.emplace_back(std::string(specialCase->second), index);
                ++index;
                continue;
            }

            size_t const nameEnd = data.find('\0', pos);
            if (nameEnd == std::string_view::npos)
            {
                std::cout << std::format("Error: unterminated string at offset {} while collecting index {}.\n", pos, index);
                return false;
            }
            outEntries.emplace_back(std::string(data.substr(pos, nameEnd - pos)), index);

            if (pos == 0 || pos - 1 < endPos)
                break;

            size_t const decremented = pos - 1;
            size_t const nextPos = data.rfind(prefix, decremented);
            if (nextPos == std::string_view::npos)
                break;

            size_t const gap = pos - nextPos;
            if (gapSamples > 0 && gap > runningMeanGap * 3.0)
            {
                std::cout << std::format(
                    "Warning: gap of {} bytes found before index {} (typical gap so far ~{} bytes) - "
                    "a name may be missing here; inspect the output around that index.\n",
                    gap, index + 1, static_cast<size_t>(runningMeanGap));
            }
            runningMeanGap = (runningMeanGap * static_cast<double>(gapSamples) + static_cast<double>(gap)) / static_cast<double>(gapSamples + 1);
            ++gapSamples;

            pos = nextPos;
            ++index;
        }

        return true;
    }

    void writeEnum(std::ostream& out, std::string_view enumName, Entries const& entries)
    {
        out << std::format("enum {} : uint8_t\n{{\n", enumName);
        for (auto const& [name, value] : entries)
            out << std::format("\t{:<60} = {},\n", name, value);
        out << "};\n";
    }
}

int main(int argc, char** argv)
{
    std::string const executablePath = argc > 1 ? argv[1] : kDefaultExecutable;

    std::vector<char> const buffer = readFile(executablePath);
    if (buffer.empty())
    {
        std::cout << std::format("Error: couldn't open '{}' for reading.\n", executablePath);
        return 1;
    }

    std::string_view const data(buffer.data(), buffer.size());

    Entries spellFailedEntries;
    if (!extractSequence(data, "SPELL_FAILED_SUCCESS", "SPELL_FAILED_UNKNOWN", "SPELL_FAILED", 0,
        spellFailedEntries, std::make_pair(kCantDoIndex, std::string_view(kCantDoName))))
        return 2;

    Entries petTameEntries;
    if (!extractSequence(data, "PETTAME_INVALIDCREATURE", "PETTAME_UNKNOWNERROR", "PETTAME", 1, petTameEntries))
        return 3;

    std::ofstream out(kOutputFile);
    if (!out)
    {
        std::cout << std::format("Error: couldn't open '{}' for writing.\n", kOutputFile);
        return 4;
    }

    out << kLicenseHeader << "\n\n#pragma once\n\n";
    writeEnum(out, "SpellCastResult", spellFailedEntries);
    out << "\n";
    writeEnum(out, "PetTameFailure", petTameEntries);

    std::cout << std::format("Done. Wrote {} ({} spell failure codes, {} pet tame failure codes).\n",
        kOutputFile, spellFailedEntries.size(), petTameEntries.size());
    return 0;
}
