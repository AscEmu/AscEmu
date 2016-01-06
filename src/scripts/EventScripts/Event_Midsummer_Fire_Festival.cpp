/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

#include "Setup.h"

//////////////////////////////////////////////////////////////////////////////////////////
///\details <b>Midsummer Fire Festival</b>\n
/// event_names entry: 1 \n
/// event_names holiday: 341 \n
///\todo Midsummer Fire Festival \n

/// Boss Ahune
#define BOSS_AHUNE              25740
#define NPC_AHUNE_BOTTLE_BUNNY  26346
#define NPC_AHUNE_ICE_BUNNY     25985
#define NPC_GHOST_OF_AHUNE      26239
#define NPC_LOOT_LOC_BUNNY      25746
#define NPC_HAILSTONE           25755
#define NPC_COLDWAVE            25756
#define NPC_FROZENCORE          25865
#define NPC_FROSTWIND           25757
/// Object
#define OBJECT_ICE_STONE        187882
#define OBJECT_SNOW_PILE        188187
/// Quest
#define QUEST_SUMMON_AHUNE      11691
/// Spells
/// Ahune intro and visual
#define SPELL_AHUNE_FLOOR_AMBIENT   46314
#define SPELL_AHUNE_FLOOR           45945
#define SPELL_AHUNE_BONFIRE         45930
#define SPELL_AHUNE_RESURFACE       46402
#define SPELL_AHUNE_GHOST_MODEL     46786
#define SPELL_AHUNE_BEAM_ATT_1      46336
#define SPELL_AHUNE_GHOST_BURST     46809
#define SPELL_AHUNE_STAND           37752
#define SPELL_AHUNE_SUBMERGED       37751
#define SPELL_SUMMONING1_VISUAL     45937
/// Combat spells.
#define SPELL_AHUNE_1_MINION    46103
#define SPELL_AHUNE_SHIELD      45954
#define SPELL_AHUNE_COLD_SLAP   46145
#define SPELL_AHUNE_STUN        46416
/// End spells
#define SPELL_AHUNE_SUMM_LOOT       45939
#define SPELL_AHUNE_SUMM_LOOT_H     46622

/// Text
#define TEXT_AHUNE_SUBMERGE "Ahune Retreats. His defenses diminish."
#define TEXT_AHUNE_EMERGE_W "Ahune will soon resurface."

//////////////////////////////////////////////////////////////////////////////////////////
/// Bonfire

void SetupMidsummerFestival(ScriptMgr* mgr)
{ }