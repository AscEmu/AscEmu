/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

/// -
#define MAX_ITEM_PROTO_DAMAGES 2

/// -
#define MAX_ITEM_PROTO_SOCKETS 3

/// -
#define MAX_ITEM_PROTO_SPELLS  5

/// -
#define MAX_ITEM_PROTO_STATS  10


/// -
#define RANDOM_SUFFIX_MAGIC_CALCULATION(__suffix, __scale) Util::float2int32(float(__suffix) * float(__scale) / 10000.0f);
