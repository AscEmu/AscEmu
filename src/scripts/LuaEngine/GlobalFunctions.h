/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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

#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

#include "WorldConf.h"
#include "LUAEngine.h"
#include "git_version.h"
#include "Management/TaxiMgr.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#if VERSION_STRING >= Cata
#include "Management/Guild/Guild.hpp"
#endif

#define ENGINE_NAME "ALE" // You should check in your scripts that GetLuaEngine() == "ALE"

namespace luaGlobalFunctions
{
    static int PerformIngameSpawn(lua_State* L)
    {
        uint32_t spawntype = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        uint32_t entry = static_cast<uint32_t>(luaL_checkinteger(L, 2));
        uint32_t map = static_cast<uint32_t>(luaL_checkinteger(L, 3));
        float x = CHECK_FLOAT(L, 4);
        float y = CHECK_FLOAT(L, 5);
        float z = CHECK_FLOAT(L, 6);
        float o = CHECK_FLOAT(L, 7);
        uint32_t faction = static_cast<uint32_t>(luaL_checkinteger(L, 8)); //also scale as percentage
        uint32_t duration = static_cast<uint32_t>(luaL_checkinteger(L, 9));
        uint32_t equip1 = static_cast<uint32_t>(luaL_optinteger(L, 10, 1));
        uint32_t equip2 = static_cast<uint32_t>(luaL_optinteger(L, 11, 1));
        uint32_t equip3 = static_cast<uint32_t>(luaL_optinteger(L, 12, 1));
        //13: instance id
        uint32_t save = static_cast<uint32_t>(luaL_optinteger(L, 14, 0));
        if (x && y && z && entry)
        {
            if (spawntype == 1)  //Unit
            {
                CreatureProperties const* p = sMySQLStore.getCreatureProperties(entry);
                if (p == nullptr)
                    return 0;

                MapMgr* mapMgr = sInstanceMgr.GetMapMgr(map);
                if (!mapMgr)
                    return 0;

                //int32_t instanceid = static_cast<int32_t>(luaL_optinteger(L, 13, mapMgr->GetInstanceID()));
                Creature* pCreature = mapMgr->CreateCreature(entry);
                pCreature->Load(p, x, y, z, o);
                pCreature->SetFaction(faction);
                pCreature->setVirtualItemSlotId(MELEE, equip1);
                pCreature->setVirtualItemSlotId(OFFHAND, equip2);
                pCreature->setVirtualItemSlotId(RANGED, equip3);
                pCreature->Phase(PHASE_SET, 1);
                pCreature->m_noRespawn = true;
                pCreature->AddToWorld(mapMgr);
                if (duration > 0)
                    pCreature->Despawn(duration, 0);
                if (save)
                    pCreature->SaveToDB();
                PUSH_UNIT(L, pCreature);
            }
            else if (spawntype == 2)  //GO
            {
                auto gameobject_info = sMySQLStore.getGameObjectProperties(entry);
                if (gameobject_info == nullptr)
                    return 0;

                MapMgr* mapMgr = sInstanceMgr.GetMapMgr(map);
                if (!mapMgr)
                    return 0;

                GameObject* go = mapMgr->CreateGameObject(entry);
                go->CreateFromProto(entry, map, x, y, z, o);
                go->Phase(PHASE_SET, 1);
                go->setScale(((float)faction) / 100.0f);

                go->AddToWorld(mapMgr);

                if (duration)
                    sEventMgr.AddEvent(go, &GameObject::ExpireAndDelete, EVENT_GAMEOBJECT_UPDATE, duration, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                if (save)
                    go->SaveToDB();
                PUSH_GO(L, go);
            }
            else
                lua_pushnil(L);
        }
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetGameTime(lua_State* L)
    {
        lua_pushnumber(L, ((uint32_t)UNIXTIME)); //in seconds.
        return 1;
    }

    static int GetPlayer(lua_State* L)
    {
        const char* plName = luaL_checkstring(L, 1);
        Player* plr = sObjectMgr.GetPlayer(plName);
        if (plr)
        {
            if (plr->IsInWorld())
            {
                PUSH_UNIT(L, plr);
                return 1;
            }
        }
        lua_pushboolean(L, 0);
        return 1;
    }

    static int GetLUAEngine(lua_State* L)  //also mapped to GetLuaEngine()
    {
        lua_pushstring(L, ENGINE_NAME);
        return 1;
    }

    static int logcol(lua_State* L)
    {
        luaL_checkinteger(L, 1);
        return 0;
    }

    static int WorldDBQuery(lua_State* L)
    {
        const char* qStr = luaL_checkstring(L, 1);
        //uint32_t fID = static_cast<uint32_t>(luaL_optinteger(L, 2, 0)); //column
        //uint32_t rID = static_cast<uint32_t>(luaL_optinteger(L, 3, 0)); //row
        if (!qStr)
            return 0;
        QueryResult* result = WorldDatabase.Query(qStr);
        lua_settop(L, 0);
        PUSH_SQLRESULT(L, result);
        return 1;
    }

    static int CharDBQuery(lua_State* L)
    {
        const char* qStr = luaL_checkstring(L, 1);
        //uint32_t fID = static_cast<uint32_t>(luaL_optinteger(L, 2, 0)); //column
        //uint32_t rID = static_cast<uint32_t>(luaL_optinteger(L, 3, 0)); //row
        if (!qStr)
            return 0;
        QueryResult* result = CharacterDatabase.Query(qStr);
        lua_settop(L, 0);
        PUSH_SQLRESULT(L, result);
        return 1;
    }

    static int WorldDBQueryTable(lua_State* L)
    {
        const char* qStr = luaL_checkstring(L, 1);
        lua_newtable(L);
        if (!qStr) return 0;
        QueryResult* result = WorldDatabase.Query(qStr);
        PUSH_SQLRESULT(L, result);
        return 1;
    }

    static int CharDBQueryTable(lua_State* L)
    {
        const char* qStr = luaL_checkstring(L, 1);
        lua_newtable(L);
        if (!qStr) return 0;
        QueryResult* result = CharacterDatabase.Query(qStr);
        PUSH_SQLRESULT(L, result);
        return 1;
    }

    static int SendWorldMessage(lua_State* L)
    {
        const char* message = luaL_checkstring(L, 1);
        uint32_t MsgType = static_cast<uint32_t>(luaL_checkinteger(L, 2));
        if (!message || !MsgType)
        {
            lua_pushnil(L);
            return 1;
        }

        if (MsgType == 1)
            sWorld.sendAreaTriggerMessage(message);
        else if (MsgType == 2)
            sWorld.sendMessageToAll(message);

        return 0;
    }

    static int ReloadTable(lua_State* L)
    {
        const char* TableName = luaL_checkstring(L, 1);
        if (!stricmp(TableName, "spell_disable"))
        {
            sObjectMgr.ReloadDisabledSpells();
        }
        else if (!stricmp(TableName, "vendors"))
        {
            sObjectMgr.ReloadVendors();
        }
        else
        {
            if (!stricmp(TableName, "command_overrides"))    // Command Overrides
            {
                sCommandTableStorage.Dealloc();
                sCommandTableStorage.Init();
                sCommandTableStorage.Load();
            }
        }
        return 0;
    }

    static int ReloadLuaEngine(lua_State* /*L*/)
    {
        /*g_luaMgr.Restart();
        MapMgr * mgr;
        LuaUnitBinding * m_binding;
        for (uint32_t i = 0; i < NUM_MAPS; ++i)
        {
        if (!sInstanceMgr.GetMapMgr(i))
        continue;
        mgr = sInstanceMgr.GetMapMgr(i);
        for(uint32_t guid=1; guid < mgr->m_CreatureArraySize; guid++)
        {
        Creature *pCreature = mgr->GetCreature(GET_LOWGUID_PART(guid));
        if(pCreature)
        {
        m_binding = g_luaMgr.GetUnitBinding(pCreature->getEntry());
        if (m_binding != NULL)
        g_engine->OnUnitEvent( pCreature, m_binding->Functions[CREATURE_EVENT_ON_LOAD], CREATURE_EVENT_ON_LOAD, NULL, 0, 0, 0, 0, NULL );
        }
        }
        }
        mgr->KillThread();*/
        return 0;
    }

    static int GetPlayersInWorld(lua_State* L)
    {
        uint32_t count = 0;

        lua_newtable(L);
        sObjectMgr._playerslock.lock();

        for (std::unordered_map<uint32_t, Player*>::const_iterator itr = sObjectMgr._players.begin(); itr != sObjectMgr._players.end(); ++itr)
        {
            count++;
            Player* ret = (*itr).second;
            lua_pushinteger(L, count);
            PUSH_UNIT(L, (static_cast<Unit*>(ret)));
            lua_rawset(L, -3);
        }
        sObjectMgr._playerslock.unlock();
        return 1;
    }

    static int Rehash(lua_State* /*L*/)
    {
        sWorld.loadWorldConfigValues(true);
        return 0;
    }

    static int GetClientVersion(lua_State* L)
    {
        lua_pushinteger(L, VERSION_STRING);
        return 1;
    }

    static int GetAERevision(lua_State* L)
    {
        lua_pushstring(L, BUILD_HASH_STR);
        return 1;
    }

    /*static int GetInstanceIdsByMap(lua_State * L)
    {
    uint32_t mapid = luaL_checkinteger(L,1);
    uint32_t ret = NULL;
    uint32_t count = 0;
    lua_newtable(L);

    InstanceMap * instancemap = sInstanceMgr.GetInstanceMap(mapid);
    for(InstanceMap::iterator itr = instancemap->begin(); itr != instancemap->end(); ++itr)
    {
    count++;
    ret = itr->second->m_instanceId;
    lua_pushinteger(L,count);
    lua_pushinteger(L,ret);
    lua_rawset(L,-3);
    }
    return 1;
    }*/
    //////////////////////////////////////////////////////////////////////////
    // WORLD PVP NOT SUPPORTED!
    //////////////////////////////////////////////////////////////////////////
    /*
    static int SendPvPCaptureMessage(lua_State * L)
    {
    uint32_t zoneid = luaL_checkinteger(L, 1);
    const char* msg = luaL_checkstring(L, 2);
    AreaTable * at = dbcArea.LookupEntry(zoneid);
    if(!zoneid || !msg || !at)
    return 1;
    MapMgr* mapmgr = sInstanceMgr.GetMapMgr(at->mapId);
    if (mapmgr)
    mapmgr->SendPvPCaptureMessage(ZONE_MASK_ALL, zoneid, msg);
    return 1;
    }
    */
    static int GetPlayersInMap(lua_State* L)
    {
        uint32_t count = 0;
        lua_newtable(L);
        uint32_t mapid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        MapMgr* mgr = sInstanceMgr.GetMapMgr(mapid);
        if (!mgr)
            return 0;

        for (PlayerStorageMap::iterator itr = mgr->m_PlayerStorage.begin(); itr != mgr->m_PlayerStorage.end(); ++itr)
        {
            count++;
            Player* ret = (*itr).second;
            lua_pushinteger(L, count);
            PUSH_UNIT(L, (static_cast<Unit*>(ret)));
            lua_rawset(L, -3);
        }
        return 1;
    }

    static int GetPlayersInZone(lua_State* L)
    {
        uint32_t count = 0;
        lua_newtable(L);
        uint32_t zoneid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        sObjectMgr._playerslock.lock();

        for (std::unordered_map<uint32_t, Player*>::const_iterator itr = sObjectMgr._players.begin(); itr != sObjectMgr._players.end(); ++itr)
        {
            if ((*itr).second->GetZoneId() == zoneid)
            {
                count++;
                Player* ret = (*itr).second;
                lua_pushinteger(L, count);
                PUSH_UNIT(L, (static_cast<Unit*>(ret)));
                lua_rawset(L, -3);
            }
        }
        sObjectMgr._playerslock.unlock();
        return 1;
    }

    static int SendMail(lua_State* L)
    {
        uint32_t type = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        uint64_t sender_guid = CHECK_GUID(L, 2);
        uint64_t recipient_guid = CHECK_GUID(L, 3);
        std::string subject = luaL_checkstring(L, 4);
        std::string body = luaL_checkstring(L, 5);
        uint32_t money = static_cast<uint32_t>(luaL_checkinteger(L, 6));
        uint32_t cod = static_cast<uint32_t>(luaL_checkinteger(L, 7));
        uint64_t item_guid = CHECK_GUID(L, 8);
        uint32_t stationery = static_cast<uint32_t>(luaL_checkinteger(L, 9));
        uint32_t deliverdelay = static_cast<uint32_t>(luaL_optinteger(L, 10, 0));
        sMailSystem.SendAutomatedMessage(type, sender_guid, recipient_guid, subject, body, money, cod, item_guid, stationery, body.empty() ? MAIL_CHECK_MASK_COPIED : MAIL_CHECK_MASK_HAS_BODY, deliverdelay);
        return 0;
    }

    static int GetTaxiPath(lua_State* L)
    {
        uint32_t path = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        TaxiPath* tp = sTaxiMgr.GetTaxiPath(path);
        if (tp != NULL)
            PUSH_TAXIPATH(L, tp);
        else
            lua_pushnil(L);
        return 1;
    }

    static int SetDBCSpellVar(lua_State* L)
    {
        uint32_t entry = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        const char* var = luaL_checkstring(L, 2);
        int subindex = 0;
        int valindex = 3;
        if (lua_gettop(L) == 4)
        {
            subindex = static_cast<int>(luaL_optinteger(L, 3, 0));
            valindex++;
        }
        SpellInfo const* proto = sSpellMgr.getSpellInfo(entry);
        if (!entry || !var || subindex < 0 || !proto)
        {
            lua_pushboolean(L, 0);
            return 1;
        }
        LuaSpellEntry l = GetLuaSpellEntryByName(var);
        if (!l.name)
            RET_NIL();
        switch (l.typeId)  //0: int, 1: char*, 2: bool, 3: float
        {
            case 0:
                GET_SPELLVAR_INT(proto, l.offset, subindex) = static_cast<int>(luaL_checkinteger(L, valindex));
                lua_pushboolean(L, 1);
                break;
            case 1:
                strcpy(GET_SPELLVAR_CHAR(proto, l.offset, subindex), luaL_checkstring(L, valindex));
                lua_pushboolean(L, 1);
                break;
            case 2:
                GET_SPELLVAR_BOOL(proto, l.offset, subindex) = CHECK_BOOL(L, valindex);
                lua_pushboolean(L, 1);
                break;
            case 3:
                GET_SPELLVAR_FLOAT(proto, l.offset, subindex) = (float)luaL_checknumber(L, valindex);
                lua_pushboolean(L, 1);
                break;
        }
        return 1;
    }

    static int GetDBCSpellVar(lua_State* L)
    {
        uint32_t entry = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        const char* var = luaL_checkstring(L, 2);
        int subindex = static_cast<int>(luaL_optinteger(L, 3, 0));
        SpellInfo const* proto = sSpellMgr.getSpellInfo(entry);
        if (!entry || !var || subindex < 0 || !proto)
        {
            lua_pushnil(L);
            return 1;
        }
        LuaSpellEntry l = GetLuaSpellEntryByName(var);
        if (!l.name)
            RET_NIL();
        switch (l.typeId)  //0: int, 1: char*, 2: bool, 3: float
        {
            case 0:
                lua_pushinteger(L, GET_SPELLVAR_INT(proto, l.offset, subindex));
                break;
            case 1:
                lua_pushstring(L, GET_SPELLVAR_CHAR(proto, l.offset, subindex));
                break;
            case 2:
                lua_pushboolean(L, (GET_SPELLVAR_BOOL(proto, l.offset, subindex)) ? 1 : 0);
                break;
            case 3:
                lua_pushnumber(L, GET_SPELLVAR_FLOAT(proto, l.offset, subindex));
                break;
        }
        return 1;
    }

    int bit_and(lua_State* L)
    {
        uint32_t left = CHECK_ULONG(L, 1);
        int top = lua_gettop(L);
        if (top > 1)
        {
            for (int i = 2; i <= top; ++i)
            {
                if (lua_isnumber(L, i))
                    left &= CHECK_ULONG(L, i);
            }
        }
        RET_NUMBER(left)
    }
    int bit_or(lua_State* L)
    {
        uint32_t left = CHECK_ULONG(L, 1);
        int top = lua_gettop(L);
        if (top > 1)
        {
            for (int i = 2; i <= top; ++i)
            {
                if (lua_isnumber(L, i))
                    left |= CHECK_ULONG(L, i);
            }
        }
        RET_NUMBER(left)
    }
    int bit_xor(lua_State* L)
    {
        uint32_t left = CHECK_ULONG(L, 1);
        int top = lua_gettop(L);
        if (top > 1)
        {
            for (int i = 2; i <= top; ++i)
            {
                if (lua_isnumber(L, i))
                    left ^= CHECK_ULONG(L, i);
            }
        }
        RET_NUMBER(left)
    }
    /*int bit_not(lua_State* L)
    {
        uint32_t left = CHECK_ULONG(L, 1);
        RET_NUMBER(~left)
    }*/
    int bit_shiftleft(lua_State* L)
    {
        uint32_t left = CHECK_ULONG(L, 1);
        uint8_t count = luaL_checkinteger(L, 2) & 0x7F;
        RET_NUMBER(left << count)
    }
    int bit_shiftright(lua_State* L)
    {
        uint32_t left = CHECK_ULONG(L, 1);
        uint8_t count = luaL_checkinteger(L, 2) & 0x7F;
        RET_NUMBER(left >> count)
    }
    int RemoveTimedEvents(lua_State* /*L*/)
    {
        sLuaEventMgr.RemoveEvents();
        return 0;
    }
    int RemoveTimedEventsWithName(lua_State* L)
    {
        const char* name = luaL_checkstring(L, 1);
        sLuaEventMgr.RemoveEventsByName(name);
        return 0;
    }
    int RemoveTimedEvent(lua_State* L)
    {
        int ref = static_cast<int>(luaL_checkinteger(L, 1));
        sLuaEventMgr.RemoveEventByRef(ref);
        return 0;
    }
    int RemoveTimedEventsInTable(lua_State* L)
    {
        const char* table = luaL_checkstring(L, 1);
        sLuaEventMgr.RemoveEventsInTable(table);
        return 0;
    }
    int HasTimedEvents(lua_State* L)
    {
        lua_pushboolean(L, sLuaEventMgr.event_HasEvents() ? 1 : 0);
        return 1;
    }
    int HasTimedEvent(lua_State* L)
    {
        int ref = static_cast<int>(luaL_checkinteger(L, 1));
        lua_pushboolean(L, sLuaEventMgr.HasEvent(ref) ? 1 : 0);
        return 1;
    }
    int HasTimedEventWithName(lua_State* L)
    {
        const char* name = luaL_checkstring(L, 1);
        lua_pushboolean(L, sLuaEventMgr.HasEventWithName(name) ? 1 : 0);
        return 1;
    }
    int HasTimedEventInTable(lua_State* L)
    {
        const char* table = luaL_checkstring(L, 1);
        lua_pushboolean(L, sLuaEventMgr.HasEventInTable(table) ? 1 : 0);
        return 1;
    }
    int GetPlatform(lua_State* L)
    {
        lua_pushliteral(L, PLATFORM_TEXT);
        return 1;
    }
    int NumberToGUID(lua_State* L)
    {
        uint64_t num = (uint64_t)luaL_checknumber(L, 1);
        PUSH_GUID(L, num);
        return 1;
    }
    int SendPacketToZone(lua_State* L)
    {
        WorldPacket* data = CHECK_PACKET(L, 1);
        uint32_t zone_id = CHECK_ULONG(L, 2);
        if (data && zone_id)
            sWorld.sendZoneMessage(data, zone_id);
        return 0;
    }

    int SendPacketToInstance(lua_State* L)
    {
        WorldPacket* data = CHECK_PACKET(L, 1);
        uint32_t instance_id = CHECK_ULONG(L, 2);
        if (data && instance_id)
            sWorld.sendInstanceMessage(data, instance_id);
        return 0;
    }

    int SendPacketToWorld(lua_State* L)
    {
        WorldPacket* data = CHECK_PACKET(L, 1);
        if (data)
            sWorld.sendGlobalMessage(data);
        return 0;
    }

    int SendPacketToChannel(lua_State* L)
    {
        WorldPacket* pack = CHECK_PACKET(L, 1);
        const char* channelName = luaL_checkstring(L, 2);
        uint32_t team = CHECK_ULONG(L, 3);
        Channel* channel = sChannelMgr.getChannel(channelName, team);
        if (!channel || !pack)
            return 0;

        channel->SendToAll(pack);

        return 1;
    }

    int GetInstanceCreature(lua_State* L)
    {
        uint32_t map = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        uint32_t iid = static_cast<uint32_t>(luaL_checkinteger(L, 2));
        uint64_t guid = 0;
        uint32_t spawnId = 0;
        const char* type = luaL_typename(L, 3);
        if (!strcmp(type, "number"))
            spawnId = static_cast<uint32_t>(luaL_checkinteger(L, 3));
        else
            guid = CHECK_GUID(L, 3);

        Instance* pInstance = sInstanceMgr.GetInstanceByIds(map, iid);
        if (pInstance == NULL || (!guid && !spawnId))
            RET_NIL();

        Creature* pCreature = NULL;
        if (guid)
        {
            WoWGuid wowGuid;
            wowGuid.Init(guid);
            pCreature = pInstance->m_mapMgr->GetCreature(wowGuid.getGuidLowPart());
        }
        else
            pCreature = pInstance->m_mapMgr->GetSqlIdCreature(spawnId);

        PUSH_UNIT(L, pCreature);
        return 1;
    }

    int GetInstancePlayerCount(lua_State* L)
    {
        uint32_t map = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        uint32_t iid = static_cast<uint32_t>(luaL_checkinteger(L, 2));

        Instance* pInstance = sInstanceMgr.GetInstanceByIds(map, iid);
        if (pInstance == NULL)
            RET_NIL();

        lua_pushnumber(L, pInstance->m_mapMgr->GetPlayerCount());
        return 1;
    }

    int GetPlayersInInstance(lua_State* L)
    {
        uint32_t map = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        uint32_t iid = static_cast<uint32_t>(luaL_checkinteger(L, 2));

        Instance* pInstance = sInstanceMgr.GetInstanceByIds(map, iid);
        if (pInstance == NULL)
            RET_NIL();

        Player* ret = NULL;
        uint32_t count = 0;
        lua_newtable(L);
        MapMgr* mgr = pInstance->m_mapMgr;
        if (!mgr)
            RET_NIL();

        for (PlayerStorageMap::iterator itr = mgr->m_PlayerStorage.begin(); itr != mgr->m_PlayerStorage.end(); ++itr)
        {
            count++;
            ret = (*itr).second;
            lua_pushinteger(L, count);
            PUSH_UNIT(L, (static_cast<Unit*>(ret)));
            lua_rawset(L, -3);
        }
        return 1;

    }

    int GetGuildByName(lua_State* L)
    {
        const char* name = luaL_checkstring(L, 1);
        Guild* guild = sGuildMgr.getGuildByName(name);
        lua_pushnumber(L, guild ? guild->getId() : -1);
        return 1;
    }

    int GetGuildByLeaderGuid(lua_State* L)
    {
        uint64_t guid = CHECK_GUID(L, 1);
        Guild* guild = sGuildMgr.getGuildByLeader(guid);
        lua_pushnumber(L, guild ? guild->getId() : -1);
        return 1;
    }
}

void RegisterGlobalFunctions(lua_State* L)
{
    lua_register(L, "PerformIngameSpawn", &luaGlobalFunctions::PerformIngameSpawn);
    lua_register(L, "GetPlayer", &luaGlobalFunctions::GetPlayer);
    lua_register(L, "GetLUAEngine", &luaGlobalFunctions::GetLUAEngine);
    lua_register(L, "GetLuaEngine", &luaGlobalFunctions::GetLUAEngine);
    lua_register(L, "GetClientVersion", &luaGlobalFunctions::GetClientVersion);
    lua_register(L, "GetGameTime", &luaGlobalFunctions::GetGameTime);
    lua_register(L, "WorldDBQuery", &luaGlobalFunctions::WorldDBQuery);
    lua_register(L, "CharDBQuery", &luaGlobalFunctions::CharDBQuery);
    lua_register(L, "WorldDBQueryTable", &luaGlobalFunctions::WorldDBQueryTable);
    lua_register(L, "CharDBQueryTable", &luaGlobalFunctions::CharDBQueryTable);
    lua_register(L, "SendWorldMessage", &luaGlobalFunctions::SendWorldMessage);
    lua_register(L, "ReloadTable", &luaGlobalFunctions::ReloadTable);
    lua_register(L, "ReloadLuaEngine", &luaGlobalFunctions::ReloadLuaEngine);
    lua_register(L, "Rehash", &luaGlobalFunctions::Rehash);
    lua_register(L, "logcol", &luaGlobalFunctions::logcol);
    lua_register(L, "GetPlayersInWorld", &luaGlobalFunctions::GetPlayersInWorld);
    lua_register(L, "GetAERevision", &luaGlobalFunctions::GetAERevision);
    lua_register(L, "GetPlayersInMap", &luaGlobalFunctions::GetPlayersInMap);
    lua_register(L, "GetPlayersInZone", &luaGlobalFunctions::GetPlayersInZone);
    lua_register(L, "SendMail", &luaGlobalFunctions::SendMail);
    lua_register(L, "GetTaxiPath", &luaGlobalFunctions::GetTaxiPath);
    lua_register(L, "SetDBCSpellVar", &luaGlobalFunctions::SetDBCSpellVar);
    lua_register(L, "GetDBCSpellVar", &luaGlobalFunctions::GetDBCSpellVar);
    //Lua's bit instructions
    lua_register(L, "bit_and", &luaGlobalFunctions::bit_and);
    lua_register(L, "bit_or", &luaGlobalFunctions::bit_or);
    lua_register(L, "bit_xor", &luaGlobalFunctions::bit_xor);
    //lua_register(L, "bit_not", &luaGlobalFunctions::bit_not);
    lua_register(L, "bit_shiftleft", &luaGlobalFunctions::bit_shiftleft);
    lua_register(L, "bit_shiftright", &luaGlobalFunctions::bit_shiftright);

    lua_register(L, "RemoveTimedEventsInTable", &luaGlobalFunctions::RemoveTimedEventsInTable);
    lua_register(L, "RemoveTimedEventsWithName", &luaGlobalFunctions::RemoveTimedEventsWithName);
    lua_register(L, "RemoveTimedEvent", &luaGlobalFunctions::RemoveTimedEvent);
    lua_register(L, "HasTimedEvents", &luaGlobalFunctions::HasTimedEvents);
    lua_register(L, "HasTimedEventInTable", &luaGlobalFunctions::HasTimedEventInTable);
    lua_register(L, "HasTimedEventWithName", &luaGlobalFunctions::HasTimedEventWithName);
    lua_register(L, "HasTimedEvent", &luaGlobalFunctions::HasTimedEvent);
    lua_register(L, "GetPlatform", &luaGlobalFunctions::GetPlatform);
    lua_register(L, "NumberToGUID", &luaGlobalFunctions::NumberToGUID);
    lua_register(L, "SendPacketToWorld", &luaGlobalFunctions::SendPacketToWorld);
    lua_register(L, "SendPacketToInstance", &luaGlobalFunctions::SendPacketToInstance);
    lua_register(L, "SendPacketToZone", &luaGlobalFunctions::SendPacketToZone);
    lua_register(L, "SendPacketToChannel", &luaGlobalFunctions::SendPacketToChannel);
    lua_register(L, "GetInstanceCreature", &luaGlobalFunctions::GetInstanceCreature);
    lua_register(L, "GetInstancePlayerCount", &luaGlobalFunctions::GetInstancePlayerCount);
    lua_register(L, "GetPlayersInInstance", &luaGlobalFunctions::GetPlayersInInstance);
    lua_register(L, "GetGuildByName", &luaGlobalFunctions::GetGuildByName);
    lua_register(L, "GetGuildByLeaderGuid", &luaGlobalFunctions::GetGuildByLeaderGuid);
}

#endif      // GLOBALFUNCTIONS_H
