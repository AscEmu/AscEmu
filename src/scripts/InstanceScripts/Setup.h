/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Units/Creatures/AIInterface.h"
#include "Management/Item.h"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Management/QuestLogEntry.hpp"
#include "Map/MapScriptInterface.h"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Map/WorldCreatorDefines.hpp"

void SetupArcatraz(ScriptMgr* mgr);
void SetupAuchenaiCrypts(ScriptMgr* mgr);
void SetupAzjolNerub(ScriptMgr* mgr);
void SetupTheBlackMorass(ScriptMgr* mgr);
void SetupBlackfathomDeeps(ScriptMgr* mgr);
void SetupBlackrockSpire(ScriptMgr* mgr);
void SetupBlackrockDepths(ScriptMgr* mgr);
void SetupBloodFurnace(ScriptMgr* mgr);
void SetupBotanica(ScriptMgr* mgr);
void SetupCullingOfStratholme(ScriptMgr* mgr);
void SetupDrakTharonKeep(ScriptMgr* pScriptMgr);
void SetupDeadmines(ScriptMgr* mgr);
void SetupDireMaul(ScriptMgr* mgr);
void SetupForgeOfSouls(ScriptMgr* mgr);
void SetupGundrak(ScriptMgr* mgr);
void SetupHellfireRamparts(ScriptMgr* mgr);
void SetupHallsOfStone(ScriptMgr* mgr);
void SetupHallsOfReflection(ScriptMgr* mgr);
void SetupHallsOfLightning(ScriptMgr* mgr);
void SetupManaTombs(ScriptMgr* mgr);
void SetupMaraudon(ScriptMgr* mgr);
void SetupNexus(ScriptMgr* mgr);
void SetupOldHillsbradFoothills(ScriptMgr* mgr);
void SetupPitOfSaron(ScriptMgr* mgr);
void SetupRagefireChasm(ScriptMgr* mgr);
void SetupRazorfenDowns(ScriptMgr* mgr);
void SetupScarletMonastery(ScriptMgr* mgr);
void SetupScholomance(ScriptMgr* mgr);
void SetupSethekkHalls(ScriptMgr* mgr);
void SetupShadowfangKeep(ScriptMgr* mgr);
void SetupShadowLabyrinth(ScriptMgr* mgr);
void SetupTheMechanar(ScriptMgr* mgr);
void SetupTheShatteredHalls(ScriptMgr* mgr);
void SetupTheSlavePens(ScriptMgr* mgr);
void SetupTheSteamvault(ScriptMgr* mgr);
void SetupTheUnderbog(ScriptMgr* mgr);
void SetupUldaman(ScriptMgr* mgr);
void SetupUtgardeKeep(ScriptMgr* mgr);
void SetupUtgardePinnacle(ScriptMgr* mgr);
void SetupTheStockade(ScriptMgr* mgr);
void SetupTheVioletHold(ScriptMgr* mgr);
void SetupWailingCaverns(ScriptMgr* mgr);
void SetupMagistersTerrace(ScriptMgr* mgr);

//Raids
void SetupBlackTemple(ScriptMgr* mgr);
void SetupBlackwingLair(ScriptMgr* mgr);
void SetupBattleOfMountHyjal(ScriptMgr* mgr);
void SetupGruulsLair(ScriptMgr* mgr);
void SetupICC(ScriptMgr* mgr);
void SetupKarazhan(ScriptMgr* mgr);
void SetupMoltenCore(ScriptMgr* mgr);
void SetupNaxxramas(ScriptMgr* mgr);
void SetupOnyxiasLair(ScriptMgr* mgr);
void SetupTheEye(ScriptMgr* mgr);
void SetupTheObsidianSanctum(ScriptMgr* mgr);
void SetupUlduar(ScriptMgr* mgr);
void SetupZulFarrak(ScriptMgr* mgr);
void SetupZulGurub(ScriptMgr* mgr);
void SetupSerpentshrineCavern(ScriptMgr* mgr);
void SetupMagtheridonsLair(ScriptMgr* mgr);
void SetupSunwellPlateau(ScriptMgr* pScriptMgr);
void SetupWorldBosses(ScriptMgr* mgr);
void SetupZulAman(ScriptMgr* mgr);

//\todo create for all instance scripts for these maps... best example how encounter states work ->Raid_IceCrownCitadel.cpp
enum InstanceMaps
{
    MAP_SHADOWFANG_KEEP     = 33,   // Shadowfang Keep
    MAP_THE_STOCKADE        = 34,   // Stormwind Stockade
    MAP_DEADMINES           = 36,   // Deadmines
    MAP_WAILING_CAVERNS     = 43,   // Wailing Caverns
    MAP_BLACKFATHOM_DEEPS   = 48,   // Blackfathom Deeps
    MAP_ULDAMAN             = 70,   // Uldaman

    MAP_RAZORFEN_DOWNS      = 129,  // Razorfen Downs
    MAP_SCARLET_MONASTERY   = 189,  // Scarlet Monastery

