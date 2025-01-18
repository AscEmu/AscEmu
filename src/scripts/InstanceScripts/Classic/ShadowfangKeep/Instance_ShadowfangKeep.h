/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "LocationVector.h"

namespace ShadowfangKeep
{
    enum SFK_encounterIndexes : uint8_t
    {
        // Main encounters
        INDEX_VOIDWALKER = 0,
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

    enum SFK_creatureEntries : uint32_t
    {
        // Bosses
        CN_RAZORCLAW_THE_BUTCHER = 3886,
        CN_BARON_SILVERLAINE = 3887,
        CN_RETHILGORE = 3914,
        CN_NANDOS = 3927,
        CN_FENRUS = 4274,
        CN_ARUGAL_BOSS = 4275,
        CN_SPRINGVALE = 4278,
        CN_BLINDWATCHER = 4279,

        // Prisoners
        CN_ADAMANT = 3849,
        CN_ASHCROMBE = 3850,

        // Intro event
        CN_ARUGAL = 10000,
        CN_DEATHSTALKER_VINCENT = 4444,

        // Trash mobs
        CN_BLEAK_WORG = 3861,
        CN_SLAVERING_WORG = 3862,
        CN_LUPINE_HORROR = 3863,
        CN_LUPINE_DELUSION = 5097,
        CN_VOIDWALKER = 4627,
    };

    enum SFK_scriptTexts : uint32_t
    {
        // Sorcerer Ashcrombe texts
        SAY_ASHCROMBE_BYE = 2115,     // "There it is! Wide open. Good luck to you conquering what lies beyond. I must report back to the Kirin Tor at once!"
        SAY_ASHCROMBE_FOLLOW = 2117,     // "Follow me and I'll open the courtyard door for you."
        SAY_ASHCROMBE_OPEN_DOOR = 2118,     // "I have just the spell to get this door open. Too bad the cell doors weren't locked so haphazardly."
        SAY_ASHCROMBE_BOSS_DEATH = 3262,     // "For once I agree with you... scum."
        SAY_ASHCROMBE_VANISH = 8788,     // "Sorcerer Ashcrombe vanishes."

        // Deathstalker Adamant texts
        SAY_ADAMANT_BOSS_DEATH = 5251,     // "About time someone killed the wretch."
        SAY_ADAMANT_FOLLOW = 2122,     // "Free from this wretched cell at last! Let me show you to the courtyard...."
        SAY_ADAMANT_BYE = 2120,     // "Good luck with Arugal. I must hurry back to Hadrec now."
        SAY_ADAMANT_BEFORE_OPEN = 2121,     // "You are indeed courageous for wanting to brave the horrors that lie beyond this door."
        SAY_ADAMANT_AFTER_OPEN = 2119,     // "There we go!"
        SAY_ADAMANT_OPENING = 8787,     // "Deathstalker Adamant fumbles with the rusty lock on the courtyard door."

        // Arugal intro event
        SAY_ARUGAL_INTRO1 = 8789,     // "I have changed my mind loyal servants, you do not need to bring the prisoner all the way to my study, I will deal with him here and now."
        SAY_ARUGAL_INTRO2 = 8790,     // "Vincent! You and your pathetic ilk will find no more success in routing my sons and I than those beggardly remnants of the Kirin Tor."
        SAY_ARUGAL_INTRO3 = 8791,     // "If you will not serve my Master with your sword and knowledge of his enemies..."
        SAY_ARUGAL_INTRO4 = 8792,     // "Your moldering remains will serve ME as a testament to what happens when one is foolish enough to trespass in my domain!"
        SAY_VINCENT_DEATH = 8793,     // "Arrrgh!"

        // Arugal boss texts
        YELL_ARUGAL_FENRUS = 2116,     // "Who dares interfere with the Sons of Arugal?"
        YELL_ARUGAL_AGROO = 8794,     // "You, too, shall serve!"
        YELL_ARUGAL_ENEMY_DEATH = 8795,     // "Another Falls!"
        YELL_ARUGAL_COMBAT = 8796      // "Release your rage!"
    };

    enum SFK_gameobjectEntries : uint32_t
    {
        GO_SORCERER_GATE = 18972,

        // Prison Levers
        GO_RIGHT_LEVER = 18900,
        GO_MIDDLE_LEVER = 18901,
        GO_LEFT_LEVER = 101811,
        GO_RIGHT_CELL = 18934,
        GO_MIDDLE_CELL = 18936,
        GO_LEFT_CELL = 18935,
        GO_ARUGALS_LAIR_GATE = 18971,
        GO_ARUGAL_FOCUS = 18973,                        // this generates the lightning visual in the Fenrus event
        GO_COURTYARD_DOOR = 18895
    };

    enum SFK_spellEntries : uint32_t
    {
        SPELL_ASHCROMBE_UNLOCK = 6421,
        SPELL_ASHCROMBE_FIRE = 6422,
        SPELL_ARUGAL_SPAWN = 7741
    };

    // On death event Arugal summons serveral void walkers
    const uint8_t ArugalVoidCount = 4;
    static LocationVector voidwalkerSpawns[ArugalVoidCount] =
    {
        { -154.274368f, 2177.196533f, 128.448517f, 5.760980f },
        { -142.647537f, 2181.019775f, 128.448410f, 4.178475f },
        { -139.146774f, 2168.201904f, 128.448364f, 2.650803f },
        { -150.860092f, 2165.156250f, 128.448502f, 0.999966f }
    };

    const uint32_t prisonerGossipOptionID = 606;

    // Deathstalker Adamant (entry: 3849) waypoints
    const uint32_t waypoint_script = 1;

    // Sorcerer Ashcrombe (entry: 3850) waypoints
    const uint8_t ashcrombeWpCount = 11;
    const LocationVector SorcererAshcrombeWPS[ashcrombeWpCount] =
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
    const LocationVector ArugalAtFenrusLoc = { -137.657944f, 2169.928467f, 136.57781f, 2.826001f };
}
