/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Management/Arenas.hpp"

class BattlegroundMap;

class RingOfTrials : public Arena
{
public:
    RingOfTrials(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t, uint32_t players_per_side);
    ~RingOfTrials();

    static Battleground* Create(BattlegroundMap* m, uint32_t i, uint32_t l, uint32_t t, uint32_t players_per_side)
    {
        return new RingOfTrials(m, i, l, t, players_per_side);
    }

    void OnCreate() override;
    void HookOnShadowSight() override;
    LocationVector GetStartingCoords(uint32_t Team) override;
    bool HookHandleRepop(Player* plr) override;
};
