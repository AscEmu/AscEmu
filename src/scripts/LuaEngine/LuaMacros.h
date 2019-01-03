/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#define TEST_UNIT() if(ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature()) { return 0; }
#define TEST_UNIT_RET() if(ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature()) { lua_pushboolean(L,0); return 1; }

#define TEST_PLAYER() if(ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer()) { return 0; }
#define TEST_PLAYER_RET() if(ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer()) { lua_pushboolean(L,0); return 1; }

#define TEST_UNITPLAYER() if(ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer()) { return 0; }
#define TEST_UNITPLAYER_RET() if(ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer()) { lua_pushboolean(L,0); return 1; }

#define TEST_GO() if(ptr == nullptr || !ptr->IsInWorld() || !ptr->isGameObject()) { return 0; }
#define TEST_GO_RET() if(ptr == nullptr || !ptr->IsInWorld() || !ptr->isGameObject()) { lua_pushboolean(L,0); return 1; }

#define RET_NIL( ){ lua_pushnil(L); return 1; }
#define RET_BOOL(exp) { (exp) ? lua_pushboolean(L,1) : lua_pushboolean(L,0); return 1; }
#define RET_STRING(str) { lua_pushstring(L,(str)); return 1; }
#define RET_NUMBER(number) { lua_pushnumber(L,(number)); return 1; }
#define RET_INT(integer) { lua_pushinteger(L,(integer)); return 1; }

// Simplicity macros.
#define CHECK_UNIT(L,narg) LuaGlobal::instance()->luaEngine()->CheckUnit(L,narg)
#define CHECK_PLAYER(L,narg) static_cast<Player*>(CHECK_UNIT(L,narg))
#define CHECK_GO(L,narg) LuaGlobal::instance()->luaEngine()->CheckGo(L,narg)
#define CHECK_ITEM(L,narg) LuaGlobal::instance()->luaEngine()->CheckItem(L,narg)
#define CHECK_PACKET(L,narg) LuaGlobal::instance()->luaEngine()->CheckPacket(L,narg)
#define CHECK_GUID(L, narg) LuaGlobal::instance()->luaEngine()->CheckGuid(L,narg)
#define CHECK_OBJECT(L, narg) LuaGlobal::instance()->luaEngine()->CheckObject(L,narg)
#define CHECK_TAXIPATH(L, narg) LuaGlobal::instance()->luaEngine()->CheckTaxiPath(L,narg)
#define CHECK_SPELL(L, narg) LuaGlobal::instance()->luaEngine()->CheckSpell(L,narg)
#define CHECK_AURA(L, narg) LuaGlobal::instance()->luaEngine()->CheckAura(L,narg)

//Its coming soon ^.^
#define CHECK_FLOAT(L,narg) (lua_isnoneornil(L,(narg)) ) ? 0.00f : (float)luaL_checknumber(L,(narg));
#define CHECK_ULONG(L,narg) (uint32)luaL_checknumber((L),(narg))
#define CHECK_USHORT(L, narg) (uint16)luaL_checkinteger((L),(narg))
#define CHECK_BOOL(L,narg) LuaGlobal::instance()->luaEngine()->CheckBool(L,narg)
#define CHECK_UINT8( L, narg ) static_cast< uint8 >( luaL_checkinteger( ( L ), ( narg ) ) )

#define PUSH_UNIT(L, unit) LuaGlobal::instance()->luaEngine()->PushUnit(static_cast<Unit*>(unit),L)
#define PUSH_GO(L, go) LuaGlobal::instance()->luaEngine()->PushGo(static_cast<GameObject*>(go),L)
#define PUSH_PACKET(L,pack) LuaGlobal::instance()->luaEngine()->PushPacket(pack,L)
#define PUSH_ITEM(L,item) LuaGlobal::instance()->luaEngine()->PushItem(static_cast<Item*>(item),L)
#define PUSH_GUID(L, obj) LuaGlobal::instance()->luaEngine()->PushGuid(obj,L)
#define PUSH_TAXIPATH(L, tp) LuaGlobal::instance()->luaEngine()->PushTaxiPath(tp,L)
#define PUSH_SPELL(L, sp) LuaGlobal::instance()->luaEngine()->PushSpell(sp,L)
#define PUSH_SQLFIELD(L, field) LuaGlobal::instance()->luaEngine()->PushSqlField(field,L)
#define PUSH_SQLRESULT(L, res) LuaGlobal::instance()->luaEngine()->PushSqlResult(res,L)
#define PUSH_AURA(L, aura) LuaGlobal::instance()->luaEngine()->PushAura(aura,L)

#define REGTYPE_UNIT (1 << 0)
#define REGTYPE_GO (1 << 1)
#define REGTYPE_QUEST (1 << 2)
#define REGTYPE_SERVHOOK (1 << 3)
#define REGTYPE_ITEM (1 << 4)
#define REGTYPE_GOSSIP (1 << 5)
#define REGTYPE_DUMMYSPELL (1 << 6)
#define REGTYPE_INSTANCE (1 << 7)
#define REGTYPE_UNIT_GOSSIP (REGTYPE_UNIT | REGTYPE_GOSSIP)
#define REGTYPE_GO_GOSSIP (REGTYPE_GO | REGTYPE_GOSSIP)
#define REGTYPE_ITEM_GOSSIP (REGTYPE_ITEM | REGTYPE_GOSSIP)

#define GET_LOCK LuaGlobal::instance()->luaEngine()->getLock().Acquire();
#define RELEASE_LOCK LuaGlobal::instance()->luaEngine()->getLock().Release();
#define CHECK_BINDING_ACQUIRELOCK GET_LOCK if(m_binding == NULL) { RELEASE_LOCK return; }
#define sLuaEventMgr LuaGlobal::instance()->luaEngine()->LuaEventMgr