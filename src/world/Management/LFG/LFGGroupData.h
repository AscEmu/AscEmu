/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "LFG.h"

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
        ~LfgGroupData();

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
        LfgState m_State;       ///< State if group in LFG
        LfgState m_OldState;    ///< Old State
        // Dungeon
        uint32_t m_Dungeon;       ///< Dungeon entry
        // Vote Kick
        uint8_t m_VotesNeeded;    ///< Votes need to kick success
        uint8_t m_KicksLeft;      ///< Number of kicks left
};
