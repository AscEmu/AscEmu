/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

class Field;
class QueryResult;

extern "C"
{
#include <lua/lua.h>
}

class LuaSqlApi
{
public:
    //QueryResult methods
    static int GetColumn(lua_State* L, QueryResult* res);
    static int NextRow(lua_State* L, QueryResult* res);
    static int GetColumnCount(lua_State* L, QueryResult* res);
    static int GetRowCount(lua_State* L, QueryResult* res);

    // Field api
    static int GetString(lua_State* L, Field* field);
    static int GetFloat(lua_State* L, Field* field);
    static int GetBool(lua_State* L, Field* field);
    static int GetUByte(lua_State* L, Field* field);
    static int GetByte(lua_State* L, Field* field);
    static int GetUShort(lua_State* L, Field* field);
    static int GetShort(lua_State* L, Field* field);
    static int GetULong(lua_State* L, Field* field);
    static int GetLong(lua_State* L, Field* field);
    static int GetGUID(lua_State* L, Field* field);
};
