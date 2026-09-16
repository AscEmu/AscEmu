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


    uint32_t getInternalIdForHex(uint16_t hex, WoW::ClientProtocol const& protocol) { return getInternalIdForHex(hex, protocol.versionId()); }

    uint32_t getInternalIdForHex(uint16_t hex, int versionId)
    {
        if (versionId < 0 || versionId >= MAX_VERSION_INDEX)
            return 0;

        uint32_t firstMatch = 0;

        for (const auto table : _versionHexTable[versionId])
        {
            if (table.hexValue != hex)
                continue;
            if (firstMatch == 0)
                firstMatch = table.internalId;
            // For incoming client packets, prefer CMSG over SMSG when hex collides (e.g. MoP 0x1061 = CLEAR_TARGET + OBJECT_UPDATE_FAILED)
            auto it = multiversionOpcodeStore.find(table.internalId);
            if (it != multiversionOpcodeStore.end() && it->second.name.size() >= 4 &&
                it->second.name[0] == 'C' && it->second.name[1] == 'M' && it->second.name[2] == 'S' && it->second.name[3] == 'G')
                return table.internalId;
        }

        return firstMatch;
    }

    std::string getNameForOpcode(uint16_t hex, WoW::ClientProtocol const& protocol) { return getNameForOpcode(hex, protocol.versionId()); }

    std::string getNameForOpcode(uint16_t hex, int versionId)
    {
        return getNameForInternalId(getInternalIdForHex(hex, versionId), versionId);
    }

    std::string getNameForInternalId(uint32_t id, WoW::ClientProtocol const& protocol) { return getNameForInternalId(id, protocol.versionId()); }

    std::string getNameForInternalId(uint32_t id, int versionId)
    {
        auto multiversionTable = multiversionOpcodeStore.find(id);
        if (multiversionTable != multiversionOpcodeStore.end())
            return multiversionTable->second.name + " [" + std::string(WoW::getNameForVersionId(versionId)) + "]";

        return "Unknown internal id!";
    }

    uint16_t getHexValueForVersionId(uint32_t internalId, WoW::ClientProtocol const& protocol) { return getHexValueForVersionId(internalId, protocol.versionId()); }

    uint16_t getHexValueForVersionId(uint32_t internalId, int versionId)
    {
        if (versionId >= 0 && versionId < MAX_VERSION_INDEX)
        {
            auto multiversionTable = multiversionOpcodeStore.find(internalId);
            if (multiversionTable != multiversionOpcodeStore.end())
                return multiversionTable->second.hexValues[versionId];
        }

        return 0;
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
            hexValue(hex), internalId(intId){}

        uint16_t hexValue;
        uint32_t internalId;
    };

    std::vector<HexToId> _versionHexTable[MAX_VERSION_INDEX] = {};
};

#define sOpcodeTables OpcodeTables::getInstance()
