/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Database/Database.hpp"
#include <memory>
#include "Master.h"

class Database;

#define WorldDatabase (sMaster().getWorldDatabase())
#define CharacterDatabase (sMaster().getCharacterDatabase())
