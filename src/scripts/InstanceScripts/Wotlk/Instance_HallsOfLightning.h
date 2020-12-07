/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum
{
    // Main Spells
    SPELL_TEMPER = 52238,

    // Molten Golem Spells
    SPELL_BLAST_WAVE = 23113,
    // 24 seconds + up to 6
    TIMER_STOMP = 24000,

    DISPRESE = 52770,
    SPELL_SUMMON_SPARK = 52746,

    PULSING_SHOCKWAVE_AURA = 59414,
    ARC_LIGHTNING = 52921,
    // 14 seconds + random up to 8
    TIMER_NOVA = 14000,
    TIMER_RESPOND = 18000,

    TIMER_STANCE_CHANGE = 18000,
};

enum GENERAL_STANCES
{
    STANCE_BATTLE = 1,
    STANCE_BERSERKER = 2,
    STANCE_DEFENSIVE = 3,
};

enum CreatureEntry
{
    //GeneralBjarnimAI
    CN_GENERAL_BJARNGRIM    = 28586,

    //Volkhan
    CN_VOLKHAN              = 28587,
    CN_MOLTEN_GOLEM         = 28695,
    CN_BRITTLE_GOLEM        = 28681,
    CN_VOLKHANS_ANVIL       = 28823,

    //Loken
    CN_LOKEN                = 28923,

    //IlonarAI
    CN_IONAR                = 28546,
    CN_SPARK                = 28926
};

enum CreatureSpells
{
    //GeneralBjarnimAI
    SPELL_ARC_WELD              = 59085,
    SPELL_BATTLE_AURA           = 41106,
    SPELL_BATTLE_STANCE         = 53792,
    SPELL_BERSERKER_AURA        = 41107,
    SPELL_BERSERKER_STANCE      = 53791,
    SPELL_CHARGE_UP             = 52098,
    SPELL_CLEAVE                = 15284,
    SPELL_DEFENSIVE_AURA        = 41105,
    SPELL_DEFENSIVE_STANCE      = 53790,
    SPELL_INTERCEPT             = 58769,
    SPELL_IRONFORM              = 52022,
    SPELL_KNOCK_AWAY            = 52029,
    SPELL_MORTAL_STRIKE         = 16856,
    SPELL_PUMMEL                = 12555,
    SPELL_SLAM                  = 52026,
    SPELL_SPELL_REFLECTION      = 36096,
    SPELL_WHIRLWIND             = 52027

};

enum CreatureSay
{

};

enum GameObjectEntry
{
    GO_GENERAL_DOORS        = 191416,
    GO_VOLKHAN_DOORS        = 191325,
    GO_LOKEN_DOORS          = 191324,
    GO_IONAR_DOORS1         = 191326,
    GO_IONAR_DOORS2         = 191328
};
