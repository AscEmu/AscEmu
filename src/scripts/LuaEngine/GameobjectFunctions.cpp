/*
Copyright (c) 2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GameobjectFunctions.h"
#include "LuaMacros.h"
#include <Objects/GameObject.h>
#include <Map/MapMgr.h>
#include "LuaGlobal.h"
#include "LUAEngine.h"
#include <Map/MapScriptInterface.h>
#include <Units/Creatures/Creature.h>
#include <Storage/MySQLDataStore.hpp>
#include <Map/WorldCreatorDefines.hpp>
#include <Spell/SpellMgr.h>
#include <Server/MainServerDefines.h>
#include <StdAfx.h>
#include "LuaHelpers.h"
#include <Spell/Customization/SpellCustomizations.hpp>
#include <Map/WorldCreator.h>

extern "C"
{
    #include <lua/lauxlib.h>
}

int LuaGameObject::GossipCreateMenu(lua_State* L, GameObject* ptr)
{
    int text_id = static_cast<int>(luaL_checkinteger(L, 1));
    Player* target = CHECK_PLAYER(L, 2);
    int autosend = static_cast<int>(luaL_checkinteger(L, 3));
    if (!target || !ptr)
        return 0;

    if (LuaGlobal::instance()->m_menu != NULL)
        delete LuaGlobal::instance()->m_menu;

    LuaGlobal::instance()->m_menu = new Arcemu::Gossip::Menu(ptr->GetGUID(), text_id);

    if (autosend)
        LuaGlobal::instance()->m_menu->Send(target);

    return 0;
}

int LuaGameObject::GossipMenuAddItem(lua_State* L, GameObject* /*ptr*/)
{
    uint8 icon = static_cast<uint8>(luaL_checkinteger(L, 1));
    const char* menu_text = luaL_checkstring(L, 2);
    int IntId = static_cast<int>(luaL_checkinteger(L, 3));
    bool coded = (luaL_checkinteger(L, 4)) ? true : false;
    const char* boxmessage = luaL_optstring(L, 5, "");
    uint32_t boxmoney = static_cast<uint32_t>(luaL_optinteger(L, 6, 0));

    if (LuaGlobal::instance()->m_menu == NULL)
    {
        LOG_ERROR("There is no menu to add items to!");
        return 0;
    }

    LuaGlobal::instance()->m_menu->AddItem(icon, menu_text, IntId, boxmoney, boxmessage, coded);
    return 0;
}

int LuaGameObject::GossipSendMenu(lua_State* L, GameObject* /*ptr*/)
{
    Player* target = CHECK_PLAYER(L, 1);
    if (!target)
        return 0;

    if (LuaGlobal::instance()->m_menu == NULL)
    {
        LOG_ERROR("There is no menu to send!");
        return 0;
    }

    LuaGlobal::instance()->m_menu->Send(target);

    return 0;
}

int LuaGameObject::GossipComplete(lua_State* L, GameObject* /*ptr*/)
{
    Player* target = CHECK_PLAYER(L, 1);
    if (!target)
        return 0;

    if (LuaGlobal::instance()->m_menu == NULL)
    {
        LOG_ERROR("There is no menu to complete!");
        return 0;
    }

    LuaGlobal::instance()->m_menu->Complete(target);

    return 0;
}

int LuaGameObject::GossipSendPOI(lua_State* L, GameObject* /*ptr*/)
{
    Player* plr = CHECK_PLAYER(L, 1);
    float x = CHECK_FLOAT(L, 2);
    float y = CHECK_FLOAT(L, 3);
    int icon = static_cast<int>(luaL_checkinteger(L, 4));
    int flags = static_cast<int>(luaL_checkinteger(L, 5));
    int data = static_cast<int>(luaL_checkinteger(L, 6));
    const char* name = luaL_checkstring(L, 7);
    if (!plr)
        return 0;

    plr->Gossip_SendPOI(x, y, icon, flags, data, name);
    return 0;
}

int LuaGameObject::GossipSendQuickMenu(lua_State* L, GameObject* ptr)
{
    TEST_GO()

    uint32_t text_id = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    Player* player = CHECK_PLAYER(L, 2);
    uint32_t itemid = static_cast<uint32_t>(luaL_checkinteger(L, 3));
    uint8 itemicon = CHECK_UINT8(L, 4);
    const char* itemtext = luaL_checkstring(L, 5);
    uint32_t requiredmoney = CHECK_ULONG(L, 6);
    const char* moneytext = luaL_checkstring(L, 7);
    uint8 extra = CHECK_UINT8(L, 8);

    if (player == NULL)
        return 0;

    Arcemu::Gossip::Menu::SendQuickMenu(ptr->GetGUID(), text_id, player, itemid, itemicon, itemtext, requiredmoney, moneytext, extra);

    return 0;
}

