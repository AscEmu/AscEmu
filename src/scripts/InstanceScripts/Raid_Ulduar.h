/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
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


#ifndef _ULDUAR_H
#define _ULDUAR_H

enum Ulduar_Encounters
{
    CN_FLAME_LEVIATHAN = 33113,
    CN_IGNIS_THE_FURNANCE_MASTER = 33118,
    CN_RAZORSCALE = 33186,
    CN_XT_002_DECONSTRUCTOR = 33293,
    CN_KOLOGARN = 32930,
    CN_AURIAYA = 33515,
    CN_FREYA = 32906,
    CN_HODIR = 32845,
    CN_MIMIRON = 33350,
    CN_THORIM = 32865,
    CN_GENERAL_VEZAX = 33271
};

void SetupUlduar(ScriptMgr* mgr);

#endif  // _ULDUAR_H
