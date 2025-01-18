/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>

#include "Management/LFG/LFGGroupData.hpp"

/// Stores all lfg data needed about the player.
class LfgPlayerData
{
public:
    LfgPlayerData();
    ~LfgPlayerData() = default;

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
