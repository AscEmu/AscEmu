/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

#include "Setup.h"

//////////////////////////////////////////////////////////////////////////////////////////
///\details <b>Leaders at Jin'Alai</b>\n
/// event_names entry: NA \n
/// event_names holiday: NA \n
/// Quest id: 12622 \n
/// Kutube'sa entry: 28494 \n
/// Gawanil entry: 28495 \n
/// Chulo entry: 28496 \n
/// after killing x npcs (entry: 28388/28504) the boss appears. 1. Kutube'sa, 2. Gawanil and 3. Chulo \n
///\todo Create Bossscripts for every leader \n

/// Kutube'sa
#define YELL_KUTIBESA_1 "Death to the Zandeleri and their puppets! Nothing can stop me now!"

/// Gawanil
#define YELL_GAWANIL_1 "Kill them! Kill them all and take back Zim'Torga!"

/// Chulo
#define YELL_CHULO_1 "Heh! You'll not get my treasure!"

void SetupLeaderOfJinAlai(ScriptMgr* mgr)
{ }
