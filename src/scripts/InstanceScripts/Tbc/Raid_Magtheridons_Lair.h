/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once


enum QuestEvents
{
    CN_CUBE_TRIGGER = 17376,

    // Encounter Settings
    BANISH_TIMER = 120, // 2 minutes
    YELL_TIMER = 260, // 4 minutes and 20 seconds
    ACTIVE_CUBES_TO_BANISH = 5, // 5 cubes

    // Magtheridon Trigger AI - Creature AI Class
    CN_MAGTHERIDON_TRIGGER = 133338,

    // Spell macros used in whole script
    BANISH = 30231, // 31797
    BANISHMENT = 40825,
    SOUL_TRANSFER = 30531,
    SHADOW_GRASP = 30166, // 30207
    SHADOW_GRASP2 = 30410,

    // Manticron Cube Gameobject

    MANTICRON_CUBE = 181713,

    MIND_EXHAUSTION = 44032,

    // Hellfire Warder
    CN_HELLFIRE_WARDER = 18829,

    HW_SHADOW_BOLT_VOLLEY = 36275, // 39175
    SHADOW_WORD_PAIN = 34441,
    UNSTABLE_AFFLICTION = 35183,
    DEATH_COIL = 33130,
    RAIN_OF_FIRE = 34435,
    HW_FEAR = 34259, // this one is probably wrong
    SHADOW_BURST = 34436,

    // Hellfire Channeler
    CN_HELLFIRE_CHANNELER = 17256,

    SHADOW_BOLT_VOLLEY = 30510, // 39175
    FEAR = 30615, // not sure
    DARK_MENDING = 30528,
    BURNING_ABYSSAL = 30511,

    CN_BURNING_ABYSSAL = 17454,

    FIRE_BLAST = 37110,

    // Magtheridon
    CN_MAGTHERIDON = 17257,

    // Normal Casts
    CLEAVE = 31345, // not sure; should be better, but not sure if it gives 8k dmg
    CONFLAGRATION = 23023, // 35840 - this one was affecting caster; not sure - it's not right spell, but better than nothing for now

    // Timed Casts
    QUAKE1 = 30571, // Each 40 sec after Phase 2 starts
    QUAKE2 = 30658, // Effect
    BLAST_NOVA = 30616,// Each 60 sec after Phase 2 starts
    CAVE_IN = 36240, // don't know correct timer
    CAMERA_SHAKE = 36455, // is used when Magtheridon uses Cave In
    ENRAGE = 34624, // dunno if it's correct spell    -- 20 min after Phase 2 starts
};

// Channelers Coords is list of spawn points of all 5 channelers casting spell on Magtheridon
static Movement::Location Channelers[] =
{
    { -55.638000f,   1.869050f, 0.630946f },
    { -31.861300f, -35.919399f, 0.630945f },
    {  10.469200f, -19.894800f, 0.630910f },
    {  10.477100f,  24.445499f, 0.630891f },
    { -32.171600f,  39.926800f, 0.630921f }
};

// Columns coords used for Cave In spell to give "collapse" effect
static Movement::Location Columns[] =
{
    {  17.7522f,  34.5464f,  0.144816f },
    {  19.0966f, -29.2772f,  0.133036f },
    { -30.8852f,  46.6723f, -0.497104f },
    { -60.2491f,  27.9361f, -0.018443f },
    { -60.8202f, -21.9269f, -0.030260f },
    { -29.7699f, -43.4445f, -0.522461f }
};

// Cave In Target Triggers coords
static Movement::Location CaveInPos[] =
{
    { -37.183399f, -19.491400f,  0.312451f },
    { -11.374900f, -29.121401f,  0.312463f },
    {  13.133100f,   2.757930f, -0.312492f },
    {  -3.110930f,  29.142401f, -0.312490f },
    { -29.691000f,  29.643000f, -0.034676f },
    { -12.111600f,   1.011050f, -0.303638f }
};

// Cube Triggers coords
static Movement::Location CubeTriggers[] =
{
    { -54.277199f,   2.343740f, 2.404560f },
    { -31.471001f, -34.155998f, 2.335100f },
    {   8.797220f, -19.480101f, 2.536460f },
    {   9.358900f,  23.228600f, 2.348950f },
    { -31.891800f,  38.430302f, 2.286470f }
};

enum CreatureEntry
{
};

enum CreatureSpells
{
};
