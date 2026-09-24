/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ParallelCheck.hpp"

#include "Logging/Logger.hpp"
#include "Server/ClientProtocol.hpp"

#include <fmt/format.h>

namespace Version
{
    void CheckReport::compareSize(const char* structName, size_t actualSize, uint16_t tableSize)
    {
        ++checks;
        if (actualSize != tableSize)
            mismatch(fmt::format("{}: sizeof is {} bytes, layout table says {}", structName, actualSize, tableSize));
    }

    void CheckReport::compareDesc(const char* structName, const char* member, uint32_t actualOffset, size_t actualSize, size_t actualCount, size_t actualStride, const FieldDesc& desc)
    {
        ++checks;
        if (desc.offset == kNoField)
        {
            mismatch(fmt::format("{}::{}: exists at offset {} but the layout table has no entry", structName, member, actualOffset));
            return;
        }

        if (actualOffset != desc.offset || actualSize != desc.size || actualCount != desc.count || actualStride != desc.stride)
        {
            mismatch(fmt::format("{}::{}: struct offset {} size {} count {} stride {}, layout table offset {} size {} count {} stride {}",
                structName, member, actualOffset, actualSize, actualCount, actualStride, desc.offset, desc.size, desc.count, desc.stride));
        }
    }

    bool runParallelChecks()
    {
        CheckReport report;
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
            sLogger.info("Version::ParallelCheck : {} struct layout checks, no mismatch (compiled {}, server {}).",
                report.checks, WoW::getExpansionName(WoW::COMPILED_EXPANSION), WoW::getExpansionName(WoW::getServerExpansion()));
            return true;
        }

        sLogger.failure("Version::ParallelCheck : {} struct layout checks, {} mismatches (compiled {}, server {}).",
            report.checks, report.mismatches.size(), WoW::getExpansionName(WoW::COMPILED_EXPANSION), WoW::getExpansionName(WoW::getServerExpansion()));
        return false;
    }
}
