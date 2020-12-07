/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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
#include "Spell/SpellMgr.h"
#include "Map/WorldCreatorDefines.hpp"

//Classic
void SetupBlackfathomDeeps(ScriptMgr* mgr);
void SetupBlackrockSpire(ScriptMgr* mgr);
void SetupBlackrockDepths(ScriptMgr* mgr);
void SetupDeadmines(ScriptMgr* mgr);
void SetupDireMaul(ScriptMgr* mgr);
void SetupGnomeregan(ScriptMgr* mgr);
void SetupMaraudon(ScriptMgr* mgr);
void SetupRagefireChasm(ScriptMgr* mgr);
void SetupRazorfenDowns(ScriptMgr* mgr);
void SetupRazorfenKraul(ScriptMgr* mgr);
void SetupScarletMonastery(ScriptMgr* mgr);
void SetupScholomance(ScriptMgr* mgr);
void SetupShadowfangKeep(ScriptMgr* mgr);
void SetupStratholme(ScriptMgr* mgr);
void SetupTheTempleOfAtalHakkar(ScriptMgr* mgr);
void SetupUldaman(ScriptMgr* mgr);
void SetupWailingCaverns(ScriptMgr* mgr);
void SetupZulFarrak(ScriptMgr* mgr);

//TBC
void SetupArcatraz(ScriptMgr* mgr);
void SetupAuchenaiCrypts(ScriptMgr* mgr);
void SetupTheBlackMorass(ScriptMgr* mgr);
void SetupBloodFurnace(ScriptMgr* mgr);
void SetupBotanica(ScriptMgr* mgr);
void SetupHellfireRamparts(ScriptMgr* mgr);
void SetupMagistersTerrace(ScriptMgr* mgr);
void SetupManaTombs(ScriptMgr* mgr);
void SetupOldHillsbradFoothills(ScriptMgr* mgr);
void SetupSethekkHalls(ScriptMgr* mgr);
void SetupShadowLabyrinth(ScriptMgr* mgr);
void SetupTheMechanar(ScriptMgr* mgr);
void SetupTheShatteredHalls(ScriptMgr* mgr);
void SetupTheSlavePens(ScriptMgr* mgr);
void SetupTheSteamvault(ScriptMgr* mgr);
void SetupTheStockade(ScriptMgr* mgr);
void SetupTheUnderbog(ScriptMgr* mgr);

//Wotlk
void SetupAhnKahetTheOldKingdom(ScriptMgr* mgr);
void SetupAzjolNerub(ScriptMgr* mgr);
void SetupCullingOfStratholme(ScriptMgr* mgr);
void SetupDrakTharonKeep(ScriptMgr* mgr);
void SetupEyeOfEternity(ScriptMgr* mgr);
void SetupForgeOfSouls(ScriptMgr* mgr);
void SetupGundrak(ScriptMgr* mgr);
void SetupHallsOfLightning(ScriptMgr* mgr);
void SetupHallsOfReflection(ScriptMgr* mgr);
void SetupHallsOfStone(ScriptMgr* mgr);
void SetupNexus(ScriptMgr* mgr);
void SetupPitOfSaron(ScriptMgr* mgr);
void SetupTheOculus(ScriptMgr* mgr);
void SetupTheVioletHold(ScriptMgr* mgr);
void SetupTrialOfTheChampion(ScriptMgr* mgr);
void SetupUtgardeKeep(ScriptMgr* mgr);
void SetupUtgardePinnacle(ScriptMgr* mgr);
void SetupVaultOfArchavon(ScriptMgr* mgr);


//////////////////////////////////////////////////////////////////////////////////////////
// Raids

//Classic
void SetupBlackwingLair(ScriptMgr* mgr);
void SetupOnyxiasLair(ScriptMgr* mgr);
void SetupMoltenCore(ScriptMgr* mgr);
void SetupZulGurub(ScriptMgr* mgr);
void SetupWorldBosses(ScriptMgr* mgr);

