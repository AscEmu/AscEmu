/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    // One visible (non-GM-invisible) participant's battleground score. Populated by Battleground,
    // which owns the m_isGmInvisible filtering and the BGScore lookup.
    struct PvpPlayerScoreEntry
    {
        uint64_t guid = 0;
        uint8_t bgTeam = 0;
        uint32_t killingBlows = 0;
        uint32_t honorableKills = 0;
        uint32_t deaths = 0;
        uint32_t bonusHonor = 0;
        uint32_t damageDone = 0;
        uint32_t healingDone = 0;
        uint32_t miscData[5] = { 0 };
    };

    // Populated by Battleground::buildPvPLogDataInput, which owns the protected m_hasEnded/
    // m_winningTeam/m_deltaRating fields and the arena-team/player-score resolution.
    struct PvpLogDataInput
    {
        bool isArena = false;
        bool hasEnded = false;
        bool rated = false;
        uint8_t winningTeam = 0;
        bool arenaTeamExists[2] = { false, false };
        uint32_t deltaRating[2] = { 0, 0 };
        std::vector<PvpPlayerScoreEntry> players;
        uint32_t fieldCount = 0;
    };
}
