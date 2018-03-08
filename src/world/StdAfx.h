/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

#pragma once

#include "WorldConf.h"

#ifdef USE_PCH_INCLUDES
#include <vector>
#include <fstream>
#include <array>
#include <set>
#include <map>


#include "Server/Definitions.h"
#include "Map/RecastIncludes.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Shared headers/defines

#include "CommonDefines.hpp"    // QuestLogEntry.h, Object.h
#include "Common.hpp"
#include "WorldPacket.h"
#include "Log.hpp"
#include "ByteBuffer.h"
#include "StackBuffer.h"
#include "Config/Config.h"
#include "crc32.h"
#include "LocationVector.h"
#include "Database/DatabaseEnv.h"
#include "Network/Network.h"
#include "Auth/MD5.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"          // Mutex.h, Guard.h errors
#include "Auth/WowCrypt.h"
#include "FastQueue.h"
#include "CircularQueue.h"
#include "Threading/RWLock.h"
#include "TLSObject.h"
#include "AuthCodes.h"
#include "CallBack.h"

#ifdef WIN32
    #include "printStackTrace.h"
#endif
//////////////////////////////////////////////////////////////////////////////////////////

#if VERSION_STRING == Cata
#include "Storage/DB2/DB2Stores.h"
#endif
#include "Storage/DBC/DBCStores.h"

//Movement
#include "Movement/UnitMovementManager.hpp"
#include "Movement/Spline/MovementSpline.hpp"
#include "Movement/Spline/MovementSplineDefines.hpp"
#include "Movement/Spline/SplineFlags.hpp"
#include "Movement/MovementCommon.hpp"

#include "Server/Packets/Movement/CreatureMovement.h"

#include "Server/IUpdatable.h"
#include "Server/WUtil.h"
#include "Server/UpdateFieldInclude.h"
#include "Server/UpdateMask.h"
#include "Server/Packets/Opcode.h"

#include "Management/WordFilter.h"
#include "Server/EventMgr.h"
#include "Server/EventableObject.h"
#include "Objects/Object.h"
#include "Management/LootMgr.h"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellDefines.hpp"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Spell/SpellProc.h"
#include "Units/Summons/SummonHandler.h"
#include "Units/Creatures/Vehicle.h"
#include "Units/Unit.h"
#include "Management/Gossip/Gossip.h"
#include "Management/AchievementMgr.h"

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
#include "Management/CalendarMgr.h"
#include "Management/Battleground/BattlegroundMgr.h"
#include "Management/Battleground/Battleground.h"
#include "Map/CellHandler.h"
#include "Chat/ChatHandler.hpp"
#include "Chat/ChatCommand.hpp"
#include "Chat/CommandTableStorage.hpp"
#include "Units/Creatures/Corpse.h"
#include "Management/Quest.h"
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

#if VERSION_STRING != Cata
#include "Management/Guild.h"
#else
#include "GameCata/Management/GuildFinderMgr.h"
#include "GameCata/Management/Guild.h"
#include "GameCata/Management/GuildMgr.h"
#endif

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
#include "Spell/Spell.h"
#include "Spell/SpellMgr.h"
#include "Spell/SpellAuras.h"
#include "Management/TaxiMgr.h"
#include "Management/TransporterHandler.h"
#include "Management/WeatherMgr.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Server/WorldConfig.h"
#include "Management/EquipmentSetMgr.h"
#include "Management/ItemInterface.h"
#include "Units/Stats.h"
#include "Map/WorldCreator.h"
#include "Objects/ObjectMgr.h"
#include "CThreads.h"

#include "Server/Script/ScriptMgr.h"
#include "Server/Script/CreatureAIScript.h"

#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Management/ArenaTeam.h"
#include "Management/Arenas.h"
#include "Server/LogonCommClient/LogonCommClient.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/MainServerDefines.h"
#include "Server/WorldRunnable.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Storage/DatabaseCleaner.h"
#include "Storage/DayWatcherThread.h"
#include "Server/BroadcastMgr.h"

#include "Server/Master.h"

#include "Server/Console/BaseConsole.h"
#include "Server/Console/ConsoleAuthMgr.h"
#include "Server/Console/ConsoleCommands.h"
#include "Server/Console/ConsoleSocket.h"
#include "Server/Console/ConsoleThread.h"

#include "Server/Warden/SpeedDetector.h"
#include "Management/WorldStates.h"
#include "Units/Players/PlayerClasses.hpp"
#include "Map/MapManagementGlobals.hpp"
#endif
