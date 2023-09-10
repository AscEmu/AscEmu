/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>
#include <map>
#include <vector>
#include <array>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "AEVersion.hpp"
#include "CommonDefines.hpp"
#include "CommonTypes.hpp"
#include "Config/Config.h"
#include "WorldPacket.h"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "ByteBuffer.h"
#include "Config/Config.h"
#include "LocationVector.h"
#include "Utilities/Strings.hpp"
#include "Utilities/Util.hpp"
#include "Cryptography/BigNumber.h"
#include "Cryptography/WowCrypt.hpp"
#include "VMapManager2.h"
#include "IVMapManager.h"
#include "VMapFactory.h"
#include "GameObjectModel.h"
#include "Cryptography/LogonCommDefines.h"
#include "WorldConf.h"
#include "Chat/Channel.hpp"
#include "Chat/ChannelDefines.hpp"
#include "Chat/ChannelMgr.hpp"
#include "Chat/ChatCommand.hpp"
#include "Chat/ChatDefines.hpp"
#include "Chat/CommandTableStorage.hpp"
#include "Chat/ChatHandler.hpp"
#include "Macros/AIInterfaceMacros.hpp"
#include "Macros/CreatureMacros.hpp"
#include "Macros/GuildMacros.hpp"
#include "Macros/ItemMacros.hpp"
#include "Macros/LFGMacros.hpp"
#include "Macros/LootMacros.hpp"
#include "Macros/MapsMacros.hpp"
#include "Macros/PetMacros.hpp"
#include "Macros/PlayerMacros.hpp"
#include "Macros/ScriptMacros.hpp"
#include "Macros/UnitMacros.hpp"
#include "Management/Group.h"
#include "Management/ItemInterface.h"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestLogEntry.hpp"
#include "Map/Area/AreaManagementGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Map/Cells/CellHandler.hpp"
#include "Map/Cells/CellHandlerDefines.hpp"
#include "Map/Cells/TerrainMgr.hpp"
#include "Map/Management/MapManagementGlobals.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Management/MapMgrDefines.hpp"
#include "Map/Maps/BaseMap.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Maps/InstanceMap.hpp"
#include "Map/Maps/InstanceMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Movement/Spline/MovementPacketBuilder.h"
#include "Movement/Spline/MovementTypedefs.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineFlag.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Movement/Spline/MoveSplineInitArgs.h"
#include "Movement/Spline/Spline.h"
#include "Movement/Spline/SplineChain.h"
#include "Movement/Spline/SplineImpl.h"
#include "Movement/MovementDefines.h"
#include "Movement/WaypointDefines.h"
#include "Objects/GameObject.h"
#include "Objects/GameObjectDefines.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Item.hpp"
#include "Objects/ItemDefines.hpp"
#include "Objects/MovementDefines.hpp"
#include "Objects/MovementInfo.hpp"
#include "Objects/Object.hpp"
#include "Objects/ObjectDefines.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/UnitDefines.hpp"
#include "Server/Definitions.h"
#include "Server/EventableObject.h"
#include "Server/Opcodes.hpp"
#include "Server/ServerState.h"
#include "Server/WorldConfig.h"
#include "Server/WorldSession.h"
#include "Server/WorldSocket.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/LogonCommClient/LogonCommClient.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellCastTargets.hpp"
#include "Spell/SpellDefines.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellProc.hpp"
#include "Spell/SpellScript.hpp"
#include "Spell/SpellTarget.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Storage/WorldStrings.h"
