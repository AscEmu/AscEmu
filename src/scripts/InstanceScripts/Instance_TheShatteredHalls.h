/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // FelOrcConvertAI
    CN_FEL_ORC_CONVERT = 17083,

    // ShatteredHandHeathenAI
    CN_SHATTERED_HAND_HEATHEN = 17420,

    // ShatteredHandLegionnaireAI
    CN_SHATTERED_HAND_LEGIONNAIRE = 16700,

    // ShatteredHandSavageAI
    CN_SHATTERED_HAND_SAVAGE = 16523,

    // ShadowmoonAcolyteAI
    CN_SHADOWMOON_ACOLYTE = 16594,

    // ShatteredHandAssassinAI
    CN_SHATTERED_HAND_ASSASSIN = 17695,

    // ShatteredHandGladiatorAI
    CN_SHATTERED_HAND_GLADIATOR = 17464,

    // ShatteredHandHoundmasterAI
    CN_SHATTERED_HAND_HOUNDMASTER = 17670,

    // ShatteredHandReaverAI
    CN_SHATTERED_HAND_REAVER = 16699,

    // ShatteredHandSentryAI
    CN_SHATTERED_HAND_SENTRY = 16507,

    // ShatteredHandSharpshooterAI
    CN_SHATTERED_HAND_SHARPSHOOTER = 16704,

    // ShatteredHandBrawlerAI
    CN_SHATTERED_HAND_BRAWLER = 16593,

    // ShadowmoonDarkcasterAI
    CN_SHADOWMOON_DARKCASTER    = 17694,

    // GrandWarlockNetherkurseAI
    CN_GRAND_WARLOCK_NETHEKURSE = 16807,

    // Blood Guard PorungAI
    CN_BLOOD_GUARD_PORUNG       = 20923,

    // WarbringerOmroggAI
    CN_WARBRINGER_OMROGG = 16809,
    CN_LEFT_HEAD = 19523,
    CN_RIGHT_HEAD = 19524,

    // HeadAI

    // Warchief Kargath BladefistAI
    CN_WARCHIEF_KARGATH_BLADEFIST = 16808,
};

enum CreatureSpells
{
    // FelOrcConvertAI
    SP_FEL_ORC_CONVERTER_HEMORRHAGE     = 30478,

    // ShatteredHandHeathenAI
    SP_HAND_HEATHEN_BLOODTHIRST         = 30474,    // 30475
    SP_HAND_HEATHEN_ENRAGE              = 30485,    // those should be correct, but still not sure

    // ShatteredHandLegionnaireAI
    SP_HAND_LEGI_AURA_OF_DISCIPLINE     = 30472,
    SP_HAND_LEGI_PUMMEL                 = 15615,    // should be all good (Idk if those are all spells [summon/spawn spell?])
    SP_HAND_LEGI_ENRAGE                 = 30485,

    // ShatteredHandSavageAI
    SP_HAND_SAVAGE_SLICE_AND_DICE       = 30470,
    SP_HAND_SAVAGE_ENRAGE               = 30485,
    SP_HAND_SAVAGE_DEATHBLOW            = 36023,

    // ShadowmoonAcolyteAI
    SP_SHADOWMOON_ACOLYTE_HEAL          = 31730,    // 32130, 31730, 39378, 31739 // is this really used?
    SP_SHADOWMOON_ACOLYTE_PW_SHIELD     = 35944,    // 41373, 29408, 36052, 35944, 32595
    SP_SHADOWMOON_ACOLYTE_MIND_BLAST    = 31516,    //26048 //38259 // ofc not sure (and this one can be really overpowered)
    SP_SHADOWMOON_ACOLYTE_RESIST_SHADOW = 30479,    // not sure to those both
    //PRAYER_OF_HEALING 15585    // crashes server
    //105 resist shadow buff?
    // Self Visual - Sleep Until Cancelled (DND) 16093 ?

    // ShatteredHandAssassinAI
    SP_SHATT_HAND_ASSASSIN_SAP           = 30980,
    SP_SHATT_HAND_ASSASSIN_STEALTH       = 30991,    // 32615, 30831, 30991, 31526, 31621, 34189, 32199 // I think should be harder to detect
    SP_SHATT_HAND_ASSASSIN_CHEAP_SHOT    = 30986,

    // ShatteredHandGladiatorAI
    SP_SHATT_HAND_GLADI_MORTAL_STRIKE   = 31911,    // 31911, 29572, 32736, 35054, 39171, 37335 // sth more?

    // ShatteredHandHoundmasterAI
    SP_SHATT_HAND_HOUNDMASTER_VOLLEY    = 34100,    // 34100, 35950, 30933, 22908

    // ShatteredHandReaverAI
    SP_SHATT_HAND_REAVER_CLEAVE         = 15754,    //34995 // no idea if this is good id
    SP_SHATT_HAND_REAVER_UPPERCUT       = 30471,    // 32055, 34014, 34996, 39069, 41388, 30471
    SP_SHATT_HAND_REAVER_ENRAGE         = 30485,    // 34624, 37023, 37648, 38046, 41305, 34670, 34970, 34971, 36992, 38947, 41447 and many others =/

    // ShatteredHandSentryAI
    SP_SHATT_HAND_SENTRY_CHARGE         = 22911,    // 35570 many others
    SP_SHATT_HAND_SENTRY_HAMSTERING     = 31553,    // not sure if it uses it

