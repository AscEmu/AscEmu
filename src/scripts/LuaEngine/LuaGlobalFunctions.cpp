/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LuaGlobalFunctions.hpp"

#include "Common.hpp"
#include "git_version.hpp"
#include "LUAEngine.hpp"
#include "LuaGlobal.hpp"
#include "LuaMacros.h"
#include "LuaSpell.hpp"
#include "Chat/Channel.hpp"
#include "Chat/ChannelMgr.hpp"
#include "Chat/CommandTableStorage.hpp"
#include "Logging/Log.hpp"
#include "Management/MailMgr.h"
#include "Management/ObjectMgr.hpp"
#include "Management/Guild/GuildMgr.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/Strings.hpp"


int LuaGlobalFunctions::PerformIngameSpawn(lua_State* L)
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

            WorldMap* mapMgr = sMapMgr.findWorldMap(map);
            if (!mapMgr)
                return 0;

            //int32_t instanceid = static_cast<int32_t>(luaL_optinteger(L, 13, mapMgr->GetInstanceID()));
            Creature* pCreature = mapMgr->createCreature(entry);
            pCreature->Load(p, x, y, z, o);
            pCreature->setFaction(faction);
            pCreature->setVirtualItemSlotId(MELEE, equip1);
            pCreature->setVirtualItemSlotId(OFFHAND, equip2);
            pCreature->setVirtualItemSlotId(RANGED, equip3);
            pCreature->setPhase(PHASE_SET, 1);
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

            WorldMap* mapMgr = sMapMgr.findWorldMap(map);
            if (!mapMgr)
                return 0;

            GameObject* go = mapMgr->createGameObject(entry);
            go->create(entry, mapMgr, 0, LocationVector(x, y, z, o), QuaternionData(), GO_STATE_CLOSED);
            go->Phase(PHASE_SET, 1);
            go->setScale(((float)faction) / 100.0f);

            go->AddToWorld(mapMgr);

            if (duration)
                go->despawn(duration, 0);
            if (save)
                go->saveToDB();
            PUSH_GO(L, go);
        }
        else
            lua_pushnil(L);
    }
    else
        lua_pushnil(L);
    return 1;
}

int LuaGlobalFunctions::GetGameTime(lua_State* L)
{
    lua_pushnumber(L, ((uint32_t)UNIXTIME)); //in seconds.
    return 1;
}