int LuaGameObject::RegisterAIUpdate(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t time = CHECK_ULONG(L, 1);
    sEventMgr.AddEvent(ptr, &GameObject::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, time, 0, 0);
    return 0;
}

int LuaGameObject::ModAIUpdate(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t newtime = CHECK_ULONG(L, 1);
    sEventMgr.ModifyEventTimeAndTimeLeft(ptr, EVENT_SCRIPT_UPDATE_EVENT, newtime);
    return 0;
}

int LuaGameObject::RemoveAIUpdate(lua_State* /*L*/, GameObject* ptr)
{
    TEST_GO()
    sEventMgr.RemoveEvents(ptr, EVENT_SCRIPT_UPDATE_EVENT);
    return 0;
}

int LuaGameObject::GetMapId(lua_State* L, GameObject* ptr)
{
    (ptr != NULL) ? lua_pushinteger(L, ptr->GetMapId()) : lua_pushnil(L);
    return 1;
}

int LuaGameObject::RemoveFromWorld(lua_State* /*L*/, GameObject* ptr)
{
    if (ptr)
        ptr->RemoveFromWorld(true);
    return 0;
}

int LuaGameObject::GetName(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    if (!ptr->GetGameObjectProperties())
        return 0;
    lua_pushstring(L, ptr->GetGameObjectProperties()->name.c_str());
    return 1;
}

int LuaGameObject::GetCreatureNearestCoords(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    float x = CHECK_FLOAT(L, 1);
    float y = CHECK_FLOAT(L, 2);
    float z = CHECK_FLOAT(L, 3);
    uint32_t entryid = CHECK_ULONG(L, 4);
    Creature* crc = ptr->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(x, y, z, entryid);
    if (crc && crc->IsUnit())
    {
        PUSH_UNIT(L, crc);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

int LuaGameObject::GetGameObjectNearestCoords(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    float x = CHECK_FLOAT(L, 1);
    float y = CHECK_FLOAT(L, 2);
    float z = CHECK_FLOAT(L, 3);
    uint32_t entryid = CHECK_ULONG(L, 4);
    GameObject* go = ptr->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(x, y, z, entryid);
    if (go != NULL)
    PUSH_GO(L, go);
    else
        lua_pushnil(L);
    return 1;
}

int LuaGameObject::GetClosestPlayer(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    float d2 = 0;
    float dist = 0;
    Player* ret = NULL;

    for (std::set<Object*>::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
    {
        d2 = (static_cast<Player*>(*itr))->getDistanceSq(ptr);
        if (!ret || d2 < dist)
        {
            dist = d2;
            ret = static_cast<Player*>(*itr);
        }
    }
    if (ret == NULL)
        lua_pushnil(L);
    else
    PUSH_UNIT(L, ret);
    return 1;
}

int LuaGameObject::GetDistance(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    Object* target = CHECK_OBJECT(L, 1);
    lua_pushnumber(L, ptr->GetDistance2dSq(target));
    return 1;
}

int LuaGameObject::IsInWorld(lua_State* L, GameObject* ptr)
{
    if (ptr)
    {
        if (ptr->IsInWorld())
        {
            lua_pushboolean(L, 1);
            return 1;
        }
    }
    lua_pushboolean(L, 0);
    return 1;
}

int LuaGameObject::GetZoneId(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    lua_pushinteger(L, ptr->GetZoneId());
    return 1;
}

int LuaGameObject::PlaySoundToSet(lua_State* L, GameObject* ptr)
{
    if (!ptr)
        return 0;
    int soundid = static_cast<int>(luaL_checkinteger(L, 1));
    ptr->PlaySoundToSet(soundid);
    return 0;
}

int LuaGameObject::SpawnCreature(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t entry = CHECK_ULONG(L, 1);
    float x = CHECK_FLOAT(L, 2);
    float y = CHECK_FLOAT(L, 3);
    float z = CHECK_FLOAT(L, 4);
    float o = CHECK_FLOAT(L, 5);
    uint32_t faction = CHECK_ULONG(L, 6);
    uint32_t duration = CHECK_ULONG(L, 7);
    uint32_t equip1 = static_cast<uint32_t>(luaL_optinteger(L, 8, 1));
    uint32_t equip2 = static_cast<uint32_t>(luaL_optinteger(L, 9, 1));
    uint32_t equip3 = static_cast<uint32_t>(luaL_optinteger(L, 10, 1));
    uint32_t phase = static_cast<uint32_t>(luaL_optinteger(L, 11, ptr->m_phase));
    bool save = (luaL_optinteger(L, 12, 0) ? true : false);

    if (!entry)
    {
        lua_pushnil(L);
        return 1;
    }
    CreatureProperties const* p = sMySQLStore.getCreatureProperties(entry);
    if (p == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }
    Creature* pCreature = ptr->GetMapMgr()->CreateCreature(entry);
    if (pCreature == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }
    pCreature->Load(p, x, y, z, o);
    pCreature->m_loadedFromDB = true;
    pCreature->SetFaction(faction);
    pCreature->SetEquippedItem(MELEE, equip1);
    pCreature->SetEquippedItem(OFFHAND, equip2);
    pCreature->SetEquippedItem(RANGED, equip3);
    pCreature->Phase(PHASE_SET, phase);
    pCreature->m_noRespawn = true;
    pCreature->PushToWorld(ptr->GetMapMgr());
    if (duration)
        pCreature->Despawn(duration, 0);
    if (save)
        pCreature->SaveToDB();
    PUSH_UNIT(L, pCreature);
    return 1;
}

int LuaGameObject::SpawnGameObject(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    uint32_t entry_id = CHECK_ULONG(L, 1);
    float x = CHECK_FLOAT(L, 2);
    float y = CHECK_FLOAT(L, 3);
    float z = CHECK_FLOAT(L, 4);
    float o = CHECK_FLOAT(L, 5);
    uint32_t duration = CHECK_ULONG(L, 6);
    float scale = (float)(luaL_optinteger(L, 7, 100) / 100.0f);
    uint32_t phase = static_cast<uint32_t>(luaL_optinteger(L, 8, ptr->m_phase));
    bool save = luaL_optinteger(L, 9, 0) ? true : false;
    if (!entry_id)
        return 0;

    GameObject* go = ptr->GetMapMgr()->CreateGameObject(entry_id);
    uint32_t mapid = ptr->GetMapId();
    go->CreateFromProto(entry_id, mapid, x, y, z, o);
    go->Phase(PHASE_SET, phase);
    go->SetScale(scale);
    go->AddToWorld(ptr->GetMapMgr());

    if (duration)
    sEventMgr.AddEvent(go, &GameObject::ExpireAndDelete, EVENT_GAMEOBJECT_UPDATE, duration, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    if (save)
        go->SaveToDB();
    PUSH_GO(L, go);
    return 1;
}

int LuaGameObject::GetSpawnX(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetSpawnX());
    return 1;
}

int LuaGameObject::GetSpawnY(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetSpawnY());
    return 1;
}

int LuaGameObject::GetSpawnZ(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetSpawnZ());
    return 1;
}

int LuaGameObject::GetSpawnO(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetSpawnO());
    return 1;
}

