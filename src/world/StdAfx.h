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
#include "Server/Definitions.h"

#include <vector>
#include <fstream>
#include <array>

#include "Map/RecastIncludes.hpp"

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
#include "Storage/DB2/DB2Stores.hpp"

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

#include "Server/IUpdatable.h"
#include "Server/WUtil.h"
#include "Server/UpdateFields.hpp"
#include "Server/UpdateMask.h"
#include "Server/Packets/Opcodes.h"
#include "AuthCodes.h"
#include "CallBack.h"   // shared
#include "Management/WordFilter.h"
#include "Server/EventMgr.h"
#include "Server/EventableObject.h"
#include "Objects/Object.h"
#include "Management/LootMgr.h"
#include "Spell/SpellDefines.hpp"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Spell/SpellProc.h"
#include "Units/Summons/SummonHandler.h"
#include "Units/Creatures/Vehicle.h"
#include "Units/Unit.h"
#include "Management/Gossip/Gossip.h"
#include "Management/Gossip/GossipMenu.hpp"

#ifdef ENABLE_ACHIEVEMENTS
#include "Management/AchievementMgr.h"
#endif

//VMAP
#include "Models/ModelInstance.h"
#include "Models/WorldModel.h"
#include "Maps/MapTree.h"
#include "BoundingIntervalHierarchy.h"
#include "VMapFactory.h"
#include "VMapManager2.h"
#include "VMapDefinitions.h"


#include "Management/GameEventMgr.h"
#include "Management/GameEvent.h"
#include "Management/AddonMgr.h"
#include "Units/Creatures/AIEvents.h"
#include "Units/Creatures/AIInterface.h"
#include "Server/Packets/Handlers/AreaTrigger.h"
#include "Management/CalendarMgr.h"
#include "Management/Battleground/BattlegroundMgr.h"
#include "Management/Battleground/Battleground.h"
#include "Map/CellHandler.h"
#include "Chat/ChatHandler.hpp"
#include "Chat/ChatCommand.hpp"
#include "Chat/CommandTableStorage.hpp"
#include "Units/Creatures/Corpse.h"
#include "Management/Quest.hpp"
#include "Management/QuestMgr.h"
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
#include "Management/CRitual.h"
#include "Management/Group.h"

#include "Management/GuildFinderMgr.hpp"
#include "Management/Guild.hpp"
#include "Management/GuildMgr.hpp"

#include "Server/Packets/Handlers/HonorHandler.h"
#include "Management/ItemPrototype.h"
#include "Management/Item.h"
#include "Management/Container.h"
#include "Management/AuctionHouse.h"
#include "Management/AuctionMgr.h"
#include "Management/LFG/LFG.h"
#include "Management/LFG/LFGMgr.h"
#include "Management/LFG/LFGGroupData.h"
#include "Management/LFG/LFGPlayerData.h"
#include "Management/MailMgr.h"
#include "Map/Map.h"
#include "Map/MapCell.h"
#include "Map/TerrainMgr.h"
#include "Server/Packets/Handlers/MiscHandler.h"
#include "Server/Packets/Handlers/NPCHandler.h"
#include "Units/Creatures/Pet.h"
#include "Server/WorldSocket.h"
#include "Server/WorldSession.h"
#include "Management/WorldStatesHandler.h"
#include "Storage/WorldStrings.h"
#include "Map/MapMgr.h"
#include "Map/MapScriptInterface.h"
#include "Units/Players/Player.h"
#include "Objects/Faction.h"
#include "Management/Skill.h"
#include "Management/SkillNameMgr.h"
#include "Spell/SpellNameHashes.h"
#include "Spell/Spell.h"
#include "Spell/SpellMgr.h"
#include "Spell/SpellAuras.h"
#include "Management/TaxiMgr.h"
#include "Management/TransporterHandler.h"
#include "Management/WeatherMgr.h"
#include "Server/World.h"
#include "Management/EquipmentSetMgr.h"
#include "Management/ItemInterface.h"
#include "Units/Stats.h"
#include "Map/WorldCreator.h"
#include "Objects/ObjectMgr.h"
#include "CThreads.h"
#include "Server/Script/ScriptMgr.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Management/ArenaTeam.h"
#include "Management/Arenas.h"
#include "Server/LogonCommClient/LogonCommClient.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/MainServerDefines.h"
#include "Server/WorldRunnable.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/DatabaseCleaner.h"
#include "Storage/DayWatcherThread.h"
#include "Server/CommonScheduleThread.h"
#include "Management/LocalizationMgr.h"
#include "Server/Master.h"
#include "Server/Console/BaseConsole.h"
#include "Server/Console/CConsole.h"
#include "Server/Warden/SpeedDetector.h"
#include "Management/WorldStates.h"

#include "Units/Players/PlayerClasses.hpp"

#include "Map/MapManagementGlobals.hpp"

#endif  // __STDAFX_H
