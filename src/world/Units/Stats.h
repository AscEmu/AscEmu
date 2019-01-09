/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
 *
 */

#ifndef STATS_H
#define STATS_H

#include "Units/Unit.h"
#include "Server/UpdateMask.h"
#include "Management/ItemInterface.h"

enum Stats
{
    STAT_STRENGTH,
    STAT_AGILITY,
    STAT_STAMINA,
    STAT_INTELLECT,
    STAT_SPIRIT,
    STAT_COUNT
};

SERVER_DECL uint32 getConColor(uint16 AttackerLvl, uint16 VictimLvl);
SERVER_DECL uint32 CalculateXpToGive(Unit* pVictim, Unit* pAttacker);
SERVER_DECL uint32 CalculateStat(uint16 level, double a3, double a2, double a1, double a0);
SERVER_DECL uint32 CalculateDamage(Unit* pAttacker, Unit* pVictim, uint32 weapon_damage_type, const uint32* spellgroup, SpellInfo const* ability);
SERVER_DECL uint32 GainStat(uint16 level, uint8 playerclass, uint8 Stat);
SERVER_DECL bool isEven(int num);

#endif      //STATS_H
