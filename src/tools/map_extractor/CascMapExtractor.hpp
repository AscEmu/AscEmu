/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "mpqlib/CascStorage.hpp"
#include "mpqlib/ClientVersion.hpp"

#include <cstdint>

// Converts every tile of every map of a CASC client (6.x and later) into the
// maps/ directory, the same .map format the MPQ clients produce.
bool extractMapsFromCasc(mpqlib::CascStorage const& storage, mpqlib::ClientVersion version, uint32_t build);
