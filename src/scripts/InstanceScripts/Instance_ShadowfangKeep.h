/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#if VERSION_STRING < Cata
const uint32 SHADOWFANG_KEEP_MAP = 33;

enum SFK_encounterIndexes : uint8
{
    // Main encounters
    INDEX_VOIDWALKER    = 0,
    INDEX_FENRUS,
    INDEX_ARUGAL,
    INDEX_NANDOS,
    INDEX_ADAMANT,
    INDEX_RETHILGORE,

    // Other
    INDEX_PRISONER_EVENT,
    INDEX_ARUGAL_INTRO,

    // Max index identifier
    INDEX_MAX
};

enum SFK_creatureEntries : uint32
{
    // Bosses
    CN_RAZORCLAW_THE_BUTCHER    = 3886,
    CN_BARON_SILVERLAINE        = 3887,
    CN_RETHILGORE               = 3914,
    CN_NANDOS                   = 3927,
    CN_FENRUS                   = 4274,
    CN_ARUGAL_BOSS              = 4275,
    CN_SPRINGVALE               = 4278,
    CN_BLINDWATCHER             = 4279,

    // Prisoners
    CN_ADAMANT                  = 3849,
    CN_ASHCROMBE                = 3850,

    // Intro event
    CN_ARUGAL                   = 10000,
    CN_DEATHSTALKER_VINCENT     = 4444,

    // Trash mobs
    CN_SON_OF_ARUGAL            = 2529,
    CN_SHADOWFANG_WHITESCALP    = 3851, // Handled in database (auras)
    CN_SHADOWFANG_MOONWALKER    = 3853,
    CN_SHADOWFANG_DARKSOUL      = 3855,
    CN_SHADOWFANG_GLUTTON       = 3857,
    CN_SHADOWFANG_RAGETOOTH     = 3859,
    CN_FEL_STEED                = 3864,
    CN_SHADOW_CHARGER           = 3865, // No info
    CN_VILE_BAT                 = 3866,
    CN_BLOOD_SEEKER             = 3868,
    CN_BLEAK_WORG               = 3861,
    CN_SLAVERING_WORG           = 3862, // No info
    CN_LUPINE_HORROR            = 3863,
    CN_DEATHSWORN_CAPTAIN       = 3872,
    CN_TORMENTED_OFFICER        = 3873,
    CN_HAUNTED_SERVITOR         = 3875,
    CN_WAILLING_GUARDSMAN       = 3877,
    CN_LUPINE_DELUSION          = 5097,
    CN_VOIDWALKER               = 4627,
    CN_WOLFGUARD_WORG           = 5058
};

enum SFK_scriptTexts : uint32
{
    // Sorcerer Ashcrombe texts
    SAY_ASHCROMBE_BYE           = 2115,     // "There it is! Wide open. Good luck to you conquering what lies beyond. I must report back to the Kirin Tor at once!"
    SAY_ASHCROMBE_FOLLOW        = 2117,     // "Follow me and I'll open the courtyard door for you."
    SAY_ASHCROMBE_OPEN_DOOR     = 2118,     // "I have just the spell to get this door open. Too bad the cell doors weren't locked so haphazardly."
    SAY_ASHCROMBE_BOSS_DEATH    = 3262,     // "For once I agree with you... scum."
    SAY_ASHCROMBE_VANISH        = 8788,     // "Sorcerer Ashcrombe vanishes."

    // Deathstalker Adamant texts
    SAY_ADAMANT_BOSS_DEATH      = 5251,     // "About time someone killed the wretch."
    SAY_ADAMANT_FOLLOW          = 2122,     // "Free from this wretched cell at last! Let me show you to the courtyard...."
    SAY_ADAMANT_BYE             = 2120,     // "Good luck with Arugal. I must hurry back to Hadrec now."
    SAY_ADAMANT_BEFORE_OPEN     = 2121,     // "You are indeed courageous for wanting to brave the horrors that lie beyond this door."
    SAY_ADAMANT_AFTER_OPEN      = 2119,     // "There we go!"
    SAY_ADAMANT_OPENING         = 8787,     // "Deathstalker Adamant fumbles with the rusty lock on the courtyard door."

