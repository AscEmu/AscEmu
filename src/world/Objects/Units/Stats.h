/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "CommonTypes.hpp"
#include <cstdint>

class Unit;
class SpellInfo;

enum Stats
{
    STAT_STRENGTH,
    STAT_AGILITY,
    STAT_STAMINA,
    STAT_INTELLECT,
    STAT_SPIRIT,
    STAT_COUNT
};

// APGL End
// MIT Start

SERVER_DECL bool isGrayLevel(uint32_t attackerLevel, uint32_t victimLevel);

// MIT End
// APGL Start

SERVER_DECL uint32_t getConColor(uint16_t AttackerLvl, uint16_t VictimLvl);
SERVER_DECL uint32_t CalculateXpToGive(Unit* pVictim, Unit* pAttacker);
SERVER_DECL uint32_t CalculateStat(uint16_t level, double a3, double a2, double a1, double a0);
SERVER_DECL uint32_t CalculateDamage(Unit* pAttacker, Unit* pVictim, uint32_t weapon_damage_type, const uint32_t* spellgroup, SpellInfo const* ability);
SERVER_DECL uint32_t GainStat(uint16_t level, uint8_t playerclass, uint8_t Stat);
SERVER_DECL bool isEven(int num);

#endif      //STATS_H
