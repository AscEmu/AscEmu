/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once


extern "C"
{
#include <lua/lua.h>
}

#define ENGINE_NAME "ALE" 

class LuaGlobalFunctions
{
public:
    static int PerformIngameSpawn(lua_State* L);
    static int GetGameTime(lua_State* L);
    static int GetPlayer(lua_State* L);
    static int GetLUAEngine(lua_State* L);  //also mapped to GetLuaEngine()
    static int logcol(lua_State* L);
    static int WorldDBQuery(lua_State* L);
    static int CharDBQuery(lua_State* L);
    static int WorldDBQueryTable(lua_State* L);
    static int CharDBQueryTable(lua_State* L);
    static int SendWorldMessage(lua_State* L);
    static int ReloadTable(lua_State* L);
    static int ReloadLuaEngine(lua_State* /*L*/);
    static int GetPlayersInWorld(lua_State* L);
    static int Rehash(lua_State* /*L*/);
    static int GetClientVersion(lua_State* L);
    static int GetAERevision(lua_State* L);
    //static int GetInstanceIdsByMap(lua_State * L);
    //static int SendPvPCaptureMessage(lua_State * L);
    static int GetPlayersInMap(lua_State* L);
    static int GetPlayersInZone(lua_State* L);
    static int SendMail(lua_State* L);
    static int SetDBCSpellVar(lua_State* L);
    static int GetDBCSpellVar(lua_State* L);
    static int bit_and(lua_State* L);
    static int bit_or(lua_State* L);
    static int bit_xor(lua_State* L);
    //static int bit_not(lua_State* L);
    static int bit_shiftleft(lua_State* L);
    static int bit_shiftright(lua_State* L);
    static int RemoveTimedEvents(lua_State* /*L*/);
    static int RemoveTimedEventsWithName(lua_State* L);
    static int RemoveTimedEvent(lua_State* L);
    static int RemoveTimedEventsInTable(lua_State* L);
    static int HasTimedEvents(lua_State* L);
    static int HasTimedEvent(lua_State* L);
    static int HasTimedEventWithName(lua_State* L);
    static int HasTimedEventInTable(lua_State* L);
    static int GetPlatform(lua_State* L);
    static int NumberToGUID(lua_State* L);
    static int SendPacketToZone(lua_State* L);
    static int SendPacketToInstance(lua_State* L);
    static int SendPacketToWorld(lua_State* L);
    static int SendPacketToChannel(lua_State* L);
    static int GetInstanceCreature(lua_State* L);
    static int GetInstancePlayerCount(lua_State* L);
    static int GetPlayersInInstance(lua_State* L);
    static int GetGuildByName(lua_State* L);
    static int GetGuildByLeaderGuid(lua_State* L);
};

void RegisterGlobalFunctions(lua_State* L)
{
    lua_register(L, "PerformIngameSpawn", &LuaGlobalFunctions::PerformIngameSpawn);
    lua_register(L, "GetPlayer", &LuaGlobalFunctions::GetPlayer);
    lua_register(L, "GetLUAEngine", &LuaGlobalFunctions::GetLUAEngine);
    lua_register(L, "GetLuaEngine", &LuaGlobalFunctions::GetLUAEngine);
    lua_register(L, "GetClientVersion", &LuaGlobalFunctions::GetClientVersion);
    lua_register(L, "GetGameTime", &LuaGlobalFunctions::GetGameTime);
    lua_register(L, "WorldDBQuery", &LuaGlobalFunctions::WorldDBQuery);
    lua_register(L, "CharDBQuery", &LuaGlobalFunctions::CharDBQuery);
    lua_register(L, "WorldDBQueryTable", &LuaGlobalFunctions::WorldDBQueryTable);
    lua_register(L, "CharDBQueryTable", &LuaGlobalFunctions::CharDBQueryTable);
    lua_register(L, "SendWorldMessage", &LuaGlobalFunctions::SendWorldMessage);
    lua_register(L, "ReloadTable", &LuaGlobalFunctions::ReloadTable);
    lua_register(L, "ReloadLuaEngine", &LuaGlobalFunctions::ReloadLuaEngine);
    lua_register(L, "Rehash", &LuaGlobalFunctions::Rehash);
    lua_register(L, "logcol", &LuaGlobalFunctions::logcol);
    lua_register(L, "GetPlayersInWorld", &LuaGlobalFunctions::GetPlayersInWorld);
    lua_register(L, "GetAERevision", &LuaGlobalFunctions::GetAERevision);
    lua_register(L, "GetPlayersInMap", &LuaGlobalFunctions::GetPlayersInMap);
    lua_register(L, "GetPlayersInZone", &LuaGlobalFunctions::GetPlayersInZone);
    lua_register(L, "SendMail", &LuaGlobalFunctions::SendMail);
    lua_register(L, "SetDBCSpellVar", &LuaGlobalFunctions::SetDBCSpellVar);
    lua_register(L, "GetDBCSpellVar", &LuaGlobalFunctions::GetDBCSpellVar);
    //Lua's bit instructions
    lua_register(L, "bit_and", &LuaGlobalFunctions::bit_and);
    lua_register(L, "bit_or", &LuaGlobalFunctions::bit_or);
    lua_register(L, "bit_xor", &LuaGlobalFunctions::bit_xor);
    //lua_register(L, "bit_not", &LuaGlobalFunctions::bit_not);
    lua_register(L, "bit_shiftleft", &LuaGlobalFunctions::bit_shiftleft);
    lua_register(L, "bit_shiftright", &LuaGlobalFunctions::bit_shiftright);
    lua_register(L, "RemoveTimedEventsInTable", &LuaGlobalFunctions::RemoveTimedEventsInTable);
    lua_register(L, "RemoveTimedEventsWithName", &LuaGlobalFunctions::RemoveTimedEventsWithName);
    lua_register(L, "RemoveTimedEvent", &LuaGlobalFunctions::RemoveTimedEvent);
    lua_register(L, "HasTimedEvents", &LuaGlobalFunctions::HasTimedEvents);
    lua_register(L, "HasTimedEventInTable", &LuaGlobalFunctions::HasTimedEventInTable);
    lua_register(L, "HasTimedEventWithName", &LuaGlobalFunctions::HasTimedEventWithName);
    lua_register(L, "HasTimedEvent", &LuaGlobalFunctions::HasTimedEvent);
    lua_register(L, "GetPlatform", &LuaGlobalFunctions::GetPlatform);
    lua_register(L, "NumberToGUID", &LuaGlobalFunctions::NumberToGUID);
    lua_register(L, "SendPacketToWorld", &LuaGlobalFunctions::SendPacketToWorld);
    lua_register(L, "SendPacketToInstance", &LuaGlobalFunctions::SendPacketToInstance);
    lua_register(L, "SendPacketToZone", &LuaGlobalFunctions::SendPacketToZone);
    lua_register(L, "SendPacketToChannel", &LuaGlobalFunctions::SendPacketToChannel);
    lua_register(L, "GetInstanceCreature", &LuaGlobalFunctions::GetInstanceCreature);
    lua_register(L, "GetInstancePlayerCount", &LuaGlobalFunctions::GetInstancePlayerCount);
    lua_register(L, "GetPlayersInInstance", &LuaGlobalFunctions::GetPlayersInInstance);
    lua_register(L, "GetGuildByName", &LuaGlobalFunctions::GetGuildByName);
    lua_register(L, "GetGuildByLeaderGuid", &LuaGlobalFunctions::GetGuildByLeaderGuid);
}
