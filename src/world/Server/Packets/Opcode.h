/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../world/WorldConf.h"

//\todo include version opcodes here.... include WotLK opcodes for now.
//#ifdef AE_CLASSIC
//#include "../world/GameClassic/Network/Opcodes.h"
//#endif
//#ifdef AE_TBC
//#include "../world/GameTBC/Network/Opcodes.h"
//#endif
//#ifdef AE_WOTLK
#include "../world/GameWotLK/Network/Opcodes.h"
//#endif
//#ifdef AE_CATA
//#include "../world/GameCata/Network/Opcodes.h"
//#endif

#include <cstdint>
#include <string>

struct OpcodeTable
{
    uint32_t opcode;
    std::string name;
};

extern OpcodeTable opcodeNameArray[];

static inline std::string getOpcodeName(uint32_t opcode)
{
    for (uint32_t i = 0; i < NUM_MSG_TYPES; ++i)
    {
        if (opcodeNameArray[i].opcode == opcode)
            return opcodeNameArray[i].name;
    }

    return "UNKNOWN_OPCODE";
}
