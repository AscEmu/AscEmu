/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Database/Database.h"

class Database;

SERVER_DECL extern std::unique_ptr<Database> Database_Character;
SERVER_DECL extern std::unique_ptr<Database> Database_World;

#define WorldDatabase (*Database_World)
#define CharacterDatabase (*Database_Character)
