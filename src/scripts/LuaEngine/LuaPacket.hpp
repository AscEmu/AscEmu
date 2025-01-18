/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

extern "C"
{
#include <lua/lua.h>
}

class WorldPacket;

class LuaPacket
{
public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Normal operations
    static int CreatePacket(lua_State* L, WorldPacket* /*packet*/);
    static int GetOpcode(lua_State* L, WorldPacket* packet);
    static int GetSize(lua_State* L, WorldPacket* packet);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Read operations
    static int ReadUByte(lua_State* L, WorldPacket* packet);
    static int ReadByte(lua_State* L, WorldPacket* packet);
    static int ReadShort(lua_State* L, WorldPacket* packet);
    static int ReadUShort(lua_State* L, WorldPacket* packet);
    static int ReadLong(lua_State* L, WorldPacket* packet);
    static int ReadULong(lua_State* L, WorldPacket* packet);
    static int ReadFloat(lua_State* L, WorldPacket* packet);
    static int ReadDouble(lua_State* L, WorldPacket* packet);
    static int ReadGUID(lua_State* L, WorldPacket* packet);
    static int ReadWoWGuid(lua_State* L, WorldPacket* packet);
    static int ReadString(lua_State* L, WorldPacket* packet);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Write operations
    static int WriteByte(lua_State* L, WorldPacket* packet);
    static int WriteUByte(lua_State* L, WorldPacket* packet);
    static int WriteShort(lua_State* L, WorldPacket* packet);
    static int WriteUShort(lua_State* L, WorldPacket* packet);
    static int WriteLong(lua_State* L, WorldPacket* packet);
    static int WriteULong(lua_State* L, WorldPacket* packet);
    static int WriteFloat(lua_State* L, WorldPacket* packet);
    static int WriteDouble(lua_State* L, WorldPacket* packet);
    static int WriteGUID(lua_State* L, WorldPacket* packet);
    static int WriteWoWGuid(lua_State* L, WorldPacket* packet);
    static int WriteString(lua_State* L, WorldPacket* packet);

    static int GetObjectType(lua_State* L, WorldPacket* packet);
};