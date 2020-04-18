/*
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "LUAEngine.h"
#include "Management/TaxiMgr.h"

namespace LuaTaxi
{
    int CreateTaxi(lua_State* L, TaxiPath* /*tp*/)
    {
        TaxiPath* ntp = new TaxiPath();
        PUSH_TAXIPATH(L, ntp);
        return 1;
    }

    int GetNodeCount(lua_State* L, TaxiPath* tp)
    {
        if (!tp) return 0;
        lua_pushinteger(L, tp->GetNodeCount());
        return 1;
    }

    int AddPathNode(lua_State* L, TaxiPath* tp)
    {
        if (!tp) return 0;
        uint32_t mapid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        float x = CHECK_FLOAT(L, 2);
        float y = CHECK_FLOAT(L, 3);
        float z = CHECK_FLOAT(L, 4);
        uint32_t index = static_cast<uint32_t>(luaL_optnumber(L, 5, static_cast<lua_Number>(tp->GetNodeCount())));

        TaxiPathNode* tpn = new TaxiPathNode();
        tpn->mapid = mapid;
        tpn->x = x;
        tpn->y = y;
        tpn->z = z;
        tp->AddPathNode(index, tpn);
        return 0;
    }

    /*int GetNodeX(lua_State * L, TaxiPath * tp)
    {
    if (!tp) return 0;
    uint32_t index = luaL_checkinteger(L, 1);
    TaxiPathNode* tpn = tp->GetPathNode(index);
    if (tpn != NULL)
    lua_pushinteger(L, tpn->x);
    else
    lua_pushnil(L);
    return 1;
    }

    int GetNodeY(lua_State * L, TaxiPath * tp)
    {
    if (!tp) return 0;
    uint32_t index = luaL_checkinteger(L, 1);
    TaxiPathNode* tpn = tp->GetPathNode(index);
    if (tpn != NULL)
    lua_pushinteger(L, tpn->y);
    else
    lua_pushnil(L);
    return 1;
    }

    int GetNodeZ(lua_State * L, TaxiPath * tp)
    {
    if (!tp) return 0;
    uint32_t index = luaL_checkinteger(L, 1);
    TaxiPathNode* tpn = tp->GetPathNode(index);
    if (tpn != NULL)
    lua_pushinteger(L, tpn->z);
    else
    lua_pushnil(L);
    return 1;
    }

    int GetNodeMapId(lua_State * L, TaxiPath * tp)
    {
    if (!tp) return 0;
    uint32_t index = luaL_checkinteger(L, 1);
    TaxiPathNode* tpn = tp->GetPathNode(index);
    if (tpn != NULL)
    lua_pushinteger(L, tpn->mapid);
    else
    lua_pushnil(L);
    return 1;
    }*/

    int GetId(lua_State* L, TaxiPath* tp)
    {
        if (!tp) return 0;
        lua_pushinteger(L, tp->GetID());
        return 1;
    }

    int GetObjectType(lua_State* L, TaxiPath* tp)
    {
        if (!tp)
        {
            lua_pushnil(L);
            return 1;
        }
        lua_pushstring(L, "TaxiPath");
        return 1;
    }
}
