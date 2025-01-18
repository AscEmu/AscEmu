/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <CommonDefines.hpp>
#include <Common.hpp>
#include <WorldPacket.h>
#include <Logging/Log.hpp>
#include <Logging/Logger.hpp>
#include <ByteBuffer.h>
#include <Config/Config.h>
#include <Utilities/Strings.hpp>
#include <Utilities/Util.hpp>
#include <Cryptography/BigNumber.h>
#include <Cryptography/Sha1.hpp>
#include <Cryptography/WowCrypt.hpp>
#include <Database/Database.h>
#include <Network/Network.h>
#include "LogonConf.hpp"
#include "Console/LogonConsole.h"
#include "Realm/RealmManager.hpp"
#include "Server/IpBanMgr.h"
#include "Server/LogonServerDefines.hpp"
#include "Server/Master.hpp"