    MAP_ZUL_FARAK           = 209,  // Zul'Farrak
    MAP_BLACKROCK_SPIRE     = 229,  // Blackrock Spire
    MAP_BLACKROCK_DEPTHS    = 230,  // Blackrock Depths
    MAP_COT_BLACK_MORASS    = 269,  // rename this Opening of the Dark Portal -> Caverns of Time: Black Morass
    MAP_SCHOLOMANCE         = 289,  // Scholomance
    MAP_MARAUDON            = 349,  // Maraudon

    MAP_RAGEFIRE_CHASM      = 389,  // Ragefire Chasm

    MAP_DIRE_MAUL           = 429,  // Dire Maul
    MAP_NAXXRAMAS           = 533,  // Naxxramas
    MAP_HYJALPAST           = 534,  // The Battle for Mount Hyjal

    MAP_HC_SHATTERED_HALLS  = 540,  // Hellfire Citadel: The Shattered Halls
    MAP_HC_BLOOD_FURNANCE   = 542,  // Hellfire Citadel: The Blood Furnace
    MAP_HC_RAMPARTS         = 543,  // Hellfire Citadel: Ramparts

    MAP_CF_STEAMVAULT       = 545,  // Coilfang: The Steamvault
    MAP_CF_THE_UNDERBOG     = 546,  // Coilfang: The Underbog
    MAP_CF_SLAVE_PENS       = 547,  // Coilfang: The Slave Pens
    MAP_CF_SERPENTSHRINE_CA = 548,  // Coilfang: Serpentshrine Cavern

    MAP_TK_THE_ARCATRAZ     = 552,  // Tempest Keep: The Arcatraz
    MAP_TK_THE_BOTANICA     = 553,  // Tempest Keep: The Botanica
    MAP_TK_THE_MECHANAR     = 554,  // Tempest Keep: The Mechanar

    MAP_AUCHENAI_SHADOWLAB  = 555,  // Auchindoun: Shadow Labyrinth
    MAP_AUCHENAI_SETHEKK    = 556,  // Auchindoun: Sethekk Halls
    MAP_AUCHENAI_MANA_TOMBS = 557,  // Auchindoun: Mana-Tombs
    MAP_AUCHENAI_CRYPT      = 558,  // Auchindoun: Auchenai Crypts

    MAP_BLACK_TEMPLE        = 564,  // Black Temple

    MAP_UTGARDE_KEEP        = 574,  // Utgarde Keep
    MAP_UTGARDE_PINNACLE    = 575,  // Utgarde Pinnacle

    MAP_NEXUS               = 576,  // Nexus
    MAP_MAGISTERS_TERRACE   = 585,  // Magister's Terrace
    MAP_HALLS_OF_STONE      = 599,  // Halls of Stone
    MAP_DRAK_THARON_KEEP    = 600,  // Drak'Tharon Keep
    MAP_AZJOL_NERUB         = 601,  // Azjol-Nerub
    MAP_HALLS_OF_LIGHTNING  = 602,  // Halls of Lightning
    MAP_GUNDRAK             = 604,  // Gundrak

    MAP_ULDUAR              = 603,  // Ulduar

    MAP_VIOLET_HOLD         = 608,  // The Violet Hold
    MAP_OBSIDIAN_SANCTUM    = 615,  // ObsidianSanctum
    MAP_ICECROWNCITADEL     = 631,  // Icecrown Citadel
    MAP_FORGE_OF_SOULS      = 632,  // Forge of Souls
    MAP_PIT_OF_SARON        = 658,  // Pit of Saron.
    MAP_HALLSOFREFLECTION   = 668,  // Halls of Reflection

    /*
    30	Alterac Valley
    44	<unused> Monastery
    47	Razorfen Kraul
    90	Gnomeregan

    109	Sunken Temple
    169	Emerald Dream

    249	Onyxia's Lair

    309	Zul'Gurub
    329	Stratholme

    409	Molten Core
    469	Blackwing Lair
    489	Warsong Gulch

    509	Ruins of Ahn'Qiraj
    529	Arathi Basin
    531	Ahn'Qiraj Temple
    532	Karazhan
    544	Magtheridon's Lair
    548	Coilfang: Serpentshrine Cavern
    550	Tempest Keep
    552	Tempest Keep: The Arcatraz
    559	Nagrand Arena
    560	The Escape From Durnholde
    562	Blade's Edge Arena
    565	Gruul's Lair
    566	Eye of the Storm
    568	Zul'Aman
    572	Ruins of Lordaeron

    578	The Oculus
    580	The Sunwell
    595	The Culling of Stratholme
    598	Sunwell Fix (Unused)

    607	Strand of the Ancients
    615	The Obsidian Sanctum
    616	The Eye of Eternity
    617	Dalaran Sewers
    618	The Ring of Valor
    619	Ahn'kahet: The Old Kingdom
    624	Vault of Archavon
    628	Isle of Conquest
    649	Trial of the Crusader
    650	Trial of the Champion
    724	The Ruby Sanctum*/
};

enum InstanceAreas
{
    AREA_VIOLET_HOLD = 4415
};