int LuaGlobalFunctions::GetPlayer(lua_State* L)
{
    const char* plName = luaL_checkstring(L, 1);
    Player* plr = sObjectMgr.getPlayer(plName);
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

int LuaGlobalFunctions::GetLUAEngine(lua_State* L)  //also mapped to GetLuaEngine()
{
    lua_pushstring(L, ENGINE_NAME);
    return 1;
}

int LuaGlobalFunctions::logcol(lua_State* L)
{
    luaL_checkinteger(L, 1);
    return 0;
}

int LuaGlobalFunctions::WorldDBQuery(lua_State* L)
{
    // TODO: possibly needs rewrite of LuaEngine to handle unique_ptr<T> -Appled

    /*const char* qStr = luaL_checkstring(L, 1);
    //uint32_t fID = static_cast<uint32_t>(luaL_optinteger(L, 2, 0)); //column
    //uint32_t rID = static_cast<uint32_t>(luaL_optinteger(L, 3, 0)); //row
    if (!qStr)
        return 0;
    auto result = WorldDatabase.Query(qStr);
    lua_settop(L, 0);
    PUSH_SQLRESULT(L, result);*/
    return 1;
}

int LuaGlobalFunctions::CharDBQuery(lua_State* L)
{
    // TODO: possibly needs rewrite of LuaEngine to handle unique_ptr<T> -Appled

    /*const char* qStr = luaL_checkstring(L, 1);
    //uint32_t fID = static_cast<uint32_t>(luaL_optinteger(L, 2, 0)); //column
    //uint32_t rID = static_cast<uint32_t>(luaL_optinteger(L, 3, 0)); //row
    if (!qStr)
        return 0;
    auto result = CharacterDatabase.Query(qStr);
    lua_settop(L, 0);
    PUSH_SQLRESULT(L, result);*/
    return 1;
}

int LuaGlobalFunctions::WorldDBQueryTable(lua_State* L)
{
    // TODO: possibly needs rewrite of LuaEngine to handle unique_ptr<T> -Appled

    /*const char* qStr = luaL_checkstring(L, 1);
    lua_newtable(L);
    if (!qStr) return 0;
    auto result = WorldDatabase.Query(qStr);
    PUSH_SQLRESULT(L, result);*/
    return 1;
}

int LuaGlobalFunctions::CharDBQueryTable(lua_State* L)
{
    // TODO: possibly needs rewrite of LuaEngine to handle unique_ptr<T> -Appled

    /*const char* qStr = luaL_checkstring(L, 1);
    lua_newtable(L);
    if (!qStr) return 0;
    auto result = CharacterDatabase.Query(qStr);
    PUSH_SQLRESULT(L, result);*/
    return 1;
}

int LuaGlobalFunctions::SendWorldMessage(lua_State* L)
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

int LuaGlobalFunctions::ReloadTable(lua_State* L)
{
    //const char* TableName = luaL_checkstring(L, 1);
    //if (AscEmu::Util::Strings::isEqual(TableName, "spell_disable"))
    //{
    //    sSpellMgr.reloadSpellDisabled();
    //}
    //else if (AscEmu::Util::Strings::isEqual(TableName, "vendors"))
    //{
    //    sObjectMgr.loadVendors();
    //}
    //else
    //{
    //    if (AscEmu::Util::Strings::isEqual(TableName, "command_overrides"))    // Command Overrides
    //    {
    //        sCommandTableStorage.loadOverridePermission();
    //    }
    //}
    return 0;
}

int LuaGlobalFunctions::ReloadLuaEngine(lua_State* /*L*/)
{
    /*g_luaMgr.Restart();
    MapMgr * mgr;
    LuaUnitBinding * m_binding;
    for (uint32_t i = 0; i < NUM_MAPS; ++i)
    {
    if (!sInstanceMgr.getWorldMap(i))
    continue;
    mgr = sInstanceMgr.getWorldMap(i);
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

int LuaGlobalFunctions::GetPlayersInWorld(lua_State* L)
{
    uint32_t count = 0;

    lua_newtable(L);
    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        Player* player = playerPair.second;
        count++;
        lua_pushinteger(L, count);
        PUSH_UNIT(L, (static_cast<Unit*>(player)));
        lua_rawset(L, -3);
    }
    sObjectMgr.m_playerLock.unlock();
    return 1;
}

int LuaGlobalFunctions::Rehash(lua_State* /*L*/)
{
    sWorld.loadWorldConfigValues(true);
    return 0;
}

int LuaGlobalFunctions::GetClientVersion(lua_State* L)
{
    lua_pushinteger(L, VERSION_STRING);
    return 1;
}

int LuaGlobalFunctions::GetAERevision(lua_State* L)
{
    lua_pushstring(L, AE_BUILD_HASH);
    return 1;
}

/*int LuaGlobalFunctions::GetInstanceIdsByMap(lua_State * L)
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
int LuaGlobalFunctions::SendPvPCaptureMessage(lua_State * L)
{
uint32_t zoneid = luaL_checkinteger(L, 1);
const char* msg = luaL_checkstring(L, 2);
AreaTable * at = dbcArea.LookupEntry(zoneid);
if(!zoneid || !msg || !at)
return 1;
MapMgr* mapmgr = sInstanceMgr.getWorldMap(at->mapId);
if (mapmgr)
mapmgr->SendPvPCaptureMessage(ZONE_MASK_ALL, zoneid, msg);
return 1;
}
*/
int LuaGlobalFunctions::GetPlayersInMap(lua_State* L)
{
    uint32_t count = 0;
    lua_newtable(L);
    uint32_t mapid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    WorldMap* mgr = sMapMgr.findWorldMap(mapid);
    if (!mgr)
        return 0;

    for (const auto& itr : mgr->getPlayers())
    {
        count++;
        Player* ret = itr.second;
        lua_pushinteger(L, count);
        PUSH_UNIT(L, (static_cast<Unit*>(ret)));
        lua_rawset(L, -3);
    }
    return 1;
}

int LuaGlobalFunctions::GetPlayersInZone(lua_State* L)
{
    uint32_t count = 0;
    lua_newtable(L);
    uint32_t zoneid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        Player* player = playerPair.second;
        if (player->getZoneId() == zoneid)
        {
            count++;
            lua_pushinteger(L, count);
            PUSH_UNIT(L, (static_cast<Unit*>(player)));
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaGlobalFunctions::SendMail(lua_State* L)
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

int LuaGlobalFunctions::SetDBCSpellVar(lua_State* L)
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
    {
        lua_pushnil(L);
        return 1;
    }
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

int LuaGlobalFunctions::GetDBCSpellVar(lua_State* L)
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
    {
        lua_pushnil(L);
        return 1;
    }
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

int LuaGlobalFunctions::bit_and(lua_State* L)
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
int LuaGlobalFunctions::bit_or(lua_State* L)
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
int LuaGlobalFunctions::bit_xor(lua_State* L)
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
/*int LuaGlobalFunctions::bit_not(lua_State* L)
{
    uint32_t left = CHECK_ULONG(L, 1);
    RET_NUMBER(~left)
}*/
int LuaGlobalFunctions::bit_shiftleft(lua_State* L)
{
    uint32_t left = CHECK_ULONG(L, 1);
    uint8_t count = luaL_checkinteger(L, 2) & 0x7F;
    RET_NUMBER(left << count)
}
int LuaGlobalFunctions::bit_shiftright(lua_State* L)
{
    uint32_t left = CHECK_ULONG(L, 1);
    uint8_t count = luaL_checkinteger(L, 2) & 0x7F;
    RET_NUMBER(left >> count)
}
int LuaGlobalFunctions::RemoveTimedEvents(lua_State* /*L*/)
{
    LuaGlobal::instance()->luaEngine()->LuaEventMgr.RemoveEvents();
    return 0;
}
int LuaGlobalFunctions::RemoveTimedEventsWithName(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    LuaGlobal::instance()->luaEngine()->LuaEventMgr.RemoveEventsByName(name);
    return 0;
}
int LuaGlobalFunctions::RemoveTimedEvent(lua_State* L)
{
    int ref = static_cast<int>(luaL_checkinteger(L, 1));
    LuaGlobal::instance()->luaEngine()->LuaEventMgr.RemoveEventByRef(ref);
    return 0;
}
int LuaGlobalFunctions::RemoveTimedEventsInTable(lua_State* L)
{
    const char* table = luaL_checkstring(L, 1);
    LuaGlobal::instance()->luaEngine()->LuaEventMgr.RemoveEventsInTable(table);
    return 0;
}
int LuaGlobalFunctions::HasTimedEvents(lua_State* L)
{
    lua_pushboolean(L, LuaGlobal::instance()->luaEngine()->LuaEventMgr.event_HasEvents() ? 1 : 0);
    return 1;
}
int LuaGlobalFunctions::HasTimedEvent(lua_State* L)
{
    int ref = static_cast<int>(luaL_checkinteger(L, 1));
    lua_pushboolean(L, LuaGlobal::instance()->luaEngine()->LuaEventMgr.HasEvent(ref) ? 1 : 0);
    return 1;
}
int LuaGlobalFunctions::HasTimedEventWithName(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    lua_pushboolean(L, LuaGlobal::instance()->luaEngine()->LuaEventMgr.HasEventWithName(name) ? 1 : 0);
    return 1;
}
int LuaGlobalFunctions::HasTimedEventInTable(lua_State* L)
{
    const char* table = luaL_checkstring(L, 1);
    lua_pushboolean(L, LuaGlobal::instance()->luaEngine()->LuaEventMgr.HasEventInTable(table) ? 1 : 0);
    return 1;
}
int LuaGlobalFunctions::GetPlatform(lua_State* L)
{
    lua_pushliteral(L, AE_PLATFORM);
    return 1;
}
int LuaGlobalFunctions::NumberToGUID(lua_State* L)
{
    uint64_t num = (uint64_t)luaL_checknumber(L, 1);
    PUSH_GUID(L, num);
    return 1;
}
int LuaGlobalFunctions::SendPacketToZone(lua_State* L)
{
    WorldPacket* data = CHECK_PACKET(L, 1);
    uint32_t zone_id = CHECK_ULONG(L, 2);
    if (data && zone_id)
        sWorld.sendZoneMessage(data, zone_id);
    return 0;
}

int LuaGlobalFunctions::SendPacketToInstance(lua_State* L)
{
    WorldPacket* data = CHECK_PACKET(L, 1);
    uint32_t instance_id = CHECK_ULONG(L, 2);
    if (data && instance_id)
        sWorld.sendInstanceMessage(data, instance_id);
    return 0;
}

int LuaGlobalFunctions::SendPacketToWorld(lua_State* L)
{
    WorldPacket* data = CHECK_PACKET(L, 1);
    if (data)
        sWorld.sendGlobalMessage(data);
    return 0;
}

int LuaGlobalFunctions::SendPacketToChannel(lua_State* L)
{
    WorldPacket* pack = CHECK_PACKET(L, 1);
    const char* channelName = luaL_checkstring(L, 2);
    uint32_t team = CHECK_ULONG(L, 3);
    auto channel = sChannelMgr.getChannel(channelName, team);
    if (!channel || !pack)
        return 0;

    channel->sendToAll(pack);

    return 1;
}

int LuaGlobalFunctions::GetInstanceCreature(lua_State* L)
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

    WorldMap* pInstance = sMapMgr.findWorldMap(map, iid);
    if (pInstance == NULL || (!guid && !spawnId))
    {
        lua_pushnil(L);
        return 1;
    }

    Creature* pCreature = NULL;
    if (guid)
    {
        WoWGuid wowGuid;
        wowGuid.Init(guid);
        pCreature = pInstance->getCreature(wowGuid.getGuidLowPart());
    }
    else
        pCreature = pInstance->getSqlIdCreature(spawnId);

    PUSH_UNIT(L, pCreature);
    return 1;
}

int LuaGlobalFunctions::GetInstancePlayerCount(lua_State* L)
{
    uint32_t map = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint32_t iid = static_cast<uint32_t>(luaL_checkinteger(L, 2));

    WorldMap* pInstance = sMapMgr.findWorldMap(map, iid);
    if (pInstance == NULL)
    {
        lua_pushnil(L);
        return 1;
    }

    lua_pushnumber(L, pInstance->getPlayerCount());
    return 1;
}

int LuaGlobalFunctions::GetPlayersInInstance(lua_State* L)
{
    uint32_t map = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint32_t iid = static_cast<uint32_t>(luaL_checkinteger(L, 2));

    WorldMap* pInstance = sMapMgr.findWorldMap(map, iid);
    if (pInstance == NULL)
    {
        lua_pushnil(L);
        return 1;
    }

    Player* ret = NULL;
    uint32_t count = 0;
    lua_newtable(L);

    for (const auto& itr : pInstance->getPlayers())
    {
        count++;
        ret = itr.second;
        lua_pushinteger(L, count);
        PUSH_UNIT(L, (static_cast<Unit*>(ret)));
        lua_rawset(L, -3);
    }
    return 1;

}

int LuaGlobalFunctions::GetGuildByName(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    Guild* guild = sGuildMgr.getGuildByName(name);
    lua_pushnumber(L, guild ? guild->getId() : -1);
    return 1;
}

int LuaGlobalFunctions::GetGuildByLeaderGuid(lua_State* L)
{
    uint64_t guid = CHECK_GUID(L, 1);
    Guild* guild = sGuildMgr.getGuildByLeader(guid);
    lua_pushnumber(L, guild ? guild->getId() : -1);
    return 1;
}
