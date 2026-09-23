/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ParallelCheck.hpp"

#include "VersionRegistry.hpp"
#include "Logging/Logger.hpp"
#include "Server/OpcodeTable.hpp"

#include <fmt/format.h>

namespace Version
{
    void CheckReport::compareSize(const char* structName, size_t actualSize, uint16_t tableSize)
    {
        ++checks;
        if (actualSize != tableSize)
            mismatch(fmt::format("{}: sizeof is {} bytes, layout table says {}", structName, actualSize, tableSize));
    }

    void CheckReport::compareDesc(const char* structName, const char* member, uint32_t actualOffset, size_t actualSize, uint16_t tableOffset, uint16_t tableSize)
    {
        ++checks;
        if (tableOffset == kNoField)
        {
            mismatch(fmt::format("{}::{}: exists at offset {} but the layout table has no entry", structName, member, actualOffset));
            return;
        }

        if (actualOffset != tableOffset || actualSize != tableSize)
        {
            mismatch(fmt::format("{}::{}: struct offset {} size {}, layout table offset {} size {}",
                structName, member, actualOffset, actualSize, tableOffset, tableSize));
        }
    }

    void checkOpcodeTables(CheckReport& report)
    {
        static constexpr WoW::Expansion versions[] =
        {
            WoW::Expansion::_Classic,
            WoW::Expansion::_TBC,
            WoW::Expansion::_WotLK,
            WoW::Expansion::_Cata,
            WoW::Expansion::_Mop,
        };

        // version neutral part: names and development state
        for (uint32_t id = 0; id < NUM_OPCODES; ++id)
        {
            ++report.checks;
            const auto legacy = multiversionOpcodeStore.find(id);
            if (legacy == multiversionOpcodeStore.end())
            {
                report.mismatch(fmt::format("opcode id {}: missing in the legacy store", id));
                continue;
            }

            if (legacy->second.name.empty())
                report.note(fmt::format("opcode id {} ({}): legacy store has an empty name, the version table uses the enum name", id, opcodeName(id)));
            else if (legacy->second.name != opcodeName(id))
                report.mismatch(fmt::format("opcode id {}: legacy name {} vs {}", id, legacy->second.name, opcodeName(id)));

            if (legacy->second.state != opcodeState(id))
                report.mismatch(fmt::format("opcode id {} ({}): development state differs", id, opcodeName(id)));
        }

        for (const auto expansion : versions)
        {
            const OpcodeTable& table = sVersionRegistry.opcodes(expansion);
            const auto versionName = WoW::getShortExpansionName(expansion);
            uint32_t hexZeroEntries = 0;

            if (table.getExpansion() != expansion)
            {
                report.mismatch(fmt::format("{}: no version opcode table built", versionName));
                continue;
            }

            // outgoing direction: internal id -> hex
            for (uint32_t id = 0; id < NUM_OPCODES; ++id)
            {
                ++report.checks;
                const uint16_t legacyHex = sOpcodeTables.getHexValueForExpansion(id, expansion);
                const uint16_t newHex = table.hexFor(id);
                if (legacyHex != newHex)
                    report.mismatch(fmt::format("{} {}: legacy hex 0x{:04X} vs 0x{:04X}", versionName, opcodeName(id), legacyHex, newHex));
            }

            // incoming direction: hex -> internal id, over every hex value the legacy table knows for this version
            const auto tableIndex = WoW::getOpcodeTableIndex(expansion);
            for (const auto& entry : sOpcodeTables._versionHexTable[tableIndex])
            {
                if (entry.hexValue == 0)
                {
                    ++hexZeroEntries;
                    continue;
                }

                ++report.checks;
                const uint32_t legacyId = sOpcodeTables.getInternalIdForHex(entry.hexValue, expansion);
                const uint32_t newId = table.internalIdFor(entry.hexValue);
                if (legacyId != newId)
                {
                    report.mismatch(fmt::format("{} hex 0x{:04X}: legacy id {} ({}) vs {} ({})",
                        versionName, entry.hexValue, legacyId, opcodeName(legacyId), newId, opcodeName(newId)));
                }
            }

            if (hexZeroEntries != 0)
            {
                // the legacy lookup resolves hex 0 to the first CMSG without a value in this version, the
                // version table treats hex 0 as "opcode not present" and answers MSG_NULL_ACTION
                report.note(fmt::format("{}: {} legacy entries without a hex value are skipped, hex 0 resolves to {} (legacy) vs {} (table)",
                    versionName, hexZeroEntries, opcodeName(sOpcodeTables.getInternalIdForHex(0, expansion)), opcodeName(table.internalIdFor(0))));
            }
        }
    }

    bool runParallelChecks()
    {
        CheckReport report;
        checkOpcodeTables(report);
        checkCompiledStructLayout(report);

        for (const auto& note : report.notes)
            sLogger.info("Version::ParallelCheck : note: {}", note);

        static constexpr size_t maxLoggedMismatches = 50;
        size_t logged = 0;
        for (const auto& mismatch : report.mismatches)
        {
            if (logged++ == maxLoggedMismatches)
            {
                sLogger.failure("Version::ParallelCheck : ... {} more mismatches not shown", report.mismatches.size() - maxLoggedMismatches);
                break;
            }

            sLogger.failure("Version::ParallelCheck : {}", mismatch);
        }

        if (report.mismatches.empty())
        {
            sLogger.info("Version::ParallelCheck : {} checks against the legacy tables, no mismatch (compiled {}, server {}).",
                report.checks, WoW::getExpansionName(WoW::COMPILED_EXPANSION), WoW::getExpansionName(WoW::getServerExpansion()));
            return true;
        }

        sLogger.failure("Version::ParallelCheck : {} checks, {} mismatches (compiled {}, server {}).",
            report.checks, report.mismatches.size(), WoW::getExpansionName(WoW::COMPILED_EXPANSION), WoW::getExpansionName(WoW::getServerExpansion()));
        return false;
    }
}
