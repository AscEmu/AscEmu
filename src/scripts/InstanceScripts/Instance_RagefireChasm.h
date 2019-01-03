/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // TRASH MOBS
    CN_RAGEFIRE_SHAMAN          = 11319,
    CN_RAGEFIRE_TROGG           = 11318,
    CN_SEARING_BLADE_WARLOCK    = 11324,
    CN_SEARING_BLADE_ENFORCER   = 11323,
    CN_BLADE_CULTIST            = 11322,
    CN_MOLTEN_ELEMENTAL         = 11321,
    CN_EARTHBORER               = 11320,
    CN_ZELMAR                   = 17830,

    // BOSSES
    CN_OGGLEFLINT               = 11517,
    CN_TARAGAMAN                = 11520,
    CN_JERGOSH                  = 11518,
    CN_BAZZALAN                 = 11519
};

enum CreatureSpells
{
    // Ragefire Shaman AI
    SP_RF_SHAMAN_HEALIN_WAVE        = 11986,
    SP_RF_SHAMAN_LIGHTNING_BOLT     = 9532,

    // Ragefire Trogg AI
    SP_RF_TROGG_STRIKE              = 11976,

    // Searing Blade Warlock AI
    SP_SB_WARLOCK_SHADOW_BOLT       = 20791,

    // SearingBladeEnforcerAI
    SP_SB_ENFORCERER_SHIELD_SLAM    = 8242,

    // Blade Cultist AI
    SP_SB_CULTIST_CURSE_OF_AGONY    = 18266,

    // Molten Elemental AI
    SP_MOLTEN_ELEMENTAL_FIRE_SHIELD = 134,

    // Earthborer AI
    SP_EARTHBORER_ACID              = 18070,

    // Oggleflint
    SP_OGGLEFLINT_CLEAVE            = 40505,

    //Taragaman the Hungerer
    SP_TARAGAMAN_FIRE_NOVA          = 11970,
    SP_TARAGAMAN_UPPERCUT           = 18072,

    //Jergosh The Invoker
    SP_JERGOSH_IMMOLATE             = 20800,
    SP_JERGOSH_CURSE_OF_WEAKNESS    = 18267,

    //Bazzalan
    SP_BAZZLAN_SINISTER_STRIKE      = 14873,
    SP_BAZZLAN_POISON               = 18197     /// \todo correct id?
};

enum CreatureSay
{

};

enum GameObjectEntry
{
    // BloodFilledOrb
    GO_BLOOD_FILLED_ORB         = 182024
};
