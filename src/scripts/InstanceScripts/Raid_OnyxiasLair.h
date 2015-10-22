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


#ifndef _ONYXIAS_LAIR_H
#define _ONYXIAS_LAIR_H

enum CreatureEntry
{
    CN_ONYXIA = 10184
};

enum CreatureSpells
{
    //Phase 1,3 Spells
    FLAME_BREATH        = 18435,    //Corrected http://www.wowhead.com/?spell=18435
    KNOCK_AWAY          = 19633,    //Reduce thread script effect main target
    WING_BUFFET         = 18500,
    CLEAVE              = 68868,    //15579,16044,19642,29832
    TAIL_SWEEP          = 68867,
    //Phase 2 Spells
    SCRIPTABLE_FIREBALL = 18392,    //Corrected http://www.wowhead.com/?spell=18392
    //Script it
    ENTANGLING_FLAMES   = 20019,
    //Onyxia's Breath (Deep Breath)
    DEEP_BREATH         = 17086,
    //Phase 3 Spells
    AOE_FEAR            = 18431     //With Activate Object
};

static Location coords[] =
{
    { 0, 0, 0, 0 },
    { -75.945f, -219.245f, -83.375f, 0.004947f },
    { -72.945f, -219.245f, -80.779f, 0.004947f },
    { 42.621f, -217.195f, -66.056f, 3.014011f },
    { 12.270f, -254.694f, -67.997f, 2.395585f },
    { -79.020f, -252.374f, -68.965f, 0.885179f },
    { -80.257f, -174.240f, -69.293f, 5.695741f },
    { 27.875f, -178.547f, -66.041f, 3.908957f },
    { -4.868f, -217.171f, -86.710f, M_PI_FLOAT }
};

static Location whelpCoords[] =
{
    { -30.812f, -166.395f, -89.000f, 5.160f },
    { -30.233f, -264.158f, -89.896f, 1.129f },
    { -35.813f, -169.427f, -90.000f, 5.384f },
    { -36.104f, -260.961f, -90.600f, 1.111f },
    { -34.643f, -164.080f, -90.000f, 5.364f },
    { -35.377f, -267.320f, -91.000f, 1.111f }
};

#endif  // _ONYXIAS_LAIR_H
