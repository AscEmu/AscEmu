/*
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

#include "Server/ClientProtocol.hpp"
#include "Server/ServerState.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Server/Script/ScriptSetup.hpp"

extern "C" SCRIPT_DECL void _exp_set_serverstate_singleton(ServerState* state)
{
    ServerState::instance(state);
}

extern "C" SCRIPT_DECL uint32_t _exp_get_script_type()
{
    return SCRIPT_TYPE_MISC;
}

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* mgr)    // Comment any script to disable it
{
    //Classic
    SetupBlackfathomDeeps(mgr);
    SetupBlackrockSpire(mgr);
    SetupBlackrockDepths(mgr);
    SetupDeadmines(mgr);
    SetupDireMaul(mgr);
    SetupGnomeregan(mgr);
    SetupMaraudon(mgr);
    SetupRagefireChasm(mgr);
    SetupRazorfenDowns(mgr);
    SetupRazorfenKraul(mgr);
    SetupScarletMonastery(mgr);
    SetupScholomance(mgr);
    SetupShadowfangKeep(mgr);
    SetupStratholme(mgr);
    SetupTheTempleOfAtalHakkar(mgr);
    SetupUldaman(mgr);
    SetupWailingCaverns(mgr);
    SetupZulFarrak(mgr);

    //TBC
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_TBC))
    {
        SetupArcatraz(mgr);
        SetupAuchenaiCrypts(mgr);
        SetupTheBlackMorass(mgr);
        SetupBloodFurnace(mgr);
        SetupBotanica(mgr);
        SetupHellfireRamparts(mgr);
        SetupMagistersTerrace(mgr);
        SetupManaTombs(mgr);
        SetupOldHillsbradFoothills(mgr);
        SetupSethekkHalls(mgr);
        // SetupShadowLabyrinth(mgr);
        SetupTheMechanar(mgr);
        SetupTheShatteredHalls(mgr);
        SetupTheSlavePens(mgr);
        SetupTheSteamvault(mgr);
        SetupTheStockade(mgr);
        SetupTheUnderbog(mgr);
    }

    //Wotlk
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
    {
        SetupAhnKahetTheOldKingdom(mgr);
        SetupAzjolNerub(mgr);
        SetupCullingOfStratholme(mgr);
        SetupDrakTharonKeep(mgr);
        SetupForgeOfSouls(mgr);
        SetupGundrak(mgr);
        SetupHallsOfLightning(mgr);
        SetupHallsOfReflection(mgr);
        SetupHallsOfStone(mgr);
        SetupNexus(mgr);
        SetupPitOfSaron(mgr);
        SetupTheOculus(mgr);
        SetupTheVioletHold(mgr);
        SetupTrialOfTheChampion(mgr);
        SetupUtgardeKeep(mgr);
        SetupUtgardePinnacle(mgr);
    }

    //Cataclysm
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
    {
        SetupBlackrockCaverns(mgr);
        SetupEndTime(mgr);
        SetupGrimBatol(mgr);
        SetupHallsOfOrigination(mgr);
        SetupHourOfTwilight(mgr);
        SetupLostCityOfTolvir(mgr);
        SetupTheStonecore(mgr);
        SetupThroneOfTides(mgr);
        SetupVortexPinnacle(mgr);
        SetupWellOfEternity(mgr);
        SetupZulAmanCata(mgr);
        SetupZulGurubCata(mgr);
    }

    //Mists of Pandaria
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
    {
        SetupTempleOfTheJadeSerpent(mgr);
        SetupStormstoutBrewery(mgr);
        SetupShadoPanMonastery(mgr);
        SetupSiegeOfNiuzaoTemple(mgr);
        SetupMogushanPalace(mgr);
        SetupGateOfTheSettingSun(mgr);
        SetupScarletHalls(mgr);
        SetupScarletMonasteryMop(mgr);
        SetupScholomanceMop(mgr);
    }

    //Warlords of Draenor
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_WoD))
    {
        SetupAuchindoun(mgr);
        SetupBloodmaulSlagMines(mgr);
        SetupGrimrailDepot(mgr);
        SetupIronDocks(mgr);
        SetupShadowmoonBurialGrounds(mgr);
        SetupSkyreach(mgr);
        SetupTheEverbloom(mgr);
        SetupUpperBlackrockSpire(mgr);
    }

    //Legion
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Legion))
    {
        SetupAssaultOnVioletHold(mgr);
        SetupBlackRookHold(mgr);
        SetupCathedralOfEternalNight(mgr);
        SetupCourtOfStars(mgr);
        SetupDarkheartThicket(mgr);
        SetupEyeOfAzshara(mgr);
        SetupHallsOfValor(mgr);
        SetupMawOfSouls(mgr);
        SetupNeltharionsLair(mgr);
        SetupReturnToKarazhan(mgr);
        SetupSeatOfTheTriumvirate(mgr);
        SetupTheArcway(mgr);
        SetupVaultOfTheWardens(mgr);
    }

    //Battle for Azeroth
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_BfA))
    {
        SetupAtalDazar(mgr);
        SetupFreehold(mgr);
        SetupKingsRest(mgr);
        SetupOperationMechagon(mgr);
        SetupShrineOfTheStorm(mgr);
        SetupSiegeOfBoralus(mgr);
        SetupTempleOfSethraliss(mgr);
        SetupTheMotherlode(mgr);
        SetupTheUnderrot(mgr);
        SetupTolDagor(mgr);
        SetupWaycrestManor(mgr);
    }

    //Shadowlands
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Shadowlands))
    {
        SetupDeOtherSide(mgr);
        SetupHallsOfAtonement(mgr);
        SetupMistsOfTirnaScithe(mgr);
        SetupPlaguefall(mgr);
        SetupSanguineDepths(mgr);
        SetupSpiresOfAscension(mgr);
        SetupTazavesh(mgr);
        SetupTheNecroticWake(mgr);
        SetupTheaterOfPain(mgr);
    }

    //Dragonflight
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Dragonflight))
    {
        SetupAlgetharAcademy(mgr);
        SetupBrackenhideHollow(mgr);
        SetupDawnOfTheInfinite(mgr);
        SetupHallsOfInfusion(mgr);
        SetupNeltharus(mgr);
        SetupRubyLifePools(mgr);
        SetupTheAzureVault(mgr);
        SetupTheNokhudOffensive(mgr);
        SetupUldamanLegacyOfTyr(mgr);
    }

    //The War Within
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_TheWarWithin))
    {
        SetupAraKara(mgr);
        SetupCinderbrewMeadery(mgr);
        SetupCityOfThreads(mgr);
        SetupDarkflameCleft(mgr);
        SetupEcoDomeAldani(mgr);
        SetupOperationFloodgate(mgr);
        SetupPrioryOfTheSacredFlame(mgr);
        SetupTheDawnbreaker(mgr);
        SetupTheRookery(mgr);
        SetupTheStonevault(mgr);
    }

    //Midnight
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Midnight))
    {
        SetupAltarOfFangs(mgr);
        SetupDenOfNalorakk(mgr);
        SetupMagistersTerraceMidnight(mgr);
        SetupMaisaraCaverns(mgr);
        SetupMurderRow(mgr);
        SetupNexusPointXenas(mgr);
        SetupTheBlindingVale(mgr);
        SetupVoidscarArena(mgr);
        SetupWindrunnerSpire(mgr);
    }

    //Classic
    SetupBlackwingLair(mgr);
    SetupOnyxiasLair(mgr);
    SetupMoltenCore(mgr);
    SetupZulGurub(mgr);
    SetupWorldBosses(mgr);

    //Tbc
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_TBC))
    {
        SetupBlackTemple(mgr);
        SetupKarazhan(mgr);
        SetupBattleOfMountHyjal(mgr);
        SetupSunwellPlateau(mgr);
        SetupSerpentshrineCavern(mgr);
        SetupMagtheridonsLair(mgr);
        SetupTheEye(mgr);
        SetupGruulsLair(mgr);
    }

    if (WoW::isServerExpansion(WoW::Expansion::_TBC))
    {
        // Zul'Aman as a 10-man raid only existed pre-Cataclysm - its 4.1 heroic revamp
        // (SetupZulAmanCata) reuses the same map id and takes over from Cata onward.
        SetupZulAman(mgr);
    }

    //Wotlk
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
    {
        SetupICC(mgr);
        SetupTheObsidianSanctum(mgr);
        SetupNaxxramas(mgr);
        SetupUlduar(mgr);
        SetupTrialOfTheCrusader(mgr);
        SetupEyeOfEternity(mgr);
        SetupVaultOfArchavon(mgr);
    }

    //Cataclysm
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
    {
        SetupBaradinHold(mgr);
        SetupBastionOfTwilight(mgr);
        SetupBlackwingDescent(mgr);
        SetupDragonSoul(mgr);
        SetupFirelands(mgr);
        SetupThroneOfTheFourWinds(mgr);
    }

    //Mists of Pandaria
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
    {
        SetupMogushanVaults(mgr);
        SetupHeartOfFear(mgr);
        SetupTerraceOfEndlessSpring(mgr);
        SetupThroneOfThunder(mgr);
        SetupSiegeOfOrgrimmar(mgr);
    }

    //Warlords of Draenor
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_WoD))
    {
        SetupHighmaul(mgr);
        SetupBlackrockFoundry(mgr);
        SetupHellfireCitadel(mgr);
    }

    //Legion
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Legion))
    {
        SetupTheEmeraldNightmare(mgr);
        SetupTrialOfValor(mgr);
        SetupTheNighthold(mgr);
        SetupTombOfSargeras(mgr);
        SetupAntorusTheBurningThrone(mgr);
    }

    //Battle for Azeroth
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_BfA))
    {
        SetupUldir(mgr);
        SetupBattleOfDazarAlor(mgr);
        SetupCrucibleOfStorms(mgr);
        SetupTheEternalPalace(mgr);
        SetupNyalotha(mgr);
    }

    //Shadowlands
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Shadowlands))
    {
        SetupCastleNathria(mgr);
        SetupSanctumOfDomination(mgr);
        SetupSepulcherOfTheFirstOnes(mgr);
    }

    //Dragonflight
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Dragonflight))
    {
        SetupVaultOfTheIncarnates(mgr);
        SetupAmirdrassil(mgr);
        SetupAberrus(mgr);
    }

    //The War Within
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_TheWarWithin))
    {
        SetupNerubarPalace(mgr);
        SetupLiberationOfUndermine(mgr);
        SetupManaforgeOmega(mgr);
    }

    //Midnight
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Midnight))
    {
        SetupSporefall(mgr);
        SetupTheVoidspire(mgr);
        SetupMarchOnQuelDanas(mgr);
        SetupTheDreamrift(mgr);
        SetupTheTideboundGrotto(mgr);
        SetupTheVenomousAbyss(mgr);
        SetupTheUnbindingOfKithix(mgr);
    }
}

#ifdef _WIN32
#include <Windows.h>
    BOOL APIENTRY DllMain(HANDLE /*hModule*/, DWORD  /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
    {
        return TRUE;
    }
#endif
