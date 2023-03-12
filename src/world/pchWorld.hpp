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
#include <CommonDefines.hpp>
#include <CommonTypes.hpp>
#include <Common.hpp>
#include <Config/Config.h>
#include <WorldPacket.h>
#include <Log.hpp>
#include <Logging/Logger.hpp>
#include <ByteBuffer.h>
#include <Config/Config.h>
#include <LocationVector.h>
#include <Utilities/Strings.hpp>
#include <Utilities/Util.hpp>
#include <Auth/BigNumber.h>
#include <Auth/WowCrypt.hpp>
#include <VMapManager2.h>
#include <IVMapManager.h>
#include <VMapFactory.h>
#include <GameObjectModel.h>
#include <LogonCommDefines.h>
#include <WorldConf.h>
#include <Chat/ChatDefines.hpp>
#include <Chat/CommandTableStorage.hpp>
#include <Chat/ChatHandler.hpp>
#include <Macros/ScriptMacros.hpp>
#include <Management/Faction.h>
#include <Management/ItemInterface.h>
#include <Management/QuestLogEntry.hpp>
#include <Map/Area/AreaStorage.hpp>
#include <Map/Management/MapMgr.hpp>
#include <Movement/Spline/Spline.h>
#include <Movement/Spline/SplineImpl.h>
#include <Movement/Spline/MoveSpline.h>
#include <Movement/Spline/MoveSplineInit.h>
#include <Objects/GameObject.h>
#include <Objects/Item.hpp>
#include <Objects/ItemDefines.hpp>
#include <Objects/MovementDefines.hpp>
#include <Objects/MovementInfo.hpp>
#include <Objects/Object.hpp>
#include <Objects/ObjectDefines.hpp>
#include <Objects/Units/Creatures/Creature.h>
#include <Objects/Units/Creatures/CreatureDefines.hpp>
#include <Objects/Units/Players/Player.hpp>
#include <Objects/Units/Unit.hpp>
#include <Objects/Units/UnitDefines.hpp>
#include <Server/Definitions.h>
#include <Server/EventableObject.h>
#include <Server/LogonCommClient/LogonCommHandler.h>
#include <Server/LogonCommClient/LogonCommClient.h>
#include <Server/Opcodes.hpp>
#include <Server/Packets/SmsgMessageChat.h>
#include <Server/Script/ScriptMgr.h>
#include <Server/WorldConfig.h>
#include <Server/WorldSession.h>
#include <Server/WorldSocket.h>
#include <Spell/Definitions/SpellEffects.hpp>
#include <Spell/Spell.h>
#include <Spell/SpellAuras.h>
#include <Spell/SpellInfo.hpp>
#include <Spell/SpellMgr.hpp>
#include <Spell/SpellProc.hpp>
#include <Spell/SpellScript.hpp>
#include <Spell/SpellTarget.h>
#include <Storage/MySQLDataStore.hpp>
#include <Objects/Units/Players/Player.hpp>
#include <Objects/Units/Unit.hpp>
#include <Management/ObjectMgr.h>
