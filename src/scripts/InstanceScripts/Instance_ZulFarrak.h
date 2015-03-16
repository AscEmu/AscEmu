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

#ifndef _INSTANCE_ZUL_FARRAK_H
#define _INSTANCE_ZUL_FARRAK_H

enum CreatureEntry
{
    //AntusulAI
    CN_ANTUSUL              = 8127,
    CN_SULLITHUZ_BROODLING  = 8138,

    //ThekaAI
    CN_THEKA                = 7272

};

enum CreatureSpells
{
    //Theka the Martyr
    SP_THEKA_TRANSFORM          = 11089,
    SP_THEKA_FEVERED_PLAGUE     = 16186,    // 8600  i dont know wich one it is he casts

    //AntusulAI
    SP_ANTUSUL_SERVANTS         = 11894
    //SP_ANTUSUL_HEALINGWARD      = 11889,
    //SP_ANTUSUL_EARTHGRABWARD    = 8376

};

enum CreatureSay
{

};

enum InstanceMisc
{
    TRIGGER_ANTUSUL = 133337
};
#endif // _INSTANCE_ZUL_FARRAK_H
