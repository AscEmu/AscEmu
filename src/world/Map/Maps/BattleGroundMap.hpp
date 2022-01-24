/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include "InstanceMgr.hpp"
#include "InstanceDefines.hpp"
#include "WorldMap.hpp"

class CBattleground;

class SERVER_DECL BattlegroundMap : public WorldMap
{
public:
    BattlegroundMap(BaseMap* baseMap, uint32_t id, time_t, uint32_t InstanceId, uint8_t SpawnMode);
    ~BattlegroundMap();

    void update(uint32_t) override;

    bool addPlayerToMap(Player*) override;
    void removePlayerFromMap(Player*) override;
    EnterState cannotEnter(Player* player) override;
    void setUnload();
    void removeAllPlayers() override;

    virtual void initVisibilityDistance() override;

    CBattleground* getBattleground() { return m_battleground; }
    void setBattleground(CBattleground* bg) { m_battleground = bg; }

private:
    CBattleground* m_battleground;
};