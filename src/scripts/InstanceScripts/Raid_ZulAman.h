/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    //Nalorakk
    CN_NALORAKK         = 23576,
    //Akil'zon
    CN_AKILZON          = 23574,
    CN_SOARING_EAGLE    = 24858,
    //Halazzi <Lynx Avatar>
    CN_HALAZZI          = 23577,
    CN_LYNX_SPIRIT      = 24143,
    CN_TOTEM            = 24224
};

enum CreatureSpells
{
    // NalorakkAI
    NALORAKK_MANGLE             = 44955,
    NALORAKK_SURGE              = 25787,    // 42402 - correct spell hits creature casting spell
    NALORAKK_LACERATING_SLASH   = 42395,
    NALORAKK_REND_FLESH         = 42397,
    NALORAKK_DEAFENING_ROAR     = 42398,
    NALORAKK_BRUTAL_SWIPE       = 42384,
    NALORAKK_BERSERK            = 41924,
    //Akil'zon <Eagle Avatar>
    AKILZON_STATIC_DISRUPTION   = 44008,
    AKILZON_CALL_LIGHTING       = 43661, 
    AKILZON_GUST_OF_WIND        = 43621,
    AKILZON_ELECTRICAL_STORM    = 43648,
    EAGLE_SWOOP                 = 44732,
    //Halazzi <Lynx Avatar>
    HALAZZI_ENRAGE              = 44779,
    HALAZZI_SABER_LASH          = 43267,    //43267 //43268 ///40810 //40816
    HALAZZI_FLAME_SHOCK         = 43303,
    HALAZZI_EARTH_SHOCK         = 43305     //INSTANT , VARIOUS
};
