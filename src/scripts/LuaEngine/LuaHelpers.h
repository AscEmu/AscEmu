/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <Units/Unit.h>
#include <Management/Item.h>
#include <Management/TaxiMgr.h>

extern "C"
{
    #include <lua/lua.h>
    #include <lua/lauxlib.h>
}
#include <string>

class LuaHelpers
{
public:
    static int ExtractfRefFromCString(lua_State* L, const char* functionName);
};

template<typename T>
struct RegType
{
    const char* name;
    int(*mfunc)(lua_State*, T*);
};
template<typename T> RegType<T>* GetMethodTable();
void report(lua_State*);

template<typename T> const char* GetTClassName();
template<typename T> const char* GetTClassName() { return "UNKNOWN"; }
template<> inline const char* GetTClassName<Unit>() { return "Unit"; }
template<> inline const char* GetTClassName<Item>() { return "Item"; }
template<> inline const char* GetTClassName<GameObject>() { return "GameObject"; }
template<> inline const char* GetTClassName<WorldPacket>() { return "LuaPacket"; }
template<> inline const char* GetTClassName<TaxiPath>() { return "LuaTaxi"; }
template<> inline const char* GetTClassName<Spell>() { return "Spell"; }
template<> inline const char* GetTClassName<Field>() { return "SQL_Field"; }
template<> inline const char* GetTClassName<QueryResult>() { return "SQL_QResult"; }
template<> inline const char* GetTClassName<Aura>() { return "LuaAura"; }

template<typename T> RegType<T>* GetMethodTable();
template<> RegType<Unit>* GetMethodTable<Unit>();
template<> RegType<Item>* GetMethodTable<Item>();
template<> RegType<GameObject>* GetMethodTable<GameObject>();
template<> RegType<WorldPacket>* GetMethodTable<WorldPacket>();
template<> RegType<TaxiPath>* GetMethodTable<TaxiPath>();
template<> RegType<Spell>* GetMethodTable<Spell>();
template<> RegType<Field>* GetMethodTable<Field>();
template<> RegType<QueryResult>* GetMethodTable<QueryResult>();
template<> RegType<Aura>* GetMethodTable<Aura>();