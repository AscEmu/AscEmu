/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//\NOTE:    Version specific opcode tables. The internal opcode id (Opcodes enum) stays version
//          neutral, one table per client version maps it to the hex value on the wire. The tables
//          live in Version/Opcodes/OpcodeTable_<Version>.cpp and are generated from the legacy
//          multiversion store until that store is retired.

#pragma once

#include "Server/Opcodes.hpp"
#include "Server/ClientProtocol.hpp"
#include "Platform/SymbolVisibility.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Version
{
    /// one line of a version table: internal id -> hex value of that client version
    struct OpcodeEntry
    {
        Opcodes internalId;
        uint16_t hex;
    };

    /// a complete version table as emitted by the generator
    struct OpcodeSource
    {
        const OpcodeEntry* entries;
        size_t count;
    };

    /// version neutral information about an internal opcode id
    struct OpcodeMeta
    {
        const char* name;
        OpcodeDevelopmentState state;
    };

    namespace Tables
    {
        extern const OpcodeSource classicOpcodes;
        extern const OpcodeSource tbcOpcodes;
        extern const OpcodeSource wotlkOpcodes;
        extern const OpcodeSource cataOpcodes;
        extern const OpcodeSource mopOpcodes;

        extern const OpcodeMeta opcodeMeta[NUM_OPCODES];
    }

    /// The generated table for an expansion, nullptr when no table exists for it
    [[nodiscard]] const OpcodeSource* opcodeSourceFor(WoW::Expansion expansion) noexcept;

    [[nodiscard]] inline std::string_view opcodeName(uint32_t internalId) noexcept
    {
        return internalId < NUM_OPCODES ? std::string_view{ Tables::opcodeMeta[internalId].name } : std::string_view{ "Unknown internal id!" };
    }

    [[nodiscard]] inline OpcodeDevelopmentState opcodeState(uint32_t internalId) noexcept
    {
        return internalId < NUM_OPCODES ? Tables::opcodeMeta[internalId].state : OpcodeDevelopmentState::Unchecked;
    }

    /// Lookup structure for one client version, built once from its OpcodeSource
    class SERVER_DECL OpcodeTable
    {
    public:
        void build(WoW::Expansion expansion);
        void clear();

        [[nodiscard]] WoW::Expansion getExpansion() const noexcept { return m_expansion; }
        [[nodiscard]] bool isBuilt() const noexcept { return !m_idToHex.empty(); }
        [[nodiscard]] size_t size() const noexcept { return m_hexToId.size(); }

        /// hex value on the wire, 0 when the opcode does not exist in this client version
        [[nodiscard]] uint16_t hexFor(uint32_t internalId) const noexcept
        {
            return internalId < m_idToHex.size() ? m_idToHex[internalId] : 0;
        }

        [[nodiscard]] bool hasOpcode(uint32_t internalId) const noexcept { return hexFor(internalId) != 0; }

        /// internal id for a hex value received from the client, MSG_NULL_ACTION (0) when unknown
        [[nodiscard]] uint32_t internalIdFor(uint16_t hex) const noexcept
        {
            const auto it = m_hexToId.find(hex);
            return it != m_hexToId.end() ? it->second : 0;
        }

    private:
        WoW::Expansion m_expansion = WoW::Expansion::Unknown;
        std::vector<uint16_t> m_idToHex;
        std::unordered_map<uint16_t, uint32_t> m_hexToId;
    };
}
