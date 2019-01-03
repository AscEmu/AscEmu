/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

static Movement::LocationWithFlag AttackerWP[] =
{
    {}, // 0
    { 1858.077f, 804.8599f, 44.00872f, 0.0f, Movement::WP_MOVE_TYPE_RUN }, // Run to middle of entrance platform
    { 1836.152f, 804.7064f, 44.2534f, 0.0f, Movement::WP_MOVE_TYPE_RUN } // Run to door to attack it
};

const LocationVector IntroPortals[] =
{
    { 1878.363770f, 845.572144f, 43.333664f, 4.825092f }, // Left portal
    { 1890.527832f, 758.085510f, 47.666927f, 1.714914f }, // Right portal
    { 1931.233154f, 802.679871f, 52.410446f, 3.112921f }, // Up portal
};

enum CreatureEntry
{
    //Main event
    CN_LIEUTNANT_SINCLARI = 30658,
    CN_VIOLET_HOLD_GUARD = 30659,
    CN_PORTAL_GUARDIAN = 30660,     //enemies
    CN_PORTAL_INTRO = 31011,        //portals, not a go its a creature ;)

    CN_INTRO_AZURE_BINDER_ARCANE = 31007,
    CN_INTRO_AZURE_INVADER_ARMS = 31008,
    CN_INTRO_AZURE_MAGE_SLAYER_MELEE = 31010,
    CN_INTRO_AZURE_SPELLBREAKER_ARCANE = 31009,

    CN_CRYSTAL_SYSTEM = 30837,      // NPC with spell arcane spher

    //Portal Guardians (Normal)
    CN_AZURE_INVADER = 30661,
    CN_AZURE_SPELLBREAKER = 30662,
    CN_AZURE_BINDER = 30663,
    CN_AZURE_MAGE_SLAYER = 30664,
    CN_AZURE_CAPTAIN = 30666,
    CN_AZURE_SORCEROR = 30667,
    CN_AZURE_RAIDER = 30668,
    CN_AZURE_STALKER = 32191,

    //Bosses
    CN_EREKEM = 29315,
    CN_MORAGG = 29316,
    CN_ICHORON = 29313,
    CN_XEVOZZ = 29266,
    CN_LAVANTHOR = 29312,
    CN_TURAMAT_THE_OBLITERATOR = 29314,
    CN_CYANIGOSA = 31134
};

const int VHIntroMobs[] = 
{ 
    CN_INTRO_AZURE_BINDER_ARCANE,
    CN_INTRO_AZURE_INVADER_ARMS,
    CN_INTRO_AZURE_MAGE_SLAYER_MELEE,
    CN_INTRO_AZURE_SPELLBREAKER_ARCANE,
};

const Movement::Location VHPortalLocations[] =
{
    { 1877.51f, 850.104f, 44.6599f, 4.7822f  },
    { 1918.37f, 853.437f, 47.1624f, 4.12294f },
    { 1936.07f, 803.198f, 53.3749f, 3.12414f },
    { 1927.61f, 758.436f, 51.4533f, 2.20891f },
    { 1890.64f, 753.471f, 48.7224f, 1.71042f },
    { 1908.31f, 809.657f, 38.7037f, 3.08701f },
};

enum VHTimers : int32
{
    VH_TIMER_UPDATE = 100,

    VH_TIMER_GUARD_DESPAWN_TIME = 1500,
    VH_TIMER_GUARD_RESPAWN_TIME = 500,
    VH_TIMER_GUARD_FLEE_DELAY = 5750,

    VH_TIMER_SPAWN_INTRO_MOB = 15000,
    VH_TIMER_INTRO_PORTAL_DESPAWN_TIME = VH_TIMER_GUARD_FLEE_DELAY,
};

enum CreatureSpells
{
    //Spell Crytals
    SPELL_ARCANE_LIGHTNING = 57930,

    MORAGG_SPELL_RAY_OF_SUFFERING_H = 59524,
    MORAGG_SPELL_RAY_OF_PAIN_H = 59523,
    MORAGG_SPELL_RAY_OF_SUFFERING = 54442,
    MORAGG_SPELL_RAY_OF_PAIN = 54438,
    MORAGG_SPELL_CORROSIVE_SALIVA = 54527,
    MORAGG_SPELL_OPTIC_LINK = 54396

};

enum GameObjects
{
    //Crystals
    GO_INTRO_ACTIVATION_CRYSTAL = 193615,
    GO_ACTIVATION_CRYSTAL = 193611,

    //Door
    GO_PRISON_SEAL = 191723,
    GO_XEVOZZ_DOOR = 191556,
    GO_LAVANTHOR_DOOR = 191566,
    GO_ICHORON_DOOR = 191722,
    GO_ZURAMAT_THE_OBLITERATOR_DOOR = 191565,
    GO_EREKEM_DOOR = 191564,
    GO_MORAGG_DOOR = 191606
};

enum CreatureSay
{

};

enum CreatureGossip
{
    //Lieutnant Sinclari
    //text
    SINCLARI_ON_HELLO = 13853,
    SINCLARI_ON_FINISH = 13854,
    SINCLARI_OUTSIDE = 14271,

    //item
    SINCLARI_ACTIVATE = 600,
    SINCLARI_GET_SAFETY = 601,
    SINCLARI_SEND_ME_IN = 602
};
