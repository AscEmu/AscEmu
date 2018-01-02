/**
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

#include "Management/LFG/LFGGroupData.h"

/// Stores all lfg data needed about the player.
class LfgPlayerData
{
    public:

        LfgPlayerData();
        ~LfgPlayerData();

        // General
        void SetState(LfgState state);
        void ClearState();
        void SetLockedDungeons(const LfgLockMap& lock);
        // Queue
        void SetRoles(uint8_t roles);
        void SetComment(const std::string& comment);
        void SetSelectedDungeons(const LfgDungeonSet& dungeons);
        void ClearSelectedDungeons();

        // General
        LfgState GetState() const;
        const LfgLockMap& GetLockedDungeons() const;
        // Queue
        uint8_t GetRoles() const;
        const std::string& GetComment() const;
        const LfgDungeonSet& GetSelectedDungeons() const;

    private:

        // General
        LfgState m_State;                                  ///< State if group in LFG
        LfgState m_OldState;                               ///< Old State
        // Player
        LfgLockMap m_LockedDungeons;                       ///< Dungeons player can't do and reason
        // Queue
        uint8_t m_Roles;                                   ///< Roles the player selected when joined LFG
        std::string m_Comment;                             ///< Player comment used when joined LFG
        LfgDungeonSet m_SelectedDungeons;                  ///< Selected Dungeons when joined LFG
};
