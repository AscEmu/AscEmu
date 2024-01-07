/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "LuaSqlApi.hpp"
#include "LUAEngine.hpp"
#include "LuaGlobal.hpp"
#include "LuaMacros.h"
#include "Database/Database.h"
#include <cstdint>

//QueryResult methods
int LuaSqlApi::GetColumn(lua_State* L, QueryResult* res)
{
    if (res != nullptr)
    {
        const uint32_t column = CHECK_ULONG(L, 1);
        const uint32_t fields = res->GetFieldCount();
        if (column > fields)
            luaL_error(L, "GetColumn, Column %d bigger than max column %d", column, res->GetFieldCount());
        else
        {
            Field* field = &(res->Fetch()[column]);
            PUSH_SQLFIELD(L, field);
        }
    }
    else
        lua_pushnil(L);
    return 1;
}

int LuaSqlApi::NextRow(lua_State* L, QueryResult* res)
{
    if (res != nullptr)
    {
        if (res->NextRow())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    }
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaSqlApi::GetColumnCount(lua_State* L, QueryResult* res)
{
    if (res == nullptr)
        lua_pushnil(L);
    else
        lua_pushnumber(L, res->GetFieldCount());
    return 1;
}

int LuaSqlApi::GetRowCount(lua_State* L, QueryResult* res)
{
    if (res == nullptr)
        lua_pushnil(L);
    else
        lua_pushnumber(L, res->GetRowCount());
    return 1;
}

// Field api
int LuaSqlApi::GetString(lua_State* L, Field* field)
{
    if (field == nullptr)
        lua_pushnil(L);
    else
        lua_pushstring(L, field->GetString());
    return 1;
}

int LuaSqlApi::GetFloat(lua_State* L, Field* field)
{
    if (field == nullptr)
        lua_pushnil(L);
    else
        lua_pushnumber(L, field->GetFloat());
    return 1;
}

int LuaSqlApi::GetBool(lua_State* L, Field* field)
{
    if (field == nullptr)
        lua_pushnil(L);
    else
    {
        if (field->GetBool())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    }
    return 1;
}

int LuaSqlApi::GetUByte(lua_State* L, Field* field)
{
    if (field == nullptr)
        lua_pushnil(L);
    else
        lua_pushinteger(L, field->GetUInt8());
    return 1;
}

int LuaSqlApi::GetByte(lua_State* L, Field* field)
{
    if (field == nullptr)
        lua_pushnil(L);
    else
        lua_pushinteger(L, field->GetInt8());
    return 1;
}

int LuaSqlApi::GetUShort(lua_State* L, Field* field)
{
    if (field == nullptr)
        lua_pushnil(L);
    else
        lua_pushinteger(L, field->GetUInt16());
    return 1;
}

int LuaSqlApi::GetShort(lua_State* L, Field* field)
{
    if (field == nullptr)
        lua_pushnil(L);
    else
        lua_pushinteger(L, field->GetInt16());
    return 1;
}

int LuaSqlApi::GetULong(lua_State* L, Field* field)
{
    if (field == nullptr)
        lua_pushnil(L);
    else
        lua_pushnumber(L, field->GetUInt32());
    return 1;
}

int LuaSqlApi::GetLong(lua_State* L, Field* field)
{
    if (field == nullptr)
        lua_pushnil(L);
    else
        lua_pushnumber(L, field->GetInt32());
    return 1;
}

int LuaSqlApi::GetGUID(lua_State* L, Field* field)
{
    if (field == nullptr)
        lua_pushnil(L);
    else
    {
        const uint64_t guid = field->GetUInt64();
        PUSH_GUID(L, guid);
    }
    return 1;
}
