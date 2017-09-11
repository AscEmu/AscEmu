/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"

#pragma pack(push, 1)
struct LogonWorldPacket
{
    uint16_t opcode;
    uint32_t size;
};
#pragma pack(pop)


static void byteSwapUInt16(uint16_t* byte16)
{
    *byte16 = ((*byte16 >> 8) & 0xFF00) | ((*byte16 << 8) & 0xFF0000);
}

static void byteSwapUInt32(uint32_t* byte32)
{
    *byte32 = ((*byte32 >> 24) & 0xFF) | ((*byte32 >> 8) & 0xFF00) | ((*byte32 << 8) & 0xFF0000) | (*byte32 << 24);
}

