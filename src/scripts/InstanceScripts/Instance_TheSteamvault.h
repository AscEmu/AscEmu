/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // CoilfangEngineerAI
    CN_COILFANG_ENGINEER            = 17721,

    // CoilfangOracleAI
    CN_COILFANG_ORACLE              = 17803,

    // CoilfangWarriorAI
    CN_COILFANG_WARRIOR             = 17802,    // at least, couldn't find any data to compare it with blizz

    // CoilfangSirenAI
    CN_COILFANG_SIREN               = 17801,

    // BogOverlordAI
    CN_BOG_OVERLORD                 = 21694,

    // CoilfangSorceressAI
    CN_COILFANG_SORCERESS           = 17722,

    // CoilfangLeperAI
    CN_COILFANG_LEPER               = 21338,    // Couldn't find source to compare it's spells

    // CoilfangSlavemasterAI
    CN_COILFANG_SLAVEMASTER         = 17805,

    // CoilfangWaterElementalAI
    CN_COILFANG_WATER_ELEMENTAL     = 17917,

    // CoilfangMyrmidonAI
    CN_COILFANG_MYRMIDON            = 17800,

    // TidalSurgerAI
    CN_TIDAL_SURGER                 = 21695,

    // SteamSurgerAI
    CN_STEAM_SURGER                 = 21696,

    // Boss AIs
    // Hydromancer ThespiaAI
    CN_HYDROMANCER_THESPIA          = 17797,

    // Steamrigger MechanicAI
    CN_STEAMRIGGER_MECHANIC         = 17951,

    // Mekgineer Steamrigger
    CN_MEKGINEER_STEAMRIGGER        = 17796,

    // Naga DistillerAI
    CN_NAGA_DISTILLER               = 17954,

    // Warlord Kalitresh AI
    CN_WARLORD_KALITRESH            = 17798
};

enum CreatureSpells
{
    // CoilfangEngineerAI
    BOMB                        = 22334,    //40332 // AOE
    NET                         = 6533,     //38338 // Should stay for 5 or 8 sec?

    // CoilfangOracleAI
    FROST_SHOCK                 = 22582,    //34353
    SONIC_BURST                 = 8281,     //39052 // Should make dmg? or maybe it shouldn't be added here?
    HEAL                        = 31730,    //39378 // Hmm... no idea if this is correct id (and how much should heal)
    // Also it should heal other naga like etc.

    // CoilfangWarriorAI
    //MORTAL_STRIKE 29572  // should it really be here?
    MORTAL_BLOW                 = 35105,
    BATTLE_SHOUT                = 31403,
    // Defensive Stance 7164 ?

    // CoilfangSirenAI
    //MOONFIRE 20690 // not sure to id    // not used ?
    AOE_FEAR                    = 29321,    //30584 // Maybe should be: 33547 or 29321
    LIGHTNING_BOLT              = 15234,    //36152 // 1 target
    ARCANE_FLARE                = 35106,

    // BogOverlordAI
    FUNGAL_DECAY                = 32065,
    TRAMPLE                     = 15550,    //40340 // Should be used when enraged
    ENRAGE_BOG_OVERLORD         = 8599,     //40683 // No idea if this is good spell id (used ENRAGE_B [B = Bog], because of redefining)
    DISEASE_CLOUD               = 37266,    //DBC: 37266, 37267 || Heroic: 37863, 37864 ?

    // CoilfangSorceressAI
    FROSTBOLT                   = 12675,    //39064
    BLIZZARD                    = 39416,    //38646 // or maybe: 37263
    FROST_NOVA                  = 15063,    //29849 // also can be: 30094 or 32365

    // CoilfangLeperAI
    SHADOW_BOLT                 = 9613,     //39025
    FIRE_BLAST_LEPER            = 13339,    // not sure to those
    STRIKE                      = 13446,
    FROST_NOVA_LEPER            = 11831,
    CLEAVE_LEPER                = 5532,     // THOSE SPELLS MUST BE VERIFIED!
    HEAL_LEPER                  = 11642,
    SUNDER_ARMOR_LEPER          = 13444,
    SHOOT                       = 15547,

    // CoilfangSlavemasterAI
    GEYSER                      = 10987,    //40089 // It don't have to be good spell id [can be: 10987, 37478, 40089, 37427] - no idea why it doesn't knock you out
    ENRAGE_SlAVEMASTER          = 8269,

