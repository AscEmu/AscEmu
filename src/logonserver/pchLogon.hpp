/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
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
#include <Log.hpp>
#include <Logging/Logger.hpp>
#include <ByteBuffer.h>
#include <Config/Config.h>
#include <Util/Strings.hpp>
#include <Util.hpp>
#include <Auth/BigNumber.h>
#include <Auth/Sha1.h>
#include <Auth/WowCrypt.hpp>
#include <Database/Database.h>
#include <Network/Network.h>
#include "LogonConf.hpp"
#include "Console/LogonConsole.h"
#include "Realm/RealmManager.hpp"
#include "Server/IpBanMgr.h"
#include "Server/LogonServerDefines.hpp"
#include "Server/Master.hpp"
