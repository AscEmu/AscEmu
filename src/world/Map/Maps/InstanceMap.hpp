/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "InstanceDefines.hpp"
#include "WorldMap.hpp"

#include <cstdint>

enum PlayerTeam : uint8_t;
class Player;

class SERVER_DECL InstanceMap : public WorldMap
{
public:
    InstanceMap(BaseMap* baseMap, uint32_t id, uint32_t expiryTime, uint32_t InstanceId, uint8_t SpawnMode, PlayerTeam InstanceTeam);
    ~InstanceMap() = default;

    void update(uint32_t) override;
    void unloadAll(bool onShutdown = false) override;

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

    PlayerTeam getTeamIdInInstance();
    uint32_t getTeamInInstance();

private:
    bool m_resetAfterUnload = false;
    bool m_unloadWhenEmpty = false;

    PlayerTeam instanceTeam;
};
