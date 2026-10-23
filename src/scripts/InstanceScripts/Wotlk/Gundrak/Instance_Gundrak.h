/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    CN_SLADRAN              = 29304,
    CN_MOORABI              = 29305,
    CN_GAL_DARAH            = 29306,
    CN_DRAKKARI_COLOSSUS    = 29307,
    CN_ECK                  = 29932,

    CN_DRAKKARI_ELEMENTAL   = 29573,
    CN_LIVING_MOJO          = 29830,
    CN_SLADRAN_VIPER        = 29680,
    CN_SLADRAN_CONSTRICTOR  = 29713,
    CN_ALTAR_TRIGGER        = 30298
};

enum CreatureSpells
{
    // Slad'ran
    SLADRAN_POWERFUL_BITE       = 48287,
    SLADRAN_POWERFUL_BITE_HC    = 59840,
    SLADRAN_VENOM_BOLT          = 54970,
    SLADRAN_VENOM_BOLT_HC       = 59839,
    SLADRAN_POISON_NOVA         = 55081,
    SLADRAN_POISON_NOVA_HC      = 59842,
    SLADRAN_VENOMOUS_BITE       = 54987,
    SLADRAN_GRIP                = 55093,
    SLADRAN_SNAKE_WRAP          = 55126,

    // Moorabi
    MOORABI_DETERMINED_GORE     = 55102,
    MOORABI_DETERMINED_STAB     = 55104,
    MOORABI_GROUND_TREMOR       = 55142,
    MOORABI_NUMBING_SHOUT       = 55106,
    MOORABI_QUAKE               = 55101,
    MOORABI_NUMBING_ROAR        = 55100,
    MOORABI_MOJO_FRENZY         = 55163,
    MOORABI_TRANSFORMATION      = 55098,

    // Drakkari Colossus
    COLOSSUS_MIGHTY_BLOW        = 54719,
    COLOSSUS_MORTAL_STRIKES     = 54715,
    COLOSSUS_MORTAL_STRIKES_HC  = 59454,
    COLOSSUS_FREEZE_ANIM        = 16245,
    ELEMENTAL_SPAWN_EFFECT      = 54888,
    ELEMENTAL_MOJO_VOLLEY       = 54849,
    ELEMENTAL_SURGE             = 54801,
    ELEMENTAL_SURGE_VISUAL      = 54827,
    MOJO_WAVE                   = 55626,
    MOJO_PUDDLE                 = 55627,

    // Gal'darah
    GALDARAH_STAMPEDE           = 55250,
    GALDARAH_STAMPEDE_HC        = 59824,

    // Altars
    ALTAR_FIRE_BEAM_MAMMOTH     = 57068,
    ALTAR_FIRE_BEAM_SNAKE       = 57071,
    ALTAR_FIRE_BEAM_ELEMENTAL   = 57072
};

// npc_script_text entries of the bosses
enum CreatureSay
{
    SLADRAN_SAY_SNAKES          = 4221,
    SLADRAN_SAY_CONSTRICTORS    = 4222,

    MOORABI_SAY_AGGRO           = 4209,
    MOORABI_SAY_SLAY_1          = 4210,
    MOORABI_SAY_SLAY_2          = 4211,
    MOORABI_SAY_DEATH           = 4212,
    MOORABI_SAY_TRANSFORM       = 4213,
    MOORABI_SAY_QUAKE           = 4214,
    MOORABI_EMOTE_TRANSFORM     = 4215,
    MOORABI_SAY_TRANSFORMED     = 7421,
    MOORABI_EMOTE_TRANSFORMED   = 7422
};

enum GameObjectEntry
{
    GO_ALTAR1_SLADRAN       = 192518,
    GO_STATUE1_SLADRAN      = 192564,
    GO_ALTAR2_COLOSSUS      = 192520,
    GO_STATUE2_COLOSSUS     = 192567,
    GO_ALTAR3_MOORABI       = 192519,
    GO_STATUE3_MOORABI      = 192565,
    GO_STATUE4_GAL_DARAH    = 192566,
    GO_ECKDOOR              = 192632,

    GO_TRAPDOOR             = 193188,
    GO_COILISION            = 192633,

    GO_GAL_DARAH_DOOR1      = 193208,
    GO_GAL_DARAH_DOOR2      = 193209
};
