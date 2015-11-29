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

#ifndef _INSTANCE_THE_UNDERBOG_H
#define _INSTANCE_THE_UNDERBOG_H

enum CreatureEntry
{
    // Boss AIs
    // HungarfenAI
    CN_HUNGARFEN        = 17770,

    // Ghaz'anAI
    CN_GHAZAN           = 18105,

    // ClawAI
    CN_CLAW             = 17827,

    // Swamplord Musel'ekAI
    CN_SWAMPLORD_MUSELEK = 17826,

    // The Black StalkerAI
    CN_THE_BLACK_STALKER    = 17882,

};

enum CreatureSpells
{
    // HungarfenAI
    UNDERBOG_MUSHROOM       = 31693,    // still not idea *confused* //34588 // No idea if this is right spell, but should be correct (also aditional core support needed!)
    FOUL_SPORES             = 31673,    //DBC: 31673, 31697 // this one needs additional core support too
                                        // Putrid Mushroom Primer 31693 ?
                                        // Despawn Underbog Mushrooms 34874 ?

    // Ghaz'anAI
    ACID_SPIT               = 34290,
    TAIL_SWEEP              = 34267,
    ACID_BREATH             = 24839,
    ENRAGE                  = 15716,    // Not sure to id as always in Enrage case: 34409, 34970

    // ClawAI
    MAUL                    = 34298,
    CL_ECHOING_ROAR         = 31429,
    FERAL_CHARGE            = 39435,
    CL_ENRAGE               = 34971,

    // Swamplord Musel'ekAI
    THROW_FREEZING_TRAP     = 31946,    // needs more core support
    KNOCK_AWAY_MUSELEK      = 18813,
    AIMED_SHOT              = 31623,
    MULTI_SHOT              = 30990,
    SHOT                    = 32103,

    // The Black StalkerAI
    CHAIN_LIGHTNING         = 31717,    //39066 // 28167, 39066
    LEVITATE                = 31704,    // Not sure to id
    STATIC_CHARGE           = 31715,
    SUMMON_SPORE_STRIDER    = 38755,    // spawning adds only on Heroic! lack of core support =/
};

enum CreatureSay
{
    // Swamplord Musel'ekAI
    SAY_SWAMPLORD_MUSEL_01      = 1462,     ///\todo unused Beast! Obey me! Kill them at once!
    SAY_SWAMPLORD_MUSEL_02      = 1463,     // We fight to the death!
    SAY_SWAMPLORD_MUSEL_03      = 1464,     // I will end this quickly....
    SAY_SWAMPLORD_MUSEL_04      = 1465,     // Acalah pek ecta!
    SAY_SWAMPLORD_MUSEL_05      = 1466,     // Krypta!
    SAY_SWAMPLORD_MUSEL_06      = 1467,     // It is finished.
    SAY_SWAMPLORD_MUSEL_07      = 1468,     // Well... done...


};

#endif // _INSTANCE_THE_UNDERBOG_H
