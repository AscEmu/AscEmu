/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "VersionRegistry.hpp"

#include "Logging/Logger.hpp"

namespace Version
{
    Registry& Registry::getInstance()
    {
        static Registry mInstance;
        return mInstance;
    }

    void Registry::initialize()
    {
        static constexpr WoW::Expansion supported[kTableCount] =
        {
            WoW::Expansion::_Classic,
            WoW::Expansion::_TBC,
            WoW::Expansion::_WotLK,
            WoW::Expansion::_Cata,
            WoW::Expansion::_Mop,
        };

        size_t opcodeCount = 0;
        for (size_t i = 0; i < kTableCount; ++i)
        {
            m_opcodeTables[i].build(supported[i]);
            opcodeCount += m_opcodeTables[i].size();
        }

        const auto serverExpansion = WoW::getServerExpansion();
        m_layouts = &layoutsForExpansion(serverExpansion);

        sLogger.info("Version::Registry : {} opcodes in {} version tables, struct layouts bound to {} ({} unit values, {} player values).",
            opcodeCount, kTableCount, WoW::getExpansionName(serverExpansion), m_layouts->unit.valueCount(), m_layouts->player.valueCount());
    }

    void Registry::finalize()
    {
        for (auto& table : m_opcodeTables)
            table.clear();

        m_layouts = nullptr;
    }

    const OpcodeTable& Registry::opcodes(WoW::Expansion expansion) const noexcept
    {
        auto index = WoW::getOpcodeTableIndex(expansion);
        if (index < 0 || static_cast<size_t>(index) >= kTableCount)
            index = WoW::getOpcodeTableIndex(WoW::getServerExpansion());

        if (index < 0 || static_cast<size_t>(index) >= kTableCount)
            index = 0;

        return m_opcodeTables[static_cast<size_t>(index)];
    }

    const ExpansionLayouts& Registry::layouts() const noexcept
    {
        return m_layouts != nullptr ? *m_layouts : layoutsForExpansion(WoW::getServerExpansion());
    }

    const ExpansionLayouts& layouts() noexcept
    {
        return sVersionRegistry.layouts();
    }
}
