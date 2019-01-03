/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Arenas.h"
#include "Management/Battleground/Battleground.h"

class CircleOfBlood : public Arena
{
public:

    CircleOfBlood(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t, uint32 players_per_side);
    ~CircleOfBlood();

    static CBattleground* Create(MapMgr* m, uint32 i, uint32 l, uint32 t, uint32 players_per_side)
    {
        return new CircleOfBlood(m, i, l, t, players_per_side);
    }

    void OnCreate();
    void HookOnShadowSight();
    LocationVector GetStartingCoords(uint32 Team);
    bool HookHandleRepop(Player *plr);
};
