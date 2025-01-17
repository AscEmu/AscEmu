/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LuaPacket.hpp"

#include "LUAEngine.hpp"
#include "LuaGlobal.hpp"
#include "LuaMacros.h"
#include "WorldPacket.h"
#include "Objects/Object.hpp"
#include "Server/Opcodes.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Normal operations
int LuaPacket::CreatePacket(lua_State* L, WorldPacket* /*packet*/)
{
    const uint16_t opcode = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    const size_t size = static_cast<size_t>(luaL_checkinteger(L, 2));
    if (opcode >= NUM_OPCODES)
        luaL_error(L, "CreatePacket got opcode %d greater than max opcode %d.", opcode, NUM_OPCODES);
    else
    {
        WorldPacket* npacket = new WorldPacket(opcode, size);
        PUSH_PACKET(L, npacket);
    }
    return 1;
}
int LuaPacket::GetOpcode(lua_State* L, WorldPacket* packet)
{
    if (packet == nullptr)
        lua_pushnil(L);
    else
        lua_pushinteger(L, packet->GetOpcode());
    return 1;
}
int LuaPacket::GetSize(lua_State* L, WorldPacket* packet)
{
    if (packet == nullptr)
        lua_pushnil(L);
    else
        lua_pushinteger(L, packet->size());
    return 1;
}
//////////////////////////////////////////////////////////////////////////////////////////
// Read operations
int LuaPacket::ReadUByte(lua_State* L, WorldPacket* packet)
{
    if (packet == nullptr)
        lua_pushnil(L);
    else
    {
        uint8_t byte;
        (*packet) >> byte;
        lua_pushinteger(L, byte);
    }
    return 1;
}
int LuaPacket::ReadByte(lua_State* L, WorldPacket* packet)
{
    if (packet == nullptr)
        lua_pushnil(L);
    else
    {
        int8_t byte;
        (*packet) >> byte;
        lua_pushinteger(L, byte);
    }
    return 1;
}
int LuaPacket::ReadShort(lua_State* L, WorldPacket* packet)
{
    int16_t val;
    if (packet != nullptr)
    {
        (*packet) >> val;
        lua_pushinteger(L, val);
    }
    else
        lua_pushnil(L);
    return 1;
}
int LuaPacket::ReadUShort(lua_State* L, WorldPacket* packet)
{
    uint16_t val;
    if (packet != nullptr)
    {
        (*packet) >> val;
        lua_pushinteger(L, val);
    }
    else
        lua_pushnil(L);
    return 1;
}
int LuaPacket::ReadLong(lua_State* L, WorldPacket* packet)
{
    int32_t val;
    if (packet != nullptr)
    {
        (*packet) >> val;
        lua_pushinteger(L, val);
    }
    else
        lua_pushnil(L);
    return 1;
}
int LuaPacket::ReadULong(lua_State* L, WorldPacket* packet)
{
    uint32_t val;
    if (packet != nullptr)
    {
        (*packet) >> val;
        lua_pushinteger(L, val);
    }
    else
        lua_pushnil(L);
    return 1;
}
int LuaPacket::ReadFloat(lua_State* L, WorldPacket* packet)
{
    float val;
    if (packet != nullptr)
    {
        (*packet) >> val;
        lua_pushnumber(L, val);
    }
    else
        lua_pushnil(L);
    return 1;
}
int LuaPacket::ReadDouble(lua_State* L, WorldPacket* packet)
{
    double val;
    if (packet != nullptr)
    {
        (*packet) >> val;
        lua_pushnumber(L, val);
    }
    else
        lua_pushnil(L);
    return 1;
}
int LuaPacket::ReadGUID(lua_State* L, WorldPacket* packet)
{
    uint64_t guid;
    if (packet != nullptr)
    {
        (*packet) >> guid;
        PUSH_GUID(L, guid);
    }
    else
        lua_pushnil(L);
    return 1;
}
int LuaPacket::ReadWoWGuid(lua_State* L, WorldPacket* packet)
{
    WoWGuid nGuid;
    if (packet != nullptr)
    {
        (*packet) >> nGuid;
        PUSH_GUID(L, nGuid.getRawGuid());
    }
    else
        lua_pushnil(L);
    return 1;
}
int LuaPacket::ReadString(lua_State* L, WorldPacket* packet)
{
    std::string str;
    if (packet != nullptr)
    {
        (*packet) >> str;
        lua_pushstring(L, str.c_str());
    }
    else
        lua_pushnil(L);
    return 1;
}
//////////////////////////////////////////////////////////////////////////////////////////
// Write operations
int LuaPacket::WriteByte(lua_State* L, WorldPacket* packet)
{
    const int8_t byte = static_cast<int8_t>(luaL_checkinteger(L, 1));
    (*packet) << byte;
    return 0;
}
int LuaPacket::WriteUByte(lua_State* L, WorldPacket* packet)
{
    const uint8_t byte = static_cast<uint8_t>(luaL_checkinteger(L, 1));
    (*packet) << byte;
    return 0;
}
int LuaPacket::WriteShort(lua_State* L, WorldPacket* packet)
{
    const int16_t val = static_cast<int16_t>(luaL_checkinteger(L, 1));
    (*packet) << val;
    return 0;
}
int LuaPacket::WriteUShort(lua_State* L, WorldPacket* packet)
{
    const uint16_t val = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    (*packet) << val;
    return 0;
}
int LuaPacket::WriteLong(lua_State* L, WorldPacket* packet)
{
    const int32_t val = static_cast<int32_t>(luaL_checkinteger(L, 1));
    (*packet) << val;
    return 0;
}
int LuaPacket::WriteULong(lua_State* L, WorldPacket* packet)
{
    const uint32_t val = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    (*packet) << val;
    return 0;
}
int LuaPacket::WriteFloat(lua_State* L, WorldPacket* packet)
{
    const float val = static_cast<float>(luaL_checknumber(L, 1));
    (*packet) << val;
    return 0;
}
int LuaPacket::WriteDouble(lua_State* L, WorldPacket* packet)
{
    const double val = luaL_checknumber(L, 1);
    (*packet) << val;
    return 0;
}
int LuaPacket::WriteGUID(lua_State* L, WorldPacket* packet)
{
    const uint64_t guid = CHECK_GUID(L, 1);
    (*packet) << guid;
    return 0;
}
int LuaPacket::WriteWoWGuid(lua_State* L, WorldPacket* packet)
{
    const Object* target = CHECK_OBJECT(L, 1);
    if (packet != nullptr)
        (*packet) << target->GetNewGUID();
    return 0;
}
int LuaPacket::WriteString(lua_State* L, WorldPacket* packet)
{
    const std::string str = std::string(luaL_checkstring(L, 1));
    (*packet) << str;
    return 0;
}

int LuaPacket::GetObjectType(lua_State* L, WorldPacket* packet)
{
    if (!packet)
    {
        lua_pushnil(L);
        return 1;
    }
    lua_pushstring(L, "Packet");
    return 1;
}
