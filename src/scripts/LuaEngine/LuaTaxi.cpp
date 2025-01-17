/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LuaTaxi.hpp"

#include "Management/TaxiMgr.hpp"

// todo aaron02 add these back when i have more spare time :)

int LuaTaxi::CreateTaxi(lua_State* L, TaxiPath* tp)
{
    /*TaxiPath* ntp = new TaxiPath();
    PUSH_TAXIPATH(L, ntp);*/
    return 1;
}

int LuaTaxi::GetNodeCount(lua_State* L, TaxiPath* tp)
{
    /*if (!tp) return 0;
    lua_pushinteger(L, tp->getPath().size());*/
    return 1;
}

int LuaTaxi::AddPathNode(lua_State* L, TaxiPath* tp)
{
    /*if (!tp) return 0;
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
    tp->AddPathNode(index, tpn);*/
    return 0;
}

int LuaTaxi::GetNodeX(lua_State* L, TaxiPath* tp)
{
    /*if (!tp) return 0;
    uint32_t index = luaL_checkinteger(L, 1);
    TaxiPathNode* tpn = tp->GetPathNode(index);
    if (tpn != NULL)
        lua_pushinteger(L, tpn->x);
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaTaxi::GetNodeY(lua_State* L, TaxiPath* tp)
{
    /*if (!tp) return 0;
    uint32_t index = luaL_checkinteger(L, 1);
    TaxiPathNode* tpn = tp->GetPathNode(index);
    if (tpn != NULL)
        lua_pushinteger(L, tpn->y);
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaTaxi::GetNodeZ(lua_State* L, TaxiPath* tp)
{
    /*if (!tp) return 0;
    uint32_t index = luaL_checkinteger(L, 1);
    TaxiPathNode* tpn = tp->GetPathNode(index);
    if (tpn != NULL)
        lua_pushinteger(L, tpn->z);
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaTaxi::GetNodeMapId(lua_State* L, TaxiPath* tp)
{
    /*if (!tp) return 0;
    uint32_t index = luaL_checkinteger(L, 1);
    TaxiPathNode* tpn = tp->GetPathNode(index);
    if (tpn != NULL)
        lua_pushinteger(L, tpn->mapid);
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaTaxi::GetId(lua_State* L, TaxiPath* tp)
{
    /*if (!tp) return 0;
    lua_pushinteger(L, tp->getCurrentTaxiPath());*/
    return 1;
}

int LuaTaxi::GetObjectType(lua_State* L, TaxiPath* tp)
{
    if (!tp)
    {
        lua_pushnil(L);
        return 1;
    }
    lua_pushstring(L, "TaxiPath");
    return 1;
}
