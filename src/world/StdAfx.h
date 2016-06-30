/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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
 *
 */

#ifndef __STDAFX_H
#define __STDAFX_H

#include "WorldConf.h"

#include "CommonDefines.hpp"  // shared

#define DEBUG_LOG(...) sLog.Debug("DEBUG_LOG", __VA_ARGS__)
#include "Definitions.h"

#include <vector>
#include <fstream>
#include <array>

#include "RecastIncludes.hpp"

// Shared headers
#include "Common.h"
#include "MersenneTwister.h"
#include "WorldPacket.h"
#include "Log.h"
#include "ByteBuffer.h"
#include "StackBuffer.h"
#include "Config/ConfigEnv.h"
#include "crc32.h"
#include "LocationVector.h"

extern SERVER_DECL SessionLogWriter* Anticheat_Log;
extern SERVER_DECL SessionLogWriter* GMCommand_Log;
extern SERVER_DECL SessionLogWriter* Player_Log;

#define sCheatLog (*Anticheat_Log)
#define sGMLog (*GMCommand_Log)
#define sPlrLog (*Player_Log)

//#include <zlib.h>

#include "Database/DatabaseEnv.h"   // shared
#include "Storage/DBC/DBCStores.h"

#include "Network/Network.h"    // shared

// Shared headers
#include "Auth/MD5.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"
#include "Auth/WowCrypt.h"
#include "FastQueue.h"
#include "CircularQueue.h"
#include "Threading/RWLock.h"
#include "TLSObject.h"
#include "Tokenizer.h"

#ifdef WIN32
#include "printStackTrace.h"
#endif

//Movement
#include "Movement/UnitMovementManager.hpp"
#include "Movement/Spline/MovementSpline.hpp"
#include "Movement/Spline/MovementSplineDefines.hpp"
#include "Movement/Spline/SplineFlags.hpp"
#include "Movement/MovementCommon.hpp"

#include "Server/Packets/ManagedPacket.hpp"
#include "Server/Packets/Movement/CreatureMovement.hpp"
#include "Server/Packets/Movement/SmsgMonsterMove.hpp"

#include "IUpdatable.h"
#include "WUtil.h"
#include "UpdateFields.h"
#include "UpdateMask.h"
#include "Server/Packets/Opcodes.h"
#include "AuthCodes.h"
#include "CallBack.h"   // shared
#include "WordFilter.h"
#include "EventMgr.h"
#include "EventableObject.h"
#include "Objects/Object.h"
#include "LootMgr.h"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Spell/SpellProc.h"
#include "Units/Summons/SummonHandler.h"
#include "Units/Creatures/Vehicle.h"
#include "Units/Unit.h"
#include "Gossip.h"

#ifdef ENABLE_ACHIEVEMENTS
#include "AchievementMgr.h"
#endif

//VMAP
#include "Models/ModelInstance.h"
#include "Models/WorldModel.h"
#include "Maps/MapTree.h"
#include "BoundingIntervalHierarchy.h"
#include "VMapFactory.h"
#include "VMapManager2.h"
#include "VMapDefinitions.h"


#include "GameEventMgr.h"
#include "GameEvent.h"
#include "AddonMgr.h"
#include "Units/Creatures/AIEvents.h"
#include "Units/Creatures/AIInterface.h"
#include "Server/Packets/Handlers/AreaTrigger.h"
#include "CalendarMgr.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "Map/CellHandler.h"
#include "Chat/Chat.h"
#include "Units/Creatures/Corpse.h"
#include "Quest.h"
#include "QuestMgr.h"
#include "Units/Creatures/Creature.h"
#include "Units/Summons/Summon.h"
#include "Units/Summons/CompanionSummon.h"
#include "Units/Summons/GuardianSummon.h"
#include "Units/Summons/PossessedSummon.h"
#include "Units/Summons/TotemSummon.h"
#include "Units/Summons/WildSummon.h"
#include "Objects/DynamicObject.h"
#include "Objects/GameObject.h"
#include "Objects/CObjectFactory.h"
#include "CRitual.h"
#include "Group.h"
#include "Guild.h"
#include "Server/Packets/Handlers/HonorHandler.h"
#include "ItemPrototype.h"
#include "Item.h"
#include "Container.h"
#include "AuctionHouse.h"
#include "AuctionMgr.h"
#include "Lfg.h"
#include "LfgMgr.h"
#include "LfgGroupData.h"
#include "LfgPlayerData.h"
#include "MailMgr.h"
#include "Map/Map.h"
#include "Map/MapCell.h"
#include "Map/TerrainMgr.h"
#include "Server/Packets/Handlers/MiscHandler.h"
#include "Server/Packets/Handlers/NPCHandler.h"
#include "Units/Creatures/Pet.h"
#include "Server/WorldSocket.h"
#include "Server/WorldSession.h"
#include "WorldStatesHandler.h"
#include "WorldStrings.h"
#include "Map/MapMgr.h"
#include "Map/MapScriptInterface.h"
#include "Units/Players/Player.h"
#include "Faction.h"
#include "Skill.h"
#include "SkillNameMgr.h"
#include "Spell/SpellNameHashes.h"
#include "Spell/Spell.h"
#include "Spell/SpellMgr.h"
#include "Spell/SpellAuras.h"
#include "TaxiMgr.h"
#include "TransporterHandler.h"
#include "WeatherMgr.h"
#include "Server/World.h"
#include "EquipmentSetMgr.h"
#include "ItemInterface.h"
#include "Stats.h"
#include "Map/WorldCreator.h"
#include "ObjectMgr.h"
#include "CThreads.h"
#include "ScriptMgr.h"
#include "Channel.h"
#include "ChannelMgr.h"
#include "ArenaTeam.h"
#include "Arenas.h"
#include "Server/LogonCommClient/LogonCommClient.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/MainServerDefines.h"
#include "Server/WorldRunnable.h"
#include "Storage/MySQLDataStore.hpp"
#include "DatabaseCleaner.h"
#include "DayWatcherThread.h"
#include "CommonScheduleThread.h"
#include "LocalizationMgr.h"
#include "CollideInterface.h"
#include "Server/Master.h"
#include "Server/Console/BaseConsole.h"
#include "Server/Console/CConsole.h"
#include "SpeedDetector.h"
#include "WorldStates.h"

#include "Units/Players/PlayerClasses.hpp"

#include "Map/MapManagementGlobals.hpp"

#endif  // __STDAFX_H
