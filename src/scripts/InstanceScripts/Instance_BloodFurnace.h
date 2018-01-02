/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

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
