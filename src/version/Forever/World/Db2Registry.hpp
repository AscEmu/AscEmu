/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace AscEmu::Version::Forever::Db2
{
    struct RecordView
    {
        std::span<uint8_t const> data{};
        std::string tableName;
        uint32_t layoutHash{0};
    };

    // Resolves a Forever WDC5 table by its embedded table hash and returns the
    // physical record bytes for the requested row. Tables are discovered from
    // <DataDir>/dbc and loaded lazily on first use.
    [[nodiscard]] bool getRecord(uint32_t tableHash, uint32_t recordId, RecordView& out);
}
