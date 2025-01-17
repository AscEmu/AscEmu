/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Management/Arenas.hpp"

class BattlegroundMap;

class DalaranSewers : public Arena
{
public:
    DalaranSewers(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t, uint32_t players_per_side);
    ~DalaranSewers();

    static Battleground* Create(BattlegroundMap* m, uint32_t i, uint32_t l, uint32_t t, uint32_t players_per_side)
    {
        return new DalaranSewers(m, i, l, t, players_per_side);
    }

    void OnCreate() override;
    LocationVector GetStartingCoords(uint32_t Team) override;
    void HookOnAreaTrigger(Player* plr, uint32_t trigger) override;
    bool HookHandleRepop(Player* plr) override;
};
