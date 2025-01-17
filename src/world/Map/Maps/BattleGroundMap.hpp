/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldMap.hpp"

#include <cstdint>

class Battleground;
class Player;

class SERVER_DECL BattlegroundMap : public WorldMap
{
public:
    BattlegroundMap(BaseMap* baseMap, uint32_t id, uint32_t expiryTime, uint32_t InstanceId, uint8_t SpawnMode);
    ~BattlegroundMap();

    void update(uint32_t) override;

    bool addPlayerToMap(Player*) override;
    void removePlayerFromMap(Player*) override;
    EnterState cannotEnter(Player* player) override;
    void setUnload();

    virtual void initVisibilityDistance() override;

    Battleground* getBattleground() { return m_battleground; }
    void setBattleground(Battleground* bg) { m_battleground = bg; }

private:
    Battleground* m_battleground;
};