    // Arugal intro event
    SAY_ARUGAL_INTRO1           = 8789,     // "I have changed my mind loyal servants, you do not need to bring the prisoner all the way to my study, I will deal with him here and now."
    SAY_ARUGAL_INTRO2           = 8790,     // "Vincent! You and your pathetic ilk will find no more success in routing my sons and I than those beggardly remnants of the Kirin Tor."
    SAY_ARUGAL_INTRO3           = 8791,     // "If you will not serve my Master with your sword and knowledge of his enemies..."
    SAY_ARUGAL_INTRO4           = 8792,     // "Your moldering remains will serve ME as a testament to what happens when one is foolish enough to trespass in my domain!"
    SAY_VINCENT_DEATH           = 8793,     // "Arrrgh!"

    // Arugal boss texts
    YELL_ARUGAL_FENRUS          = 2116,     // "Who dares interfere with the Sons of Arugal?"
    YELL_ARUGAL_AGROO           = 8794,     // "You, too, shall serve!"
    YELL_ARUGAL_ENEMY_DEATH     = 8795,     // "Another Falls!"
    YELL_ARUGAL_COMBAT          = 8796      // "Release your rage!"
};

enum SFK_gameobjectEntries : uint32
{
    GO_SORCERER_GATE            = 18972,

    // Prison Levers
    GO_RIGHT_LEVER              = 18900,
    GO_MIDDLE_LEVER             = 18901,
    GO_LEFT_LEVER               = 101811,
    GO_RIGHT_CELL               = 18934,
    GO_MIDDLE_CELL              = 18936,
    GO_LEFT_CELL                = 18935,
    GO_ARUGALS_LAIR_GATE        = 18971,
    GO_ARUGAL_FOCUS             = 18973,                        // this generates the lightning visual in the Fenrus event
    GO_COURTYARD_DOOR           = 18895
};

enum SFK_spellEntries : uint32
{
    SPELL_ASHCROMBE_UNLOCK      = 6421,
    SPELL_ASHCROMBE_FIRE        = 6422,
    SPELL_ARUGAL_SPAWN          = 7741
};

// On death event Arugal summons serveral void walkers
const uint8 ArugalVoidCount = 4;
static Movement::Location voidwalkerSpawns[ArugalVoidCount] =
{
    { -154.274368f, 2177.196533f, 128.448517f, 5.760980f },
    { -142.647537f, 2181.019775f, 128.448410f, 4.178475f },
    { -139.146774f, 2168.201904f, 128.448364f, 2.650803f },
    { -150.860092f, 2165.156250f, 128.448502f, 0.999966f }
};

const uint32 prisonerGossipOptionID = 606;

