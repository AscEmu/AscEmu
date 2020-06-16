/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
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

#define _CRT_SECURE_NO_DEPRECATE

#include "adt.h"
#include <cstring>

// Helper
int holetab_h[4] = { 0x1111, 0x2222, 0x4444, 0x8888 };
int holetab_v[4] = { 0x000F, 0x00F0, 0x0F00, 0xF000 };

u_map_fcc MHDRMagic = { { 'R', 'D', 'H', 'M' } };
u_map_fcc MCINMagic = { { 'N', 'I', 'C', 'M' } };
u_map_fcc MH2OMagic = { { 'O', '2', 'H', 'M' } };
u_map_fcc MCNKMagic = { { 'K', 'N', 'C', 'M' } };
u_map_fcc MCVTMagic = { { 'T', 'V', 'C', 'M' } };
u_map_fcc MCLQMagic = { { 'Q', 'L', 'C', 'M' } };

bool isHole(int holes, int i, int j)
{
    int testi = i / 2;
    int testj = j / 4;
    if (testi > 3) testi = 3;
    if (testj > 3) testj = 3;
    return (holes & holetab_h[testi] & holetab_v[testj]) != 0;
}
