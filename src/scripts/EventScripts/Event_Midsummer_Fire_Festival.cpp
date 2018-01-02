/*
 Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 This file is released under the MIT license. See README-MIT for more information.
 */

#include "Setup.h"

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>Midsummer Fire Festival</b>\n
// event_properties entry: 1 \n
// event_properties holiday: 341 \n
//\todo Midsummer Fire Festival \n

// Boss Ahune
enum MidsummerCreatures
{
    BOSS_AHUNE = 25740,
    NPC_AHUNE_BOTTLE_BUNNY = 26346,
    NPC_AHUNE_ICE_BUNNY = 25985,
    NPC_GHOST_OF_AHUNE = 26239,
    NPC_LOOT_LOC_BUNNY = 25746,
    NPC_HAILSTONE = 25755,
    NPC_COLDWAVE = 25756,
    NPC_FROZENCORE = 25865,
    NPC_FROSTWIND = 25757
};

enum MidsummerObjects
{
    OBJECT_ICE_STONE = 187882,
    OBJECT_SNOW_PILE = 188187
};

enum MidsummerQuests
{
    QUEST_SUMMON_AHUNE = 11691
};

// Ahune intro and visual
enum MidsummerSpells
{
    SPELL_AHUNE_FLOOR_AMBIENT = 46314,
    SPELL_AHUNE_FLOOR = 45945,
    SPELL_AHUNE_BONFIRE = 45930,
    SPELL_AHUNE_RESURFACE = 46402,
    SPELL_AHUNE_GHOST_MODEL = 46786,
    SPELL_AHUNE_BEAM_ATT_1 = 46336,
    SPELL_AHUNE_GHOST_BURST = 46809,
    SPELL_AHUNE_STAND = 37752,
    SPELL_AHUNE_SUBMERGED = 37751,
    SPELL_SUMMONING1_VISUAL = 45937,
    // Combat spells.
    SPELL_AHUNE_1_MINION = 46103,
    SPELL_AHUNE_SHIELD = 45954,
    SPELL_AHUNE_COLD_SLAP = 46145,
    SPELL_AHUNE_STUN = 46416,
    // End spells
    SPELL_AHUNE_SUMM_LOOT = 45939,
    SPELL_AHUNE_SUMM_LOOT_H = 46622
};

// Text
#define TEXT_AHUNE_SUBMERGE "Ahune Retreats. His defenses diminish."
#define TEXT_AHUNE_EMERGE_W "Ahune will soon resurface."

//////////////////////////////////////////////////////////////////////////////////////////
// Bonfire

void SetupMidsummerFestival(ScriptMgr* /*mgr*/)
{ }