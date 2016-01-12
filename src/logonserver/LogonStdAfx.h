/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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

#include "Common.h"
#include <Network/Network.h>

#include "../shared/Log.h"
#include "../shared/Util.h"
#include "../shared/ByteBuffer.h"
#include "../shared/Config/ConfigEnv.h"
//#include <zlib.h>

#include "../shared/Database/DatabaseEnv.h"

#include "../shared/Auth/BigNumber.h"
#include "../shared/Auth/Sha1.h"
#include "../shared/Auth/WowCrypt.h"

#include "../world/Opcodes.h"
#include "AccountCache.h"
#include "PeriodicFunctionCall_Thread.h"
#include "../logonserver/AutoPatcher.h"
#include "../logonserver/AuthSocket.h"
#include "../logonserver/AuthStructs.h"
#include "../logonserver/LogonCommServer.h"
#include "../logonserver/LogonConsole.h"
#include "../shared/WorldPacket.h"
#include "Master.hpp"

// database decl
extern Database* sLogonSQL;
