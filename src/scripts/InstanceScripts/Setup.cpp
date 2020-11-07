/*
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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
#include "Server/Script/ScriptSetup.h"

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
    SetupShadowLabyrinth(mgr);
    SetupTheMechanar(mgr);
    SetupTheShatteredHalls(mgr);
    SetupTheSlavePens(mgr);
    SetupTheSteamvault(mgr);
    SetupTheStockade(mgr);
    SetupTheUnderbog(mgr);

    //Wotlk
    SetupAhnKahetTheOldKingdom(mgr);
    SetupAzjolNerub(mgr);
    SetupCullingOfStratholme(mgr);
    SetupDrakTharonKeep(mgr);
    SetupEyeOfEternity(mgr);
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
    SetupVaultOfArchavon(mgr);

    //Classic
    SetupBlackwingLair(mgr);
    SetupOnyxiasLair(mgr);
    SetupMoltenCore(mgr);
    SetupZulGurub(mgr);
    SetupWorldBosses(mgr);

    //Tbc
    SetupBlackTemple(mgr);
    SetupKarazhan(mgr);
    SetupBattleOfMountHyjal(mgr);
    SetupZulAman(mgr);
    SetupSunwellPlateau(mgr);
    SetupSerpentshrineCavern(mgr);
    SetupMagtheridonsLair(mgr);
    SetupTheEye(mgr);
    SetupGruulsLair(mgr);

    //Wotlk
    SetupICC(mgr);
    SetupTheObsidianSanctum(mgr);
    SetupNaxxramas(mgr);
    SetupUlduar(mgr);
    SetupTrialOfTheCrusader(mgr);
}

#ifdef WIN32
    BOOL APIENTRY DllMain(HANDLE /*hModule*/, DWORD  /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
    {
        return TRUE;
    }
#endif