//Tbc
void SetupBlackTemple(ScriptMgr* mgr);
void SetupKarazhan(ScriptMgr* mgr);
void SetupBattleOfMountHyjal(ScriptMgr* mgr);
void SetupZulAman(ScriptMgr* mgr);
void SetupSunwellPlateau(ScriptMgr* mgr);
void SetupSerpentshrineCavern(ScriptMgr* mgr);
void SetupMagtheridonsLair(ScriptMgr* mgr);
void SetupTheEye(ScriptMgr* mgr);
void SetupGruulsLair(ScriptMgr* mgr);

//Wotlk
void SetupICC(ScriptMgr* mgr);
void SetupTheObsidianSanctum(ScriptMgr* mgr);
void SetupNaxxramas(ScriptMgr* mgr);
void SetupUlduar(ScriptMgr* mgr);
void SetupTrialOfTheCrusader(ScriptMgr* mgr);


enum InstanceMaps
{
    MAP_SHADOWFANG_KEEP         = 33,   // Shadowfang Keep
    MAP_THE_STOCKADE            = 34,   // Stormwind Stockade
    MAP_DEADMINES               = 36,   // Deadmines
    MAP_WAILING_CAVERNS         = 43,   // Wailing Caverns
    MAP_RAZORFEN_KRAUL          = 47,   // Razorfen Kraul
    MAP_BLACKFATHOM_DEEPS       = 48,   // Blackfathom Deeps
    MAP_ULDAMAN                 = 70,   // Uldaman
    MAP_GNOMEREGAN              = 90,   // Gnomeregan

    MAP_SUNKEN_TEMPLE           = 109,  // Tempel of Atal Hakkar
    MAP_RAZORFEN_DOWNS          = 129,  // Razorfen Downs
    MAP_SCARLET_MONASTERY       = 189,  // Scarlet Monastery

    MAP_ZUL_FARAK               = 209,  // Zul'Farrak
    MAP_BLACKROCK_SPIRE         = 229,  // Blackrock Spire
    MAP_BLACKROCK_DEPTHS        = 230,  // Blackrock Depths
    MAP_ONYXIAS_LAIR            = 249,  // Onyxia's Lair
    MAP_COT_BLACK_MORASS        = 269,  // rename this Opening of the Dark Portal -> Caverns of Time: Black Morass
    MAP_SCHOLOMANCE             = 289,  // Scholomance
    MAP_ZUL_GURUB               = 309,  // Zul'Gurub
    MAP_STRATHOLME              = 329,  // Stratholme
    MAP_MARAUDON                = 349,  // Maraudon

    MAP_RAGEFIRE_CHASM          = 389,  // Ragefire Chasm
    MAP_MOLTEN_CORE             = 409,  // Molten Core

    MAP_DIRE_MAUL               = 429,  // Dire Maul
    MAP_BLACKWING_LAIR          = 469,  // Blackwing Lair
    MAP_KARAZHAN                = 532,  // Karazhan
    MAP_NAXXRAMAS               = 533,  // Naxxramas
    MAP_HYJALPAST               = 534,  // The Battle for Mount Hyjal

    MAP_HC_SHATTERED_HALLS      = 540,  // Hellfire Citadel: The Shattered Halls
    MAP_HC_BLOOD_FURNANCE       = 542,  // Hellfire Citadel: The Blood Furnace
    MAP_HC_RAMPARTS             = 543,  // Hellfire Citadel: Ramparts
    MAP_MAGTHERIDONS_LAIR       = 544,  // Magtheridon's Lair
    MAP_CF_STEAMVAULT           = 545,  // Coilfang: The Steamvault
    MAP_CF_THE_UNDERBOG         = 546,  // Coilfang: The Underbog
    MAP_CF_SLAVE_PENS           = 547,  // Coilfang: The Slave Pens
    MAP_CF_SERPENTSHRINE_CA     = 548,  // Coilfang: Serpentshrine Cavern

