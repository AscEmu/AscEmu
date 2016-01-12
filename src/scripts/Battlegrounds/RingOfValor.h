/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

#ifndef _RING_OF_VALOR_H
#define _RING_OF_VALOR_H

#include "StdAfx.h"

class RingOfValor : public Arena
{
    public:

        RingOfValor(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t, uint32 players_per_side);
        ~RingOfValor();

        static CBattleground* Create(MapMgr* m, uint32 i, uint32 l, uint32 t, uint32 players_per_side)
        {
            return new RingOfValor(m, i, l, t, players_per_side);
        }

        void OnCreate();
        LocationVector GetStartingCoords(uint32 Team);
        void HookOnAreaTrigger(Player* plr, uint32 trigger);
        bool HookHandleRepop(Player* plr);
};

#endif  // _RING_OF_VALOR_H
