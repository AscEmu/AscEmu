/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // CabalAcolyteAI
    CN_CABAL_ACOLYTE            = 18633,

    // CabalDeathswornAI
    CN_CABAL_DEATHSWORN         = 18635,

    // CabalFanaticAI
    CN_CABAL_FANATIC            = 18830,

    // CabalShadowPriestAI
    CN_CABAL_SHADOW_PRIEST      = 18637,

    // CabalSpellbinderAI
    CN_CABAL_SPELLBINDER        = 18639,

    // CabalWarlockAI
    CN_CABAL_WARLOCK            = 18640,

    // CabalZealotAI
    CN_CABAL_ZEALOT             = 18638,

    // CabalRitualistAI
    CN_CABAL_RITUALIST          = 18794,

    // FelOverseerAI
    CN_FEL_OVERSEER             = 18796,

    // MaliciousInstructorAI
    CN_MALICIOUS_INSTRUCTOR     = 18848,

    // AmbassadorHellmawAI
    CN_AMBASSADOR_HELLMAW       = 18731,

    // BlackheartTheInciterAI
    CN_BLACKHEART_THE_INCITER   = 18667,

    // GrandmasterVorpilAI
    CN_GRANDMASTER_VORPIL       = 18732,

    // MurmurAI
    CN_MURMUR                   = 18708

};

/// \todo No accurate info about spells of Cabal Assassins, Cultist, Executioner, Summoner so they aren't scripted
enum CreatureSpells
{
    // CabalAcolyteAI
    SP_CABAL_ACOLYTE_SHADOW_PROTECTION      = 17548,    // couldn't find more accurate id
    SP_CABAL_ACOLYTE_HEAL                   = 31739,    // no idea if this is good id

    // CabalDeathswornAI
    SP_CABAL_DEATHSWORN_SHADOW_CLEAVE       = 30495,    // should be 29832, 30495 or maybe other ?
    SP_CABAL_DEATHSWORN_KNOCKBACK           = 37317,    // 37317 or 38576 or other?
    SP_CABAL_DEATHSWORN_BLACK_CLEAVE        = 38226,    // Does it use black cleave too?

    // CabalFanaticAI
    SP_CABAL_FANATIC_FIXATE                 = 40414,

    // CabalShadowPriestAI
    SP_CABAL_SHADOW_PRIEST_MIND_FLAY        = 29570,    // 29570 or 37276
    SP_CABAL_SHADOW_PRIEST_WORD_PAIN        = 24212,    // 24212 or 34441

    // CabalSpellbinderAI
    SP_CABAL_SPELLBINDER_MIND_CONTROL       = 36797,    // 36797 or 36798
    SP_CABAL_SPELLBINDER_EARTH_SHOCK        = 26194,    // 26194 or 24685 or 22885

    // CabalWarlockAI
    SP_CABAL_WARLOCK_SHADOW_BOLT            = 31618,    // can be also: 31618, 32860, 36714, 30686, 36986 and many others
    SP_CABAL_WARLOCK_SEED_OF_CORRUPTION     = 32863,    // can be: 32863, 36123, 38252, 39367 // Should come with Succubus or Felhunter as pet

    // CabalZealotAI
    SP_CABAL_ZEALOT_SHADOW_BOLT             = 31618,    // no idea to this spell (added same as for warlock)
    SP_CABAL_ZEALOT_TRANSFROMATION          = 0,        // can't find correct id for now
    //turns into a mini-Magmadar at low health, cannot cast but does extremely heavy melee damage.

    // CabalRitualistAI
    SP_CABAL_RITUALIST_GOUGE                = 36862,    // 36862 or 29425 or 34940 or 28456 (?)
    SP_CABAL_RITUALIST_ARCANE_MISSILES      = 35034,    // not sure
    SP_CABAL_RITUALIST_ADDLE_HUMANOID       = 33487,
    SP_CABAL_RITUALIST_FIRE_BLAST           = 36339,
    SP_CABAL_RITUALIST_FLAME_BUFFET         = 34121,
    SP_CABAL_RITUALIST_FROSTBOLT            = 32370,    // also can be: 36710, 32370, 32364, 36279

    // no idea how to separate spells for same creature (as in DB/sites is only one type of that creature!)
    // so I am giving "all in one" and I don't have time to think about it.

    // FelOverseerAI
    SP_FEL_OVERSEER_INTIMIDATING_SHOUT      = 33789,
    SP_FEL_OVERSEER_CHARGE_OVERSEER         = 33709,    // no idea
    SP_FEL_OVERSEER_HEAL_OVERSEER           = 33144,    // 32130/33144 || here too :P    // doesn't work?:|
    SP_FEL_OVERSEER_MORTAL_STRIKE           = 24573,    // same
    SP_FEL_OVERSEER_UPPERCUT                = 32055,

    // MaliciousInstructorAI
    SP_MILICIOUS_INSTRUCT_SHADOW_NOVA       = 33846,
    SP_MILICIOUS_INSTRUCT_MARK_OF_MALICE    = 33493,

    // AmbassadorHellmawAI
    SP_AMBASSADOR_HELMAW_CORROSIVE_ACID     = 33551,
    SP_AMBASSADOR_HELMAW_AOE_FEAR           = 33547,

    // BlackheartTheInciterAI
    SP_BLACKHEART_INCITER_CHAOS             = 33684,    // 33684 or 33676?
    SP_BLACKHEART_INCITER_CHARGE            = 39574,    //39574 (HM) or 38461 // couldn't find more accurate
    SP_BLACKHEART_INCITER_WAR_STOMP         = 33707,
    //SP_BLACKHEART_INCITER_AOE_KNOCKBACK 30056 // 30056, 37317 or 38576
    // Help sound? And other (as there were sounds like _BlckHrt02_ (and I used only with 01)
    // Is sound id 10488 for aggro or mind control?

