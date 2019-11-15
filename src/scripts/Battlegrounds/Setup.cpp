/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Server/Script/ScriptSetup.h"
#include "AlteracValley/AlteracValley.h"
#include "ArathiBasin/ArathiBasin.h"
#include "CircleOfBlood/CircleOfBlood.h"
#include "DalaranSewers/DalaranSewers.h"
#include "EyeOfTheStorm/EyeOfTheStorm.h"
#include "IsleOfConquest/IsleOfConquest.h"
#include "RingOfTrials/RingOfTrials.h"
#include "RingOfValor/RingOfValor.h"
#include "RuinsOfLordaeron/RuinsOfLordaeron.h"
#include "StrandOfTheAncient/StrandOfTheAncient.h"
#include "WarsongGulch/WarsongGulch.h"
#include "Server/Script/ScriptMgr.h"

extern "C" SCRIPT_DECL void _exp_set_serverstate_singleton(ServerState* state)
{
    ServerState::instance(state);
}

extern "C" SCRIPT_DECL uint32_t _exp_get_script_type()
{
    return SCRIPT_TYPE_MISC;
}

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* /*mgr*/)
{
    // Mapping Battleground type to map Id
    sBattlegroundManager.RegisterMapForBgType(BATTLEGROUND_ALTERAC_VALLEY, 30);
    sBattlegroundManager.RegisterMapForBgType(BATTLEGROUND_WARSONG_GULCH, 489);
    sBattlegroundManager.RegisterMapForBgType(BATTLEGROUND_ARATHI_BASIN, 529);
    sBattlegroundManager.RegisterMapForBgType(BATTLEGROUND_EYE_OF_THE_STORM, 566);
    sBattlegroundManager.RegisterMapForBgType(BATTLEGROUND_STRAND_OF_THE_ANCIENT, 607);
    sBattlegroundManager.RegisterMapForBgType(BATTLEGROUND_ISLE_OF_CONQUEST, 628);

    // Registering factory methods
    sBattlegroundManager.RegisterBgFactory(30, &AlteracValley::Create);
    sBattlegroundManager.RegisterBgFactory(489, &WarsongGulch::Create);
    sBattlegroundManager.RegisterBgFactory(529, &ArathiBasin::Create);
    sBattlegroundManager.RegisterBgFactory(566, &EyeOfTheStorm::Create);
    sBattlegroundManager.RegisterBgFactory(607, &StrandOfTheAncient::Create);
    sBattlegroundManager.RegisterBgFactory(628, &IsleOfConquest::Create);

    sBattlegroundManager.RegisterArenaFactory(559, &RingOfTrials::Create);
    sBattlegroundManager.RegisterArenaFactory(562, &CircleOfBlood::Create);
    sBattlegroundManager.RegisterArenaFactory(572, &RuinsOfLordaeron::Create);
    sBattlegroundManager.RegisterArenaFactory(617, &DalaranSewers::Create);
    sBattlegroundManager.RegisterArenaFactory(618, &RingOfValor::Create);
}

#ifdef WIN32
BOOL APIENTRY DllMain(HANDLE /*hModule*/, DWORD  /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
{
    return TRUE;
}
#endif
