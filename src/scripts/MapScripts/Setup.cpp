/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/Script/ScriptSetup.h"

extern "C" SCRIPT_DECL void _exp_set_serverstate_singleton(ServerState* serverState)
{
    ServerState::instance(serverState);
}

extern "C" SCRIPT_DECL uint32_t _exp_get_script_type()
{
    return SCRIPT_TYPE_MISC;
}

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* scriptMgr)
{
    //EasterKingdoms
    AlteracValleyScripts(scriptMgr);
    BlackrockDepthsScripts(scriptMgr);
    BlackrockSpireScripts(scriptMgr);
    BlackwingLairScripts(scriptMgr);
    MoltenCoreScripts(scriptMgr);
    DeadminesScripts(scriptMgr);
    GnomreganScripts(scriptMgr);
    KarazhanScripts(scriptMgr);
    MagisterTerraceScripts(scriptMgr);
    ScarletEnclaveScripts(scriptMgr);
    ScarletMonasteryScripts(scriptMgr);
    ScholomanceScripts(scriptMgr);
    ShadowfangKeepScripts(scriptMgr);
    StratholmeScripts(scriptMgr);
    SunkenTempleScripts(scriptMgr);
    SunwellPlateauScripts(scriptMgr);
    TheStockadeScripts(scriptMgr);
    UldamanScripts(scriptMgr);
    ZulAmanScripts(scriptMgr);
    ZulGurubScripts(scriptMgr);

    //Kalimdor
    BlackfathomDeepsScripts(scriptMgr);
    BattleForMountHyjalScripts(scriptMgr);
    CullingOfStratholmScripts(scriptMgr);
    TheBlackMorassScripts(scriptMgr);
    OldHillsbradFoothillsScripts(scriptMgr);
    DireMaulScripts(scriptMgr);
    MaraudonScripts(scriptMgr);
    OnyxiasLairScripts(scriptMgr);
    RagefireChasmScripts(scriptMgr);
    RazorfenDownsScripts(scriptMgr);
    RazorfenKraulScripts(scriptMgr);
    RuinsOfAhnQirajScripts(scriptMgr);
    TempleOfAhnQirajScripts(scriptMgr);
    WailingCavernsScripts(scriptMgr);
    ZulFarrakScripts(scriptMgr);

    //Northrend
    AhnkahetScripts(scriptMgr);
    AzjolNerubScripts(scriptMgr);
    ObsidianSanctumScripts(scriptMgr);
    TrialOfTheChampionScripts(scriptMgr);
    TrialOfTheCrusaderScripts(scriptMgr);
    DraktharonKeepScripts(scriptMgr);
    ForgeOfSoulsScripts(scriptMgr);
    HallsOfReflectionScripts(scriptMgr);
    PitOfSaronScripts(scriptMgr);
    GundrakScripts(scriptMgr);
    IcecrownCitadel(scriptMgr);
    //IsleOfConquestScripts(scriptMgr);
    NaxxramasScripts(scriptMgr);
    EyeOfEternityScripts(scriptMgr);
    NexusScripts(scriptMgr);
    OculusScripts(scriptMgr);
    HallsOfLightningScripts(scriptMgr);
    HallsOfStoneScripts(scriptMgr);
    UlduarScripts(scriptMgr);
    UtgardeKeepScripts(scriptMgr);
    UtgardePinnacleScripts(scriptMgr);
    VaultOfArchavonScripts(scriptMgr);
    VioletHoldScripts(scriptMgr);

    //Outland
    AuchenaiCryptsScripts(scriptMgr);
    ManaTombsScripts(scriptMgr);
    SethekkHallsScripts(scriptMgr);
    ShadowLabyrinthScripts(scriptMgr);
    BlackTempleScript(scriptMgr);
    SerpentShrineScripts(scriptMgr);
    SteamVaultScripts(scriptMgr);
    TheSlavePensScripts(scriptMgr);
    TheUnderbogScripts(scriptMgr);
    GruulsLairScripts(scriptMgr);
    BloodFurnaceScripts(scriptMgr);
    HellfireRampartsScripts(scriptMgr);
    MagtheridonsLairScripts(scriptMgr);
    ShatteredHallsScripts(scriptMgr);

    TheArcatrazScripts(scriptMgr);
    TheBotanicaScripts(scriptMgr);
    TheEyeScripts(scriptMgr);
    TheMechanarScripts(scriptMgr);
}

#ifdef WIN32
    BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
    {
        return TRUE;
    }
#endif
