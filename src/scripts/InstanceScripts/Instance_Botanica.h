/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // Bloodwarder Protector AI
    CN_BLOOD_PROTECTOR          = 17993,

    // Bloodwarder Mender AI
    CN_BLOOD_MENDER             = 19633,

    // Bloodwarder Greenkeeper AI
    CN_BLOOD_GREENKEEPER        = 18419,

    // Sunseeker Chemist AI
    CN_SUN_CHEMIST              = 19486,

    // Sunseeker Researcher AI
    CN_SUN_RESEARCHER           = 18421,

    // Commander Sarannis AI
    CN_COMMANDER_SARANNIS       = 17976,    // spawn adds (or maybe write spell which will be harder)

    // High Botanist Freywinn AI
    CN_HIGH_BOTANIST_FREYWINN   = 17975,

    // Thorngrin the Tender AI
    CN_THORNGRIN_THE_TENDER     = 17978,

    // Laj AI
    CN_LAJ                      = 17980,

    // Warp Splinter AI
    CN_WARP_SPLINTER            = 17977
};

enum CreatureSpells
{
    // Bloodwarder Protector AI
    CRYSTAL_STRIKE              = 29765,    // 1 target

    // Bloodwarder Greenkeeper AI
    GREENKEEPER_FURY            = 39121,

    // Sunseeker Chemist AI
    FLAME_BREATH                = 18435,
    POISON_CLOUD                = 37469,

    // Sunseeker Researcher AI
    POISON_SHIELD               = 34355,    // self
    MIND_SHOCK                  = 34352,    // 1 target
    FROST_SHOCK                 = 39062,    // 1 target
    FLAME_SHOCK                 = 22423,    // 1 target

    // Commander Sarannis AI
    ARCANE_RESONANCE            = 34794,
    ARCANE_DEVASTATION          = 34799,
    SUMMON_REINFORCEMENTS       = 34803,    // it's dummy (sss) and must be scripted separately

    // High Botanist Freywinn AI
    PLANT_RED_SEEDLING          = 34763,
    PLANT_GREEN_SEEDLING        = 34761,
    PLANT_WHITE_SEEDLING        = 34759,
    PLANT_BLUE_SEEDLING         = 34762,
    SUMMON_FRAYER_PROTECTOR     = 34557,
    TREE_FORM                   = 34551,
    TRANQUILITY                 = 34550,

    // Thorngrin the Tender AI
    HELLFIRE                    = 34659,    // DBC: 34659, 34660
    SACRIFICE                   = 34661,
    ENRAGE                      = 34670,

    // Laj AI
    ALERGIC_REACTION            = 34697,
    SUMMON_THORN_LASHER         = 34684,    // DBC: 34684, 34681
    SUMMON_THORN_FLAYER         = 34682,    // DBC: 34685, 34682    // they should be spawned on platforms
    TELEPORT_SELF               = 34673,

    // Warp Splinter AI
    STOMP                       = 34716,
    SUMMON_SAPLINGS             = 34727,    // DBC: 34727, 34731, 34733, 34734, 34736, 34739, 34741 (with Ancestral Life spell 34742)   // won't work (guardian summon)
    ARCANE_VOLLEY               = 34785,    //37078, 34785  // must additional script them (because Splinter eats them after 20 sec ^)
    // ^ Doesn't work somehow when used by mob :O
};

enum CreatureSay
{
    // Commander Sarannis AI
    SAY_COMMANDER_SARANNIS_01   = 6109, // Step forward.  I will see that you are properly welcomed!
    SAY_COMMANDER_SARANNIS_02   = 6110, // Oh, stop your whimpering!
    SAY_COMMANDER_SARANNIS_03   = 6111, // Mission accomplished!
    SAY_COMMANDER_SARANNIS_04   = 6112, // You are no longer dealing with some underling!
    SAY_COMMANDER_SARANNIS_05   = 6113, // Band'or shorel'aran!
    SAY_COMMANDER_SARANNIS_06   = 6115, // Guards, rally!  Cut these invaders down!
    SAY_COMMANDER_SARANNIS_07   = 6116, // I have not yet... begun to...

    BROADCAST_SARANNIS          = 6114, // \todo not used %s calls for reinforcements!

    // High Botanist Freywinn AI
    SAY_HIGH_BOTANIS_FREYWIN_01 = 5043, // Endorel anuminor!
    SAY_HIGH_BOTANIS_FREYWIN_02 = 5044, // You will feed the worms.
    SAY_HIGH_BOTANIS_FREYWIN_03 = 5045, // Your life cycle is now concluded.
    SAY_HIGH_BOTANIS_FREYWIN_04 = 5046, // What are you doing ? These specimens are very delicate!
    SAY_HIGH_BOTANIS_FREYWIN_05 = 5489, // Nature bends to my will....
    SAY_HIGH_BOTANIS_FREYWIN_06 = 5490, // The specimens... must be preserved.
    SAY_HIGH_BOTANIS_FREYWIN_07 = 5491, // \todo not used ...thorny vines...mumble...ouch!
    SAY_HIGH_BOTANIS_FREYWIN_08 = 5492, // \todo not used ...mumble mumble...
    SAY_HIGH_BOTANIS_FREYWIN_09 = 5493, // \todo not used ...mumble...Petals of Fire...mumble...
    SAY_HIGH_BOTANIS_FREYWIN_10 = 5494, // \todo not used ...with the right mixture, perhaps...

    // Thorngrin the Tender AI
    SAY_THORNIN_01              = 6117, // What aggravation is this ? You will die!
    SAY_THORNIN_02              = 6118, // You seek a prize, eh ? How about death ?
    SAY_THORNIN_03              = 6119, // I hate to say I told you so...
    SAY_THORNIN_04              = 6120, // Your life will be mine!
    SAY_THORNIN_05              = 6121, // I revel in your pain!
    SAY_THORNIN_06              = 6122, // I'll incinerate you!
    SAY_THORNIN_07              = 6123, // Scream while you burn!
    SAY_THORNIN_08              = 6124, // You won't... get far.

    BROADCAST_THORNIN_01        = 6125, // \todo not used % s becomes enraged!

    // Warp Splinter AI
    SAY_WARP_SPLINTER_01        = 5048, // Who disturbs this sanctuary ?
    SAY_WARP_SPLINTER_02        = 5049, // You must die!But wait : this does not-- No, no... you must die!
    SAY_WARP_SPLINTER_03        = 5050, // What am I doing ? Why do I...
    SAY_WARP_SPLINTER_04        = 5051, // Children, come to me!
    SAY_WARP_SPLINTER_05        = 5052, // Maybe this is not-- No, we fight!Come to my aid!
    SAY_WARP_SPLINTER_06        = 5053  // So... confused.Do not... belong here.

};
