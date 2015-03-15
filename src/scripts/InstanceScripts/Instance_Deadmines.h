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

#ifndef _INSTANCE_DEADMINES_H
#define _INSTANCE_DEADMINES_H

enum CreatureEntry
{
    NPC_EDWIN_VANCLEEF      = 639,
    NPC_SNEEDS_SHREDDER     = 642,
    NPC_SNEED               = 643,
    NPC_RHAHK_ZOR           = 644,
    NPC_MR_SMITE            = 646,
    NPC_GUARD1              = 657,    //Pirate
    NPC_GILNID              = 1763,
    NPC_GUARD2              = 3450    //Parrot
};

enum CreatureSpells
{
    //MrSmiteAI
    SMITE_STOMP     = 6432,
    SMITES_HAMMER   = 6436,
    SMITE_SLAM      = 6435
};

enum CreatureSay
{

};

enum DeadMinesGOIDs
{
    GO_FACTORY_DOOR         = 13965,
    GO_IRONCLAD_DOOR        = 16397,
    GO_DEFIAS_CANNON        = 16398,
    GO_HEAVY_DOOR           = 17153,
    GO_FACTORY_DOOR_LEVER   = 101831,
    GO_SNEED_DOOR_LEVER     = 101832,
    GO_IRONCLAD_LEVER       = 101833,
    GO_GILNID_DOOR_LEVER    = 101834,
    GO_MR_SMITE_CHEST       = 144111
};

#endif // _INSTANCE_DEADMINES_H
