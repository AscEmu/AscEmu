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

#ifndef _INSTANCE_BLOOD_FURNANCE_H
#define _INSTANCE_BLOOD_FURNANCE_H

enum CreatureEntry
{
    // Keli'dan the BreakerAI
    CN_KELIDAN_THE_BREAKER  = 17377,

    // Broggok
    CN_BROGGOK              = 17380,

    // The Maker
    CN_THE_MAKER            = 17381
};

enum CreatureSpells
{
    // Keli'dan the BreakerAI
    KELIDAN_BURNING_NOVA            = 30940,
    KELIDAN_SHADOW_BOLT_VOLLEY      = 28599,
    KELIDAN_SHADOW_BOLT_VOLLEY_H    = 40070,    // Heroic
    KELIDAN_FIRE_NOVA               = 33132,
    KELIDAN_FIRE_NOVA_H             = 37371,    // Heroic
    KELIDAN_CORRUPTION              = 30938,
    KELIDAN_EVOCATION               = 30935,
    KELIDAN_VORTEX                  = 37370,

    // Broggok
    POISON_BOLT                     = 30917,
    POISON_CLOUD                    = 31259,    // DBC: 30916; no idea if correct
    SLIME_SPRAY                     = 30913,

    // The Maker
    DOMINATION                      = 30923,    // 36866
    ACID_SPRAY                      = 38973,    // 38973 or 38153    // not sure about casting of this
    THROW_BEAKER                    = 30925     // Throw beaker <--- maybe this is it?

};

enum CreatureSay
{ };

enum GameObjectEntry
{
    // Broggok
    GO_BROGGOK      = 181819,  // Doodad_Hellfire_DW_PrisonEntry04
    // The Maker
    GO_THE_MAKER    = 181812   // Doodad_Hellfire_DW_PrisonEntry03
};

#endif // _INSTANCE_BLOOD_FURNANCE_H
