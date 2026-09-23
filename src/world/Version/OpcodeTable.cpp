/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "OpcodeTable.hpp"

namespace Version
{
    const OpcodeSource* opcodeSourceFor(WoW::Expansion expansion) noexcept
    {
        switch (expansion)
        {
            case WoW::Expansion::_Classic: return &Tables::classicOpcodes;
            case WoW::Expansion::_TBC:     return &Tables::tbcOpcodes;
            case WoW::Expansion::_WotLK:   return &Tables::wotlkOpcodes;
            case WoW::Expansion::_Cata:    return &Tables::cataOpcodes;
            case WoW::Expansion::_Mop:     return &Tables::mopOpcodes;
            default:                       return nullptr;
        }
    }

    void OpcodeTable::build(WoW::Expansion expansion)
    {
        clear();
        m_expansion = expansion;

        const OpcodeSource* source = opcodeSourceFor(expansion);
        if (source == nullptr)
            return;

        m_idToHex.assign(NUM_OPCODES, 0);
        m_hexToId.reserve(source->count);

        for (size_t i = 0; i < source->count; ++i)
        {
            const OpcodeEntry& entry = source->entries[i];
            const auto internalId = static_cast<uint32_t>(entry.internalId);
            if (internalId >= NUM_OPCODES || entry.hex == 0)
                continue;

            m_idToHex[internalId] = entry.hex;

            // several internal ids can share one hex value (e.g. MoP 0x1061 = CLEAR_TARGET + OBJECT_UPDATE_FAILED).
            // Incoming packets come from the client, so a CMSG wins over a SMSG, otherwise the lower id wins.
            const auto existing = m_hexToId.find(entry.hex);
            if (existing == m_hexToId.end())
            {
                m_hexToId.emplace(entry.hex, internalId);
                continue;
            }

            const bool newIsCmsg = opcodeName(internalId).starts_with("CMSG");
            const bool oldIsCmsg = opcodeName(existing->second).starts_with("CMSG");
            if ((newIsCmsg && !oldIsCmsg) || (newIsCmsg == oldIsCmsg && internalId < existing->second))
                existing->second = internalId;
        }
    }

    void OpcodeTable::clear()
    {
        m_expansion = WoW::Expansion::Unknown;
        m_idToHex.clear();
        m_hexToId.clear();
    }
}