    MAP_TK_THE_ARCATRAZ         = 552,  // Tempest Keep: The Arcatraz
    MAP_TK_THE_BOTANICA         = 553,  // Tempest Keep: The Botanica
    MAP_TK_THE_MECHANAR         = 554,  // Tempest Keep: The Mechanar

    MAP_AUCHENAI_SHADOWLAB      = 555,  // Auchindoun: Shadow Labyrinth
    MAP_AUCHENAI_SETHEKK        = 556,  // Auchindoun: Sethekk Halls
    MAP_AUCHENAI_MANA_TOMBS     = 557,  // Auchindoun: Mana-Tombs
    MAP_AUCHENAI_CRYPTS         = 558,  // Auchindoun: Auchenai Crypts

    MAP_BLACK_TEMPLE            = 564,  // Black Temple
    MAP_GRUULS_LAIR             = 565,  // Gruul's Lair
    MAP_ZUL_AMAN                = 568,  // Zul'Aman
    MAP_UTGARDE_KEEP            = 574,  // Utgarde Keep
    MAP_UTGARDE_PINNACLE        = 575,  // Utgarde Pinnacle

    MAP_NEXUS                   = 576,  // Nexus
    MAP_THE_OCULUS              = 578,  // The Oculus
    MAP_SUNWELL_PLATEAU         = 580,  // The Sunwell
    MAP_MAGISTERS_TERRACE       = 585,  // Magister's Terrace
    MAP_COT_CILLING_OF_STRATHOLME = 595,  // The Culling of Stratholme
    MAP_HALLS_OF_STONE          = 599,  // Halls of Stone
    MAP_DRAK_THARON_KEEP        = 600,  // Drak'Tharon Keep
    MAP_AZJOL_NERUB             = 601,  // Azjol-Nerub
    MAP_HALLS_OF_LIGHTNING      = 602,  // Halls of Lightning
    MAP_GUNDRAK                 = 604,  // Gundrak

    MAP_ULDUAR                  = 603,  // Ulduar

    MAP_VIOLET_HOLD             = 608,  // The Violet Hold
    MAP_OBSIDIAN_SANCTUM        = 615,  // ObsidianSanctum
    MAP_THE_EYE_OF_ETERNITY     = 616,  // The Eye of Eternity
    MAP_AHN_KAHET               = 619,  // Ahn'kahet: The Old Kingdom
    MAP_VAULT_OF_ARCHAVON       = 624,  // Vault of Archavon
    MAP_ICECROWNCITADEL         = 631,  // Icecrown Citadel
    MAP_FORGE_OF_SOULS          = 632,  // Forge of Souls
    MAP_TRIAL_OF_THE_CRUSADER   = 649,  // Trial of the Crusader
    MAP_TRIAL_OF_THE_CHAMPION   = 650,  // Trial of the Champion
    MAP_PIT_OF_SARON            = 658,  // Pit of Saron.
    MAP_HALLSOFREFLECTION       = 668,  // Halls of Reflection


    /*
     30 - Alterac Valley
     44 - <unused> Monastery
    169 - Emerald Dream
    489 - Warsong Gulch
    509 - Ruins of Ahn'Qiraj
    529 - Arathi Basin
    531 - Ahn'Qiraj Temple
    548 - Coilfang: Serpentshrine Cavern
    550 - Tempest Keep
    552 - Tempest Keep: The Arcatraz
    559 - Nagrand Arena
    560 - The Escape From Durnholde
    562 - Blade's Edge Arena
    566 - Eye of the Storm
    572 - Ruins of Lordaeron
    598 - Sunwell Fix (Unused)

    607 - Strand of the Ancients
    615 - The Obsidian Sanctum
    617 - Dalaran Sewers
    618 - The Ring of Valor
    628 - Isle of Conquest
    724 - The Ruby Sanctum*/
};

enum InstanceAreas
{
    AREA_VIOLET_HOLD = 4415
};
