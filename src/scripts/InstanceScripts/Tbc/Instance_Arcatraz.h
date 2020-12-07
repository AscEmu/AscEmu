/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // Zereketh the UnboundAI
    CN_ZEREKETH                 = 20870,
    CN_VOIDZONEARC              = 21101,

    // Dalliah the DoomsayerAI
    CN_DALLIAH_THE_DOOMSAYER    = 20885,

    // Wrath-Scryer SoccothratesAI
    CN_WRATH_SCRYER_SOCCOTHRATES = 20886,

    // Harbinger SkyrissAI
    CN_HARBRINGER_SKYRISS       = 20912,

    // Warden MellicharAI
    CN_WARDEN_MELLICHAR         = 20904,
    CN_BLAZING_TRICKSTER        = 20905,    // ORB ONE
    CN_WARP_STALKER             = 20906,    // ORB ONE
    CN_MILLHOUSE_MANASTORM      = 20977,    // ORB TWO
    CN_AKKIRIS_LIGHTNING_WAKER  = 20908,    // ORB THREE
    CN_SULFURON_MAGMA_THROWER   = 20909,    // ORB THREE
    CN_TWILIGHT_DRAKONAAR       = 20910,    // ORB FOUR
    CN_BLACKWING_DRAKONAAR      = 20911     // ORB FOUR

};

enum CreatureSpells
{
    // Zereketh the UnboundAI
    SEED_OF_C               = 36123,    // 32865, 36123
    SHADOW_NOVA             = 36127,    // 30533, 39005, 36127 (normal mode), 39005 (heroic mode?)
    SHADOW_NOVA_H           = 39005,
    CONSUMPTION             = 30498,
    CONSUMPTION_H           = 39004,

    // Dalliah the DoomsayerAI
    GIFT_OF_THE_DOOMSAYER   = 36173,    // DBC: 36173
    WHIRLWIND               = 36175,    // DBC: 36142, 36175
    HEAL                    = 36144,
    SHADOW_WAVE             = 39016,    // Heroic mode spell

    // Wrath-Scryer SoccothratesAI
    IMMOLATION              = 35959,    // DBC: 36051, 35959
    FELFIRE_SHOCK           = 35759,
    FELFIRE_LINE_UP         = 35770,    // ?
    KNOCK_AWAY              = 20686,    // DBC: 36512; but it uses it on himself too so changed to other
    CHARGE                  = 35754,    // DBC: 36058, 35754 =(=(

    // Harbinger SkyrissAI
    MIND_REND               = 36924,    // DBC: 36859, 36924;
    FEAR                    = 39415,
    DOMINATION              = 37162,
    SUMMON_ILLUSION_66      = 36931,    // don't work
    SUMMON_ILLUSION_33      = 36932     // don't work

    // Warden MellicharAI

};

enum CreatureSay
{
    // Zereketh the UnboundAI
    SAY_ZEREKETH_01         = 5499,     // The shadow... will engulf you.
    SAY_ZEREKETH_02         = 5500,     // Darkness... consumes all.

    // Warden MellicharAI
    SAY_MELLICHAR_01        = 431,      // I knew the prince would be angry but, I... I have not been myself. I had to let them out!...
    SAY_MELLICHAR_02        = 432,      // The naaru kept some of the most dangerous beings in existence here in the...
    SAY_MELLICHAR_03        = 433,      // Yes, yes... another! Your will is mine!
    SAY_MELLICHAR_04        = 435,      // What is this? A lowly gnome? I will do better, oh great one.
    SAY_MELLICHAR_05        = 436,      // Anarchy!Bedlam!Oh, you are so wise!
    SAY_MELLICHAR_06        = 437,      // One final cell remains. Yes, O great one, right away!
                                        // Welcome, O great one. I am your humble servant.
                                        // Behold, yet another terrifying creature of incomprehensible power!

    // Millhouse
    SAY_MILLHOUS_01         = 439,      // Where in Bonzo's Brass Buttons am I? And who are--...
    SAY_MILLHOUS_02         = 440       // "Lowly"? I don't care who you are, friend: no one refers ...
                                        // ID: 441 I just need to get some things ready first. You guys go ahead and get started. I need to summon up some water....
                                        // ID: 442 Fantastic! Next, some protective spells. Yeah, now we're cookin'!
                                        // ID: 443 And of course I'll need some mana. You guys are gonna love this; just wait....
                                        // ID: 444 Aaalllriiiight!! Who ordered up an extra large can of whoop-ass?
                                        // ID: 445 I didn't even break a sweat on that one!
                                        // ID: 446 You guys feel free to jump in anytime.
                                        // ID: 447 I'm gonna light you up, sweet cheeks!
                                        // ID: 448 Ice, ice baby.
                                        // ID: 449 Heal me! For the love of all that's holy, heal me! I'm dying!!
                                        // ID: 450 You'll be hearing from my lawyer!
                                        // ID: 451 Who's bad? Who's bad? That's right: we bad!

};
