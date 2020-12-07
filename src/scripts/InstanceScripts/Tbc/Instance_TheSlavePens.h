/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // CoilfangChampionAI
    CN_COILFANG_CHAMPION            = 17957,

    // CoilfangObserverAI
    CN_COILFANG_OBSERVER            = 17938,

    // CoilfangDefenderAI
    CN_COILFANG_DEFENDER            = 17958,

    // CoilfangScaleHealerAI
    CN_COILFANG_SCALE_HEALER        = 21126,

    // CoilfangSoothsayerAI
    CN_COILFANG_SOOTHSAYER          = 17960,

    // CoilfangTechnicianAI
    CN_COILFANG_TECHNICIAN          = 17940,

    // CoilfangRayAI
    CN_COILFANG_RAY                 = 21128,

    // TotemsAI
    CN_MENNUS_HEALING_WARD          = 20208,
    CN_TAINED_EARTHGRAB_TOTEM       = 18176,
    CN_TAINED_STONESKIN_TOTEM       = 18177,
    CN_CORRUPTED_NOVA_TOTEM         = 14662,    // wrong id?

    // Mennu the BetrayerAI
    CN_MENNU_THE_BETRAYER           = 17941,

    // Rokmar the CracklerAI
    CN_ROKMAR_THE_CRACKLER          = 17991,

    // QuagmirranAI
    CN_QUAGMIRRAN                   = 17942

};

enum CreatureSpells
{
    // CoilfangChampionAI
    INTIMIDATING_SHOUT          = 33789,    // or 38945 || after it on off can go to next unfeared target (mostly healer or ranged unit)


    // CoilfangObserverAI
    IMMOLATE                    = 29928,

    // CoilfangDefenderAI
    REFLECTIVE_SHIELD           = 41475,    // No idea which id it should be: Reflective Damage Shield (35159), Reflective Magic Shield (35158), Reflective Shield (41475)

    // CoilfangScaleHealerAI
    HOLY_NOVA                   = 37669,    // can be: 37669, 34944, 41380, 40096, 36985
    POWER_WORD_SHIELD           = 36052,    // can be also: 36052, 29408, 41373, 32595, 35944
    GREATER_HEAL                = 35096,    // all spellids are just my thoughtfulness

    // CoilfangSoothsayerAI
    /// \todo still spell doesn't work, because of lack of core support
    MIND_CONTROL                = 36797,    // maybe: 36797 or 36798 or ... no idea to id, but 

    // CoilfangTechnicianAI
    RAIN_OF_FIRE                = 34435,    // can be: 34360, 34435, 37465, 38635, 39024, 31340, 33617, 39363
    BLIZZARD                    = 30093,    // can be: 30093, 29951, 37263, 38646, 31266, 34356

    // CoilfangRayAI
    HOWL_OF_TERROR              = 39048,

    // TotemsAI
    HW_MENNUS_HEALING_WARD      = 34977,
    ET_ENTANGLING_ROOTS         = 20654,
    ST_                         = 31985,
    NT_                         = 31991,

    // Mennu the BetrayerAI
    /// \todo First 4 spells don't work as more core support is needed for them
    MENNUS_HEALING_WARD         = 34980,
    TAINTED_EARTHGRAB_TOTEM     = 31981,    //31982 //31981
    TAINTED_STONESKIN_TOTEM     = 31985,
    CORRUPTED_NOVA_TOTEM        = 31991,
    LIGHTNING_BOLT              = 36152,

    // Rokmar the CracklerAI
    GRIEVOUS_WOUND              = 31956,
    WATER_SPIT                  = 35008,
    ENSNARING_MOSS              = 31948,
    ENRAGE                      = 37023,    // ofc not sure ;) maybe: 41305

    // QuagmirranAI
    ACID_GEYSER                 = 38971,    // it isn't right spell (TOO POWERFUL), but I couldn't find correct one for now (as others Idk why don't want to work)
    POISON_BOLT_VOLLEY          = 39340,    // maybe be also: 40095, but it isn't dispelable
    CLEAVE                      = 38474,    // 31345, no idea if this is correct

};

enum CreatureSay
{
    // Mennu the BetrayerAI
    SAY_MENNU_BETRAYER_01       = 7385,     // The work must continue!
    SAY_MENNU_BETRAYER_02       = 7386,     // You brought this on yourselves!
    SAY_MENNU_BETRAYER_03       = 7387,     // Don't make me kill you!
    SAY_MENNU_BETRAYER_04       = 7388,     // It had to be done....
    SAY_MENNU_BETRAYER_05       = 7389,     // You should not have come....
    SAY_MENNU_BETRAYER_06       = 7390,     // I... deserve this....

    // Rokmar the CracklerAI
    // boss without sounds

};
