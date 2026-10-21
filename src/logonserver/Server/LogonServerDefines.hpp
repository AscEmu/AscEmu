/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Config/Config.hpp"
#include <cstdint>
#include "Platform/SymbolVisibility.hpp"

// expansion bits (flags column of accounts) of a new account: the logonserver serves every client version, so a
// new account may enter every expansion the world servers check (TBC 0x08, WotLK 0x10, Cata 0x20, Mop 0x40);
// a world server only looks at the bits of its own expansion
inline constexpr uint32_t NEW_ACCOUNT_EXPANSION_FLAGS = 0x78;

struct AllowedIP
{
    unsigned int IP;
    unsigned char Bytes;
};

class SERVER_DECL ConfigMgr
{
public:
    ConfigFile MainConfig;
};

inline ConfigMgr Config;