int LuaGameObject::GetX(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetPositionX());
    return 1;
}

int LuaGameObject::GetY(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetPositionY());
    return 1;
}

int LuaGameObject::GetZ(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    if (ptr)
        lua_pushnumber(L, ptr->GetPositionZ());
    return 1;
}

int LuaGameObject::GetO(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetOrientation());
    return 1;
}

int LuaGameObject::GetInRangePlayersCount(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, static_cast<lua_Number>(ptr->GetInRangePlayersCount()));
    return 1;
}

int LuaGameObject::GetEntry(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetEntry());
    return 1;
}

int LuaGameObject::SetOrientation(lua_State* L, GameObject* ptr)
{
    float newo = CHECK_FLOAT(L, 1);
    if (!newo | !ptr)
        return 0;
    ptr->SetOrientation(newo);
    return 0;
}

int LuaGameObject::CalcRadAngle(lua_State* L, GameObject* ptr)
{
    float x = CHECK_FLOAT(L, 1);
    float y = CHECK_FLOAT(L, 2);
    float x2 = CHECK_FLOAT(L, 3);
    float y2 = CHECK_FLOAT(L, 4);
    if (!x || !y || !x2 || !y2 || !ptr)
        return 0;
    lua_pushnumber(L, ptr->calcRadAngle(x, y, x2, y2));
    return 1;
}

int LuaGameObject::GetInstanceID(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    if (ptr->GetMapMgr()->GetMapInfo()->type == INSTANCE_NULL)
        lua_pushnil(L);
    else
        lua_pushinteger(L, ptr->GetInstanceID());
    return 1;
}

