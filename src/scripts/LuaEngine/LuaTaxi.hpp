/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

extern "C"
{
#include <lua/lua.h>
}

class TaxiPath;

class LuaTaxi
{
public:
    static int CreateTaxi(lua_State* L, TaxiPath* tp);
    static int GetNodeCount(lua_State* L, TaxiPath* tp);
    static int AddPathNode(lua_State* L, TaxiPath* tp);
    static int GetNodeX(lua_State * L, TaxiPath * tp);
    static int GetNodeY(lua_State * L, TaxiPath * tp);
    static int GetNodeZ(lua_State * L, TaxiPath * tp);
    static int GetNodeMapId(lua_State * L, TaxiPath * tp);
    static int GetId(lua_State* L, TaxiPath* tp);

    static int GetObjectType(lua_State* L, TaxiPath* tp);
};
