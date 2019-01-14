/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/Script/ScriptSetup.h"

extern "C" SCRIPT_DECL void _exp_set_serverstate_singleton(ServerState* state)
{
    ServerState::instance(state);
}

extern "C" SCRIPT_DECL uint32 _exp_get_script_type()
{
    return SCRIPT_TYPE_MISC;
}

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* mgr)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Classes Quests
    SetupDruid(mgr);
    SetupMage(mgr);
    SetupPaladin(mgr);
    SetupWarrior(mgr);
    SetupDeathKnight(mgr);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Proffessions Quests
    SetupFirstAid(mgr);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Zones Quests
    SetupArathiHighlands(mgr);
    SetupAzuremystIsle(mgr);
    SetupBladeEdgeMountains(mgr);
    SetupBlastedLands(mgr);
    SetupBloodmystIsle(mgr);
    SetupBurningSteppes(mgr);
    SetupDesolace(mgr);
    SetupDragonblight(mgr);
    SetupDuskwood(mgr);
    SetupDustwallowMarsh(mgr);
    SetupEasternPlaguelands(mgr);
    SetupEversongWoods(mgr);
    SetupGhostlands(mgr);
    SetupHellfirePeninsula(mgr);
    SetupHillsbradFoothills(mgr);
    SetupHowlingFjord(mgr);
    SetupIsleOfQuelDanas(mgr);
    SetupLochModan(mgr);
    SetupMulgore(mgr);
    SetupNagrand(mgr);
    SetupNetherstorm(mgr);
    SetupRedrigeMountains(mgr);
    SetupShadowmoon(mgr);
    SetupSilithus(mgr);
    SetupSilvermoonCity(mgr);
    SetupSilverpineForest(mgr);
    SetupStonetalonMountains(mgr);
    SetupStormwind(mgr);
    SetupStranglethornVale(mgr);
    SetupTanaris(mgr);
    SetupTeldrassil(mgr);
    SetupTerrokarForest(mgr);
    SetupTheStormPeaks(mgr);
    SetupThousandNeedles(mgr);
    SetupTirisfalGlades(mgr);
    SetupUndercity(mgr);
    SetupUnGoro(mgr);
    SetupWestfall(mgr);
    SetupWetlands(mgr);
    SetupZangarmarsh(mgr);
    SetupBarrens(mgr);
    SetupBoreanTundra(mgr);
    SetupSholazarBasin(mgr);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    SetupQuestGossip(mgr);
    SetupQuestHooks(mgr);
    SetupUnsorted(mgr);
}

#ifdef WIN32
BOOL APIENTRY DllMain(HANDLE /*hModule*/, DWORD  /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
{
    return TRUE;
}
#endif
