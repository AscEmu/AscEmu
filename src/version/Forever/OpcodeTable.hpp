/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "version/Forever/Opcodes.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace AscEmu::Version::Forever
{
    enum class OpcodeDirection : uint8_t
    {
        Client,
        Server
    };

    struct OpcodeEntry
    {
        Opcode id{Opcode::NONE};
        uint32_t rawOpcode{0};
        std::string_view name{};
        OpcodeDirection direction{OpcodeDirection::Client};
    };

    class OpcodeTable
    {
    public:
        static OpcodeTable& instance();

        [[nodiscard]] Opcode getInternalIdForHex(uint32_t rawOpcode) const;
        [[nodiscard]] uint32_t getHexValueForInternalId(Opcode opcode) const;
        [[nodiscard]] std::string_view getNameForOpcode(uint32_t rawOpcode) const;
        [[nodiscard]] std::string_view getNameForInternalId(Opcode opcode) const;
        [[nodiscard]] const std::vector<OpcodeEntry>& entries() const;

    private:
        OpcodeTable() = default;
    };

    inline OpcodeTable& sOpcodeTable = OpcodeTable::instance();
}
