/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "AlteracValley/AlteracValley.h"
#include "ArathiBasin/ArathiBasin.h"
#include "CircleOfBlood/CircleOfBlood.h"
#include "DalaranSewers/DalaranSewers.h"
#include "EyeOfTheStorm/EyeOfTheStorm.h"
#include "IsleOfConquest/IsleOfConquest.h"
#include "Management/Battleground/BattlegroundMgr.hpp"
#include "RingOfTrials/RingOfTrials.h"
#include "RingOfValor/RingOfValor.h"
#include "RuinsOfLordaeron/RuinsOfLordaeron.h"
#include "Server/ServerState.h"
#include "StrandOfTheAncient/StrandOfTheAncient.h"
#include "WarsongGulch/WarsongGulch.h"
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

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* /*mgr*/)
{
    // Mapping Battleground type to map Id
    sBattlegroundManager.registerMapForBgType(BattlegroundDef::TYPE_ALTERAC_VALLEY, 30);
    sBattlegroundManager.registerMapForBgType(BattlegroundDef::TYPE_WARSONG_GULCH, 489);
    sBattlegroundManager.registerMapForBgType(BattlegroundDef::TYPE_ARATHI_BASIN, 529);
    sBattlegroundManager.registerMapForBgType(BattlegroundDef::TYPE_EYE_OF_THE_STORM, 566);
    sBattlegroundManager.registerMapForBgType(BattlegroundDef::TYPE_STRAND_OF_THE_ANCIENT, 607);
    sBattlegroundManager.registerMapForBgType(BattlegroundDef::TYPE_ISLE_OF_CONQUEST, 628);

    // Registering factory methods
    sBattlegroundManager.registerBgFactory(30, &AlteracValley::Create);
    sBattlegroundManager.registerBgFactory(489, &WarsongGulch::Create);
    sBattlegroundManager.registerBgFactory(529, &ArathiBasin::Create);
    sBattlegroundManager.registerBgFactory(566, &EyeOfTheStorm::Create);
    sBattlegroundManager.registerBgFactory(607, &StrandOfTheAncient::Create);
    sBattlegroundManager.registerBgFactory(628, &IsleOfConquest::Create);

    sBattlegroundManager.registerArenaFactory(559, &RingOfTrials::Create);
    sBattlegroundManager.registerArenaFactory(562, &CircleOfBlood::Create);
    sBattlegroundManager.registerArenaFactory(572, &RuinsOfLordaeron::Create);
    sBattlegroundManager.registerArenaFactory(617, &DalaranSewers::Create);
    sBattlegroundManager.registerArenaFactory(618, &RingOfValor::Create);
}

#ifdef WIN32
BOOL APIENTRY DllMain(HANDLE /*hModule*/, DWORD  /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
{
    return TRUE;
}
#endif
