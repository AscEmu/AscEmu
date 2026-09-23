/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//\NOTE:    Parallel run of the version layout tables against the legacy structs they are going
//          to replace. Runs once at startup and logs every difference. Nothing in the object path
//          uses the layout tables until this reports zero mismatches for every version.

#pragma once

#include "ObjectLayout.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Version
{
    struct CheckReport
    {
        uint32_t checks = 0;
        std::vector<std::string> mismatches;
        std::vector<std::string> notes;

        void mismatch(std::string text) { mismatches.emplace_back(std::move(text)); }
        void note(std::string text) { notes.emplace_back(std::move(text)); }

        void compareSize(const char* structName, size_t actualSize, uint16_t tableSize);

        template <typename FieldId>
        void compareField(const char* structName, const char* member, uint32_t actualOffset, size_t actualSize, const LayoutTable& table, FieldId id)
        {
            const FieldDesc& desc = table.get(id);
            compareDesc(structName, member, actualOffset, actualSize, desc.offset, desc.size);
        }

    private:
        void compareDesc(const char* structName, const char* member, uint32_t actualOffset, size_t actualSize, uint16_t tableOffset, uint16_t tableSize);
    };

    /// generated: compares the layout tables of the compiled expansion with the structs of this binary
    void checkCompiledStructLayout(CheckReport& report);

    /// runs every check, logs the outcome and returns true when no mismatch was found
    bool runParallelChecks();
}
