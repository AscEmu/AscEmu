/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org/>
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

#ifndef _RUINS_OF_LORDAERON_H
#define _RUINS_OF_LORDAERON_H

#include "StdAfx.h"

class RuinsOfLordaeron : public Arena
{
    public:

        RuinsOfLordaeron(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t, uint32 players_per_side);
        ~RuinsOfLordaeron();

        static CBattleground* Create(MapMgr* m, uint32 i, uint32 l, uint32 t, uint32 players_per_side)
        {
            return new RuinsOfLordaeron(m, i, l, t, players_per_side);
        }

        void OnCreate();
        void HookOnShadowSight();
        LocationVector GetStartingCoords(uint32 Team);
        void HookOnAreaTrigger(Player* plr, uint32 trigger);
        bool HookHandleRepop(Player* plr);
};

#endif  // _RUINS_OF_LORDAERON_H
