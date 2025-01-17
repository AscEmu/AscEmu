/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//\NOTE:    This file is part of an attempt to replace version specific opcode files.
//          It works with internal indexes and maps version hex values to them. Do not
//          use or work with this file unless you are able to understand what is
//          happening here ;)

#include <iostream>

#include "OpcodeTable.hpp"

OpcodeTables& OpcodeTables::getInstance()
{
    static OpcodeTables mInstance;
    return mInstance;
}

void OpcodeTables::initialize()
{
    std::cout << "OpcodeTables preparing version specific tables." << "\n";

    uint32_t valueCount = 0;
    // fill vector
    for (const auto opcodeStore : multiversionOpcodeStore)
    {   
        for (auto hexIndex = 0; hexIndex < MAX_VERSION_INDEX; ++hexIndex)
        {
            _versionHexTable[hexIndex].emplace_back(opcodeStore.second.hexValues[hexIndex], opcodeStore.first);
            ++valueCount;
        }
    }
    std::cout << "OpcodeTables prepared " << valueCount << " hexvalues for 5 version" << "\n";
}

void OpcodeTables::finalize()
{
    for (auto hexIndex = 0; hexIndex < MAX_VERSION_INDEX; ++hexIndex)
        _versionHexTable[hexIndex].clear();
}
