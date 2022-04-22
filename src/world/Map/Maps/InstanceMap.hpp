/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include "InstanceMgr.hpp"
#include "InstanceDefines.hpp"
#include "WorldMap.hpp"

class SERVER_DECL InstanceMap : public WorldMap
{
public:
    InstanceMap(BaseMap* baseMap, uint32_t id, uint32_t expiryTime, uint32_t InstanceId, uint8_t SpawnMode, PlayerTeam InstanceTeam);
    ~InstanceMap() = default;

    void update(uint32_t) override;
    void unloadAll() override;

    void createInstanceData(bool load);

    virtual bool addPlayerToMap(Player*) override;
    virtual void removePlayerFromMap(Player*) override;

    virtual void initVisibilityDistance() override;
    EnterState cannotEnter(Player* player) override;

    void permBindAllPlayers();
    void sendResetWarnings(uint32_t timeLeft);
    void setResetSchedule(bool on);

    bool reset(uint8_t method);
    bool hasPermBoundPlayers();
    uint32_t getMaxPlayers();

    PlayerTeam getTeamIdInInstance() { return instanceTeam; }
    uint32_t getTeamInInstance() { return instanceTeam == TEAM_ALLIANCE ? ALLIANCE : HORDE; }

private:
    bool m_resetAfterUnload = false;
    bool m_unloadWhenEmpty = false;

    PlayerTeam instanceTeam;
};