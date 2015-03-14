/*
 * AscScripts for AscEmu Framework
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

#ifndef _INSTANCE_BLACKFATHOM_DEEPS_H
#define _INSTANCE_BLACKFATHOM_DEEPS_H

enum CreatureEntry
{
    // LadySarevessAI
    CN_LADY_SAREVESS    = 4831,

    // BaronAquanisAI
    CN_BARON_AQUANIS    = 12876,

    // KelrisAI
    CN_LORD_KELRIS      = 4832,

    // AkumaiAI
    CN_AKUMAI           = 4829,

    // MorriduneGossip
    CN_MORRIDUNE        = 6729

};

enum CreatureSpells
{};

enum CreatureSay
{
    // MorriduneGossip
    MORRIDUNE_ON_HELLO      = 7247,     // need to be checked not shure. old: menu(pObject->GetGUID(), 7247);
    MORRIDUNE_OPTION_1      = 423,      // Please Teleport me to Darnassus.
    MORRIDUNE_OPTION_2      = 424,      // I wish to leave this horrible place.
};

enum GameObjectEntry
{
    // FathomStone
    GO_FATHOM_STONE = 177964,
};

#endif // _INSTANCE_BLACKFATHOM_DEEPS_H
