/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//\NOTE:    Binds the version tables at startup. Two different versions matter here:
//          - the server expansion from world.conf selects the struct layouts (one layout in memory)
//          - the client expansion of a session selects its opcode table (every supported table is built)

#pragma once

#include "ObjectLayout.hpp"
#include "OpcodeTable.hpp"
#include "Server/ClientProtocol.hpp"
#include "Platform/SymbolVisibility.hpp"

#include <array>
#include <string>

#include <fmt/format.h>

namespace Version
{
    class SERVER_DECL Registry
    {
    private:
        Registry() = default;
        ~Registry() = default;

    public:
        static Registry& getInstance();

        Registry(Registry&&) = delete;
        Registry(Registry const&) = delete;
        Registry& operator=(Registry&&) = delete;
        Registry& operator=(Registry const&) = delete;

        /// builds the opcode tables of every supported expansion and binds the layouts of the server expansion
        void initialize();
        void finalize();

        [[nodiscard]] bool isInitialized() const noexcept { return m_layouts != nullptr; }

        /// opcode table of a client expansion, the server expansion when the client version is unknown
        [[nodiscard]] const OpcodeTable& opcodes(WoW::Expansion expansion) const noexcept;
        [[nodiscard]] const OpcodeTable& opcodes(WoW::ClientProtocol const& protocol) const noexcept { return opcodes(protocol.expansion); }

        /// struct layouts bound to the server expansion
        [[nodiscard]] const ExpansionLayouts& layouts() const noexcept;

    private:
        static constexpr size_t kTableCount = 7;

        std::array<OpcodeTable, kTableCount> m_opcodeTables{};
        const ExpansionLayouts* m_layouts = nullptr;
    };
}

#define sVersionRegistry Version::Registry::getInstance()

namespace Version
{
    // Lookup helpers for the packet path, the client version decides which table answers

    /// internal id for a hex value received from a client of this expansion, MSG_NULL_ACTION when unknown
    [[nodiscard]] inline uint32_t opcodeIdForHex(uint16_t hex, WoW::Expansion expansion) noexcept
    {
        return sVersionRegistry.opcodes(expansion).internalIdFor(hex);
    }

    [[nodiscard]] inline uint32_t opcodeIdForHex(uint16_t hex, WoW::ClientProtocol const& protocol) noexcept
    {
        return opcodeIdForHex(hex, protocol.expansion);
    }

    /// hex value sent to a client of this expansion, 0 when the opcode does not exist there
    [[nodiscard]] inline uint16_t opcodeHexFor(uint32_t internalId, WoW::Expansion expansion) noexcept
    {
        return sVersionRegistry.opcodes(expansion).hexFor(internalId);
    }

    [[nodiscard]] inline uint16_t opcodeHexFor(uint32_t internalId, WoW::ClientProtocol const& protocol) noexcept
    {
        return opcodeHexFor(internalId, protocol.expansion);
    }

    /// readable name with the expansion tag, for logs
    [[nodiscard]] inline std::string opcodeNameForId(uint32_t internalId, WoW::Expansion expansion)
    {
        return fmt::format("{} [{}]", opcodeName(internalId), WoW::getShortExpansionName(expansion));
    }

    [[nodiscard]] inline std::string opcodeNameForHex(uint16_t hex, WoW::Expansion expansion)
    {
        return opcodeNameForId(opcodeIdForHex(hex, expansion), expansion);
    }

    [[nodiscard]] inline std::string opcodeNameForHex(uint16_t hex, WoW::ClientProtocol const& protocol)
    {
        return opcodeNameForHex(hex, protocol.expansion);
    }
}