    // CoilfangWaterElementalAI
    //FROSTBOLT_VOLLEY 36741
    WATER_BOLT_WOLLEY           = 34449,

    // CoilfangMyrmidonAI
    SWEEPING_STRIKES            = 18765,    // DBC: 18765, 12723 //35429
    CLEAVE                      = 15622,    //38260 // no idea about it's id
    EXECUTE                     = 7160,     //38959 // should activate only on target with <= 25% hp // not sure about it

    // TidalSurgerAI
    //KNOCKBACK 30056
    WATER_SPOUT                 = 37250,
    FROST_NOVA_SURGER           = 15531,

    // SteamSurgerAI
    WATER_BOLT                  = 37252,

    // Hydromancer ThespiaAI
    ENVELOPING_WINDS            = 31718,
    LIGHTNING_CLOUD             = 25033,
    LUNG_BURST                  = 31481,

    // Steamrigger MechanicAI
    REPAIR                      = 31532,

    // Mekgineer Steamrigger
    SUPER_SHRINK_RAY            = 31485,
    SAW_BLADE                   = 31486,
    ELECTRIFIED_NET             = 35107,
    ENRAGE                      = 41447,    // No idea if this is good id

    // Naga DistillerAI
    //REPAIR                      = 31532, not used

    // Warlord Kalitresh AI
    IMPALE                      = 34451,
    HEAD_CRACK                  = 16172,
    SPELL_REFLECTION            = 31534,
    WARLORDS_RAGE               = 36453     // DBC: 37081, 36453    // still he must be forced by Driller to cast it

};

enum CreatureSay
{
    // Hydromancer ThespiaAI
    SAY_HYDROMACER_THESPIA_01       = 4796,     /// \todo unused Surge forth my pets!
    SAY_HYDROMACER_THESPIA_02       = 4797,     // The depths will consume you!
    SAY_HYDROMACER_THESPIA_03       = 4798,     // Meet your doom, surface dwellers!
    SAY_HYDROMACER_THESPIA_04       = 4799,     // You will drown in blood!
    SAY_HYDROMACER_THESPIA_05       = 4800,     // To the depths of oblivion with you!
    SAY_HYDROMACER_THESPIA_06       = 4801,     // For my lady and master!
    SAY_HYDROMACER_THESPIA_07       = 4802,     // Our matron will be... the end of you.

    // Mekgineer Steamrigger
    SAY_MEKGINEER_STEAMRIGGER_01    = 4803,     // I'm bringin' the pain!
    SAY_MEKGINEER_STEAMRIGGER_02    = 4804,     // You're in for a world o' hurt!
    SAY_MEKGINEER_STEAMRIGGER_03    = 4805,     // Eat hot metal, scumbag!
    SAY_MEKGINEER_STEAMRIGGER_04    = 4806,     // I'll come over there!
    SAY_MEKGINEER_STEAMRIGGER_05    = 4807,     // I'm bringin' the pain!
    SAY_MEKGINEER_STEAMRIGGER_06    = 4808,     // You just got served, punk!
    SAY_MEKGINEER_STEAMRIGGER_07    = 4809,     // I own you!
    SAY_MEKGINEER_STEAMRIGGER_08    = 4810,     // Have fun dyin', cupcake!
    SAY_MEKGINEER_STEAMRIGGER_09    = 4811,     // Mommy!

    // Warlord Kalitresh AI
    SAY_WARLORD_KALITRESH_01        = 4812,     ///\todo unused You deem yourselves worthy simply because you bested my guards? Our work here will not be compromised!
    SAY_WARLORD_KALITRESH_02        = 4813,     // This is not nearly over...
    SAY_WARLORD_KALITRESH_03        = 4814,     // Your head will roll!
    SAY_WARLORD_KALITRESH_04        = 4815,     // I despise all of your kind!
    SAY_WARLORD_KALITRESH_05        = 4816,     // Ba'anthalso-dorei!
    SAY_WARLORD_KALITRESH_06        = 4817,     // Squirm, surface filth!
    SAY_WARLORD_KALITRESH_07        = 4818,     // Ah ha ha ha ha ha ha!
    SAY_WARLORD_KALITRESH_08        = 4819,     // For her Excellency... for... Vashj!

};
