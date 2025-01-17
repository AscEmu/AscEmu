/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LFG.hpp"

enum LfgGroupEnum
{
    LFG_GROUP_MAX_KICKS         = 3,
    LFG_GROUP_KICK_VOTES_NEEDED = 3
};

/// Stores all lfg data needed about a group.
class LfgGroupData
{
public:
    LfgGroupData();
    ~LfgGroupData() = default;

    // General
    void SetState(LfgState state);
    void RestoreState();
    // Dungeon
    void SetDungeon(uint32_t dungeon);
    void DecreaseKicksLeft();

    // General
    LfgState GetState() const;
    // Dungeon
    uint32_t GetDungeon(bool asId = true) const;
    // VoteKick
    uint8_t GetVotesNeeded() const;
    uint8_t GetKicksLeft() const;

private:
    // General
    LfgState m_State;         ///< State if group in LFG
    LfgState m_OldState;      ///< Old State
    // Dungeon
    uint32_t m_Dungeon;       ///< Dungeon entry
    // Vote Kick
    uint8_t m_VotesNeeded;    ///< Votes need to kick success
    uint8_t m_KicksLeft;      ///< Number of kicks left
};