    // ShatteredHandSharpshooterAI
    SP_SHATT_HAND_SHARP_SCATTER_SHOT    = 23601,    // 36732 // not sure
    SP_SHATT_HAND_SHARP_IMMO_ARROW      = 35932,    // same here (Idk if it uses it for sure)
    SP_SHATT_HAND_SHARP_SHOT            = 15620,    // must find way to force mob to cast this only when dist > xx
    SP_SHATT_HAND_SHARP_INCENDIARY_SHOT = 30481,    // not sure to these

    // ShatteredHandBrawlerAI
    SP_CURSE_OF_THE_SHATTERED_HAND      = 36020,    //36020
    SP_SHATT_HAND_BRAWLER_KICK          = 36033,    // no idea about these spells
    SP_SHATT_HAND_BRAWLER_TRASH         = 3391,     // W00T? doesn't work (maybe lack of core support?)

    // ShadowmoonDarkcasterAI (NOT USED)
    SP_DARKCASTER_RAIN_OF_FIRE          = 37279,    // DBC: 11990; 37279, 39376, 36808, 34360, 33617
    SP_DARKCASTER_FEAR                  = 12542,    //38154 // 38595, 38660, 39119, 39210, 39415, 38154, 34259, 33924, 31358, 30615
    SP_DARKCASTER_SHADOW_BOLT           = 12471,    // not sure

    // Grand Warlock NethekurseAI
    SP_GRAND_WARLOCK_NETH_DEATH_COIL    = 30500,    // 30741 or 30500; not sure if this is right id and if it's working like it should
    SP_GRAND_WARLOCK_NETH_DARK_SPIN     = 30502,    // this should be correct    // doesn't work because of lack of core support? (so can't check)
    SP_LESSER_SHADOW_FISSURE            = 30496,    // can be: 36147, 30496, 30744 // doesn't work, coz lack of core support for summons

    // Blood Guard PorungAI
    SP_BLOOD_GUARD_PORUNG_CLEAVE        = 37476,    // right description, but no idea if this is right spell
    //FEAR <-- disabled in 2.1

    // WarbringerOmroggAI
    SP_WARBRINGER_OMROGG_THUNDERCLAP    = 30633,
    SP_WARBRINGER_OMROGG_FEAR           = 30584,
    SP_WARBRINGER_OMROGG_BURNING_MAUL   = 30598,    // 30598 or 30599
    SP_WARBRINGER_OMROGG_BLAST_WAVE     = 30600,

    // Warchief Kargath BladefistAI
    SP_WARCHIEF_LARAGATH_BLADE_DANCE    = 30739,    // should be each 30 sec, but Idk correct attktime
};

enum CreatureSay
{
    //GrandWarlock
    SAY_GRAND_WARLOCK_01    = 4879,     /// \todo unused You wish to fight us all at once? This should be amusing!
    SAY_GRAND_WARLOCK_02    = 4880,     // You can have that one, I no longer need him!
    SAY_GRAND_WARLOCK_03    = 4881,     // Yes, beat him mercilessly! His skull is as thick as an ogre's!
    SAY_GRAND_WARLOCK_04    = 4882,     // Don't waste your time on that one, he's weak!
    SAY_GRAND_WARLOCK_05    = 4883,     // You want him? Very well, take him!
    SAY_GRAND_WARLOCK_06    = 4884,     // One pitiful wretch down. Go on, take another one! 
    SAY_GRAND_WARLOCK_07    = 4885,     // Ah, what a waste... next!
    SAY_GRAND_WARLOCK_08    = 4886,     // I was going to kill him anyway!
    SAY_GRAND_WARLOCK_09    = 4887,     /// \todo unused Thank you for saving me the trouble. Now it's my turn to have some fun!
    SAY_GRAND_WARLOCK_10    = 4888,     /// \todo unused Beg for your pitiful life!
    SAY_GRAND_WARLOCK_11    = 4889,     /// \todo unused Run, coward, run!   
    SAY_GRAND_WARLOCK_12    = 4890,     /// \todo unused Your pain amuses me!
    SAY_GRAND_WARLOCK_13    = 4891,     // I'm already bored!
    SAY_GRAND_WARLOCK_14    = 4892,     // Come on, show me a real fight!
    SAY_GRAND_WARLOCK_15    = 4893,     // I had more fun torturing the peons!
    SAY_GRAND_WARLOCK_16    = 4894,     // You lose.
    SAY_GRAND_WARLOCK_17    = 4895,     // Oh, just die!
    SAY_GRAND_WARLOCK_18    = 4896,     // What... a shame.

    // Warchief Kargath BladefistAI
    SAY_WARCHIEF_KARGATH_01 = 8730,     // Ours is the true horde! The only horde!
    SAY_WARCHIEF_KARGATH_02 = 8731,     // I'll carve the meat from your bones!
    SAY_WARCHIEF_KARGATH_03 = 8732,     // I am called Bladefists for a reason... as you will see!
    SAY_WARCHIEF_KARGATH_04 = 8733,     // For the real horde!
    SAY_WARCHIEF_KARGATH_05 = 8734,     // I am the only warchief!
    SAY_WARCHIEF_KARGATH_06 = 8735      // The true horde... will... prevail!

};