// Deathstalker Adamant (entry: 3849) waypoints
const uint8 adamantWpCount = 31;
const Movement::LocationWithFlag DeathstalkerAdamantWPS[adamantWpCount] =
{
    // Walking to gates
    { -250.923f, 2116.26f, 81.179f, 0, Movement::WP_MOVE_TYPE_WALK },    // 1
    { -255.049f, 2119.39f, 81.179f, 0, Movement::WP_MOVE_TYPE_WALK },    // 2
    { -254.129f, 2123.45f, 81.179f, 0, Movement::WP_MOVE_TYPE_WALK },    // 3
    { -253.898f, 2130.87f, 81.179f, 0, Movement::WP_MOVE_TYPE_WALK },    // 4
    { -249.889f, 2142.31f, 86.972f, 0, Movement::WP_MOVE_TYPE_WALK },    // 5
    { -248.205f, 2144.02f, 87.013f, 0, Movement::WP_MOVE_TYPE_WALK },    // 6
    { -240.553f, 2140.55f, 87.012f, 0, Movement::WP_MOVE_TYPE_WALK },    // 7
    { -237.514f, 2142.07f, 87.012f, 0, Movement::WP_MOVE_TYPE_WALK },    // 8
    { -235.638f, 2149.23f, 90.587f, 0, Movement::WP_MOVE_TYPE_WALK },    // 9
    { -237.188f, 2151.95f, 90.624f, 0, Movement::WP_MOVE_TYPE_WALK },    // 10
    { -239.075424f, 2155.250244f, 90.624168f, 0, Movement::WP_MOVE_TYPE_WALK },    // 11

    // Running out of dungeon
    { -208.764f, 2141.6f, 90.6257f, 0, Movement::WP_MOVE_TYPE_RUN },     // 12
    { -206.441f, 2143.51f, 90.4287f, 0, Movement::WP_MOVE_TYPE_RUN },    // 13
    { -203.715f, 2145.85f, 88.7052f, 0, Movement::WP_MOVE_TYPE_RUN },    // 14
    { -199.199f, 2144.88f, 86.501f, 0, Movement::WP_MOVE_TYPE_RUN },     // 15
    { -195.798f, 2143.58f, 86.501f, 0, Movement::WP_MOVE_TYPE_RUN },     // 16
    { -190.029f, 2141.38f, 83.2712f, 0, Movement::WP_MOVE_TYPE_RUN },    // 17
    { -189.353f, 2138.65f, 83.1102f, 0, Movement::WP_MOVE_TYPE_RUN },    // 18
    { -190.304f, 2135.73f, 81.5288f, 0, Movement::WP_MOVE_TYPE_RUN },    // 19
    { -207.325f, 2112.43f, 81.0548f, 0, Movement::WP_MOVE_TYPE_RUN },    // 20
    { -208.754f, 2109.9f, 81.0527f, 0, Movement::WP_MOVE_TYPE_RUN },     // 21
    { -206.248f, 2108.62f, 81.0555f, 0, Movement::WP_MOVE_TYPE_RUN },    // 22
    { -202.017f, 2106.64f, 78.6836f, 0, Movement::WP_MOVE_TYPE_RUN },    // 23
    { -200.928f, 2104.49f, 78.5569f, 0, Movement::WP_MOVE_TYPE_RUN },    // 24
    { -201.845f, 2101.17f, 76.9256f, 0, Movement::WP_MOVE_TYPE_RUN },    // 25
    { -202.844f, 2100.11f, 76.8911f, 0, Movement::WP_MOVE_TYPE_RUN },    // 26
    { -213.326f, 2105.83f, 76.8925f, 0, Movement::WP_MOVE_TYPE_RUN },    // 27
    { -226.993f, 2111.47f, 76.8892f, 0, Movement::WP_MOVE_TYPE_RUN },    // 28
    { -227.955f, 2112.34f, 76.8895f, 0, Movement::WP_MOVE_TYPE_RUN },    // 39
    { -229.159378f, 2109.524170f, 76.889519f, 0, Movement::WP_MOVE_TYPE_RUN },    // 30
    { }                                                                  // 31 (unused)
};

// Sorcerer Ashcrombe (entry: 3850) waypoints
const uint8 ashcrombeWpCount = 11;
const Movement::Location SorcererAshcrombeWPS[ashcrombeWpCount] =
{
    { -252.528229f, 2126.949951f, 81.179657f, 0 },   // 1
    { -253.898f, 2130.87f, 81.179f, 0 },             // 2
    { -249.889f, 2142.31f, 86.972f, 0 },             // 3
    { -248.205f, 2144.02f, 87.013f, 0 },             // 4
    { -240.553f, 2140.55f, 87.012f, 0 },             // 5
    { -237.514f, 2142.07f, 87.012f, 0 },             // 6
    { -235.638f, 2149.23f, 90.587f, 0 },             // 7
    { -237.188f, 2151.95f, 90.624f, 0 },             // 8
    { -241.162f, 2153.65f, 90.624f, 0 },             // 9
    { -242.24f, 2155.51f, 90.624f, 0 },             // 10
    { }                                                                           // 11 (unused)
};

// Arugal intro 2 (event after nendos death)
const Movement::Location ArugalAtFenrusLoc = { -137.657944f, 2169.928467f, 136.57781f, 2.826001f };

#endif // VERSION_STRING != Cata