    // GrandmasterVorpilAI
    SP_GRDMASTER_VORPIL_SHADOW_BOLT_VOLLEY  = 33841,
    SP_GRDMASTER_VORPIL_DRAW_SHADOWS        = 33563,    // can't find it =/
    SP_GRDMASTER_VORPIL_RAIN_OF_FIRE        = 33617,    // 33617 or 34360    // breaks model behavior
    SP_GRDMASTER_VORPIL_VOID_PORTAL_VISUAL  = 33569,    // must work on that 33566
    // Help sound and it's case?
    // Should OnCombatStart should spawn 3 portals for Voidwalkers? (33569 ?)

    // MurmurAI
    SP_MURMUR_SHOCKWAVE                     = 33686,   // 25425
    SP_MURMUR_MURMURS_TOUCH                 = 33711,
    SP_MURMUR_SONIC_BOOM1                   = 33923,   // anything missed? additional dmging spell?
    SP_MURMUR_SONIC_BOOM2                   = 33666,
    SP_MURMUR_RESONANCE                     = 33657    // should be applied only when no target in melee combat range (each 5 sec)
    /*const uint32 THUNDERING_STORM = 39365
    const uint32 SUPPRESSION_BLAST = 33332*/
    // const uint32 SONIC_BOOM 33666
    // 33666 or 38795 (I think it's dummy (33923); it should be connected with Murmur's Touch; Murmur should say
    // Murmur draws energy from the air then use Murmur's Touch (33711), Sonic Boom (33923) and
    // release Sonic Boom (38052); it also shouldn't attack during all those casts;

};

enum CreatureSay
{
    // AmbassadorHellmawAI
    SAY_AMBASSADOR_HELMAW_01        = 4605,     /// \todo unused Infidels have invaded the sanctuary!Sniveling pests...You have yet to learn the true meaning of agony!
    SAY_AMBASSADOR_HELMAW_02        = 4606,     // Pathetic mortals!You will pay dearly.
    SAY_AMBASSADOR_HELMAW_03        = 4607,     // I will break you!
    SAY_AMBASSADOR_HELMAW_04        = 4608,     // Finally, something to relieve the tedium!
    SAY_AMBASSADOR_HELMAW_05        = 4609,     /// \todo unused Aid me, you fools, before it's too late!
    SAY_AMBASSADOR_HELMAW_06        = 4610,     // Do you fear death ?
    SAY_AMBASSADOR_HELMAW_07        = 4611,     // This is the part I enjoy most...
    SAY_AMBASSADOR_HELMAW_08        = 4612,     // Do not... grow... overconfident, mortal.

    // BlackheartTheInciterAI
    SAY_BLACKHEART_INCITER_01       = 4613,     /// \todo unused All flesh must burn.
    SAY_BLACKHEART_INCITER_02       = 4614,     /// \todo unused All creation must be unmade!
    SAY_BLACKHEART_INCITER_03       = 4615,     /// \todo unused Power will be yours!
    SAY_BLACKHEART_INCITER_04       = 4616,     // You'll be sorry!
    SAY_BLACKHEART_INCITER_05       = 4617,     // Time for fun!
    SAY_BLACKHEART_INCITER_06       = 4618,     // I see dead people!
    SAY_BLACKHEART_INCITER_07       = 4619,     // No coming back for you!
    SAY_BLACKHEART_INCITER_08       = 4620,     // Nice try.
    SAY_BLACKHEART_INCITER_09       = 4621,     /// \todo unused Help us, hurry!
    SAY_BLACKHEART_INCITER_10       = 4622,     // This... no... good...
    SAY_BLACKHEART_INCITER_11       = 4623,     /// \todo unused Be ready for Dark One's return.
    SAY_BLACKHEART_INCITER_12       = 4624,     /// \todo unused So we have place in new universe.
    SAY_BLACKHEART_INCITER_13       = 4625,     /// \todo unused Dark one promise!
    SAY_BLACKHEART_INCITER_14       = 4626,     /// \todo unused You'll be sorry!
    SAY_BLACKHEART_INCITER_15       = 4627,     /// \todo unused Time to kill!
    SAY_BLACKHEART_INCITER_16       = 4628,     /// \todo unused You be dead people!
    SAY_BLACKHEART_INCITER_17       = 4629,     /// \todo unused Now you gone for good!
    SAY_BLACKHEART_INCITER_18       = 4630,     /// \todo unused You fail!
    SAY_BLACKHEART_INCITER_19       = 4631,     /// \todo unused Help us, hurry!
    SAY_BLACKHEART_INCITER_20       = 4632,     /// \todo unused Arrgh, aah...ahhh

    // GrandmasterVorpilAI
    SAY_GRD_VORPIL_01               = 4633,     /// \todo unused Keep your minds focused for the days of reckoning are close at hand.Soon, the destroyer of worlds will return to make good on his promise.Soon the destruction of all that is will begin!
    SAY_GRD_VORPIL_02               = 4634,     // I'll make an offering of your blood!
    SAY_GRD_VORPIL_03               = 4635,     // You'll be a fine example for the others!
    SAY_GRD_VORPIL_04               = 4636,     // Good, a worthy sacrifice!
    SAY_GRD_VORPIL_05               = 4637,     /// \todo unused Come to my aid!Heed your master now!
    SAY_GRD_VORPIL_06               = 4638,     // I serve with pride.
    SAY_GRD_VORPIL_07               = 4639,     // Your death is for the greater cause...
    SAY_GRD_VORPIL_08               = 4640      // I give my life... gladly.

};
