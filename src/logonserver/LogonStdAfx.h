/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

#pragma once

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <sstream>
#include <string>

#include "Common.hpp"
#include <Network/Network.h>

#include "../shared/Log.hpp"
#include "../shared/Util.hpp"
#include "../shared/ByteBuffer.h"
#include "../shared/Config/Config.h"
//#include <zlib.h>

#include "../shared/Database/DatabaseEnv.h"

#include "../shared/Auth/BigNumber.h"
#include "../shared/Auth/Sha1.h"
#include "../shared/Auth/WowCrypt.h"

#include "../world/Server/Packets/Opcode.h"
#include "LogonConf.h"
#include "Auth/AccountCache.h"
#include "Server/PeriodicFunctionCall_Thread.h"
#include "Auth/AutoPatcher.h"
#include "Auth/AuthSocket.h"
#include "Auth/AuthStructs.h"
#include "LogonCommServer/LogonCommServer.h"
#include "Console/LogonConsole.h"
#include "WorldPacket.h"
#include "Server/Master.hpp"
#include "Server/LogonServerDefines.hpp"

// database decl
extern Database* sLogonSQL;
