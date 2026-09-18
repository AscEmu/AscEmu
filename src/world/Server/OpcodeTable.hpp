/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//\NOTE:    This file is part of an attempt to replace version specific opcode files.
//          It works with internal indexes and maps version hex values to them. Do not
//          use or work with this file unless you are able to understand what is
//          happening here ;)

#pragma once

#include "Opcodes.hpp"
#include "ClientProtocol.hpp"
#include "Platform/SymbolVisibility.hpp"

#include <cstdint>
#include <string>
#include <vector>

class SERVER_DECL OpcodeTables
{
private:
    OpcodeTables() = default;
    ~OpcodeTables() = default;

public:
    static OpcodeTables& getInstance();
    void initialize();
    void finalize();

    OpcodeTables(OpcodeTables&&) = delete;
    OpcodeTables(OpcodeTables const&) = delete;
    OpcodeTables& operator=(OpcodeTables&&) = delete;
    OpcodeTables& operator=(OpcodeTables const&) = delete;

    // Internal ID Lookups

    [[nodiscard]] uint32_t getInternalIdForHex(uint16_t const hex, int32_t const versionId) const
    {
        if (versionId < 0 || versionId >= MAX_VERSION_INDEX)
        {
            return 0;
        }

        uint32_t firstMatch = 0;

        for (const auto& entry : _versionHexTable[versionId])
        {
            if (entry.hexValue != hex)
            {
                continue;
            }

            if (firstMatch == 0)
            {
                firstMatch = entry.internalId;
            }

            // For incoming client packets, prefer CMSG over SMSG when hex collides (e.g. MoP 0x1061 = CLEAR_TARGET + OBJECT_UPDATE_FAILED)
            const auto it = multiversionOpcodeStore.find(entry.internalId);
            if (it != multiversionOpcodeStore.end() && it->second.name.starts_with("CMSG"))
            {
                return entry.internalId;
            }
        }

        return firstMatch;
    }

    [[nodiscard]] uint32_t getInternalIdForHex(uint16_t const hex, WoW::Expansion const expansion) const
    {
        return getInternalIdForHex(hex, WoW::getOpcodeTableIndex(expansion));
    }

    [[nodiscard]] uint32_t getInternalIdForHex(uint16_t const hex, WoW::ClientProtocol const& protocol) const
    {
        return getInternalIdForHex(hex, protocol.expansion);
    }

    // Opcode name lookups (hex -> name)

    [[nodiscard]] std::string getNameForOpcode(uint16_t const hex, WoW::Expansion const expansion) const
    {
        return getNameForInternalId(getInternalIdForHex(hex, expansion), expansion);
    }

    [[nodiscard]] std::string getNameForOpcode(uint32_t const opcode, WoW::Expansion const expansion) const
    {
        return getNameForOpcode(static_cast<uint16_t>(opcode), expansion);
    }

    [[nodiscard]] std::string getNameForOpcode(uint16_t const hex, WoW::ClientProtocol const& protocol) const
    {
        return getNameForOpcode(hex, protocol.expansion);
    }

    // Internal ID name lookups (Internal ID -> name)

    [[nodiscard]] std::string getNameForInternalId(uint32_t const id, WoW::Expansion const expansion) const
    {
        const auto it = multiversionOpcodeStore.find(id);
        if (it != multiversionOpcodeStore.end())
        {
            return fmt::format("{} [{}]", it->second.name, WoW::getShortExpansionName(expansion));
        }

        return "Unknown internal id!";
    }

    [[nodiscard]] std::string getNameForInternalId(uint32_t const id, WoW::ClientProtocol const& protocol) const
    {
        return getNameForInternalId(id, protocol.expansion);
    }

    // Hex value lookups (Internal ID -> hex)

    [[nodiscard]] uint16_t getHexValueForExpansion(uint32_t const internalId, WoW::Expansion const expansion) const
    {
        const int32_t tableIndex = WoW::getOpcodeTableIndex(expansion);
        if (tableIndex >= 0 && tableIndex < MAX_VERSION_INDEX)
        {
            const auto it = multiversionOpcodeStore.find(internalId);
            if (it != multiversionOpcodeStore.end())
            {
                return it->second.hexValues[tableIndex];
            }
        }

        return 0;
    }

    [[nodiscard]] uint16_t getHexValueForExpansion(uint32_t const internalId, WoW::ClientProtocol const& protocol) const
    {
        return getHexValueForExpansion(internalId, protocol.expansion);
    }

    OpcodeDevelopmentState getStateForInternalId(uint32_t internalId)
    {
        const auto it = multiversionOpcodeStore.find(internalId);
        if (it != multiversionOpcodeStore.end())
            return it->second.state;

        return OpcodeDevelopmentState::Unchecked;
    }

    struct HexToId
    {
        HexToId(uint16_t hex, uint32_t intId) :
            hexValue(hex), internalId(intId) {}

        uint16_t hexValue;
        uint32_t internalId;
    };

    std::vector<HexToId> _versionHexTable[MAX_VERSION_INDEX] = {};
};

#define sOpcodeTables OpcodeTables::getInstance()
