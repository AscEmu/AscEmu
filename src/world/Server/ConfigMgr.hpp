/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Config/Config.h"

class SERVER_DECL ConfigMgr
{
public:
    ConfigFile MainConfig;
    ConfigFile ClusterConfig;
};

extern SERVER_DECL ConfigMgr Config;
