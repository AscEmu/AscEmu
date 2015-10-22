/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _OBSIDIAN_SANCTUM_H
#define _OBSIDIAN_SANCTUM_H

enum CreatureEntry
{
    CN_SARTHARION       = 28860,
    CN_FLAME_TSUNAMI    = 30616,
    CN_LAVA_BLAZE       = 30643,
    CN_CYCLON           = 30648,
    CN_DRAKE_TENEBRON   = 30452,
    CN_DRAKE_VESPERON   = 30449,
    CN_DRAKE_SHADRON    = 30451
};

enum CreatureSpells
{
    // Sartharion
    SARTHARION_CLEAVE   = 56909,
    SARTHARION_ENRAGE   = 61632,
    SARTHARION_AURA     = 61254,
    // Tsunami spells
    TSUNAMI             = 57492,
    TSUNAMI_VISUAL      = 57494,
    // Cyclon spells
    CYCLON_AURA         = 57562,
    CYCLON_SPELL        = 57560,
    ///\todo  add drake spells
    SHADRON_AURA        = 58105,
    TENEBRON_AURA       = 61248,
    VESPERON_AURA       = 61251
};

enum Sarathion_Data
{
    DRAKE_TENEBRON,
    DRAKE_VESPERON,
    DRAKE_SHADRON,
    BOSS_SARTHARION,

    OS_DATA_END = 4
};

static Location TSUNAMI_SPAWN[] =
{
    // Right
    { 3283.215820f, 511.234100f, 59.288776f, 3.148659f },
    { 3286.661133f, 533.317261f, 59.366989f, 3.156505f },
    { 3283.311035f, 556.839611f, 59.397129f, 3.105450f },
    // Left
    { 3211.564697f, 505.982727f, 59.556610f, 0.000000f },
    { 3214.280029f, 531.491089f, 59.168331f, 0.000000f },
    { 3211.609131f, 560.359375f, 59.420803f, 0.000000f },
};

static Location TSUNAMI_MOVE[] =
{
    // Left  if right
    { 3211.564697f, 505.982727f, 59.556610f, 3.148659f },
    { 3214.280029f, 531.491089f, 59.168331f, 3.156505f },
    { 3211.609131f, 560.359375f, 59.420803f, 3.105450f },
    // Right 1 if left 1
    { 3283.215820f, 511.234100f, 59.288776f, 3.148659f },
    { 3286.661133f, 533.317261f, 59.366989f, 3.156505f },
    { 3283.311035f, 556.839611f, 59.397129f, 3.105450f }
};

#endif  // _OBSIDIAN_SANCTUM_H
