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
        static constexpr size_t kTableCount = 5;

        std::array<OpcodeTable, kTableCount> m_opcodeTables{};
        const ExpansionLayouts* m_layouts = nullptr;
    };
}

#define sVersionRegistry Version::Registry::getInstance()
