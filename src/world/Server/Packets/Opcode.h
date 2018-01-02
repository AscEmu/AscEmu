/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../world/WorldConf.h"

#if VERSION_STRING == Classic
    #include "../world/GameClassic/Network/Opcodes.h"
#elif VERSION_STRING == TBC
    #include "../world/GameTBC/Network/Opcodes.h"
#elif VERSION_STRING == WotLK
    #include "../world/GameWotLK/Network/Opcodes.h"
#elif VERSION_STRING == Cata
    #include "../world/GameCata/Network/Opcodes.h"
#endif

#include <cstdint>
#include <string>

struct OpcodeTable
{
    uint16_t opcode;
    std::string name;
};

extern OpcodeTable opcodeNameArray[];

static inline std::string getOpcodeName(uint16_t opcode)
{
    for (uint16_t i = 0; i < NUM_MSG_TYPES; ++i)
    {
        if (opcodeNameArray[i].opcode == opcode)
            return opcodeNameArray[i].name;
    }

    return "UNKNOWN_OPCODE";
}
