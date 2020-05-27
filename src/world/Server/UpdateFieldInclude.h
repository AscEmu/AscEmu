/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../world/WorldConf.h"
#include <stddef.h>

#define getOffsetForStructuredField(s,m) static_cast<uint32_t>(offsetof(s,m) / 4)

#define getSizeOfStructure(s) static_cast<uint32_t>(sizeof(s) / 4)

#define getSizeOfStructuredField(s,m)

