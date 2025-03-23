/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include <memory>

class SessionLog;

extern SERVER_DECL std::unique_ptr<SessionLog> Anticheat_Log;
extern SERVER_DECL std::unique_ptr<SessionLog> GMCommand_Log;
extern SERVER_DECL std::unique_ptr<SessionLog> Player_Log;

#define sCheatLog (*Anticheat_Log)
#define sGMLog (*GMCommand_Log)
#define sPlrLog (*Player_Log)
