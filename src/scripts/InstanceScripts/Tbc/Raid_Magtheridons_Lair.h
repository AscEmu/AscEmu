/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Channelers Coords is list of spawn points of all 5 channelers casting spell on Magtheridon
static LocationVector Channelers[] =
{
    { -55.638000f,   1.869050f, 0.630946f, 0.0f },
    { -31.861300f, -35.919399f, 0.630945f, 0.0f },
    {  10.469200f, -19.894800f, 0.630910f, 0.0f },
    {  10.477100f,  24.445499f, 0.630891f, 0.0f },
    { -32.171600f,  39.926800f, 0.630921f, 0.0f }
};

// Columns coords used for Cave In spell to give "collapse" effect
static LocationVector Columns[] =
{
    {  17.7522f,  34.5464f,  0.144816f, 0.0f },
    {  19.0966f, -29.2772f,  0.133036f, 0.0f },
    { -30.8852f,  46.6723f, -0.497104f, 0.0f },
    { -60.2491f,  27.9361f, -0.018443f, 0.0f },
    { -60.8202f, -21.9269f, -0.030260f, 0.0f },
    { -29.7699f, -43.4445f, -0.522461f, 0.0f }
};

// Cave In Target Triggers coords
static LocationVector CaveInPos[] =
{
    { -37.183399f, -19.491400f,  0.312451f, 0.0f },
    { -11.374900f, -29.121401f,  0.312463f, 0.0f },
    {  13.133100f,   2.757930f, -0.312492f, 0.0f },
    {  -3.110930f,  29.142401f, -0.312490f, 0.0f },
    { -29.691000f,  29.643000f, -0.034676f, 0.0f },
    { -12.111600f,   1.011050f, -0.303638f, 0.0f }
};

// Cube Triggers coords
static LocationVector CubeTriggers[] =
{
    { -54.277199f,   2.343740f, 2.404560f, 0.0f },
    { -31.471001f, -34.155998f, 2.335100f, 0.0f },
    {   8.797220f, -19.480101f, 2.536460f, 0.0f },
    {   9.358900f,  23.228600f, 2.348950f, 0.0f },
    { -31.891800f,  38.430302f, 2.286470f, 0.0f }
};

enum GameObjectIds
{
    GO_MAGTHERIDON_DOOR                 = 183847,
    GO_MANTICRON_CUBE                   = 181713,
    GO_MAGTHERIDON_HALL                 = 184653,
    GO_MAGTHERIDON_COLUMN_0             = 184638,
    GO_MAGTHERIDON_COLUMN_1             = 184639,
    GO_MAGTHERIDON_COLUMN_2             = 184635,
    GO_MAGTHERIDON_COLUMN_3             = 184634,
    GO_MAGTHERIDON_COLUMN_4             = 184636,
    GO_MAGTHERIDON_COLUMN_5             = 184637
};

enum CreatureIds
{
    NPC_MAGTHERIDON                     = 17257,
    NPC_ABYSSAL                         = 17454,
    NPC_HELLFIRE_CHANNELLER             = 17256,
    NPC_TARGET_TRIGGER                  = 17474,
    NPC_WORLD_TRIGGER                   = 21252,
    NPC_HELFIRE_RAID_TRIGGER            = 17376,
    NPC_OLDWORLD_TRIGGER                = 15384,
    NPC_MAGTHERIDON_ROOM                = 17516,
    NPC_HELLFIRE_WARDER                 = 18829
};

enum CreatureSpells
{
    // Magtheridon
    SPELL_BLAST_NOVA                    = 30616,
    SPELL_CLEAVE                        = 30619,
    SPELL_BLAZE_TARGET                  = 30541,
    SPELL_CAMERA_SHAKE                  = 36455,
    SPELL_BERSERK                       = 27680,
    SPELL_QUAKE                         = 30657,
    SPELL_DEBRIS_SERVERSIDE             = 30630,

    // Player or Manticron Cube
    SPELL_SHADOW_CAGE                   = 30168,
    SPELL_SHADOW_GRASP                  = 30410,
    SPELL_MIND_EXHAUSTION               = 44032,

    // Hellfire Raid Trigger
    SPELL_SHADOW_GRASP_VISUAL           = 30166,

    // HellFire Channeler
    SPELL_SHADOW_CAGE_C                 = 30205,
    SPELL_SHADOW_GRASP_C                = 30207,
    SPELL_SHADOW_BOLT_VOLLEY            = 30510,
    SPELL_DARK_MENDING                  = 30528,
    SPELL_BURNING_ABYSSAL               = 30511,
    SPELL_SOUL_TRANSFER                 = 30531,
    SPELL_FEAR                          = 30530,

    // WorldTrigger
    SPELL_DEBRIS_KNOCKDOWN              = 36449,

    // Magtheridon Room
    SPELL_DEBRIS_VISUAL                 = 30632,
    SPELL_DEBRIS_DAMAGE                 = 30631,

    // Target Trigger
    SPELL_BLAZE                         = 30542
};

enum Actions
{
    ACTION_ENABLE                       = 1,
    ACTION_DISABLE                      = 2
};

enum DataTypes
{
    DATA_MAGTHERIDON                    = 0,
    DATA_WORLD_TRIGGER                  = 1,
    DATA_MAGTHERIDON_HALL               = 2,
    DATA_MANTICRON_CUBE                 = 3,
    DATA_COLLAPSE                       = 4,
    DATA_COLLAPSE_2                     = 5
};

enum Events
{
    // Magtheridon
    EVENT_BERSERK = 1,
    EVENT_CLEAVE,
    EVENT_BLAZE,
    EVENT_BLAST_NOVA,
    EVENT_QUAKE,
    EVENT_START_FIGHT,
    EVENT_RELEASED,
    EVENT_COLLAPSE,
    EVENT_DEBRIS_KNOCKDOWN,
    EVENT_DEBRIS,
    EVENT_NEARLY_EMOTE,
    EVENT_TAUNT,
    // Hellfire Channelers events
    EVENT_SHADOWBOLT,
    EVENT_FEAR1,
    EVENT_CHECK_FRIEND,
    EVENT_DARK_MENDING,
    EVENT_ABYSSAL
};

enum Phases
{
    PHASE_BANISH = 1,
    PHASE_1,
    PHASE_2,
    PHASE_3
};

enum Yells
{
    SAY_TAUNT01                         = 8740,
    SAY_TAUNT02                         = 8741,
    SAY_TAUNT03                         = 8742,
    SAY_TAUNT04                         = 8743,
    SAY_TAUNT05                         = 8743,
    SAY_TAUNT06                         = 8745,
    SAY_FREE                            = 8748,
    SAY_SLAY                            = 8751,
    SAY_BANISHED                        = 8749,
    SAY_COLLAPSE                        = 8752,
    SAY_DEATH                           = 8750
    //EMOTE_WEAKEN                        = not in database "%s's bonds begin to weaken!",
    //EMOTE_NEARLY_FREE                   = not in database "%s is nearly free of his bonds!",
    //EMOTE_BREAKS_FREE                   = not in database "%s breaks free!",
    //EMOTE_BLAST_NOVA                    = not in database "%s begins to cast Blast Nova!"
};
