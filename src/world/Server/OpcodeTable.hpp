/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//\NOTE:    This file is part of an attempt to replace version specific opcode files.
//          It works with internal indexes and maps version hex values to them. Do not
//          use or work with this file unless you are able to understand what is
//          happening here ;)

#pragma once

#include "Opcodes.hpp"
#include "AEVersion.hpp"
#include "CommonTypes.hpp"

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

    int getVersionIdForAEVersion()
    {
        switch (VERSION_STRING)
        {
            case Classic:
                return 0;
            case TBC:
                return 1;
            case WotLK:
                return 2;
            case Cata:
                return 3;
            case Mop:
                return 4;
            default:
                return 0;
        }
    }

    std::string getNameForAEVersion()
    {
        switch (VERSION_STRING)
        {
            case Classic:
                return "Classic";
            case TBC:
                return "BC";
            case WotLK:
                return "WotLK";
            case Cata:
                return "Cata";
            case Mop:
                return "Mop";
            default:
                return "";
        }
    }

    std::string getNameForVersionId(int versionId)
    {
        switch (versionId)
        {
            case 0:
                return "Classic";
            case 1:
                return "BC";
            case 2:
                return "WotLK";
            case 3:
                return "Cata";
            case 4:
                return "Mop";
            default:
                return "";
        }
    }

    uint32_t getInternalIdForHex(uint16_t hex, int versionId = -1)
    {
        if (versionId == -1 || versionId >= MAX_VERSION_INDEX)
            versionId = getVersionIdForAEVersion();

        for (const auto table : _versionHexTable[versionId])
            if (table.hexValue == hex)
                return table.internalId;

        return 0;
    }

    std::string getNameForOpcode(uint16_t hex, int versionId = -1)
    {
        const auto internalId = getInternalIdForHex(hex, versionId);

        auto multiversionTable = multiversionOpcodeStore.find(internalId);
        if (multiversionTable != multiversionOpcodeStore.end())
            return multiversionTable->second.name + " [" + getNameForAEVersion() + "]";

        return "Unknown internal id!";
    }

    std::string getNameForInternalId(uint32_t id)
    {
        auto multiversionTable = multiversionOpcodeStore.find(id);
        if (multiversionTable != multiversionOpcodeStore.end())
            return multiversionTable->second.name + " [" + getNameForAEVersion() + "]";

        return "Unknown internal id!";
    }

    uint16_t getHexValueForVersionId(int versionId, uint32_t internalId)
    {
        if (versionId >= 0 && versionId < MAX_VERSION_INDEX)
        {
            auto multiversionTable = multiversionOpcodeStore.find(internalId);
            if (multiversionTable != multiversionOpcodeStore.end())
                return multiversionTable->second.hexValues[versionId];
        }

        return 0;
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
