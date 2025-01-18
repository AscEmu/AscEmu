/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LuaHelpers.hpp"

#include <cstdlib>
#include <string>

int LuaHelpers::ExtractfRefFromCString(lua_State* L, const char* functionName)
{
    auto functionRef = 0;
    auto top = lua_gettop(L);
    if (functionName != nullptr)
    {
#if defined(linux) || defined(__linux) || defined(FreeBSD) || defined(__FreeBSD__) || defined(__APPLE__)
        char* copy = strdup(functionName);
#else
        char* copy = _strdup(functionName);
#endif
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
                
                if (lua_istable(L, -1))
                {
                    token = strtok(nullptr, ".:");
                    continue;
                }

                luaL_error(L, "Reference creation failed! (%s) is not a valid Lua function. \n", functionName);
                break;

            }
        }
        free((void*)copy);
        lua_settop(L, top);
    }
    return functionRef;
}
