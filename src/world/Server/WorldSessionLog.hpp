/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include <memory>
#include "Master.h"

class SessionLog;

#define sCheatLog (sMaster().getAnticheatLog())
#define sGMLog (sMaster().getGmCommandLog())
#define sPlrLog (sMaster().getPlayerLog())
