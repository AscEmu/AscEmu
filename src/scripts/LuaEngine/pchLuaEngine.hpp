/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LUAEngine.hpp"
#include "LuaGlobal.hpp"
#include "LuaHelpers.hpp"
#include "LuaMacros.h"
#include "LuaSpell.hpp"
#include "Database/Database.h"
#include "Logging/Log.hpp"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/ItemInterface.h"
#include "Management/MailMgr.h"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestProperties.hpp"
#include "Management/Guild/Guild.hpp"
#include "Management/Guild/GuildMgr.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Item.hpp"
#include "Objects/Object.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Unit.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Spell/Spell.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"
#include <lua/lua.h>
#include <cstdlib>
#include <string>
