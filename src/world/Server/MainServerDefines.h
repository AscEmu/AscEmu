/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Config/Config.h"

//////////////////////////////////////////////////////////////////////////////////////////
class Database;

SERVER_DECL extern Database* Database_Character;
SERVER_DECL extern Database* Database_World;

#define WorldDatabase (*Database_World)
#define CharacterDatabase (*Database_Character)


//////////////////////////////////////////////////////////////////////////////////////////
class SessionLog;

extern SERVER_DECL SessionLog* Anticheat_Log;
extern SERVER_DECL SessionLog* GMCommand_Log;
extern SERVER_DECL SessionLog* Player_Log;

#define sCheatLog (*Anticheat_Log)
#define sGMLog (*GMCommand_Log)
#define sPlrLog (*Player_Log)


//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL ConfigMgr
{
    public:

        ConfigFile MainConfig;
        ConfigFile ClusterConfig;
};
extern SERVER_DECL ConfigMgr Config;
