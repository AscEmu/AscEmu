/*
 * AscScripts for AscEmu Framework
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INSTANCE_RAGEFIRE_CHASM_H
#define _INSTANCE_RAGEFIRE_CHASM_H

enum CreatureEntry
{
    /* TRASH MOBS */
    // Ragefire Shaman AI
    CN_RAGEFIRE_SHAMAN          = 11319,

    // Ragefire Trogg AI
    CN_RAGEFIRE_TROGG           = 11318,

    // Searing Blade Warlock AI
    CN_SEARING_BLADE_WARLOCK    = 11324,

    // SearingBladeEnforcerAI
    CN_SEARING_BLADE_ENFORCER   = 11323,

    // Blade Cultist AI
    CN_BLADE_CULTIST            = 11322,

    // Molten Elemental AI
    CN_MOLTEN_ELEMENTAL         = 11321,

    // Earthborer AI
    CN_EARTHBORER               = 11320,

    // Zelmar
    CN_ZELMAR                   = 17830,

    /* BOSSES */
    // Oggleflint
    CN_OGGLEFLINT               = 11517,

    //Taragaman the Hungerer
    CN_TARAGAMAN                = 11520,

    //Jergosh The Invoker
    CN_JERGOSH                  = 11518,

    //Bazzalan
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


#endif // _INSTANCE_RAGEFIRE_CHASM_H
