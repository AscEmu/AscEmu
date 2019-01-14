/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // Interrogator Vishas
    CN_VISHAS       = 3983,

    // Bloodmage Thalnos
    CN_THALNOS      = 4543,

    //Houndmaster Loksey
    CN_LOKSEY       = 3974,

    // Arcanist Doan
    CN_DOAN         = 6487,

    // Herod
    CN_HEROD        = 3975,

    // Scarlet Commander Mograine
    CN_COMMANDER_MOGRAINE   = 3976,

    // High Inquisitor Whitemane
    CN_WHITEMANE    = 3977,

    // High Inquisitor Fairbanks
    CN_FAIRBANKS    = 4542,


};

enum CreatureSpells
{
    // Interrogator Vishas
    SP_VISHAS_SHADOW_WORD       = 2767,

    // Bloodmage Thalnos
    SP_THALNOS_SHADOW_BOLT      = 9613,
    SP_THALNOS_FLAME_SPIKE      = 8814,

    //Houndmaster Loksey
    SP_LOKSEY_BLOODLUST         = 6742,

    // Arcanist Doan
    SP_DOAN_SHIELD              = 9438,
    SP_DOAN_NOVA                = 9435,
    SP_DOAN_POLY                = 13323,
    SP_DOAN_SILENCE             = 30225,
    SP_DOAN_ARCANE_EXP          = 9433,

    // Herod
    SP_HEROD_WHIRLWINDSPELL     = 9632,
    SP_HEROD_CHARGE             = 22911,
    SP_HEROD_ENRAGESPELL        = 8269,
    SP_HEROD_AGGRO4             = 5830,
    SP_HEROD_KILL               = 5831,
    SP_HEROD_WHIRLWIND          = 5832,
    SP_HEROD_HEROD_ENRAGE       = 5833,

    // Scarlet Commander Mograine
    SP_MORGRAINE_HAMMER         = 32416,
    SP_MORGRAINE_CRUSADER       = 14517,
    SP_MORGRAINE_RESTALK        = 5835,
    SP_MORGRAINE_SHIELD         = 9438,

    // High Inquisitor Whitemane
    SP_WHITEMANE_SMITE          = 9481,
    SP_WHITEMANE_SLEEP          = 9256,
    SP_WHITEMANE_RESURRECTION   = 25435,

    // High Inquisitor Fairbanks
    SP_FAIRBANKS_BLOOD          = 40412,    /// \todo Need a better spell
    SP_FAIRBANKS_PWS            = 11647     //PWS = Power Word: Shield 
};

enum CreatureSay
{
    // Scarlet Commander Mograine
    SAY_MORGRAINE_01        = 2101,     // Infidels!They must be purified!
    SAY_MORGRAINE_02        = 2102,     // Unworthy!
    SAY_MORGRAINE_03        = 2103,     // At your side, milady!

    // High Inquisitor Whitemane
    SAY_SOUND_RESTALK2      = 5840,
    SAY_WHITEMANE_01        = 2104,     // Mograine has fallen!You shall pay for this treachery!Arise, my champion!Arise!
    SAY_WHITEMANE_02        = 2105,     // The Light has spoken!
    SAY_WHITEMANE_03        = 2106,     // Arise, my champion!

};

enum GameObjectEntry
{
    GO_INQUISITORS_DOOR     = 104600,
    GO_SCARLET_SECRET_DOOR  = 97700,
    GO_SCARLET_TORCH        = 97701,
    GO_ARMORY_DOOR          = 101851,
    GO_ARMORY_LEVER         = 101852,
    GO_CATHEDRAL_DOOR       = 101850,
    GO_CATHEDRAL_LEVER      = 101853
};