int LuaGameObject::GetInRangePlayers(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    uint32_t count = 0;
    lua_newtable(L);
    for (std::set<Object*>::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
    {
        if ((*itr)->IsUnit())
        {
            count++ ,
                    lua_pushinteger(L, count);
            PUSH_UNIT(L, *itr);
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaGameObject::GetInRangeGameObjects(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    uint32_t count = 0;
    lua_newtable(L);
    for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
    {
        if ((*itr)->IsGameObject())
        {
            count++ ,
                    lua_pushinteger(L, count);
            PUSH_GO(L, *itr);
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaGameObject::GetInRangeUnits(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    uint32_t count = 0;
    lua_newtable(L);
    for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
    {
        if ((*itr)->IsUnit())
        {
            count++ ,
                    lua_pushinteger(L, count);
            PUSH_UNIT(L, *itr);
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaGameObject::IsInFront(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    Object* target = CHECK_OBJECT(L, 1);
    if (!target)
    {
        lua_pushnil(L);
        return 1;
    }
    if (ptr->isInFront(target))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaGameObject::IsInBack(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    Object* target = CHECK_OBJECT(L, 1);
    if (!target)
    {
        lua_pushnil(L);
        return 1;
    }
    if (ptr->isInBack(target))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaGameObject::GetUInt32Value(lua_State* L, GameObject* ptr)
{
    uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    if (ptr && field > 0)
        lua_pushinteger(L, ptr->getUInt32Value(field));
    else
        lua_pushnil(L);
    return 1;
}

int LuaGameObject::GetUInt64Value(lua_State* L, GameObject* ptr)
{
    uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    if (ptr && field)
    PUSH_GUID(L, ptr->getUInt64Value(field));
    else
        lua_pushnil(L);
    return 1;
}

int LuaGameObject::SetUInt32Value(lua_State* L, GameObject* ptr)
{
    uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (ptr && field)
        ptr->setUInt32Value(field, value);
    return 0;
}

int LuaGameObject::SetUInt64Value(lua_State* L, GameObject* ptr)
{
    uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint64 guid = CHECK_GUID(L, 1);
    if (ptr && field)
        ptr->setUInt64Value(field, guid);
    return 0;
}

int LuaGameObject::SetFloatValue(lua_State* L, GameObject* ptr)
{
    uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    float value = CHECK_FLOAT(L, 2);
    if (ptr)
        ptr->setFloatValue(field, value);
    return 0;
}

int LuaGameObject::RemoveFlag(lua_State* L, GameObject* ptr)
{
    uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (ptr)
        ptr->RemoveFlag(field, value);
    return 0;
}

int LuaGameObject::SetFlag(lua_State* L, GameObject* ptr)
{
    uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (ptr)
        ptr->SetFlag(field, value);
    return 0;
}

int LuaGameObject::Update(lua_State* /*L*/, GameObject* ptr)
{
    //just despawns/respawns to update GO visuals
    //credits: Sadikum
    TEST_GO()
    MapMgr* mapmgr = ptr->GetMapMgr();
    uint32_t NewGuid = mapmgr->GenerateGameobjectGuid();
    ptr->RemoveFromWorld(true);
    ptr->SetNewGuid(NewGuid);
    ptr->PushToWorld(mapmgr);
    ptr->SaveToDB();
    return 0;
}

int LuaGameObject::GetFloatValue(lua_State* L, GameObject* ptr)
{
    uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    if (ptr && field)
        lua_pushnumber(L, ptr->getFloatValue(field));
    else
        lua_pushnil(L);
    return 1;
}

int LuaGameObject::ModUInt32Value(lua_State* L, GameObject* ptr)
{
    int field = static_cast<int>(luaL_checkinteger(L, 1));
    int value = static_cast<int>(luaL_checkinteger(L, 2));
    if (ptr && field)
        ptr->ModSignedInt32Value(field, value);
    return 0;
}

int LuaGameObject::CastSpell(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t sp = CHECK_ULONG(L, 1);
    if (sp)
    {
        Spell* tSpell = sSpellFactoryMgr.NewSpell(ptr, sSpellCustomizations.GetSpellInfo(sp), true, NULL);
        SpellCastTargets tar(ptr->GetGUID());
        tSpell->prepare(&tar);
    }
    return 0;
}

int LuaGameObject::CastSpellOnTarget(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t sp = CHECK_ULONG(L, 1);
    Object* target = CHECK_OBJECT(L, 2);
    if (sp && target != NULL)
    {
        Spell* tSpell = sSpellFactoryMgr.NewSpell(ptr, sSpellCustomizations.GetSpellInfo(sp), true, NULL);
        SpellCastTargets spCastTargets(target->GetGUID());
        tSpell->prepare(&spCastTargets);
    }
    return 0;
}

int LuaGameObject::GetLandHeight(lua_State* L, GameObject* ptr)
{
    float x = CHECK_FLOAT(L, 1);
    float y = CHECK_FLOAT(L, 2);
    if (!ptr || !x || !y)
        lua_pushnil(L);
    else
    {
        float lH = ptr->GetMapMgr()->GetADTLandHeight(x, y);
        lua_pushnumber(L, lH);
    }
    return 1;
}

int LuaGameObject::SetZoneWeather(lua_State* L, GameObject* /*ptr*/)
{
    /*
    WEATHER_TYPE_NORMAL            = 0, // NORMAL (SUNNY)
    WEATHER_TYPE_FOG               = 1, // FOG
    WEATHER_TYPE_RAIN              = 2, // RAIN
    WEATHER_TYPE_HEAVY_RAIN        = 4, // HEAVY_RAIN
    WEATHER_TYPE_SNOW              = 8, // SNOW
    WEATHER_TYPE_SANDSTORM         = 16 // SANDSTORM
    */
    uint32_t zone_id = CHECK_ULONG(L, 1);
    uint32_t type = CHECK_ULONG(L, 2);
    float Density = CHECK_FLOAT(L, 3); //min: 0.30 max: 2.00
    if (Density < 0.30f || Density > 2.0f || !zone_id)
        return 0;

    uint32_t sound;
    if (Density <= 0.30f)
        sound = 0;

    switch (type)
    {
        case 0: //sunny
        case 1: //fog
            Density = 0;
            sound = 0;
            break;
        case 2: //rain
        case 4: //heavy rain
            if (Density < 0.40f)
                sound = 8533;
            else if (Density < 0.70f)
                sound = 8534;
            else
                sound = 8535;
            break;
        case 8: //snow
            if (Density < 0.40f)
                sound = 8536;
            else if (Density < 0.70f)
                sound = 8537;
            else
                sound = 8538;
            break;
        case 16: //storm
            if (Density < 0.40f)
                sound = 8556;
            else if (Density < 0.70f)
                sound = 8557;
            else
                sound = 8558;
            break;
        default: //no valid type
            return 0;
    }
    WorldPacket data(SMSG_WEATHER, 9);
    data.Initialize(SMSG_WEATHER);
    data << type;
    data << Density;
    data << sound;
    data << uint8(0);

    sWorld.sendZoneMessage(&data, zone_id);

    return 0;
}

int LuaGameObject::GetDistanceYards(lua_State* L, GameObject* ptr)
{
    Object* target = CHECK_OBJECT(L, 1);
    if (!ptr || !target)
        lua_pushnil(L);
    else
    {
        LocationVector vec = ptr->GetPosition();
        lua_pushnumber(L, (float)vec.Distance(target->GetPosition()));
    }
    return 1;
}

int LuaGameObject::PhaseSet(lua_State* L, GameObject* ptr)
{
    uint32_t newphase = CHECK_ULONG(L, 1);
    bool Save = (luaL_optinteger(L, 2, false) > 0 ? true : false);
    if (!ptr)
        return 0;
    ptr->Phase(PHASE_SET, newphase);
    if (ptr->m_spawn)
        ptr->m_spawn->phase = newphase;
    if (Save)
    {
        ptr->SaveToDB();
        ptr->m_loadedFromDB = true;
    }
    return 0;
}

int LuaGameObject::PhaseAdd(lua_State* L, GameObject* ptr)
{
    uint32_t newphase = CHECK_ULONG(L, 1);
    bool Save = (luaL_optinteger(L, 2, false) > 0 ? true : false);
    if (!ptr)
        return 0;
    ptr->Phase(PHASE_ADD, newphase);
    if (ptr->m_spawn)
        ptr->m_spawn->phase |= newphase;
    if (Save)
    {
        ptr->SaveToDB();
        ptr->m_loadedFromDB = true;
    }
    return 0;
}

int LuaGameObject::PhaseDelete(lua_State* L, GameObject* ptr)
{
    uint32_t newphase = CHECK_ULONG(L, 1);
    bool Save = (luaL_optinteger(L, 2, false) > 0 ? true : false);
    if (!ptr)
        return 0;
    ptr->Phase(PHASE_DEL, newphase);
    if (ptr->m_spawn)
        ptr->m_spawn->phase &= ~newphase;
    if (Save)
    {
        ptr->SaveToDB();
        ptr->m_loadedFromDB = true;
    }
    return 0;
}

int LuaGameObject::GetPhase(lua_State* L, GameObject* ptr)
{
    if (ptr)
        lua_pushnumber(L, ptr->m_phase);
    else
        lua_pushnil(L);
    return 1;
}

int LuaGameObject::SendPacket(lua_State* L, GameObject* ptr)
{
    WorldPacket* data = CHECK_PACKET(L, 1);
    bool self = CHECK_BOOL(L, 2);
    if (ptr != NULL && data != NULL)
        ptr->SendMessageToSet(data, self);
    return 0;
}

int LuaGameObject::GetGUID(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    PUSH_GUID(L, ptr->GetGUID());
    return 1;
}

int LuaGameObject::IsActive(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    if (ptr->GetState())
    RET_BOOL(true)
    RET_BOOL(false)
}

int LuaGameObject::Activate(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    if (ptr->GetState() == 1)
        ptr->SetState(GO_STATE_OPEN);
    else
        ptr->SetState(GO_STATE_CLOSED);
    ptr->SetFlags((ptr->GetFlags() & ~1));
    RET_BOOL(true)
}

int LuaGameObject::DespawnObject(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    int delay = static_cast<int>(luaL_checkinteger(L, 1));
    int respawntime = static_cast<int>(luaL_checkinteger(L, 2));
    if (!delay)
        delay = 1; //Delay 0 might cause bugs
    ptr->Despawn(delay, respawntime);
    return 0;
}

int LuaGameObject::AddLoot(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    if ((lua_gettop(L) != 3) || (lua_gettop(L) != 5))
        return 0;

    if (!ptr->IsLootable())
        return 0;
    GameObject_Lootable* lt = static_cast<GameObject_Lootable*>(ptr);

    uint32_t itemid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint32_t mincount = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    uint32_t maxcount = static_cast<uint32_t>(luaL_checkinteger(L, 3));
    bool perm = ((luaL_optinteger(L, 4, 0) == 1) ? true : false);
    if (perm)
    {
        float chance = CHECK_FLOAT(L, 5);
        QueryResult* result = WorldDatabase.Query("SELECT * FROM loot_gameobjects WHERE entryid = %u, itemid = %u", ptr->GetEntry(), itemid);
        if (!result)
        WorldDatabase.Execute("REPLACE INTO loot_gameobjects VALUES (%u, %u, %f, 0, 0, 0, %u, %u )", ptr->GetEntry(), itemid, chance, mincount, maxcount);
        delete result;
    }
    lootmgr.AddLoot(&lt->loot, itemid, mincount, maxcount);
    return 0;
}

int LuaGameObject::GetInstanceOwner(lua_State* L, GameObject* ptr)
{
    MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(ptr->GetMapId());
    if (pMapinfo) //this block = IsInInstace()
    {
        if (pMapinfo->type != INSTANCE_NULL)
        {
            lua_pushboolean(L, 0);
            return 1;
        }
    }
    Instance* pInstance = sInstanceMgr.GetInstanceByIds(ptr->GetMapId(), ptr->GetInstanceID());
    if (pInstance == nullptr)
        return 0;

    if (pInstance->m_creatorGuid != 0) // creator guid is 0 if its owned by a group.
    {
        Player* owner = pInstance->m_mapMgr->GetPlayer(pInstance->m_creatorGuid);
        PUSH_UNIT(L, owner);
    }
    else
    {
        uint32_t group_id = pInstance->m_creatorGroup;
        if (group_id == 0)
        {
            LOG_ERROR("Instance is not not owned by a group or a guid!");
            return 0;
        }

        auto get_group_id = objmgr.GetGroupById(group_id);
        if (get_group_id == nullptr)
            return 0;

        auto group_leader = get_group_id->GetLeader();
        if (group_leader == nullptr)
            return 0;

        auto group_leader_online = group_leader->m_loggedInPlayer;
        if (group_leader_online == nullptr)
            return 0;

        PUSH_UNIT(L, group_leader_online);
    }
    return 1;
}

int LuaGameObject::GetDungeonDifficulty(lua_State* L, GameObject* ptr)
{
    MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(ptr->GetMapId());
    if (pMapinfo) //this block = IsInInstace()
    {
        if (pMapinfo->type != INSTANCE_NULL)
        {
            lua_pushboolean(L, 0);
            return 1;
        }
    }
    Instance* pInstance = sInstanceMgr.GetInstanceByIds(ptr->GetMapId(), ptr->GetInstanceID());
    if (pInstance != nullptr)
    {
        lua_pushnumber(L, pInstance->m_difficulty);
        return 1;
    }
    return 0;
}

int LuaGameObject::SetDungeonDifficulty(lua_State* L, GameObject* ptr)
{
    uint8 difficulty = static_cast<uint8>(luaL_checkinteger(L, 1));
    MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(ptr->GetMapId());
    if (pMapinfo) //this block = IsInInstace()
    {
        if (pMapinfo->type != INSTANCE_NULL)
        {
            lua_pushboolean(L, 0);
            return 1;
        }
    }
    Instance* pInstance = sInstanceMgr.GetInstanceByIds(ptr->GetMapId(), ptr->GetInstanceID());
    if (pInstance != nullptr)
    {
        pInstance->m_difficulty = difficulty;
        lua_pushboolean(L, 1);
        return 1;
    }
    return 0;
}

int LuaGameObject::HasFlag(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t flag = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    lua_pushboolean(L, ptr->HasFlag(index, flag) ? 1 : 0);
    return 1;
}

int LuaGameObject::IsInPhase(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    uint32_t phase = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    lua_pushboolean(L, ((ptr->m_phase & phase) != 0) ? 1 : 0);
    return 1;
}

int LuaGameObject::GetSpawnId(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->m_spawn != NULL ? ptr->m_spawn->id : 0);
    return 1;
}

int LuaGameObject::GetAreaId(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET()

    auto area = ptr->GetArea();
    lua_pushnumber(L, area ? area->id : 0);
    return 1;
}

int LuaGameObject::SetPosition(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    MapMgr* mapMgr = ptr->GetMapMgr();
    uint32_t NewGuid = mapMgr->GenerateGameobjectGuid();
    ptr->RemoveFromWorld(true);
    ptr->SetNewGuid(NewGuid);
    float x = CHECK_FLOAT(L, 1);
    float y = CHECK_FLOAT(L, 2);
    float z = CHECK_FLOAT(L, 3);
    float o = CHECK_FLOAT(L, 4);
    bool save = luaL_optinteger(L, 5, 0) ? true : false;
    ptr->SetPosition(x, y, z, o);
    ptr->PushToWorld(mapMgr);
    if (save)
        ptr->SaveToDB();
    RET_BOOL(true)
}

int LuaGameObject::GetObjectType(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    RET_STRING("GameObject");
}

int LuaGameObject::ChangeScale(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    float nScale = CHECK_FLOAT(L, 1);
    bool updateNow = CHECK_BOOL(L, 2);
    nScale = (nScale <= 0) ? 1 : nScale;
    ptr->SetScale(nScale);
    if (updateNow)
    {
        MapMgr* mapMgr = ptr->GetMapMgr();
        uint32_t nguid = mapMgr->GenerateGameobjectGuid();
        ptr->RemoveFromWorld(true);
        ptr->SetNewGuid(nguid);
        ptr->PushToWorld(mapMgr);
    }
    RET_BOOL(true)
}

int LuaGameObject::GetByte(lua_State* L, GameObject* ptr)
{
    TEST_GO()
        uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint8_t index2 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    uint8 value = ptr->getByteValue(index, index2);
    RET_INT(value);
}

int LuaGameObject::SetByte(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint8_t index2 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    uint8_t value = static_cast<uint8_t>(luaL_checkinteger(L, 3));
    ptr->setByteValue(index, index2, value);
    RET_BOOL(true)
}

int LuaGameObject::FullCastSpellOnTarget(lua_State* L, GameObject* ptr)
{
    TEST_GO()
        uint32_t sp = CHECK_ULONG(L, 1);
    Object* target = CHECK_OBJECT(L, 2);
    if (sp && target != NULL)
    {
        Spell* tSpell = sSpellFactoryMgr.NewSpell(ptr, sSpellCustomizations.GetSpellInfo(sp), false, NULL);
        SpellCastTargets sct(target->GetGUID());
        tSpell->prepare(&sct);
    }
    return 0;
}

int LuaGameObject::FullCastSpell(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t sp = CHECK_ULONG(L, 1);
    if (sp)
    {
        Spell* tSpell = sSpellFactoryMgr.NewSpell(ptr, sSpellCustomizations.GetSpellInfo(sp), false, NULL);
        SpellCastTargets sct(ptr->GetGUID());
        tSpell->prepare(&sct);
    }
    return 0;
}

int LuaGameObject::CustomAnimate(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t aindex = CHECK_ULONG(L, 1);
    if (aindex < 2 && ptr != NULL)
    {
        ptr->SetCustomAnim(aindex);
        RET_BOOL(true)
    }
    RET_BOOL(false)
}

int LuaGameObject::GetLocation(lua_State* L, GameObject* ptr)
{
    TEST_GO()

    lua_pushnumber(L, ptr->GetPositionX());
    lua_pushnumber(L, ptr->GetPositionY());
    lua_pushnumber(L, ptr->GetPositionZ());
    lua_pushnumber(L, ptr->GetOrientation());
    return 4;
}

int LuaGameObject::GetSpawnLocation(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    lua_pushnumber(L, ptr->GetPositionX());
    lua_pushnumber(L, ptr->GetPositionY());
    lua_pushnumber(L, ptr->GetPositionZ());
    lua_pushnumber(L, ptr->GetOrientation());
    return 4;
}

int LuaGameObject::GetWoWObject(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    uint64 guid = CHECK_GUID(L, 1);
    Object* obj = ptr->GetMapMgr()->_GetObject(guid);
    if (obj != NULL && obj->IsUnit())
    PUSH_UNIT(L, obj);
    else if (obj != NULL && obj->IsGameObject())
    PUSH_GO(L, obj);
    else
        lua_pushnil(L);
    return 1;
}

int LuaGameObject::RegisterEvent(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    const char* typeName = luaL_typename(L, 1);
    int delay = static_cast<int>(luaL_checkinteger(L, 2));
    int repeats = static_cast<int>(luaL_checkinteger(L, 3));
    if (!delay)
        return 0;

    lua_settop(L, 1);
    int functionRef = 0;
    if (!strcmp(typeName, "function"))
        functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    else if (!strcmp(typeName, "string"))
        functionRef = LuaHelpers::ExtractfRefFromCString(L, luaL_checkstring(L, 1));
    if (functionRef)
    {
        TimedEvent* ev = TimedEvent::Allocate(ptr, new CallbackP1<LuaEngine, int>(LuaGlobal::instance()->luaEngine().get(), &LuaEngine::CallFunctionByReference, functionRef), EVENT_LUA_GAMEOBJ_EVENTS, delay, repeats);
        ptr->event_AddEvent(ev);
        std::map<uint64, std::set<int>>& objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
        std::map<uint64, std::set<int>>::iterator itr = objRefs.find(ptr->GetGUID());
        if (itr == objRefs.end())
        {
            std::set<int> refs;
            refs.insert(functionRef);
            objRefs.insert(make_pair(ptr->GetGUID(), refs));
        }
        else
        {
            std::set<int>& refs = itr->second;
            refs.insert(functionRef);
        }
    }
    return 0;
}

int LuaGameObject::RemoveEvents(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    sEventMgr.RemoveEvents(ptr, EVENT_LUA_GAMEOBJ_EVENTS);
    std::map<uint64, std::set<int>>& objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
    std::map<uint64, std::set<int>>::iterator itr = objRefs.find(ptr->GetGUID());
    if (itr != objRefs.end())
    {
        std::set<int>& refs = itr->second;
        for (std::set<int>::iterator it = refs.begin(); it != refs.end(); ++it)
            luaL_unref(L, LUA_REGISTRYINDEX, (*it));
        refs.clear();
    }
    return 0;
}

int LuaGameObject::SetScale(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    float scale = static_cast<float>(luaL_checknumber(L, 1));
    if (scale > 0)
        ptr->SetScale(scale);
    return 0;
}

int LuaGameObject::GetScale(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    lua_pushnumber(L, ptr->GetScale());
    return 1;
}

int LuaGameObject::GetClosestUnit(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    float closest_dist = 99999.99f;
    float current_dist = 0;
    Object* closest_unit = NULL;
    Unit* ret = NULL;
    for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
    {
        closest_unit = (*itr);
        if (!closest_unit->IsUnit())
            continue;
        current_dist = ptr->GetDistance2dSq(closest_unit);
        if (current_dist < closest_dist)
        {
            closest_dist = current_dist;
            ret = static_cast<Unit*>(closest_unit);
        }
    }
    PUSH_UNIT(L, ret);
    return 1;
}

int LuaGameObject::Damage(lua_State* L, GameObject* ptr)
{
    TEST_GO();

    if (ptr->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return 0;

    if (lua_gettop(L) != 3)
        return 0;

    GameObject_Destructible* dt = static_cast<GameObject_Destructible*>(ptr);
    uint32_t damage = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint64 guid = CHECK_GUID(L, 2);
    uint32_t spellid = static_cast<uint32_t>(luaL_checkinteger(L, 3));

    dt->Damage(damage, guid, guid, spellid);

    return 0;
}

int LuaGameObject::Rebuild(lua_State* /*L*/, GameObject* ptr)
{
    TEST_GO();

    if (ptr->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return 0;

    GameObject_Destructible* dt = static_cast<GameObject_Destructible*>(ptr);
    dt->Rebuild();

    return 1;
}

int LuaGameObject::GetHP(lua_State* L, GameObject* ptr)
{
    TEST_GO();

    if (ptr->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return 0;

    GameObject_Destructible* dt = static_cast<GameObject_Destructible*>(ptr);

    lua_pushinteger(L, dt->GetHP());

    return 1;
}

int LuaGameObject::GetMaxHP(lua_State* L, GameObject* ptr)
{
    TEST_GO();

    if (ptr->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return 0;

    GameObject_Destructible* dt = static_cast<GameObject_Destructible*>(ptr);

    lua_pushinteger(L, dt->GetMaxHP());

    return 1;
}

int LuaGameObject::GetWorldStateForZone(lua_State* L, GameObject* ptr)
{
    TEST_GO();

    if (lua_gettop(L) != 1)
        return 0;

    uint32_t field = static_cast<uint32_t>(luaL_checkinteger(L, 1));

    auto a = ptr->GetMapMgr()->GetArea(ptr->GetPositionX(), ptr->GetPositionY(), ptr->GetPositionZ());
    if (a == NULL)
        return 0;

    uint32_t zone = a->zone;

    if (zone == 0)
        zone = a->id;

    if (zone == 0)
        return 0;

    uint32_t value
            = ptr->GetMapMgr()->GetWorldStatesHandler().GetWorldStateForZone(zone, 0, field);

    lua_pushinteger(L, value);

    return 1;
}

int LuaGameObject::SetWorldStateForZone(lua_State* L, GameObject* ptr)
{
    TEST_GO();

    if (lua_gettop(L) != 2)
        return 0;

    uint32_t field = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));

    auto a = ptr->GetMapMgr()->GetArea(ptr->GetPositionX(), ptr->GetPositionY(), ptr->GetPositionZ());
    if (a == NULL)
        return 0;

    uint32_t zone = a->zone;

    if (zone == 0)
        zone = a->id;

    if (zone == 0)
        return 0;

    ptr->GetMapMgr()->GetWorldStatesHandler().SetWorldStateForZone(zone, 0, field, value);

    return 0;
}
