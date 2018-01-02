/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LuaHelpers.h"

int LuaHelpers::ExtractfRefFromCString(lua_State* L, const char* functionName)
{
    auto functionRef = 0;
    auto top = lua_gettop(L);
    if (functionName != nullptr)
    {
        char* copy = strdup(functionName);
        char* token = strtok(copy, ".:");
        if (strpbrk(functionName, ".:") == nullptr)
        {
            lua_getglobal(L, functionName);
            if (lua_isfunction(L, -1) && !lua_iscfunction(L, -1))
            {
                functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
            }
            else
            {
                luaL_error(L, "Reference creation failed! (%s) is not a valid Lua function. \n", functionName);
            }
        }
        else
        {
            lua_getglobal(L, "_G");
            while (token != nullptr)
            {
                lua_getfield(L, -1, token);
                if (lua_isfunction(L, -1) && !lua_iscfunction(L, -1))
                {
                    functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
                    break;
                }
                else if (lua_istable(L, -1))
                {
                    token = strtok(nullptr, ".:");
                    continue;
                }
                else
                {
                    luaL_error(L, "Reference creation failed! (%s) is not a valid Lua function. \n", functionName);
                    break;
                }
            }
        }
        free((void*)copy);
        lua_settop(L, top);
    }
    return functionRef;
}