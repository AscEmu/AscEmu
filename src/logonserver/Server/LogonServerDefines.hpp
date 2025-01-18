/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Config/Config.h"

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

extern SERVER_DECL ConfigMgr Config;
