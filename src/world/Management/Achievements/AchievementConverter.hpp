/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace Achievements
{
    // characters (guid, account) stored with the criteria of the builds before 5.4 get the criteria ids of the trees once
    void convertCharacters(std::vector<std::pair<uint32_t, uint32_t>> const& characters);

    // the account wide achievements of the converted characters, once the logonserver accepted the realm
    void sendPendingConversions();
}
