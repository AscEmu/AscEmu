/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "LuaUnit.hpp"

#include "LUAEngine.hpp"
#include "LuaGlobal.hpp"
#include "LuaHelpers.hpp"
#include "LuaMacros.h"
#include "Chat/Channel.hpp"
#include "Chat/ChannelMgr.hpp"
#include "Management/Group.h"
#include "Management/ItemInterface.h"
#include "Management/Loot/LootMgr.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestLogEntry.hpp"
#include "Management/QuestMgr.h"
#include "Management/WeatherMgr.hpp"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Guild/GuildMgr.hpp"
#include "Map/AreaBoundary.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Container.hpp"
#include "Objects/GameObject.h"
#include "Objects/Item.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Vehicle.hpp"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Master.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Server/Script/HookInterface.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"

int LuaUnit::GetDisplay(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        lua_pushinteger(L, 0);
    else
        lua_pushinteger(L, ptr->getDisplayId());

    return 1;
}

int LuaUnit::GetNativeDisplay(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        lua_pushinteger(L, 0);
    else
        lua_pushinteger(L, ptr->getNativeDisplayId());

    return 1;
}

int LuaUnit::GossipCreateMenu(lua_State* L, Unit* ptr)
{
    const int text_id = static_cast<int>(luaL_checkinteger(L, 1));
    Player* plr = CHECK_PLAYER(L, 2);
    const int autosend = static_cast<int>(luaL_checkinteger(L, 3));

    if (plr == nullptr)
        return 0;

    if (LuaGlobal::instance()->m_menu != nullptr)
        delete LuaGlobal::instance()->m_menu;

    LuaGlobal::instance()->m_menu = new GossipMenu(ptr->getGuid(), text_id);

    if (autosend != 0)
        LuaGlobal::instance()->m_menu->sendGossipPacket(plr);

    return 0;
}

int LuaUnit::GossipMenuAddItem(lua_State* L, Unit* /*ptr*/)
{
    const uint8_t icon = static_cast<uint8_t>(luaL_checkinteger(L, 1));
    const char* menu_text = luaL_checkstring(L, 2);
    const int IntId = static_cast<int>(luaL_checkinteger(L, 3));
    const bool coded = (luaL_checkinteger(L, 4) ? true : false);
    const char* boxmessage = luaL_optstring(L, 5, "");
    const uint32_t boxmoney = static_cast<uint32_t>(luaL_optinteger(L, 6, 0));

    if (LuaGlobal::instance()->m_menu == nullptr)
    {
        DLLLogDetail("There is no menu to add items to!");
        return 0;
    }

    LuaGlobal::instance()->m_menu->addItem(icon, 0, IntId, menu_text, boxmoney, boxmessage, coded);

    return 0;
}

int LuaUnit::GossipSendMenu(lua_State* L, Unit* /*ptr*/)
{
    Player* plr = CHECK_PLAYER(L, 1);

    if (LuaGlobal::instance()->m_menu == nullptr)
    {
        DLLLogDetail("There is no menu to send!");
        return 0;
    }

    if (plr != nullptr)
        LuaGlobal::instance()->m_menu->sendGossipPacket(plr);

    return 0;
}

int LuaUnit::GossipSendPOI(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    Player* plr = dynamic_cast<Player*>(ptr);
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const int icon = static_cast<int>(luaL_checkinteger(L, 3));
    const int flags = static_cast<int>(luaL_checkinteger(L, 4));
    const int data = static_cast<int>(luaL_checkinteger(L, 5));
    const char* name = luaL_checkstring(L, 6);

    plr->sendGossipPoiPacket(x, y, icon, flags, data, name);

    return 0;
}

int LuaUnit::GossipSendQuickMenu(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const uint32_t text_id = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    Player* player = CHECK_PLAYER(L, 2);
    const uint32_t itemid = static_cast<uint32_t>(luaL_checkinteger(L, 3));
    const uint8_t itemicon = CHECK_UINT8(L, 4);
    const char* itemtext = luaL_checkstring(L, 5);
    const uint32_t requiredmoney = CHECK_ULONG(L, 6);
    const char* moneytext = luaL_checkstring(L, 7);
    const uint8_t extra = CHECK_UINT8(L, 8);

    if (player == nullptr)
        return 0;

    GossipMenu::sendQuickMenu(ptr->getGuid(), text_id, player, itemid, itemicon, itemtext, requiredmoney, moneytext, extra);

    return 0;
}

int LuaUnit::GossipAddQuests(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    if (LuaGlobal::instance()->m_menu == nullptr)
    {
        DLLLogDetail("There's no menu to fill quests into.");
        return 0;
    }

    Player* player = CHECK_PLAYER(L, 1);
    sQuestMgr.FillQuestMenu(dynamic_cast<Creature*>(ptr), player, *LuaGlobal::instance()->m_menu);
    return 0;
}

int LuaUnit::GossipComplete(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    Player* plr = dynamic_cast<Player*>(ptr);
    if (LuaGlobal::instance()->m_menu == nullptr)
    {
        DLLLogDetail("There is no menu to complete!");
        return 0;
    }

    LuaGlobal::instance()->m_menu->senGossipComplete(plr);

    return 0;
}

int LuaUnit::IsPlayer(lua_State* L, Unit* ptr)
{
    if (!ptr)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (ptr->isPlayer())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

int LuaUnit::IsCreature(lua_State* L, Unit* ptr)
{
    if (!ptr)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (ptr->isCreature())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

int LuaUnit::Emote(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    uint32_t emote_id = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint32_t time = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (emote_id == 0)
        return 0;

    if (time > 0)
        ptr->eventAddEmote((EmoteType)emote_id, time);
    else
        ptr->emote((EmoteType)emote_id);

    return 1;
}

int LuaUnit::GetName(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    switch (ptr->getObjectTypeId())
    {
        case TYPEID_UNIT:
            lua_pushstring(L, dynamic_cast<Creature*>(ptr)->GetCreatureProperties() ? dynamic_cast<Creature*>(ptr)->GetCreatureProperties()->Name.c_str() : "Unknown");
            break;

        case TYPEID_PLAYER:
            lua_pushstring(L, dynamic_cast<Player*>(ptr)->getName().c_str());
            break;

        default:
            lua_pushstring(L, "Unknown");
            break;
    }

    return 1;
}

int LuaUnit::PhaseSet(lua_State* L, Unit* ptr)
{
    const uint32_t newphase = CHECK_ULONG(L, 1);
    const bool Save = (luaL_optinteger(L, 2, false) > 0 ? true : false);
    Creature* crt = nullptr;
    Player* p_target = nullptr;
    //Save is only for creatures. if you want to save to DB with players, use your own query (security purposes).
    //Lua: CharDBQuery("UPDATE `characters` SET `phase`='"..phase.."' WHERE (`name`='"..player:GetName().."'",0)
    if (!ptr)
        return 0;

    switch (ptr->getObjectTypeId())
    {
        case TYPEID_UNIT:
            crt = dynamic_cast<Creature*>(ptr);
            crt->setPhase(PHASE_SET, newphase);
            if (crt->m_spawn)
                crt->m_spawn->phase = newphase;
            if (Save)
            {
                crt->SaveToDB();
                crt->m_loadedFromDB = true;
            }
            break;

        case TYPEID_PLAYER:
            p_target = dynamic_cast<Player*>(ptr);
            p_target->setPhase(PHASE_SET, newphase);
            break;

        default:
            break;
    }
    return 0;
}

int LuaUnit::PhaseAdd(lua_State* L, Unit* ptr)
{
    const uint32_t newphase = CHECK_ULONG(L, 1);
    const bool Save = (luaL_optinteger(L, 2, false) > 0 ? true : false);
    Creature* crt = nullptr;
    Player* p_target = nullptr;
    //Save is only for creatures. if you want to save to DB with players, use your own query (security purposes).
    //Lua: CharDBQuery("UPDATE `characters` SET `phase`='"..player:GetPhase().."' WHERE (`name`='"..player:GetName().."'",0)
    if (!ptr)
        return 0;

    switch (ptr->getObjectTypeId())
    {
        case TYPEID_UNIT:
            crt = dynamic_cast<Creature*>(ptr);
            crt->setPhase(PHASE_ADD, newphase);
            if (crt->m_spawn)
                crt->m_spawn->phase |= newphase;

            if (Save)
            {
                crt->SaveToDB();
                crt->m_loadedFromDB = true;
            }
            break;

        case TYPEID_PLAYER:
            p_target = dynamic_cast<Player*>(ptr);
            p_target->setPhase(PHASE_ADD, newphase);
            break;

        default:
            break;
    }
    return 0;
}

int LuaUnit::PhaseDelete(lua_State* L, Unit* ptr)
{
    const uint32_t newphase = CHECK_ULONG(L, 1);
    const bool Save = (luaL_checkinteger(L, 2) > 0 ? true : false);
    Creature* crt = nullptr;
    Player* p_target = nullptr;
    //Save is only for creatures. if you want to save to DB with players, use your own query (security purposes).
    //Lua: CharDBQuery("UPDATE `characters` SET `phase`='"..player:GetPhase().."' WHERE (`name`='"..player:GetName().."'",0)
    if (!ptr)
        return 0;

    switch (ptr->getObjectTypeId())
    {
        case TYPEID_UNIT:
            crt = dynamic_cast<Creature*>(ptr);
            crt->setPhase(PHASE_DEL, newphase);
            if (crt->m_spawn)
                crt->m_spawn->phase &= ~newphase;

            if (Save)
            {
                crt->SaveToDB();
                crt->m_loadedFromDB = true;
            }
            break;

        case TYPEID_PLAYER:
            p_target = dynamic_cast<Player*>(ptr);
            p_target->setPhase(PHASE_DEL, newphase);
            break;

        default:
            break;
    }
    return 0;
}

int LuaUnit::GetPhase(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;
    lua_pushnumber(L, ptr->m_phase);
    return 1;
}

int LuaUnit::SendChatMessage(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const uint8_t typ = static_cast<uint8_t>(CHECK_ULONG(L, 1));
    const uint32_t lang = CHECK_ULONG(L, 2);
    const char* message = luaL_checklstring(L, 3, nullptr);
    if (message == nullptr)
        return 0;

    ptr->sendChatMessage(typ, lang, message);
    return 0;
}

int LuaUnit::PlayerSendChatMessage(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint8_t type = CHECK_UINT8(L, 1);
    const uint32_t lang = CHECK_ULONG(L, 2);
    const char* msg = luaL_checklstring(L, 3, nullptr);
    const Player* plr = dynamic_cast<Player*>(ptr);
    if (msg == nullptr)
        return 0;

    plr->getSession()->SendChatPacket(AscEmu::Packets::SmsgMessageChat(type, lang, 0, msg, plr->getGuid()).serialise().get(), 1, lang, plr->getSession());
    for (const auto& itr : plr->getInRangePlayersSet())
    {
        if (itr)
            dynamic_cast<Player*>(itr)->getSession()->SendChatPacket(AscEmu::Packets::SmsgMessageChat(type, lang, 0, msg, plr->getGuid()).serialise().get(), 1, lang, plr->getSession());
    }
    return 0;
}

int LuaUnit::AggroWithInRangeFriends(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    // If Pointer isn't in combat skip everything
    if (!ptr->getCombatHandler().isInCombat())
        return 0;

    Unit* pTarget = ptr->getAIInterface()->getCurrentTarget();
    if (!pTarget)
        return 0;

    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        if (!itr)
            continue;

        Object* obj = itr;
        // Object Isn't a Unit, Unit is Dead
        if (!obj->isCreatureOrPlayer() || dynamic_cast<Unit*>(obj)->isDead())
            continue;

        if (!obj->isFriendlyTo(ptr))
            continue;

        if (ptr->GetDistance2dSq(obj) > 10 * 10) // 10yrd range?
            continue;

        const Unit* pUnit = dynamic_cast<Unit*>(obj);

        pUnit->getAIInterface()->setCurrentTarget(pTarget);
        pUnit->getAIInterface()->onHostileAction(pTarget);
    }
    return 0;
}

int LuaUnit::MoveTo(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float z = CHECK_FLOAT(L, 3);
    //float o = CHECK_FLOAT(L, 4);

    ptr->getAIInterface()->moveTo(x, y, z);
    return 0;
}

int LuaUnit::MoveRandomArea(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const float x1 = CHECK_FLOAT(L, 1);
    const float y1 = CHECK_FLOAT(L, 2);
    const float z1 = CHECK_FLOAT(L, 3);
    const float x2 = CHECK_FLOAT(L, 4);
    const float y2 = CHECK_FLOAT(L, 5);
    const float z2 = CHECK_FLOAT(L, 6);
    //float o2 = CHECK_FLOAT(L, 7);

    ptr->getAIInterface()->moveTo(x1 + (Util::getRandomFloat(x2 - x1)), y1 + (Util::getRandomFloat(y2 - y1)), z1 + (Util::getRandomFloat(z2 - z1)));
    return 0;
}

int LuaUnit::GetX(lua_State* L, Unit* ptr)
{
    if (ptr != nullptr)
        lua_pushnumber(L, ptr->GetPositionX());
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetY(lua_State* L, Unit* ptr)
{
    if (ptr != nullptr)
        lua_pushnumber(L, ptr->GetPositionY());
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetZ(lua_State* L, Unit* ptr)
{
    if (ptr != nullptr)
        lua_pushnumber(L, ptr->GetPositionZ());
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetO(lua_State* L, Unit* ptr)
{
    if (ptr != nullptr)
        lua_pushnumber(L, ptr->GetOrientation());
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::CastSpell(lua_State* L, Unit* ptr)
{
    const uint32_t sp = CHECK_ULONG(L, 1);
    if (sp && ptr)
        ptr->castSpell(ptr, sSpellMgr.getSpellInfo(sp), true);
    return 0;
}

int LuaUnit::FullCastSpell(lua_State* L, Unit* ptr)
{
    const uint32_t sp = CHECK_ULONG(L, 1);
    if (sp && ptr)
        ptr->castSpell(ptr, sSpellMgr.getSpellInfo(sp), false);
    return 0;
}

int LuaUnit::FullCastSpellOnTarget(lua_State* L, Unit* ptr)
{
    if (ptr != nullptr)
    {
        const uint32_t sp = CHECK_ULONG(L, 1);
        const Object* target = CHECK_OBJECT(L, 2);
        if (sp && target != nullptr)
            ptr->castSpell(target->getGuid(), sp, false);
    }
    return 0;
}

int LuaUnit::CastSpellOnTarget(lua_State* L, Unit* ptr)
{
    const uint32_t sp = CHECK_ULONG(L, 1);
    const Object* target = CHECK_OBJECT(L, 2);
    if (ptr != nullptr && sp && target != nullptr)
        ptr->castSpell(target->getGuid(), sp, true);
    return 0;
}

int LuaUnit::SpawnCreature(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    const uint32_t entry = CHECK_ULONG(L, 1);
    const float x = CHECK_FLOAT(L, 2);
    const float y = CHECK_FLOAT(L, 3);
    const float z = CHECK_FLOAT(L, 4);
    const float o = CHECK_FLOAT(L, 5);
    const uint32_t faction = CHECK_ULONG(L, 6);
    const uint32_t duration = CHECK_ULONG(L, 7);
    const uint32_t equip1 = static_cast<uint32_t>(luaL_optinteger(L, 8, 1));
    const uint32_t equip2 = static_cast<uint32_t>(luaL_optinteger(L, 9, 1));
    const uint32_t equip3 = static_cast<uint32_t>(luaL_optinteger(L, 10, 1));
    const uint32_t phase = static_cast<uint32_t>(luaL_optinteger(L, 11, ptr->m_phase));
    const bool save = (luaL_optinteger(L, 12, 0) ? true : false);

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
    Creature* pCreature = ptr->getWorldMap()->createCreature(entry);
    if (pCreature == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }
    pCreature->Load(p, x, y, z, o);
    pCreature->setFaction(faction);
    pCreature->setVirtualItemSlotId(MELEE, equip1);
    pCreature->setVirtualItemSlotId(OFFHAND, equip2);
    pCreature->setVirtualItemSlotId(RANGED, equip3);
    pCreature->setPhase(PHASE_SET, phase);
    pCreature->m_noRespawn = true;
    pCreature->AddToWorld(ptr->getWorldMap());
    if (duration)
        pCreature->Despawn(duration, 0);
    if (save)
        pCreature->SaveToDB();
    PUSH_UNIT(L, pCreature);
    return 1;
}

int LuaUnit::SpawnGameObject(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;
    const uint32_t entry_id = CHECK_ULONG(L, 1);
    const float x = CHECK_FLOAT(L, 2);
    const float y = CHECK_FLOAT(L, 3);
    const float z = CHECK_FLOAT(L, 4);
    const float o = CHECK_FLOAT(L, 5);
    const uint32_t duration = CHECK_ULONG(L, 6);
    const float scale = (float)(luaL_optinteger(L, 7, 100) / 100.0f);
    const uint32_t phase = static_cast<uint32_t>(luaL_optinteger(L, 8, ptr->m_phase));
    const bool save = luaL_optinteger(L, 9, 0) ? true : false;
    if (entry_id)
    {
        GameObjectProperties const* info = sMySQLStore.getGameObjectProperties(entry_id);
        if (info == nullptr)
        {
            DLLLogDetail("Lua script tried to spawn a gameobject that doesn't exist ( %u ). Aborting.", entry_id);
            lua_pushnil(L);
            return 1;
        }

        GameObject* go = ptr->getWorldMap()->createGameObject(entry_id);
        go->create(entry_id, ptr->getWorldMap(), ptr->GetPhase(), LocationVector(x, y, z, o), QuaternionData(), GO_STATE_CLOSED);
        go->Phase(PHASE_SET, phase);
        go->setScale(scale);
        go->AddToWorld(ptr->getWorldMap());

        if (duration)
            go->despawn(duration, 0);
        if (save)
            go->saveToDB();
        PUSH_GO(L, go);
    }
    else
        lua_pushnil(L);
    return 1;
}
int LuaUnit::RegisterEvent(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const char* typeName = luaL_typename(L, 1);
    const int delay = static_cast<int>(luaL_checkinteger(L, 2));
    const int repeats = static_cast<int>(luaL_checkinteger(L, 3));
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
        Creature* creature = dynamic_cast<Creature*>(ptr);
        sEventMgr.AddEvent(creature, &Creature::TriggerScriptEvent, functionRef, EVENT_LUA_CREATURE_EVENTS, delay, repeats, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        std::map< uint64_t, std::set<int> >& objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
        const std::map< uint64_t, std::set<int> >::iterator itr = objRefs.find(ptr->getGuid());
        if (itr == objRefs.end())
        {
            std::set<int> refs;
            refs.insert(functionRef);
            objRefs.insert(make_pair(ptr->getGuid(), refs));
        }
        else
        {
            std::set<int>& refs = itr->second;
            refs.insert(functionRef);
        }
    }
    return 0;
}
/* This one just simply calls the function directly w/o any arguments,
the trick to arguments is done Lua side through closures(function that
calls the wanted function  with the wanted arguments */
int LuaUnit::CreateLuaEvent(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    const char* typeName = luaL_typename(L, 1);
    const int delay = static_cast<int>(luaL_checkinteger(L, 2));
    const int repeats = static_cast<int>(luaL_checkinteger(L, 3));
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
        auto ev = TimedEvent::Allocate(ptr, std::make_unique<CallbackP1<LuaEngine, int>>(LuaGlobal::instance()->luaEngine().get(), &LuaEngine::CallFunctionByReference, functionRef), EVENT_LUA_CREATURE_EVENTS, delay, repeats);
        ptr->event_AddEvent(std::move(ev));
        std::map< uint64_t, std::set<int> >& objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
        const std::map< uint64_t, std::set<int> >::iterator itr = objRefs.find(ptr->getGuid());
        if (itr == objRefs.end())
        {
            std::set<int> refs;
            refs.insert(functionRef);
            objRefs.insert(make_pair(ptr->getGuid(), refs));
        }
        else
        {
            std::set<int>& refs = itr->second;
            refs.insert(functionRef);
        }
    }
    return 0;
}

int LuaUnit::RemoveEvents(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
        return 0;

    sEventMgr.RemoveEvents(ptr, EVENT_LUA_CREATURE_EVENTS);
    //Unref all contained references
    std::map< uint64_t, std::set<int> >& objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
    const std::map< uint64_t, std::set<int> >::iterator itr = objRefs.find(ptr->getGuid());
    if (itr != objRefs.end())
    {
        std::set<int>& refs = itr->second;
        for (std::set<int>::iterator it = refs.begin(); it != refs.end(); ++it)
            luaL_unref(L, LUA_REGISTRYINDEX, (*it));
        refs.clear();
    }
    return 0;
}

int LuaUnit::SetFaction(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    const int faction = static_cast<int>(luaL_checkinteger(L, 1));
    if (!faction)
        return 0;

    ptr->setFaction(faction);
    return 0;
}

int LuaUnit::GetNativeFaction(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (ptr->isPlayer())
        RET_INT(dynamic_cast<Player*>(ptr)->getInitialFactionId());

    if (dynamic_cast<Creature*>(ptr)->GetCreatureProperties())
        RET_INT(dynamic_cast<Creature*>(ptr)->GetCreatureProperties()->Faction);

    RET_INT(ptr->getFactionTemplate());
}

int LuaUnit::SetStandState(lua_State* L, Unit* ptr)   //states 0..8
{
    if (!ptr)
        return 0;

    const uint8_t state = static_cast<uint8_t>(luaL_checkinteger(L, 1));

    ptr->setStandState(state);
    return 0;
}

int LuaUnit::IsInCombat(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld())
    {
        lua_pushnil(L);
        return 1;
    }

    if (ptr->getCombatHandler().isInCombat())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

int LuaUnit::SetScale(lua_State* L, Unit* ptr)
{
    const float scale = CHECK_FLOAT(L, 1);
    if (scale && ptr)
        ptr->setScale(scale);
    else
        RET_BOOL(false)

        RET_BOOL(true)
}

int LuaUnit::SetModel(lua_State* L, Unit* ptr)
{
    const uint32_t model = CHECK_ULONG(L, 1);
    if (ptr != nullptr)
        ptr->setDisplayId(model);
    else
        RET_BOOL(false)

        RET_BOOL(true)
}

int LuaUnit::ResetModel(lua_State* /*L*/, Unit* ptr)
{
    if (ptr != nullptr)
        ptr->resetDisplayId();

    return 0;
}

int LuaUnit::SetNPCFlags(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const uint32_t flags = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    ptr->setNpcFlags(flags);
    return 0;
}

int LuaUnit::SetMount(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    const uint32_t DsplId = CHECK_ULONG(L, 1);
    ptr->setMountDisplayId(DsplId);
    return 0;
}

int LuaUnit::RemoveItem(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint32_t id = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint32_t count = static_cast<uint32_t>(luaL_checkinteger(L, 2));

    dynamic_cast<Player*>(ptr)->getItemInterface()->RemoveItemAmt(id, count);
    return 0;
}

int LuaUnit::AddItem(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint32_t id = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint32_t count = static_cast<uint32_t>(luaL_checkinteger(L, 2));

    const auto player = dynamic_cast<Player*>(ptr);
    ItemProperties const* item_proto = sMySQLStore.getItemProperties(id);
    if (item_proto == nullptr)
        return 0;

    player->getItemInterface()->AddItemById(id, count, 0);
    // TODO
    //PUSH_ITEM(L, item_add);
    return 1;
}

int LuaUnit::GetInstanceID(lua_State* L, Unit* ptr)
{
    //if(ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature()) { return 0; }
    if (!ptr || ptr->getWorldMap() == nullptr || ptr->getWorldMap()->getBaseMap()->isWorldMap())
        lua_pushnil(L);
    else
        lua_pushinteger(L, ptr->GetInstanceID());
    return 1;
}

int LuaUnit::GetClosestPlayer(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    float dist = 0;
    float d2 = 0;
    Player* ret = nullptr;

    for (const auto& itr : ptr->getInRangePlayersSet())
    {
        d2 = itr->getDistanceSq(ptr);
        if (!ret || d2 < dist)
        {
            dist = d2;
            ret = dynamic_cast<Player*>(itr);
        }
    }

    if (ret == nullptr)
        lua_pushnil(L);
    else
        PUSH_UNIT(L, ret);

    return 1;
}

int LuaUnit::GetRandomPlayer(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    const int flag = static_cast<int>(luaL_checkinteger(L, 1));
    Player* ret = nullptr;
    std::vector<Player*> players;
    switch (flag)
    {
        case RANDOM_ANY:
        {
            uint32_t count = static_cast<uint32_t>(ptr->getInRangePlayersCount());
            const uint32_t r = Util::getRandomUInt(count - 1);
            count = 0;
            for (const auto& itr : ptr->getInRangePlayersSet())
            {
                if (count == r)
                {
                    ret = dynamic_cast<Player*>(itr);
                    break;
                }
                ++count;
            }
        }
        break;
        case RANDOM_IN_SHORTRANGE:
        {
            for (const auto& itr : ptr->getInRangePlayersSet())
            {
                Player* obj = dynamic_cast<Player*>(itr);
                if (obj && obj->CalcDistance(obj, ptr) <= 8)
                    players.push_back(obj);
            }
            if (players.size())
                ret = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
        }
        break;
        case RANDOM_IN_MIDRANGE:
        {
            for (const auto& itr : ptr->getInRangePlayersSet())
            {
                Player* obj = dynamic_cast<Player*>(itr);
                const float distance = obj->CalcDistance(obj, ptr);
                if (distance < 20 && distance > 8)
                    players.push_back(obj);
            }
            if (players.size())
                ret = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
        }
        break;
        case RANDOM_IN_LONGRANGE:
        {
            for (const auto& itr : ptr->getInRangePlayersSet())
            {
                Player* obj = dynamic_cast<Player*>(itr);
                if (obj && obj->CalcDistance(obj, ptr) >= 20)
                    players.push_back(obj);
            }
            if (players.size())
                ret = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
        }
        break;
        case RANDOM_WITH_MANA:
        {
            for (const auto& itr : ptr->getInRangePlayersSet())
            {
                Player* obj = dynamic_cast<Player*>(itr);
                if (obj && obj->getPowerType() == POWER_TYPE_MANA)
                    players.push_back(obj);
            }
            if (players.size())
                ret = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
        }
        break;
        case RANDOM_WITH_ENERGY:
        {
            for (const auto& itr : ptr->getInRangePlayersSet())
            {
                Player* obj = dynamic_cast<Player*>(itr);
                if (obj && obj->getPowerType() == POWER_TYPE_ENERGY)
                    players.push_back(obj);
            }
            if (players.size())
                ret = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
        }
        break;
        case RANDOM_WITH_RAGE:
        {
            for (const auto& itr : ptr->getInRangePlayersSet())
            {
                Player* obj = dynamic_cast<Player*>(itr);
                if (obj && obj->getPowerType() == POWER_TYPE_RAGE)
                    players.push_back(obj);
            }
            if (players.size())
                ret = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
        }
        break;
        case RANDOM_NOT_MAINTANK:
        {
            const Unit* mt = ptr->getAIInterface()->getCurrentTarget();
            if (mt == nullptr || !mt->isPlayer())
                return 0;

            for (const auto& itr : ptr->getInRangePlayersSet())
            {
                Player* obj = dynamic_cast<Player*>(itr);
                if (obj != mt)
                    players.push_back(obj);
            }
            if (players.size())
                ret = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
        }
        break;
    }

    if (ret == nullptr)
        lua_pushnil(L);
    else
        PUSH_UNIT(L, ret);

    return 1;
}

int LuaUnit::GetRandomFriend(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
        return 0;

    std::vector<Object*> allies;

    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        Object* obj = itr;
        if (obj && obj->isCreatureOrPlayer() && obj->isFriendlyTo(ptr))
            allies.push_back(obj);
    }

    if (allies.size())
        PUSH_UNIT(L, allies[Util::getRandomUInt(static_cast<uint32_t>(allies.size() - 1))]);
    else
        lua_pushnil(L);

    return 1;
}

int LuaUnit::GetRandomEnemy(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
        return 0;

    std::vector<Object*> enemies;

    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        Object* obj = itr;
        if (obj && obj->isCreatureOrPlayer() && ptr->isHostileTo(obj))
            enemies.push_back(obj);
    }

    if (enemies.size())
        PUSH_UNIT(L, enemies[Util::getRandomUInt(static_cast<uint32_t>(enemies.size() - 1))]);
    else
        lua_pushnil(L);

    return 1;
}

int LuaUnit::StopMovement(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const uint32_t tim = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    ptr->pauseMovement(tim);
    return 0;
}

int LuaUnit::RemoveAura(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
        return 0;

    const uint32_t auraid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    ptr->removeAllAurasById(auraid);
    return 0;
}

int LuaUnit::CanAttack(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    Unit* target = CHECK_UNIT(L, 1);
    if (!target)
        return 0;

    if (ptr->isValidTarget(target))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

int LuaUnit::PlaySoundToSet(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
        return 0;

    const uint32_t soundid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    ptr->PlaySoundToSet(soundid);
    return 0;
}

int LuaUnit::PlaySoundToPlayer(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint32_t soundid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    Player* plr = dynamic_cast<Player*>(ptr);
    plr->sendPlayObjectSoundPacket(plr->getGuid(), soundid);
    return 0;
}

int LuaUnit::GetUnitBySqlId(lua_State* L, Unit* ptr)
{
    const uint32_t sqlid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (!ptr || !sqlid)
        return 0;

    PUSH_UNIT(L, ptr->getWorldMap()->getSqlIdCreature(sqlid));
    return 1;
}

int LuaUnit::GetInventoryItem(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const int8_t containerslot = static_cast<int8_t>(luaL_checkinteger(L, 1));
    const int16_t slot = static_cast<int16_t>(luaL_checkinteger(L, 2));
    const Player* plr = dynamic_cast<Player*>(ptr);
    PUSH_ITEM(L, plr->getItemInterface()->GetInventoryItem(containerslot, slot));
    return 1;
}

int LuaUnit::GetInventoryItemById(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint32_t entry = CHECK_ULONG(L, 1);
    const Player* plr = dynamic_cast<Player*>(ptr);
    const int16_t slot = plr->getItemInterface()->GetInventorySlotById(entry);
    if (slot == -1)  //check bags
    {
        for (uint8_t contslot = INVENTORY_SLOT_BAG_START; contslot != INVENTORY_SLOT_BAG_END; contslot++)
        {
            Container* bag = dynamic_cast<Container*>(plr->getItemInterface()->GetInventoryItem(contslot));
            if (bag == nullptr)
                continue;

            for (uint8_t bslot = 0; bslot != bag->getSlotCount(); bslot++)
            {
                if (bag->getItem(bslot) && bag->getItem(bslot)->getEntry() == entry)
                {
                    PUSH_ITEM(L, bag->getItem(bslot));
                    return 1;
                }
            }
        }
    }
    PUSH_ITEM(L, plr->getItemInterface()->GetInventoryItem(slot));
    return 1;
}

int LuaUnit::SetZoneWeather(lua_State* L, Unit* /*ptr*/)
{
    const uint32_t zoneId = CHECK_ULONG(L, 1);
    const uint32_t type = CHECK_ULONG(L, 2);
    const float density = CHECK_FLOAT(L, 3);
    if (!zoneId)
        return 0;

    sWeatherMgr.sendWeatherForZone(type, density, zoneId);
    return 0;
}

int LuaUnit::SetPlayerWeather(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const auto player = dynamic_cast<Player*>(ptr);
    const uint32_t type = CHECK_ULONG(L, 1);
    const float density = CHECK_FLOAT(L, 2);

    sWeatherMgr.sendWeatherForPlayer(type, density, player);

    return 0;
}

int LuaUnit::Despawn(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const uint32_t delay = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint32_t respawntime = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    dynamic_cast<Creature*>(ptr)->Despawn(delay, respawntime);
    return 0;
}

int LuaUnit::GetInRangeFriends(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    uint32_t count = 0;
    lua_newtable(L);
    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        if (itr && itr->isCreatureOrPlayer() && ptr->isFriendlyTo(itr))
        {
            count++;
            lua_pushinteger(L, count);
            PUSH_UNIT(L, itr);
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaUnit::GetInRangeEnemies(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    uint32_t count = 0;
    lua_newtable(L);
    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        if (itr && itr->isCreatureOrPlayer() && !ptr->isFriendlyTo(itr))
        {
            count++;
            lua_pushinteger(L, count);
            PUSH_UNIT(L, itr);
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaUnit::GetInRangeUnits(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    uint32_t count = 0;
    lua_newtable(L);
    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        if (itr && itr->isCreatureOrPlayer())
        {
            ++count;
            lua_pushinteger(L, count);
            PUSH_UNIT(L, itr);
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaUnit::getHealthPct(lua_State* L, Unit* ptr)
{
    if (!ptr)
        lua_pushinteger(L, 0);
    else
        lua_pushinteger(L, ptr->getHealthPct());
    return 1;
}

int LuaUnit::SetHealthPct(lua_State* L, Unit* ptr)
{
    const uint32_t val = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (val && ptr)
        ptr->setHealthPct(val);
    return 0;
}

int LuaUnit::GetItemCount(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint32_t itemid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    lua_pushinteger(L, dynamic_cast<Player*>(ptr)->getItemInterface()->GetItemCount(itemid, false));
    return 1;
}

int LuaUnit::GetMainTank(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    Unit* ret = ptr->getAIInterface()->getCurrentTarget();
    if (!ret)
        lua_pushnil(L);
    else
        PUSH_UNIT(L, ret);
    return 1;
}

int LuaUnit::GetAddTank(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    Unit* ret = ptr->getThreatManager().getSecondMostHated();
    if (ret == nullptr)
        lua_pushnil(L);
    else
        PUSH_UNIT(L, ret);
    return 1;
}

int LuaUnit::ClearThreatList(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    ptr->clearHateList();
    return 0;
}

int LuaUnit::SetTauntedBy(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    Unit* target = CHECK_UNIT(L, 1);
    if (!target || target == ptr)
        return 0;

    ptr->castSpell(target, 53798, false);
    return 0;
}

int LuaUnit::ModThreat(lua_State* L, Unit* ptr)
{
    Unit* target = CHECK_UNIT(L, 1);
    const auto amount = static_cast<float>(luaL_checkinteger(L, 2));
    if (ptr && target && amount)
        ptr->getThreatManager().addThreat(target, amount, nullptr, false, true);
    return 0;
}

int LuaUnit::GetThreatByPtr(lua_State* L, Unit* ptr)
{
    const Unit* target = CHECK_UNIT(L, 1);
    if (ptr && target)
        lua_pushnumber(L, ptr->getThreatManager().getThreat(target));
    return 1;
}

int LuaUnit::ChangeTarget(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    Unit* target = CHECK_UNIT(L, 1);
    if (!target || !ptr->isHostileTo(target) || ptr == target)
        return 0;

    ptr->getAIInterface()->setCurrentTarget(target);
    return 0;
}

int LuaUnit::HasFinishedQuest(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    const uint32_t questid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (dynamic_cast<Player*>(ptr)->hasQuestFinished(questid))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::FinishQuest(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    const uint32_t quest_id = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    Player* plr = dynamic_cast<Player*>(ptr);

    if (auto* qst = sMySQLStore.getQuestProperties(quest_id))
    {
        if (plr->hasQuestFinished(quest_id))
        {
            lua_pushnumber(L, 0);
            return 1;
        }

        if (auto* questLog = plr->getQuestLogByQuestId(quest_id))
        {
            sQuestMgr.GenerateQuestXP(plr, qst);
            sQuestMgr.BuildQuestComplete(plr, qst);

            questLog->finishAndRemove();
            plr->addQuestToFinished(quest_id);
            plr->updateNearbyQuestGameObjects();
            lua_pushnumber(L, 1);
            return 1;
        }

        lua_pushnumber(L, 2);
        return 1;
    }
    return 0;
}

int LuaUnit::StartQuest(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    const uint32_t quest_id = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    Player* player = dynamic_cast<Player*>(ptr);

    if (auto* questProperties = sMySQLStore.getQuestProperties(quest_id))
    {
        if (player->hasQuestFinished(quest_id))
        {
            lua_pushnumber(L, 0);
            return 1;
        }

        if (player->hasQuestInQuestLog(quest_id))
        {
            lua_pushnumber(L, 1);
            return 1;
        }

        const uint8_t open_slot = player->getFreeQuestSlot();
        if (open_slot > MAX_QUEST_SLOT)
        {
            sQuestMgr.SendQuestLogFull(player);
            lua_pushnumber(L, 2);
            return 1;
        }

        QuestLogEntry* questLogEntry = new QuestLogEntry(questProperties, player, open_slot);
        questLogEntry->updatePlayerFields();
        // If the quest should give any items on begin, give them the items.
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (questProperties->receive_items[i])
            {
                auto item = sObjectMgr.createItem(questProperties->receive_items[i], player);
                if (item == nullptr)
                    return false;

                player->getItemInterface()->AddItemToFreeSlot(std::move(item));
            }
        }

        if (questProperties->srcitem && questProperties->srcitem != questProperties->receive_items[0])
        {
            auto item = sObjectMgr.createItem(questProperties->srcitem, player);
            if (item)
            {
                item->setStackCount(questProperties->srcitemcount ? questProperties->srcitemcount : 1);
                player->getItemInterface()->AddItemToFreeSlot(std::move(item));
            }
        }

        sHookInterface.OnQuestAccept(player, questProperties, nullptr);
        lua_pushnumber(L, 3);
        return 1;

    }
    return 0;
} // StartQuest

int LuaUnit::UnlearnSpell(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint32_t spellid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    dynamic_cast<Player*>(ptr)->removeSpell(spellid, false);
    return 0;
}

int LuaUnit::LearnSpell(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint32_t spellid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    dynamic_cast<Player*>(ptr)->addSpell(spellid);
    return 0;
}

int LuaUnit::LearnSpells(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (!strcmp("table", luaL_typename(L, 1)))
    {
        const int table = lua_gettop(L);
        lua_pushnil(L);
        while (lua_next(L, table) != 0)
        {
            if (lua_isnumber(L, -1))
                dynamic_cast<Player*>(ptr)->addSpell(CHECK_ULONG(L, -1));
            lua_pop(L, 1);
        }
        lua_pushboolean(L, 1);
        lua_replace(L, table);
        lua_settop(L, table); // Paroxysm: The stack should be balanced but just in case.
    }
    else
    {
        lua_settop(L, 0);
        lua_pushboolean(L, 0);
    }
    return 1;
}

int LuaUnit::MarkQuestObjectiveAsComplete(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint32_t questid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint8_t objective = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    Player* player = dynamic_cast<Player*>(ptr);

    if (!player->hasQuestFinished(questid))
    {
        if (const auto questLog = player->getQuestLogByQuestId(questid))
        {
            questLog->setMobCountForIndex(objective, questLog->getQuestProperties()->required_mob_or_go[objective]);
            questLog->sendUpdateAddKill(objective);
            if (questLog->canBeFinished())
            {
                questLog->sendQuestComplete();
                questLog->updatePlayerFields();
            }
        }
    }
    return 0;
}

int LuaUnit::SendAreaTriggerMessage(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const char* msg = luaL_checkstring(L, 1);
    if (!msg)
        return 0;

    dynamic_cast<Player*>(ptr)->sendAreaTriggerMessage(msg);
    return 0;
}

int LuaUnit::SendBroadcastMessage(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const char* msg = luaL_checkstring(L, 1);
    if (!msg)
        return 0;

    dynamic_cast<Player*>(ptr)->broadcastMessage(msg);
    return 0;
}

int LuaUnit::TeleportUnit(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint32_t mapId = CHECK_ULONG(L, 1);
    const float posX = CHECK_FLOAT(L, 2);
    const float posY = CHECK_FLOAT(L, 3);
    const float posZ = CHECK_FLOAT(L, 4);
    const float Orientation = CHECK_FLOAT(L, 5);

    if (!posX || !posY || !posZ)
    {
        DLLLogDetail("LuaEngineMgr : LUATeleporter ERROR - Wrong Coordinates given (Map, X, Y, Z) :: Map%f%s%f%s%f%s%f", mapId, " X", posX, " Y", posY, " Z", posZ);
        return 0;
    }

    const LocationVector vec(posX, posY, posZ, Orientation);
    dynamic_cast<Player*>(ptr)->safeTeleport(mapId, 0, vec);
    return 0;
}

int LuaUnit::GetHealth(lua_State* L, Unit* ptr)
{
    if (!ptr)
        lua_pushinteger(L, 0);
    else
        lua_pushinteger(L, ptr->getHealth());
    return 1;
}

int LuaUnit::GetMaxHealth(lua_State* L, Unit* ptr)
{
    if (!ptr)
        lua_pushinteger(L, 0);
    else
        lua_pushinteger(L, ptr->getMaxHealth());

    return 1;
}

int LuaUnit::SetHealth(lua_State* L, Unit* ptr)
{
    const uint32_t val = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (ptr != nullptr && val > 0)
    {
        if (val > ptr->getMaxHealth())
            ptr->setHealth(ptr->getMaxHealth());
        else
            ptr->setHealth(val);
    }
    return 0;
}

int LuaUnit::SetMaxHealth(lua_State* L, Unit* ptr)
{
    const uint32_t val = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (ptr != nullptr && val > 0)
    {
        if (val < ptr->getHealth())
            ptr->setHealth(val);

        ptr->setMaxHealth(val);
    }
    return 0;
}

int LuaUnit::WipeHateList(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    ptr->wipeHateList();
    ptr->getThreatManager().clearAllThreat();
    ptr->getThreatManager().removeMeFromThreatLists();
    return 0;
}

int LuaUnit::WipeTargetList(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    ptr->getThreatManager().clearAllThreat();
    ptr->getThreatManager().removeMeFromThreatLists();
    return 0;
}

int LuaUnit::WipeCurrentTarget(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    ptr->getThreatManager().clearThreat(ptr->getAIInterface()->getCurrentTarget());
    return 0;
}

int LuaUnit::GetPlayerClass(lua_State* L, Unit* ptr)
{
    if (!ptr || !ptr->isPlayer())
    {
        lua_pushstring(L, "Unknown");
        return 1;
    }
    const int plrclass = dynamic_cast<Player*>(ptr)->getClass();

    switch (plrclass)
    {
        case 1:
            lua_pushstring(L, "Warrior");
            break;
        case 2:
            lua_pushstring(L, "Paladin");
            break;
        case 3:
            lua_pushstring(L, "Hunter");
            break;
        case 4:
            lua_pushstring(L, "Rogue");
            break;
        case 5:
            lua_pushstring(L, "Priest");
            break;
        case 6:
            lua_pushstring(L, "Death Knight");
            break;
        case 7:
            lua_pushstring(L, "Shaman");
            break;
        case 8:
            lua_pushstring(L, "Mage");
            break;
        case 9:
            lua_pushstring(L, "Warlock");
            break;
            //case 10:
        case 11:
            lua_pushstring(L, "Druid");
            break;
        default:
            lua_pushstring(L, "Unknown");
            break;
    }

    return 1;
}


int LuaUnit::ClearHateList(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    ptr->clearHateList();
    return 0;
}

int LuaUnit::SetMana(lua_State* L, Unit* ptr)
{
    const uint32_t val = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (ptr != nullptr)
        ptr->setPower(POWER_TYPE_MANA, val);
    return 0;
}

int LuaUnit::SetMaxMana(lua_State* L, Unit* ptr)
{
    const uint32_t val = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (ptr != nullptr && val > 0)
    {
        if (val < ptr->getPower(POWER_TYPE_MANA))
            ptr->setPower(POWER_TYPE_MANA, val);
        ptr->setMaxPower(POWER_TYPE_MANA, val);
    }
    return 1;
}

int LuaUnit::GetPlayerRace(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    lua_pushinteger(L, dynamic_cast<Player*>(ptr)->getRace());
    return 1;
}

int LuaUnit::SetFlying(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    ptr->setMoveHover(true);
    ptr->getAIInterface()->setMeleeDisabled(true);
    ptr->setMoveCanFly(true);
    ptr->emote(EMOTE_ONESHOT_LIFTOFF);
    return 0;
}

int LuaUnit::Land(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    ptr->setMoveHover(false);
    ptr->setMoveCanFly(false);
    ptr->getAIInterface()->setMeleeDisabled(false);
    ptr->emote(EMOTE_ONESHOT_LAND);
    return 0;
}

int LuaUnit::HasAura(lua_State* L, Unit* ptr)
{
    const uint32_t spellid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (!ptr || !spellid)
        return 0;

    if (ptr->hasAurasWithId(spellid))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

int LuaUnit::ReturnToSpawnPoint(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    if (ptr->isCreature())
    {
        const float x = ptr->GetSpawnX();
        const float y = ptr->GetSpawnY();
        const float z = ptr->GetSpawnZ();
        const float o = ptr->GetSpawnO();

        ptr->getAIInterface()->moveTo(x, y, z);
        ptr->SetOrientation(o);
    }

    return 0;
}

int LuaUnit::GetGUID(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    PUSH_GUID(L, ptr->getGuid());
    return 1;
}

int LuaUnit::GetDistance(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    Object* target = CHECK_OBJECT(L, 1);
    lua_pushnumber(L, (float)ptr->GetDistance2dSq(target));
    return 1;
}

int LuaUnit::GetDistanceYards(lua_State* L, Unit* ptr)
{
    const Object* target = CHECK_OBJECT(L, 1);
    if (!ptr || !target)
        return 0;

    LocationVector vec = ptr->GetPosition();
    lua_pushnumber(L, (float)vec.Distance(target->GetPosition()));
    return 1;
}

int LuaUnit::GetDuelState(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const Player* plr = dynamic_cast<Player*>(ptr);
    lua_pushnumber(L, plr->getDuelState());

    return 1;
}

int LuaUnit::GetCreatureNearestCoords(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    const uint32_t entryid = CHECK_ULONG(L, 4);
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float z = CHECK_FLOAT(L, 3);
    PUSH_UNIT(L, ptr->getWorldMap()->getInterface()->getCreatureNearestCoords(x, y, z, entryid));
    return 1;
}

int LuaUnit::GetGameObjectNearestCoords(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    const uint32_t entryid = CHECK_ULONG(L, 4);
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float z = CHECK_FLOAT(L, 3);
    PUSH_GO(L, ptr->getWorldMap()->getInterface()->getGameObjectNearestCoords(x, y, z, entryid));
    return 1;
}

int LuaUnit::SetPosition(lua_State* L, Unit* ptr)
{
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float z = CHECK_FLOAT(L, 3);
    const float o = CHECK_FLOAT(L, 4);
    ptr->setFacing(o);
    ptr->SetOrientation(o);

    WorldPacket data(SMSG_MONSTER_MOVE, 50);
    data << ptr->GetNewGUID();
    data << uint8_t(0);
    data << ptr->GetPositionX();
    data << ptr->GetPositionY();
    data << ptr->GetPositionZ();
    data << Util::getMSTime();
    data << uint8_t(0x00);
    data << uint32_t(256);
    data << uint32_t(1);
    data << uint32_t(1);
    data << x << y << z;

    ptr->sendMessageToSet(&data, true);
    ptr->SetPosition(x, y, z, o, true);
    return 0;
}

int LuaUnit::GetLandHeight(lua_State* L, Unit* ptr)
{
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    if (!ptr || !x || !y)
        return 0;

    const float lH = ptr->getWorldMap()->getGridHeight(x, y);
    lua_pushnumber(L, lH);
    return 1;
}

int LuaUnit::IsInPhase(lua_State* L, Unit* ptr)
{
    const uint32_t phase = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    lua_pushboolean(L, ((ptr->m_phase & phase) != 0) ? 1 : 0);
    return 1;
}

int LuaUnit::HasFlag(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    /*uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t flag = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    lua_pushboolean(L, ptr->HasFlag(index, flag) ? 1 : 0);*/
    return 1;
}

int LuaUnit::QuestAddStarter(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    Creature* unit = dynamic_cast<Creature*>(ptr);
    const uint32_t quest_id = (uint32_t)luaL_checknumber(L, 1);
    if (!(unit->getNpcFlags() & UNIT_NPC_FLAG_QUESTGIVER))
        unit->addNpcFlags(UNIT_NPC_FLAG_QUESTGIVER);

    if (!quest_id)
        return 0;

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (!qst)
        return 0;

    const uint32_t quest_giver = unit->getEntry();

    char my_query1[200];
    sprintf(my_query1, "SELECT id FROM creature_quest_starter WHERE id = %d AND quest = %d AND min_build <= %u AND max_build >= %u", quest_giver, quest_id, VERSION_STRING, VERSION_STRING);
    const auto selectResult1 = WorldDatabase.Query(my_query1);
    if (selectResult1 == nullptr)
    {
        char my_insert1[200];
        sprintf(my_insert1, "INSERT INTO creature_quest_starter (id, quest) VALUES (%d,%d,%u,%u)", quest_giver, quest_id, VERSION_STRING, VERSION_STRING);
        WorldDatabase.Execute(my_insert1);
    }

    sQuestMgr.LoadExtraQuestStuff();

    QuestRelation* qstrel = new QuestRelation;
    qstrel->qst = qst;
    qstrel->type = QUESTGIVER_QUEST_START;

    uint8_t qstrelid;
    if (unit->HasQuests())
    {
        qstrelid = (uint8_t)unit->GetQuestRelation(quest_id);
        unit->DeleteQuest(qstrel);
    }
    unit->_LoadQuests();
    return 0;
}

int LuaUnit::QuestAddFinisher(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    Creature* unit = dynamic_cast<Creature*>(ptr);
    const uint32_t quest_id = CHECK_ULONG(L, 1);
    if (!(unit->getNpcFlags() & UNIT_NPC_FLAG_QUESTGIVER))
        unit->addNpcFlags(UNIT_NPC_FLAG_QUESTGIVER);

    if (!quest_id)
        return 0;

    QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
    if (!qst)
        return 0;

    const uint32_t quest_giver = unit->getEntry();

    char my_query1[200];
    sprintf(my_query1, "SELECT id FROM creature_quest_finisher WHERE id = %d AND quest = %d AND min_build <= %u AND max_build >= %u", quest_giver, quest_id, VERSION_STRING, VERSION_STRING);
    const auto selectResult1 = WorldDatabase.Query(my_query1);
    if (selectResult1 == nullptr)
    {
        char my_insert1[200];
        sprintf(my_insert1, "INSERT INTO creature_quest_finisher (id, quest, min_build, max_build) VALUES (%d,%d,%u,%u)", quest_giver, quest_id, VERSION_STRING, VERSION_STRING);
        WorldDatabase.Execute(my_insert1);
    }

    sQuestMgr.LoadExtraQuestStuff();

    QuestRelation* qstrel = new QuestRelation;
    qstrel->qst = qst;
    qstrel->type = QUESTGIVER_QUEST_END;

    uint8_t qstrelid;
    if (unit->HasQuests())
    {
        qstrelid = (uint8_t)unit->GetQuestRelation(quest_id);
        unit->DeleteQuest(qstrel);
    }
    unit->_LoadQuests();
    return 0;
}

int LuaUnit::castSpellLoc(lua_State* L, Unit* ptr)
{
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float z = CHECK_FLOAT(L, 3);
    const uint32_t sp = CHECK_ULONG(L, 4);
    if (!sp || !ptr)
        return 0;

    ptr->castSpellLoc(LocationVector(x, y, z), sSpellMgr.getSpellInfo(sp), true);
    return 0;
}

int LuaUnit::FullCastSpellAoF(lua_State* L, Unit* ptr)
{
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float z = CHECK_FLOAT(L, 3);
    const uint32_t sp = CHECK_ULONG(L, 4);
    if (!sp || !ptr)
        return 0;

    ptr->castSpellLoc(LocationVector(x, y, z), sSpellMgr.getSpellInfo(sp), false);
    return 0;
}

int LuaUnit::SetInFront(lua_State* L, Unit* ptr)
{
    const Unit* target = CHECK_UNIT(L, 1);
    if (!target || !ptr)
        return 0;

    ptr->setInFront(target);
    return 0;
}

int LuaUnit::RemoveAllAuras(lua_State* /*L*/, Unit* ptr)
{
    if (!ptr)
        return 0;

    ptr->removeAllAuras();
    return 0;
}

int LuaUnit::CancelSpell(lua_State* /*L*/, Unit* ptr)
{
    if (!ptr)
        return 0;

    ptr->interruptSpell();
    return 0;
}

int LuaUnit::IsAlive(lua_State* L, Unit* ptr)
{
    if (ptr)
    {
        if (ptr->isAlive())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    }
    return 1;
}

int LuaUnit::IsDead(lua_State* L, Unit* ptr)
{
    if (ptr)
    {
        if (ptr->isAlive())
            lua_pushboolean(L, 0);
        else
            lua_pushboolean(L, 1);
    }
    return 1;
}

int LuaUnit::IsInWorld(lua_State* L, Unit* ptr)
{
    if (ptr)
    {
        if (ptr->IsInWorld())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    }
    return 1;
}

int LuaUnit::GetZoneId(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    lua_pushinteger(L, (ptr->getZoneId()));
    return 1;
}

int LuaUnit::Root(lua_State* /*L*/, Unit* ptr)
{
    if (ptr)
        ptr->setMoveRoot(true);
    return 0;
}

int LuaUnit::Unroot(lua_State* /*L*/, Unit* ptr)
{
    if (ptr)
        ptr->setMoveRoot(false);
    return 0;
}

int LuaUnit::SetOutOfCombatRange(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const auto range = static_cast<float>(luaL_checkinteger(L, 1));
    if (range)
        ptr->getAIInterface()->addBoundary(std::make_unique<CircleBoundary>(ptr->GetPosition(), range), true);
    return 0;
}

int LuaUnit::ModifyRunSpeed(lua_State* L, Unit* ptr)
{
    const float Speed = CHECK_FLOAT(L, 1);
    if (ptr && Speed)
        ptr->setSpeedRate(TYPE_RUN, Speed, true);
    return 0;
}

int LuaUnit::ModifyWalkSpeed(lua_State* L, Unit* ptr)
{
    const float Speed = CHECK_FLOAT(L, 1);
    if (ptr && Speed)
        ptr->setSpeedRate(TYPE_WALK, Speed, true);
    return 0;
}

int LuaUnit::ModifyFlySpeed(lua_State* L, Unit* ptr)
{
    const float Speed = CHECK_FLOAT(L, 1);
    if (ptr && Speed)
        ptr->setSpeedRate(TYPE_FLY, Speed, true);
    return 0;
}

int LuaUnit::isFlying(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    if (ptr->IsFlying())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::SendAIReaction(lua_State* /*L*/, Unit* ptr)
{
    ptr->SendAIReaction();
    return 0;
}

int LuaUnit::SetOrientation(lua_State* L, Unit* ptr)
{
    const float O = CHECK_FLOAT(L, 1);
    if (ptr)
        ptr->SetOrientation(O);
    return 0;
}

int LuaUnit::GetSpawnX(lua_State* L, Unit* ptr)
{
    if (ptr)
        lua_pushnumber(L, ptr->GetSpawnX());
    return 1;
}

int LuaUnit::GetSpawnY(lua_State* L, Unit* ptr)
{
    if (ptr)
        lua_pushnumber(L, ptr->GetSpawnY());
    return 1;
}

int LuaUnit::GetSpawnZ(lua_State* L, Unit* ptr)
{
    if (ptr)
        lua_pushnumber(L, ptr->GetSpawnZ());
    return 1;
}

int LuaUnit::GetSpawnO(lua_State* L, Unit* ptr)
{
    if (ptr)
        lua_pushnumber(L, ptr->GetSpawnO());
    return 1;
}

int LuaUnit::GetInRangePlayersCount(lua_State* L, Unit* ptr)
{
    if (ptr)
        lua_pushnumber(L, static_cast<lua_Number>(ptr->getInRangePlayersCount()));
    return 1;
}

int LuaUnit::GetEntry(lua_State* L, Unit* ptr)
{
    if (ptr)
        lua_pushnumber(L, ptr->getEntry());
    return 1;
}

int LuaUnit::HandleEvent(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    Unit* target = CHECK_UNIT(L, 1);
    const uint32_t event_id = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    const uint32_t misc_1 = static_cast<uint32_t>(luaL_checkinteger(L, 3));
    ptr->getAIInterface()->handleEvent(event_id, target, misc_1);
    return 1;
}

int LuaUnit::GetCurrentSpellId(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    uint32_t spellId = 0;
    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        if (ptr->getCurrentSpell(CurrentSpellType(i)) == nullptr)
            continue;
        spellId = ptr->getCurrentSpell(CurrentSpellType(i))->getSpellInfo()->getId();
        break;
    }

    if (spellId != 0)
        lua_pushnumber(L, spellId);
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetCurrentSpell(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    Spell* curSpell = nullptr;
    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        if (ptr->getCurrentSpell(CurrentSpellType(i)) == nullptr)
            continue;
        curSpell = ptr->getCurrentSpell(CurrentSpellType(i));
        break;
    }

    if (curSpell != nullptr)
        PUSH_SPELL(L, curSpell);
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetFloatValue(lua_State* /*L*/, Unit* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    if (ptr)
        lua_pushnumber(L, ptr->getFloatValue(field));*/
    return 1;
}

int LuaUnit::SendPacket(lua_State* L, Unit* ptr)
{
    WorldPacket* data = CHECK_PACKET(L, 1);
    const int self = lua_toboolean(L, 2);
    if (data && ptr)
        ptr->sendMessageToSet(data, (self > 0) ? true : false);
    return 0;
}

int LuaUnit::SendPacketToGroup(lua_State* L, Unit* ptr)
{
    WorldPacket* data = CHECK_PACKET(L, 1);
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    Player* plr = dynamic_cast<Player*>(ptr);
    if (!data)
        return 0;

    if (plr->getGroup())
        plr->getGroup()->SendPacketToAll(data);
    return 0;
}

int LuaUnit::SendPacketToPlayer(lua_State* L, Unit* ptr)
{
    WorldPacket* data = CHECK_PACKET(L, 1);
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const Player* plr = dynamic_cast<Player*>(ptr);
    if (data)
        plr->getSession()->SendPacket(data);
    return 0;
}

int LuaUnit::ModUInt32Value(lua_State* /*L*/, Unit* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    int32_t value = static_cast<int32_t>(luaL_checkinteger(L, 2));
    if (ptr)
        ptr->modInt32Value(field, value);*/
    return 0;
}

int LuaUnit::ModFloatValue(lua_State* /*L*/, Unit* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    float value = CHECK_FLOAT(L, 2);
    if (ptr)
        ptr->modFloatValue(field, value);*/
    return 0;
}

int LuaUnit::SetUInt32Value(lua_State* /*L*/, Unit* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (ptr)
        ptr->setUInt32Value(field, value);*/
    return 0;
}

int LuaUnit::SetUInt64Value(lua_State* /*L*/, Unit* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(CHECK_ULONG(L, 1));
    uint64_t guid = CHECK_GUID(L, 2);
    if (ptr)
        ptr->setUInt64Value(field, guid);*/
    return 0;
}

int LuaUnit::RemoveFlag(lua_State* /*L*/, Unit* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (ptr)
        ptr->RemoveFlag(field, value);*/
    return 0;
}

int LuaUnit::SetFlag(lua_State* /*L*/, Unit* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (ptr)
        ptr->SetFlag(field, value);*/
    return 0;
}

int LuaUnit::SetFloatValue(lua_State* /*L*/, Unit* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    float value = CHECK_FLOAT(L, 2);
    if (ptr)
        ptr->setFloatValue(field, value);*/
    return 0;
}

int LuaUnit::GetUInt32Value(lua_State* /*L*/, Unit* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    if (ptr)
        lua_pushnumber(L, ptr->getUInt32Value(field));*/
    return 1;
}

int LuaUnit::GetUInt64Value(lua_State* /*L*/, Unit* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    if (ptr)
        PUSH_GUID(L, ptr->getUInt64Value(field));*/
    return 1;
}

int LuaUnit::AdvanceQuestObjective(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint32_t questid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint8_t objective = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    const Player* player = dynamic_cast<Player*>(ptr);

    if (auto* questLog = player->getQuestLogByQuestId(questid))
    {
        questLog->setMobCountForIndex(objective, questLog->getMobCountByIndex(objective) + 1);
        questLog->sendUpdateAddKill(objective);
        if (questLog->canBeFinished())
            questLog->sendQuestComplete();

        questLog->updatePlayerFields();
    }
    return 0;
}

int LuaUnit::Heal(lua_State* L, Unit* ptr)
{
    Unit* target = CHECK_UNIT(L, 1);
    const uint32_t spellid = CHECK_ULONG(L, 2);
    const uint32_t amount = CHECK_ULONG(L, 3);
    if (!target || !spellid || !amount || !ptr)
        return 0;

    target->addSimpleHealingBatchEvent(amount, ptr, sSpellMgr.getSpellInfo(spellid));
    return 0;
}

int LuaUnit::Energize(lua_State* L, Unit* ptr)
{
    Unit* target = CHECK_UNIT(L, 1);
    const uint32_t spellid = CHECK_ULONG(L, 2);
    const uint32_t amount = CHECK_ULONG(L, 3);
    uint32_t type = CHECK_ULONG(L, 4);
    if (!target || !spellid || !amount || !type || !ptr)
        return 0;

    ptr->energize(target, spellid, amount, static_cast<PowerType>(type));
    return 0;
}

int LuaUnit::SendChatMessageAlternateEntry(lua_State* L, Unit* ptr)
{
    const uint32_t entry = CHECK_ULONG(L, 1);
    const uint8_t type = static_cast<uint8_t>(CHECK_ULONG(L, 2));
    const uint32_t lang = CHECK_ULONG(L, 3);
    const char* msg = luaL_checkstring(L, 4);
    if (!entry || !lang || !msg)
        return 0;

    ptr->sendChatMessageAlternateEntry(entry, type, lang, msg);
    return 0;
}

int LuaUnit::SendChatMessageToPlayer(lua_State* L, Unit* ptr)
{
    const uint8_t type = static_cast<uint8_t>(CHECK_ULONG(L, 1));
    const uint32_t lang = CHECK_ULONG(L, 2);
    const char* msg = luaL_checkstring(L, 3);
    Player* plr = CHECK_PLAYER(L, 4);
    if (!plr || !msg || !ptr)
        return 0;

    ptr->sendChatMessageToPlayer(type, lang, msg, plr);
    return 0;
}

int LuaUnit::GetManaPct(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    if (ptr->getPowerType() == (uint8_t)POWER_TYPE_MANA)
        lua_pushnumber(L, (int)(ptr->getPower(POWER_TYPE_MANA) * 100.0f / ptr->getMaxPower(POWER_TYPE_MANA)));
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetPowerPct(lua_State* L, Unit* ptr)
{
    if (!ptr)
    {
        lua_pushnil(L);
        return 1;
    }
    uint16_t powertype;
    if (luaL_optinteger(L, 1, -1) == -1)
        powertype = ptr->getPowerType();
    else
        powertype = static_cast<uint16_t>(luaL_optinteger(L, 1, -1));

    lua_pushnumber(L, static_cast<int>(ptr->getPower(static_cast<PowerType>(powertype)) * 100.0f / ptr->getMaxPower(static_cast<PowerType>(powertype))));
    return 1;
}

int LuaUnit::GetMana(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        lua_pushinteger(L, 0);
    else
        lua_pushinteger(L, ptr->getPower(POWER_TYPE_MANA));

    return 1;
}

int LuaUnit::GetPower(lua_State* L, Unit* ptr)
{
    if (!ptr)
    {
        lua_pushnil(L);
        return 1;
    }
    uint16_t powertype;
    if (luaL_optinteger(L, 1, -1) == -1)
        powertype = ptr->getPowerType();
    else
        powertype = static_cast<uint16_t>(luaL_optinteger(L, 1, -1));

    lua_pushnumber(L, ptr->getPower(static_cast<PowerType>(powertype)));
    return 1;
}

int LuaUnit::GetMaxMana(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        lua_pushinteger(L, 0);
    else
        lua_pushinteger(L, ptr->getMaxPower(POWER_TYPE_MANA));

    return 1;
}

int LuaUnit::GetMaxPower(lua_State* L, Unit* ptr)
{
    if (!ptr)
    {
        lua_pushnil(L);
        return 1;
    }
    uint16_t powertype;
    if (luaL_optinteger(L, 1, -1) == -1)
        powertype = ptr->getPowerType();
    else
        powertype = static_cast<uint16_t>(luaL_optinteger(L, 1, -1));

    lua_pushnumber(L, ptr->getMaxPower(static_cast<PowerType>(powertype)));
    return 1;
}

int LuaUnit::SetPowerType(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;
    /* POWER_TYPE_MANA         = 0,
    POWER_TYPE_RAGE         = 1,
    POWER_TYPE_FOCUS        = 2,
    POWER_TYPE_ENERGY       = 3,
    POWER_TYPE_HAPPINESS    = 4,
    POWER_TYPE_RUNES        = 5,
    POWER_TYPE_RUNIC_POWER  = 6 */

    uint8_t powertype;
    if (luaL_optinteger(L, 1, -1) == -1)
        powertype = static_cast<uint8_t>(ptr->getPowerType());
    else
        powertype = static_cast<uint8_t>(luaL_optinteger(L, 1, -1));

    ptr->setPowerType(powertype);
    return 0;
}

int LuaUnit::SetMaxPower(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    const uint32_t amount = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint16_t powertype;
    if (luaL_optinteger(L, 2, -1) == -1)
        powertype = ptr->getPowerType();
    else
        powertype = static_cast<uint16_t>(luaL_optinteger(L, 2, -1));

    ptr->setMaxPower(static_cast<PowerType>(powertype), amount);
    return 0;
}

int LuaUnit::SetPower(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    const uint32_t amount = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint32_t powertype;
    if (luaL_optinteger(L, 2, -1) == -1)
        powertype = ptr->getPowerType();
    else
        powertype = static_cast<uint32_t>(luaL_optinteger(L, 2, -1));

    ptr->setPower(static_cast<PowerType>(powertype), amount);
    return 0;
}

int LuaUnit::SetPowerPct(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    const uint32_t amount = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint16_t powertype;
    if (luaL_optinteger(L, 2, -1) == -1)
        powertype = ptr->getPowerType();
    else
        powertype = static_cast<uint16_t>(luaL_optinteger(L, 2, -1));

    ptr->setPower(static_cast<PowerType>(powertype), static_cast<int>(amount / 100) * (ptr->getMaxPower(static_cast<PowerType>(powertype))));
    return 0;
}

int LuaUnit::GetPowerType(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    lua_pushinteger(L, ptr->getPowerType());
    return 1;
}

int LuaUnit::Strike(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        lua_pushboolean(L, 0); return 1;
    }
    Unit* target = CHECK_UNIT(L, 1);
    const WeaponDamageType weapon_damage_type = static_cast<WeaponDamageType>(luaL_checkinteger(L, 2));
    const uint32_t sp = CHECK_ULONG(L, 3);
    const int32_t adddmg = static_cast<int32_t>(luaL_checkinteger(L, 4));
    const uint32_t exclusive_damage = CHECK_ULONG(L, 5);
    const int32_t pct_dmg_mod = static_cast<int32_t>(luaL_checkinteger(L, 6));

    if (!target)
        return 0;
    ptr->strike(target, weapon_damage_type, sSpellMgr.getSpellInfo(sp), adddmg, pct_dmg_mod, exclusive_damage, false, false);
    return 0;
}

int LuaUnit::SetAttackTimer(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    const int32_t timer = static_cast<int32_t>(luaL_checkinteger(L, 1));
    const bool offhand = CHECK_BOOL(L, 2);
    if (!timer)
        return 0;
    ptr->setAttackTimer(offhand == true ? OFFHAND : MELEE, timer);
    return 0;
}

int LuaUnit::Kill(lua_State* L, Unit* ptr)
{
    Unit* target = CHECK_UNIT(L, 1);
    if (!ptr || !target)
        return 0;
    ptr->dealDamage(target, target->getHealth(), 0);
    return 0;
}

int LuaUnit::DealDamage(lua_State* L, Unit* ptr)
{
    Unit* target = CHECK_UNIT(L, 1);
    const uint32_t damage = CHECK_ULONG(L, 2);
    const uint32_t spellid = CHECK_ULONG(L, 3);
    if (!ptr || !target)
        return 0;
    ptr->dealDamage(target, damage, spellid, false);
    return 0;
}

int LuaUnit::setCurrentTarget(lua_State* L, Unit* ptr)
{
    Unit* target = CHECK_UNIT(L, 1);
    if (ptr && target)
        ptr->getAIInterface()->setCurrentTarget(target);
    return 0;
}

int LuaUnit::getCurrentTarget(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    PUSH_UNIT(L, ptr->getAIInterface()->getCurrentTarget());
    return 1;
}

int LuaUnit::SetPetOwner(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    Unit* owner = CHECK_UNIT(L, 1);
    if (owner)
        ptr->getAIInterface()->setPetOwner(owner);
    return 0;
}

int LuaUnit::DismissPet(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    // DissmissPet in AIInterface got deleted
    return 0;
}

int LuaUnit::IsPet(lua_State* L, Unit* ptr)
{
    if (ptr)
    {
        if (ptr->isPet())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    }
    return 1;
}

int LuaUnit::GetPetOwner(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    PUSH_UNIT(L, ptr->getAIInterface()->getPetOwner());
    return 1;
}

int LuaUnit::SetUnitToFollow(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    Unit* target = CHECK_UNIT(L, 1);
    const float dist = CHECK_FLOAT(L, 2);
    const float angle = CHECK_FLOAT(L, 3);

    ptr->getMovementManager()->moveFollow(target, dist, angle);
    return 0;
}

int LuaUnit::IsInFront(lua_State* L, Unit* ptr)
{
    Object* target = CHECK_OBJECT(L, 1);
    if (ptr && target)
    {
        if (ptr->isInFront(target))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    }
    return 1;
}

int LuaUnit::IsInBack(lua_State* L, Unit* ptr)
{
    Object* target = CHECK_OBJECT(L, 1);
    if (ptr && target)
    {
        if (ptr->isInBack(target))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    }
    return 1;
}

int LuaUnit::IsPacified(lua_State* L, Unit* ptr)
{
    if (ptr)
        lua_pushboolean(L, (ptr->isPacified()) ? 1 : 0);
    return 1;
}

int LuaUnit::SetPacified(lua_State* L, Unit* ptr)
{
    const bool pacified = CHECK_BOOL(L, 1);
    if (!ptr)
        return 0;
    ptr->m_pacified = pacified ? 1 : 0;
    if (pacified)
        ptr->addUnitFlags(UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
    else
        ptr->removeUnitFlags(UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
    return 0;
}

int LuaUnit::IsFeared(lua_State* L, Unit* ptr)
{
    if (ptr)
        lua_pushboolean(L, (ptr->isFeared()) ? 1 : 0);
    return 1;
}

int LuaUnit::IsStunned(lua_State* L, Unit* ptr)
{
    if (ptr)
        lua_pushboolean(L, (ptr->isStunned()) ? 1 : 0);
    return 1;
}

int LuaUnit::CreateGuardian(lua_State* L, Unit* ptr)
{
    const uint32_t entry = CHECK_ULONG(L, 1);
    //uint32_t duration = CHECK_ULONG(L, 2);
    const float angle = CHECK_FLOAT(L, 3);
    const uint32_t lvl = CHECK_ULONG(L, 4);

    if ((ptr == nullptr) || (entry == 0) || (lvl == 0))
        return 0;

    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(entry);
    if (cp == nullptr)
        return 0;

    LocationVector v(ptr->GetPosition());
    v.x += (3 * (cosf(angle + v.o)));
    v.y += (3 * (sinf(angle + v.o)));

    Summon* guardian = ptr->getWorldMap()->summonCreature(entry, v);
    if (guardian == nullptr)
        return 0;

    guardian->load(cp, ptr, v, 0, 0);
    guardian->PushToWorld(ptr->getWorldMap());

    PUSH_UNIT(L, guardian);

    return 1;
}

int LuaUnit::IsInArc(lua_State* L, Unit* ptr)
{
    Unit* target = CHECK_UNIT(L, 1);
    const float degrees = CHECK_FLOAT(L, 2);
    if (!target || !ptr || !degrees)
        return 0;

    if (ptr->isInArc(target, degrees))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

int LuaUnit::IsInWater(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    if (dynamic_cast<Player*>(ptr)->m_underwaterState)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

int LuaUnit::GetAITargetsCount(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    lua_pushnumber(L, static_cast<lua_Number>(ptr->getThreatManager().getThreatListSize()));
    return 1;
}

int LuaUnit::GetUnitByGUID(lua_State* L, Unit* ptr)
{
    const uint64_t guid = CHECK_GUID(L, 1);
    if (ptr && guid)
        PUSH_UNIT(L, ptr->getWorldMap()->getUnit(guid));
    return 1;
}

int LuaUnit::GetAITargets(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    Unit* ret = nullptr;
    lua_newtable(L);
    int count = 0;
    for (const ThreatReference* ref : ptr->getThreatManager().getModifiableThreatList())
    {
        ret = ptr->getWorldMap()->getUnit(ref->getOwner()->getGuid());
        count++;
        lua_pushvalue(L, count);
        PUSH_UNIT(L, ret);
        lua_rawset(L, -3);
    }
    return 1;
}

int LuaUnit::GetInRangeObjectsCount(lua_State* L, Unit* ptr)
{
    if (ptr)
        lua_pushnumber(L, static_cast<lua_Number>(ptr->getInRangeObjectsCount()));
    return 1;
}

int LuaUnit::GetInRangePlayers(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;
    uint32_t count = 0;
    lua_newtable(L);
    for (const auto& itr : ptr->getInRangePlayersSet())
    {
        if (itr && itr->isPlayer())
        {
            count++;
            lua_pushinteger(L, count);
            PUSH_UNIT(L, itr);
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaUnit::GetGroupPlayers(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    Player* _player = dynamic_cast<Player*>(ptr);

    uint32_t count = 0;
    lua_newtable(L);
    if (const auto group = _player->getGroup())
    {
        group->getLock().lock();
        for (uint32_t i = 0; i < group->GetSubGroupCount(); i++)
        {
            const SubGroup* sgrp = group->GetSubGroup(i);
            for (const auto itr : sgrp->getGroupMembers())
            {
                if (Player* loggedInPlayer = sObjectMgr.getPlayer(itr->guid))
                {
                    if (loggedInPlayer->getZoneId() == _player->getZoneId() && _player->GetInstanceID() == loggedInPlayer->GetInstanceID())
                    {
                        count++;
                        lua_pushinteger(L, count);
                        PUSH_UNIT(L, loggedInPlayer);
                        lua_rawset(L, -3);
                    }
                }
            }
        }
        group->getLock().unlock();
    }
    return 1;
}

int LuaUnit::GetDungeonDifficulty(lua_State* L, Unit* ptr)
{
    /*
    MODE_NORMAL_10MEN   =   0,
    MODE_NORMAL_25MEN   =   1,
    MODE_HEROIC_10MEN   =   2,
    MODE_HEROIC_25MEN   =   3
    */
    if (ptr->isPlayer())
    {
        Player* plr = dynamic_cast<Player*>(ptr);
        if (plr->getGroup())
        {
            if (plr->getGroup()->getGroupType() == GROUP_TYPE_PARTY)
                lua_pushnumber(L, plr->getGroup()->m_difficulty);
            else
                lua_pushnumber(L, plr->getGroup()->m_raiddifficulty);
        }
        else
        {
            if (!plr->isInInstance())
                return 0;
            const WorldMap* pInstance = sMapMgr.findWorldMap(plr->GetMapId(), plr->GetInstanceID());
            lua_pushinteger(L, pInstance->getDifficulty());
        }
        return 1;
    }

    if (!ptr->isInInstance())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    const WorldMap* pInstance = sMapMgr.findWorldMap(ptr->GetMapId(), ptr->GetInstanceID());
    lua_pushinteger(L, pInstance->getDifficulty());

    return 1;
}

int LuaUnit::IsGroupFull(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->getGroup())
        lua_pushboolean(L, plr->getGroup()->IsFull() ? 1 : 0);

    return 1;
}

int LuaUnit::GetGroupLeader(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->getGroup())
        PUSH_UNIT(L, sObjectMgr.getPlayer(plr->getGroup()->GetLeader()->guid));
    return 1;
}

int LuaUnit::SetGroupLeader(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* _plr = CHECK_PLAYER(L, 1);
    const bool silent = CHECK_BOOL(L, 2);
    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->getGroup())
        plr->getGroup()->SetLeader(_plr, silent);
    return 0;
}

int LuaUnit::AddGroupMember(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    const Player* _plr = CHECK_PLAYER(L, 1);
    const int32_t subgroup = static_cast<int32_t>(luaL_optinteger(L, 2, -1));
    if (plr->getGroup())
        plr->getGroup()->AddMember(_plr->getPlayerInfo(), subgroup);
    return 0;
}

int LuaUnit::SetDungeonDifficulty(lua_State* L, Unit* ptr)
{
    /*
    MODE_NORMAL_10MEN   =   0,
    MODE_NORMAL_25MEN   =   1,
    MODE_HEROIC_10MEN   =   2,
    MODE_HEROIC_25MEN   =   3
    */
    const uint8_t difficulty = static_cast<uint8_t>(CHECK_ULONG(L, 1));
    if (!ptr)
        return 0;
    if (ptr->isInInstance())
    {
        if (ptr->isPlayer())
        {
            Player* plr = dynamic_cast<Player*>(ptr);
            if (plr->getGroup())
                (difficulty > 1 ? plr->getGroup()->m_difficulty : plr->getGroup()->m_raiddifficulty) = difficulty;
            else
            {
                WorldMap* pInstance = sMapMgr.findWorldMap(plr->GetMapId(), plr->GetInstanceID());
                pInstance->setSpawnMode(difficulty);
            }
        }
        else
        {
            WorldMap* pInstance = sMapMgr.findWorldMap(ptr->GetMapId(), ptr->GetInstanceID());
            pInstance->setSpawnMode(difficulty);
        }
    }
    return 0;
}

int LuaUnit::ExpandToRaid(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->getGroup())
        plr->getGroup()->ExpandToRaid();
    return 0;
}

int LuaUnit::GetInRangeGameObjects(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    lua_newtable(L);
    uint32_t count = 0;
    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        if (itr && itr->isGameObject())
        {
            count++;
            lua_pushinteger(L, count);
            PUSH_GO(L, itr);
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaUnit::HasInRangeObjects(lua_State* L, Unit* ptr)
{
    if (ptr)
    {
        if (ptr->hasInRangeObjects())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    }
    return 1;
}

int LuaUnit::SetFacing(lua_State* L, Unit* ptr)
{
    const float newo = CHECK_FLOAT(L, 1);
    if (!ptr)
        return 0;
    ptr->setFacing(newo);
    return 0;
}

int LuaUnit::CalcToDistance(lua_State* L, Unit* ptr)
{
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float z = CHECK_FLOAT(L, 3);
    if (!ptr || !x || !y || !z)
        return 0;
    lua_pushnumber(L, ptr->CalcDistance(x, y, z));
    return 1;
}

int LuaUnit::CalcAngle(lua_State* L, Unit* ptr)
{
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float x2 = CHECK_FLOAT(L, 3);
    const float y2 = CHECK_FLOAT(L, 4);
    if (!x || !y || !x2 || !y2 || !ptr)
        return 0;
    lua_pushnumber(L, ptr->calcAngle(x, y, x2, y2));
    return 1;
}

int LuaUnit::CalcRadAngle(lua_State* L, Unit* ptr)
{
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float dx = CHECK_FLOAT(L, 3);
    const float dy = CHECK_FLOAT(L, 4);

    if (!x || !y || !dx || !dy || !ptr)
        return 0;

    const float ang = ptr->calcRadAngle(x, y, dx, dy);
    lua_pushnumber(L, ang);

    return 1;
}

int LuaUnit::IsInvisible(lua_State* /*L*/, Unit* ptr)   //THIS IS NOT "IS" IT'S SET!
{
    if (!ptr)
        return 0;
    //bool enabled = CHECK_BOOL(L, 1);
    // TODO: remove this
    return 0;
}

int LuaUnit::MoveFly(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const bool enabled = CHECK_BOOL(L, 1);
    ptr->setMoveCanFly(enabled);
    return 0;
}

int LuaUnit::IsInvincible(lua_State* L, Unit* ptr)   //THIS IS NOT "IS" IT'S SET!
{
    const bool enabled = CHECK_BOOL(L, 1);
    if (ptr)
        ptr->m_isInvincible = enabled;
    return 0;
}

int LuaUnit::ResurrectPlayer(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    dynamic_cast<Player*>(ptr)->setResurrect();
    return 0;
}

int LuaUnit::KickPlayer(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const uint32_t delay = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    dynamic_cast<Player*>(ptr)->kickFromServer(delay);
    return 0;
}

int LuaUnit::CanCallForHelp(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const bool enabled = CHECK_BOOL(L, 1);
    ptr->getAIInterface()->setCanCallForHelp(enabled);
    return 0;
}

int LuaUnit::CallForHelpHp(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    const float hp = CHECK_FLOAT(L, 1);
    ptr->getAIInterface()->setCallForHelpHealth(hp);
    return 0;
}

int LuaUnit::SetDeathState(lua_State* L, Unit* ptr)
{
    const int state = static_cast<int>(luaL_checkinteger(L, 1));
    if (ptr)
    {
        switch (state)
        {
            case 0:
                ptr->setDeathState(ALIVE);
                break;
            case 1:
                ptr->setDeathState(JUST_DIED);
                break;
            case 2:
                ptr->setDeathState(CORPSE);
                break;
            case 3:
                ptr->setDeathState(DEAD);
                break;
        }
    }
    return 0;
}

int LuaUnit::SetCreatureName(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
        return 0;

    return 0;
}

int LuaUnit::SetBindPoint(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    Player* plr = dynamic_cast<Player*>(ptr);
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float z = CHECK_FLOAT(L, 3);
    const float o = CHECK_FLOAT(L, 4);
    const uint32_t map = CHECK_ULONG(L, 5);
    const uint32_t zone = CHECK_ULONG(L, 6);
    if (!x || !y || !z || !o || !zone)
        return 0;

    plr->setBindPoint(x, y, z, o, map, zone);
    return 0;
}

int LuaUnit::SoftDisconnect(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    dynamic_cast<Player*>(ptr)->softDisconnect();
    return 0;
}

int LuaUnit::Possess(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    Unit* target = CHECK_UNIT(L, 1);
    if (target)
        dynamic_cast<Player*>(ptr)->possess(target);

    return 0;
}

int LuaUnit::Unpossess(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    dynamic_cast<Player*>(ptr)->unPossess();
    return 0;
}

int LuaUnit::RemoveFromWorld(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    Creature* unit = dynamic_cast<Creature*>(ptr);
    if (unit->IsInWorld())
    {
        if (unit->m_spawn)
        {
            const uint32_t cellx = uint32_t(((Map::Terrain::_maxX - unit->m_spawn->x) / Map::Cell::cellSize));
            const uint32_t celly = uint32_t(((Map::Terrain::_maxY - unit->m_spawn->y) / Map::Cell::cellSize));

            if (cellx <= Map::Cell::_sizeX && celly <= Map::Cell::_sizeY)
            {
                CellSpawns* sp = unit->getWorldMap()->getBaseMap()->getSpawnsList(cellx, celly);
                if (sp != nullptr)
                {
                    for (CreatureSpawnList::iterator itr = sp->CreatureSpawns.begin(); itr != sp->CreatureSpawns.end(); ++itr)
                        if ((*itr) == unit->m_spawn)
                        {
                            sp->CreatureSpawns.erase(itr);
                            break;
                        }
                }
                delete unit->m_spawn;
                unit->m_spawn = nullptr;
            }
        }
        unit->RemoveFromWorld(false, true);
    }
    return 0;
}

int LuaUnit::GetFaction(lua_State* L, Unit* ptr)
{
    if (ptr)
        lua_pushnumber(L, ptr->getFactionTemplate());
    return 1;
}

int LuaUnit::SpellNonMeleeDamageLog(lua_State* L, Unit* ptr)
{
    Unit* pVictim = CHECK_UNIT(L, 1);
    const uint32_t spellid = CHECK_ULONG(L, 2);
    const uint32_t damage = CHECK_ULONG(L, 3);
    const uint8_t effIndex = CHECK_UINT8(L, 4);
    const bool isTriggered = CHECK_BOOL(L, 5);
    if (pVictim && spellid && damage)
    {
        ptr->doSpellDamage(pVictim, spellid, static_cast<float_t>(damage), effIndex, isTriggered);
    }
    return 0;
}

int LuaUnit::NoRespawn(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    const bool enabled = CHECK_BOOL(L, 1);
    dynamic_cast<Creature*>(ptr)->m_noRespawn = enabled;
    return 0;
}

int LuaUnit::GetMapId(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;
    lua_pushnumber(L, ptr->GetMapId());
    return 1;
}

int LuaUnit::AttackReaction(lua_State* L, Unit* ptr)
{
    Unit* target = CHECK_UNIT(L, 1);
    const uint32_t damage = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    //uint32_t spell = static_cast<uint32_t>(luaL_checkinteger(L, 3));
    if (ptr && target && damage)
    {
        ptr->getAIInterface()->onHostileAction(target);
        ptr->getThreatManager().addThreat(target, static_cast<float>(damage));
    }
    return 0;
}

int LuaUnit::eventCastSpell(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    Unit* target = CHECK_UNIT(L, 1);
    const uint32_t sp = CHECK_ULONG(L, 2);
    const uint32_t delay = CHECK_ULONG(L, 3);
    const uint32_t repeats = CHECK_ULONG(L, 4);
    if (sp)
    {
        switch (ptr->getObjectTypeId())
        {
            case TYPEID_PLAYER:
                sEventMgr.AddEvent(ptr, &Player::eventCastSpell, target, sSpellMgr.getSpellInfo(sp), EVENT_PLAYER_UPDATE, delay, repeats, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                break;
            case TYPEID_UNIT:
                sEventMgr.AddEvent(ptr, &Unit::eventCastSpell, target, sSpellMgr.getSpellInfo(sp), EVENT_CREATURE_UPDATE, delay, repeats, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                break;
        }
    }
    return 0;
}

int LuaUnit::IsPlayerMoving(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    lua_pushboolean(L, (dynamic_cast<Player*>(ptr)->isMoving()) ? 1 : 0);
    return 1;
}

int LuaUnit::IsPlayerAttacking(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (dynamic_cast<Player*>(ptr)->isAttacking())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::GetFactionStanding(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t faction = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (faction)
    {
        switch (dynamic_cast<Player*>(ptr)->getFactionStandingRank(faction))
        {
            case STANDING_HATED:
                lua_pushstring(L, "Hated");
                break;
            case STANDING_HOSTILE:
                lua_pushstring(L, "Hostile");
                break;
            case STANDING_UNFRIENDLY:
                lua_pushstring(L, "Unfriendly");
                break;
            case STANDING_NEUTRAL:
                lua_pushstring(L, "Neutral");
                break;
            case STANDING_FRIENDLY:
                lua_pushstring(L, "Friendly");
                break;
            case STANDING_HONORED:
                lua_pushstring(L, "Honored");
                break;
            case STANDING_REVERED:
                lua_pushstring(L, "Revered");
                break;
            case STANDING_EXALTED:
                lua_pushstring(L, "Exalted");
                break;
        }
    }
    return 1;
}

int LuaUnit::SetPlayerAtWar(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    const uint32_t faction = CHECK_ULONG(L, 1);
    const bool set = CHECK_BOOL(L, 3);
    if (faction)
    {
        dynamic_cast<Player*>(ptr)->setFactionAtWar(faction, set);
    }
    return 0;
}

int LuaUnit::SetPlayerStanding(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t faction = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const int32_t value = static_cast<int32_t>(luaL_checkinteger(L, 2));
    if (faction && value)
        dynamic_cast<Player*>(ptr)->setFactionStanding(faction, value);
    return 0;
}

int LuaUnit::SetPlayerSpeed(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    const float Speed = CHECK_FLOAT(L, 1);
    if (Speed < 1 || Speed > 255)
        return 0;
    plr->setSpeedRate(TYPE_RUN, Speed, true);
    plr->setSpeedRate(TYPE_SWIM, Speed, true);
    plr->setSpeedRate(TYPE_RUN_BACK, Speed / 2, true);
    plr->setSpeedRate(TYPE_FLY, Speed * 2, true);
    return 0;
}

int LuaUnit::GiveHonor(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    const uint32_t honor = CHECK_ULONG(L, 1);
    plr->addHonor(honor, true);
    return 0;
}

int LuaUnit::TakeHonor(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    const uint32_t honor = CHECK_ULONG(L, 1);
    plr->removeHonor(honor, true);
    return 0;
}

int LuaUnit::GetStanding(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t faction = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (faction)
        lua_pushinteger(L, dynamic_cast<Player*>(ptr)->getFactionStanding(faction));
    return 1;
}

int LuaUnit::RemoveThreatByPtr(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    Unit* target = CHECK_UNIT(L, 1);
    if (target)
        ptr->getThreatManager().clearThreat(target);
    return 0;
}

int LuaUnit::HasItem(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t itemid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (itemid)
    {
        if (dynamic_cast<Player*>(ptr)->getItemInterface()->GetItemCount(itemid, false) > 0)
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    }
    return 1;
}

int LuaUnit::PlaySpellVisual(lua_State* L, Unit* ptr)
{
    const uint32_t visualId = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint32_t type = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (ptr && visualId)
    {
        ptr->playSpellVisual(visualId, type);
    }
    return 1;
}

int LuaUnit::GetLevel(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;
    lua_pushinteger(L, ptr->getLevel());
    return 1;
}

int LuaUnit::SetLevel(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;
    const uint32_t level = CHECK_ULONG(L, 1);
    if (level <= worldConfig.player.playerLevelCap && level > 0)
    {
        if (ptr->isPlayer())
            dynamic_cast<Player*>(ptr)->applyLevelInfo(level);
        else
            ptr->setLevel(level);
    }
    return 0;
}

int LuaUnit::AddSkill(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t skill = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint32_t current = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    uint32_t max = static_cast<uint32_t>(luaL_checkinteger(L, 3));
    Player* plr = dynamic_cast<Player*>(ptr);
    if (!max)
        max = 475;
    if (current > max)
        return 0;
    if (!plr->hasSkillLine(skill))
        plr->addSkillLine(skill, current, max);
    return 0;
}

int LuaUnit::RemoveSkill(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t skill = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (!skill)
        return 0;
    Player* plr = dynamic_cast<Player*>(ptr);
    plr->removeSkillLine(skill);
    return 0;
}

int LuaUnit::FlyCheat(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const bool enabled = CHECK_BOOL(L, 1);
    dynamic_cast<Player*>(ptr)->m_cheats.hasFlyCheat = enabled;
    return 0;
}

int LuaUnit::AdvanceSkill(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t skill = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint32_t count = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    Player* plr = dynamic_cast<Player*>(ptr);
    if (skill && count)
    {
        if (plr->hasSkillLine(skill))
            plr->advanceSkillLine(skill, count);
    }
    return 0;
}

int LuaUnit::RemoveAurasByMechanic(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    const auto mechanic = static_cast<SpellMechanic>(luaL_checkinteger(L, 1));
    const bool hostileonly = CHECK_BOOL(L, 2);
    if (mechanic)
        ptr->removeAllAurasBySpellMechanic(mechanic, hostileonly);
    return 0;
}

int LuaUnit::RemoveAurasType(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    const auto type = static_cast<AuraEffect>(luaL_checkinteger(L, 1));
    if (type)
        ptr->removeAllAurasByAuraEffect(type);
    return 0;
}

int LuaUnit::AddAura(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    const uint32_t spellid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const int32_t duration = static_cast<int32_t>(luaL_checkinteger(L, 2));
    const bool temp = CHECK_BOOL(L, 3);
    if (spellid)
    {
        auto aura = sSpellMgr.newAura(sSpellMgr.getSpellInfo(spellid), duration, ptr, ptr, temp);
        ptr->addAura(std::move(aura));
        lua_pushboolean(L, 1);
    }
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::SetStealth(lua_State* /*L*/, Unit* ptr)
{
    if (!ptr)
        return 0;
    // TODO: remove this!
    return 0;
}

int LuaUnit::GetStealthLevel(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;
    uint32_t stealthFlag = CHECK_ULONG(L, 1);
    lua_pushinteger(L, ptr->getStealthLevel(StealthFlag(stealthFlag)));
    return 1;
}

int LuaUnit::IsStealthed(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;
    if (ptr->isStealthed())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::RemoveStealth(lua_State* /*L*/, Unit* ptr)
{
    if (!ptr)
        return 0;
    ptr->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);
    return 0;
}

int LuaUnit::InterruptSpell(lua_State* /*L*/, Unit* ptr)
{
    if (!ptr)
        return 0;
    ptr->interruptSpell();
    return 0;
}

int LuaUnit::IsPoisoned(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;
    if (ptr->isPoisoned())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::ModifyAIUpdateEvent(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    const time_t newtime = static_cast<time_t>(luaL_checkinteger(L, 1));
    sEventMgr.ModifyEventTimeAndTimeLeft(ptr, EVENT_SCRIPT_UPDATE_EVENT, newtime);
    return 0;
}

int LuaUnit::RemoveAIUpdateEvent(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    sEventMgr.RemoveEvents(ptr, EVENT_SCRIPT_UPDATE_EVENT);
    return 0;
}

int LuaUnit::DealGoldCost(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    const uint32_t debt = static_cast<uint32_t>(luaL_checkinteger(L, 1));

    if (!plr->hasEnoughCoinage(debt))
    {
        lua_pushboolean(L, 0);
    }
    else
    {
        plr->modCoinage(-(int32_t)debt);
        lua_pushboolean(L, 1);
    }
    return 1;
}

int LuaUnit::DealGoldMerit(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t profit = CHECK_ULONG(L, 1);
    dynamic_cast<Player*>(ptr)->modCoinage(profit);
    return 0;
}

int LuaUnit::DeMorph(lua_State* /*L*/, Unit* ptr)
{
    if (!ptr)
        return 0;
    ptr->deMorph();
    return 0;
}

int LuaUnit::Attack(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    Unit* target = CHECK_UNIT(L, 1);
    if (target && ptr->getThreatManager().canHaveThreatList())
    {
        auto& threatManager = ptr->getThreatManager();
        if (threatManager.getCurrentVictim() == target)
        {
            // Unit is already attacking this target
            lua_pushboolean(L, 0);
            return 1;
        }

        if (!threatManager.isThreatListEmpty())
        {
            threatManager.addThreat(target, 20.0f);
            // Set threat equal to highest threat currently on target
            threatManager.matchUnitThreatToHighestThreat(target);
        }

        lua_pushboolean(L, 1);
    }
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::CanUseCommand(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char cmdlevel = (char)luaL_checkstring(L, 1)[0];
    const Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->getSession()->CanUseCommand(cmdlevel))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::GetSelection(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    Unit* selection = plr->getWorldMap()->getUnit(plr->getTargetGuid());
    if (selection)
        PUSH_UNIT(L, selection);
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetSelectedGO(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    PUSH_GO(L, plr->getSelectedGo());
    return 1;
}

int LuaUnit::SetSelectedGO(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    const GameObject* newsel = CHECK_GO(L, 1);
    if (!newsel)
        return 0;
    plr->setSelectedGo(newsel->getGuid());
    return 0;
}

int LuaUnit::RepairAllPlayerItems(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    Player* plr = dynamic_cast<Player*>(ptr);
    Item* pItem = nullptr;
    Container* pContainer = nullptr;
    uint16_t j;
    uint16_t i;

    for (i = 0; i < MAX_INVENTORY_SLOT; i++)
    {
        pItem = plr->getItemInterface()->GetInventoryItem(i);
        if (pItem != nullptr)
        {
            if (pItem->isContainer())
            {
                pContainer = dynamic_cast<Container*>(pItem);
                for (j = 0; j < pContainer->getItemProperties()->ContainerSlots; ++j)
                {
                    pItem = pContainer->getItem(j);
                    if (pItem != nullptr)
                    {
                        pItem->setDurabilityToMax();
                    }
                }
            }
            else
            {
                if (pItem->getItemProperties()->MaxDurability > 0 && i < INVENTORY_SLOT_BAG_END && pItem->getDurability() <= 0)
                {
                    pItem->setDurabilityToMax();
                    plr->applyItemMods(pItem, i, true);
                }
                else
                {
                    pItem->setDurabilityToMax();
                }
            }
        }
    }
    return 0;
}

int LuaUnit::SetKnownTitle(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    int title = static_cast<int>(luaL_checkinteger(L, 1));
    Player* plr = dynamic_cast<Player*>(ptr);
    plr->setKnownPvPTitle(RankTitles(title), true);
    plr->saveToDB(false);
    return 0;
}

int LuaUnit::UnsetKnownTitle(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    int title = static_cast<int>(luaL_checkinteger(L, 1));
    Player* plr = dynamic_cast<Player*>(ptr);
    plr->setKnownPvPTitle(RankTitles(title), false);
    plr->saveToDB(false);
    return 0;
}

int LuaUnit::LifeTimeKills(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const int kills = static_cast<int>(luaL_checkinteger(L, 1));
    const char* check = luaL_checklstring(L, 2, nullptr);
    Player* plr = dynamic_cast<Player*>(ptr);
    const int killscheck = plr->getLifetimeHonorableKills();
    if (check && strncmp(check, "add", 4) == 0 && kills > 0)
    {
        plr->setLifetimeHonorableKills(killscheck + kills);
        plr->saveToDB(false);
        return 0;
    }
    if (check && strncmp(check, "del", 4) == 0 && killscheck >= kills)
    {
        plr->setLifetimeHonorableKills(killscheck - kills);
        plr->saveToDB(false);
        return 0;
    }
    if (check && strncmp(check, "set", 4) == 0 && kills >= 0)
    {
        plr->setLifetimeHonorableKills(kills);
        plr->saveToDB(false);
        return 0;
    }
    if (check == nullptr || kills == 0)
    {
        lua_pushinteger(L, killscheck);
        return 1;
    }
    return 0;
}

int LuaUnit::HasTitle(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    int title = static_cast<int>(luaL_checkinteger(L, 1));
    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->hasPvPTitle(RankTitles(title)))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::GetMaxSkill(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t skill = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    lua_pushinteger(L, dynamic_cast<Player*>(ptr)->getSkillLineMax(skill));
    return 1;
}

int LuaUnit::GetCurrentSkill(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t skill = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    lua_pushinteger(L, dynamic_cast<Player*>(ptr)->getSkillLineCurrent(skill));
    return 1;
}

int LuaUnit::HasSkill(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t skill = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    lua_pushboolean(L, (dynamic_cast<Player*>(ptr)->hasSkillLine(skill)) ? 1 : 0);
    return 1;
}

int LuaUnit::GetGuildName(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Guild* pGuild = sGuildMgr.getGuildById(dynamic_cast<Player*>(ptr)->getGuildId());
    if (pGuild != nullptr)
        lua_pushstring(L, pGuild->getNameChar());
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::ClearCooldownForSpell(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    plr->clearCooldownForSpell(static_cast<uint32_t>(luaL_checkinteger(L, 1)));
    return 0;
}

int LuaUnit::HasSpell(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t sp = CHECK_ULONG(L, 1);
    lua_pushboolean(L, (sp && dynamic_cast<Player*>(ptr)->hasSpell(sp)) ? 1 : 0);
    return 1;
}

int LuaUnit::ClearAllCooldowns(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    dynamic_cast<Player*>(ptr)->resetAllCooldowns();
    return 0;
}

int LuaUnit::ResetAllTalents(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    dynamic_cast<Player*>(ptr)->resetAllTalents();
    return 0;
}

int LuaUnit::GetAccountName(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* aName = dynamic_cast<Player*>(ptr)->getSession()->GetAccountNameS();
    lua_pushstring(L, aName);
    return 1;
}

int LuaUnit::GetGmRank(lua_State* L, Unit* ptr)
{
    // TODO: possibly needs rewrite of LuaEngine to handle unique_ptr<T> -Appled

    /*if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* level = dynamic_cast<Player*>(ptr)->getSession()->GetPermissions();
    if (level != nullptr)
        lua_pushstring(L, level);
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaUnit::IsGm(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    if (dynamic_cast<Player*>(ptr)->getSession()->HasGMPermissions())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::SavePlayer(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    dynamic_cast<Player*>(ptr)->saveToDB(false);
    return 0;
}

int LuaUnit::HasQuest(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t quest_id = CHECK_ULONG(L, 1);
    if (quest_id && dynamic_cast<Player*>(ptr)->hasQuestInQuestLog(quest_id))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::CreatureHasQuest(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    Creature* ctr = dynamic_cast<Creature*>(ptr);
    const uint32_t questid = CHECK_ULONG(L, 1);
    QuestProperties const* qst = sMySQLStore.getQuestProperties(questid);
    if (ctr->HasQuest(qst->id, qst->type))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::RemovePvPFlag(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->isPvpFlagSet())
        plr->removePvpFlag();
    return 0;
}

int LuaUnit::RemoveNegativeAuras(lua_State* /*L*/, Unit* ptr)
{
    if (!ptr)
        return 0;
    ptr->removeAllNegativeAuras();
    return 0;
}

int LuaUnit::GossipMiscAction(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    Player* plr = dynamic_cast<Player*>(ptr);
    Creature* crc = dynamic_cast<Creature*>(CHECK_UNIT(L, 2));
    const uint32_t miscint = static_cast<uint32_t>(luaL_checkinteger(L, 3));
    const uint32_t actionid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (!crc && actionid < 9)
        return 0;
    if (actionid == 1) plr->getSession()->sendInventoryList(crc);
    else if (actionid == 2) plr->getSession()->sendTrainerList(crc);
    else if (actionid == 3) plr->getSession()->sendInnkeeperBind(crc);
    else if (actionid == 4) plr->getSession()->sendBankerList(crc);
    else if (actionid == 5) plr->getSession()->sendBattlegroundList(crc, miscint);
    else if (actionid == 6) plr->getSession()->sendAuctionList(crc);
    else if (actionid == 7) plr->getSession()->sendTabardHelp(crc);
    else if (actionid == 8) plr->getSession()->sendSpiritHealerRequest(crc);
    else if (actionid == 9) plr->sendTalentResetConfirmPacket();
    else if (actionid == 10) plr->sendPetUnlearnConfirmPacket();
    return 0;
}

int LuaUnit::SendVendorWindow(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    Creature* object = dynamic_cast<Creature*>(CHECK_UNIT(L, 1)); // NOT entry. The unit pointer.
    if (object != nullptr)
        plr->getSession()->sendInventoryList(object);
    return 0;
}

int LuaUnit::SendTrainerWindow(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    Creature* crc = dynamic_cast<Creature*>(CHECK_UNIT(L, 1)); // NOT entry. The unit pointer.
    if (crc != nullptr)
        plr->getSession()->sendTrainerList(crc);
    return 0;
}

int LuaUnit::SendInnkeeperWindow(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    Creature* crc = dynamic_cast<Creature*>(CHECK_UNIT(L, 1)); // NOT entry. The unit pointer.
    if (crc != nullptr)
        plr->getSession()->sendInnkeeperBind(crc);
    return 0;
}

int LuaUnit::SendBankWindow(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    Creature* crc = dynamic_cast<Creature*>(CHECK_UNIT(L, 1)); // NOT entry. The unit pointer.
    if (crc != nullptr)
        plr->getSession()->sendBankerList(crc);
    return 0;
}

int LuaUnit::SendAuctionWindow(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    Creature* crc = dynamic_cast<Creature*>(CHECK_UNIT(L, 1)); // NOT entry. The unit pointer.
    if (crc != nullptr)
        plr->getSession()->sendAuctionList(crc);
    return 0;
}

int LuaUnit::SendBattlegroundWindow(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    Creature* crc = dynamic_cast<Creature*>(CHECK_UNIT(L, 1));
    const uint32_t bgid = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (bgid && crc != nullptr)
        plr->getSession()->sendBattlegroundList(crc, bgid); // player filler ftw
    return 0;
}

int LuaUnit::SendLootWindow(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint64_t guid = CHECK_GUID(L, 1);
    const uint8_t loot_type = (uint8_t)luaL_checkinteger(L, 2);
    uint8_t loot_type2 = 1;
    Player* plr = dynamic_cast<Player*>(ptr);
    plr->setLootGuid(guid);

    WoWGuid wowGuid;
    wowGuid.Init(guid);

    if (wowGuid.isUnit())
    {
        Unit* pUnit = plr->getWorldMap()->getUnit(guid);
        CreatureProperties const* creature_properties = dynamic_cast<Creature*>(pUnit)->GetCreatureProperties();
        const auto lootType = pUnit->getWorldMap() ? (pUnit->getWorldMap()->getDifficulty() ? true : false) : false;
        switch (loot_type)
        {
            default:
                sLootMgr.fillCreatureLoot(plr, &pUnit->loot, pUnit->getEntry(), lootType);
                pUnit->loot.gold = creature_properties ? creature_properties->money : 0;
                loot_type2 = 1;
                break;
            case 2:
                sLootMgr.fillSkinningLoot(plr, &pUnit->loot, pUnit->getEntry(), lootType);
                loot_type2 = 2;
                break;
            case 3:
                sLootMgr.fillPickpocketingLoot(plr, &pUnit->loot, pUnit->getEntry(), lootType);
                loot_type2 = 2;
                break;
        }
    }
    else if (wowGuid.isGameObject())
    {
        GameObject* pGO = plr->getWorldMap()->getGameObject(wowGuid.getGuidLowPart());
        if (pGO != nullptr && pGO->IsLootable())
        {
            GameObject_Lootable* lt = dynamic_cast<GameObject_Lootable*>(pGO);
            const auto lootType = pGO->getWorldMap() ? (pGO->getWorldMap()->getDifficulty() ? true : false) : false;
            switch (loot_type)
            {
                default:
                    sLootMgr.fillGOLoot(plr, &lt->loot, pGO->getEntry(), lootType);
                    loot_type2 = 1;
                    break;
                case 5:
                    sLootMgr.fillSkinningLoot(plr, &lt->loot, pGO->getEntry(), lootType);
                    loot_type2 = 2;
                    break;
            }
        }
    }
    else if (wowGuid.isItem())
    {
        const Item* pItem = plr->getItemInterface()->GetItemByGUID(guid);
        switch (loot_type)
        {
            case 6:
                sLootMgr.fillItemLoot(plr, pItem->m_loot.get(), pItem->getEntry(), plr->getWorldMap() ? (plr->getWorldMap()->getDifficulty() ? true : false) : false);
                loot_type2 = 1;
                break;
            default:
                break;
        }
    }
    plr->sendLoot(guid, 2, plr->GetMapId());
    return 0;
}

int LuaUnit::AddLoot(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    if ((lua_gettop(L) != 3) || (lua_gettop(L) != 5))
        return 0;

    const uint32_t itemid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint32_t mincount = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    const uint32_t maxcount = static_cast<uint32_t>(luaL_checkinteger(L, 3));
    std::vector<float> ichance;

    const float chance = CHECK_FLOAT(L, 5);

    for (uint8_t i = 0; i == 3; i++)
        ichance.push_back(chance);

    const bool perm = ((luaL_optinteger(L, 4, 0) == 1) ? true : false);
    if (perm)
    {
        const auto result = WorldDatabase.Query("SELECT * FROM loot_creatures WHERE entryid = %u, itemid = %u", ptr->getEntry(), itemid);
        if (!result)
            WorldDatabase.Execute("REPLACE INTO loot_creatures VALUES (%u, %u, %f, 0, 0, 0, %u, %u )", ptr->getEntry(), itemid, chance, mincount, maxcount);
    }
    sLootMgr.addLoot(&ptr->loot, itemid, ichance, mincount, maxcount, ptr->getWorldMap()->getDifficulty());
    return 0;
}

int LuaUnit::VendorAddItem(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    Creature* ctr = dynamic_cast<Creature*>(ptr);
    const uint32_t itemid = (uint32_t)luaL_checknumber(L, 1);
    const uint32_t amount = (uint32_t)luaL_checknumber(L, 2);
    const uint32_t costid = (uint32_t)luaL_checknumber(L, 3);

    const auto item_extended_cost = (costid > 0) ? sItemExtendedCostStore.lookupEntry(costid) : nullptr;
    if (itemid && amount)
        ctr->AddVendorItem(itemid, amount, item_extended_cost);

    return 0;
}

int LuaUnit::VendorRemoveItem(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    Creature* ctr = dynamic_cast<Creature*>(ptr);
    const uint32_t itemid = (uint32_t)luaL_checknumber(L, 1);
    const int slot = ctr->GetSlotByItemId(itemid);
    if (itemid && slot > 0)
        ctr->RemoveVendorItem(itemid);
    return 0;
}

int LuaUnit::VendorRemoveAllItems(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    Creature* ctr = dynamic_cast<Creature*>(ptr);
    uint32_t i = 0;
    if (ctr->HasItems())
    {
        uint32_t creatureitemids[200];
        const size_t count = ctr->GetSellItemCount();
        for (std::vector<CreatureItem>::iterator itr = ctr->GetSellItemBegin(); itr != ctr->GetSellItemEnd(); ++itr)
        {
            creatureitemids[i] = itr->itemid;
            i += 1;
        }
        for (i = 0; i < count; i++)
        {
            ctr->RemoveVendorItem(creatureitemids[i]);
        }
    }
    return 0;
}

int LuaUnit::EquipWeapons(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    const uint32_t equip1 = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint32_t equip2 = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    const uint32_t equip3 = static_cast<uint32_t>(luaL_checkinteger(L, 3));
    ptr->setVirtualItemSlotId(MELEE, equip1);
    ptr->setVirtualItemSlotId(OFFHAND, equip2);
    ptr->setVirtualItemSlotId(RANGED, equip3);
    return 0;
}

int LuaUnit::Dismount(lua_State* /*L*/, Unit* ptr)
{
    if (!ptr)
        return 0;
    if (ptr->isPlayer())
    {
        Player* plr = dynamic_cast<Player*>(ptr);
        plr->removeAllAurasById(plr->getMountSpellId());
        plr->setMountDisplayId(0);
    }
    else
        ptr->setMountDisplayId(0);
    return 0;
}

int LuaUnit::GiveXp(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* pl = dynamic_cast<Player*>(ptr);
    const uint32_t exp = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    pl->giveXp(exp, pl->getGuid(), true);
    return 0;
}

int LuaUnit::AdvanceAllSkills(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    const uint32_t skillvalue = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    plr->advanceAllSkills(skillvalue);
    return 0;
}

int LuaUnit::GetTeam(lua_State* L, Unit* ptr) // returns 0 for alliance, 1 for horde.
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    lua_pushinteger(L, plr->getTeam());
    return 1;
}

int LuaUnit::StartTaxi(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    const uint32_t pathId = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    plr->activateTaxiPathTo(pathId, ptr->ToCreature());
    return 0;
}

int LuaUnit::IsOnTaxi(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    lua_pushboolean(L, dynamic_cast<Player*>(ptr)->isOnTaxi() ? 1 : 0);
    return 1;
}

int LuaUnit::GetTaxi(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    PUSH_TAXIPATH(L, dynamic_cast<Player*>(ptr)->getTaxiData());
    return 1;
}

int LuaUnit::SetPlayerLock(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const bool lock = CHECK_BOOL(L, 1);
    if (lock)
    {
        ptr->m_pacified = 1;
        ptr->addUnitFlags(UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
        dynamic_cast<Player*>(ptr)->sendClientControlPacket(ptr, 0);
    }
    else
    {
        ptr->m_pacified = 0;
        ptr->removeUnitFlags(UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
        dynamic_cast<Player*>(ptr)->sendClientControlPacket(ptr, 1);
    }
    return 0;
}

int LuaUnit::MovePlayerTo(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float z = CHECK_FLOAT(L, 3);
    const float o = CHECK_FLOAT(L, 4);
    const uint32_t mov_flag = CHECK_ULONG(L, 5); //0 - walk, 256 - teleport, 4096 - run, 12288 - fly
    float moveSpeed = (float)luaL_optnumber(L, 6, 1.0f);
    if (moveSpeed == 1.0f)
    {
        if (mov_flag == 0)
            moveSpeed = 2.5f * 0.001f;
        else
            moveSpeed = 7.0f * 0.001f;
    }
    ptr->setFacing(o);
    ptr->SetOrientation(o);
    const float distance = ptr->CalcDistance(ptr->GetPositionX(), ptr->GetPositionY(), ptr->GetPositionZ(), x, y, z);
    const uint32_t moveTime = uint32_t(distance / moveSpeed);
    WorldPacket data(SMSG_MONSTER_MOVE, 50);
    data << ptr->GetNewGUID();
    data << uint8_t(0);
    data << ptr->GetPositionX();
    data << ptr->GetPositionY();
    data << ptr->GetPositionZ();
    data << Util::getMSTime();
    data << uint8_t(0x00);
    data << uint32_t(mov_flag);
    data << moveTime;
    data << uint32_t(1);
    data << x << y << z;

    ptr->sendMessageToSet(&data, true);
    ptr->SetPosition(x, y, z, o);
    return 0;
}

int LuaUnit::ChannelSpell(lua_State* L, Unit* ptr)
{
    const uint32_t Csp = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const Object* target = CHECK_OBJECT(L, 2);
    if (Csp && target != nullptr)
    {
        ptr->castSpell(target->getGuid(), sSpellMgr.getSpellInfo(Csp), false);
        ptr->setChannelObjectGuid(target->getGuid());
        ptr->setChannelSpellId(Csp);
    }
    return 0;
}

int LuaUnit::StopChannel(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;
    ptr->setChannelObjectGuid(0);
    ptr->setChannelSpellId(0);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// WORLDSTATES/WORLD PVP NOT SUPPORTED

/*
int LuaUnit::SetWorldState(lua_State * L, Unit * ptr)
{
    int zone = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);

    if(!zone || !index || !value)
        lua_pushnil(L);

    ptr->getWorldMap()->SetWorldState(zone, index, value);
    lua_pushboolean(L, 1);
    return 1;
}
*/

int LuaUnit::EnableFlight(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const bool enable_fly = CHECK_BOOL(L, 1);
    ptr->setMoveCanFly(enable_fly);
    return 0;
}

int LuaUnit::GetCoinage(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    lua_pushinteger(L, plr->getCoinage());
    return 1;
}

int LuaUnit::FlagPvP(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    plr->setPvpFlag();
    return 0;
}

int LuaUnit::IsMounted(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    if (ptr->isPlayer())
    {
        const Player* plr = dynamic_cast<Player*>(ptr);
        if (plr->isMounted())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    }
    else
        lua_pushboolean(L, (ptr->getMountDisplayId() > 0) ? 1 : 0);
    return 1;
}

// credits to alvanaar for the following 9 functions:
int LuaUnit::IsGroupedWith(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* target = CHECK_PLAYER(L, 1);
    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->getGroup() && plr->getGroup()->HasMember(target))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::GetGroupType(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    Player* plr = dynamic_cast<Player*>(ptr);
    if (const auto group = plr->getGroup())
        lua_pushinteger(L, group->getGroupType());
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetTotalHonor(lua_State* L, Unit* ptr) // I loathe typing "honour" like "honor".
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    lua_pushinteger(L, dynamic_cast<Player*>(ptr)->getHonor());
    return 1;
}

int LuaUnit::GetHonorToday(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    lua_pushinteger(L, dynamic_cast<Player*>(ptr)->getHonorToday());
    return 1;
}

int LuaUnit::GetHonorYesterday(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    lua_pushinteger(L, dynamic_cast<Player*>(ptr)->getHonorYesterday());
    return 1;
}

int LuaUnit::GetArenaPoints(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    lua_pushinteger(L, dynamic_cast<Player*>(ptr)->getArenaPoints());
    return 1;
}

int LuaUnit::AddArenaPoints(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t pnts = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    Player* plr = dynamic_cast<Player*>(ptr);
    if (pnts > 0)
    {
        plr->addArenaPoints(pnts, true);
    }
    return 0;
}

int LuaUnit::RemoveArenaPoints(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t pnts = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    Player* plr = dynamic_cast<Player*>(ptr);

    if (pnts > 0)
        plr->removeArenaPoints(pnts, true);

    return 0;
}

int LuaUnit::AddLifetimeKills(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t pnts = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    Player* plr = dynamic_cast<Player*>(ptr);
    plr->incrementKills(pnts);
    return 0;
}

int LuaUnit::GetGender(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;
    lua_pushinteger(L, ptr->getGender());
    return 1;
}

int LuaUnit::SetGender(lua_State* L, Unit* ptr)
{
    if (!ptr)
        return 0;

    const uint8_t gender = static_cast<uint8_t>(luaL_checkinteger(L, 1));
    ptr->setGender(gender);
    return 0;
}

int LuaUnit::SendPacketToGuild(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    WorldPacket* data = CHECK_PACKET(L, 1);
    const Player* plr = dynamic_cast<Player*>(ptr);
    const Guild* guild = nullptr;
    if (luaL_optinteger(L, 2, -1) > 0)
    {
        guild = sGuildMgr.getGuildById(static_cast<uint32_t>(luaL_optinteger(L, 2, 0)));
    }
    else
    {
        guild = plr->getGuild();
    }
    if (data != nullptr && guild != nullptr)
        guild->sendPacket(data);
    return 0;
}

int LuaUnit::GetGuildId(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->isInGuild())
        lua_pushinteger(L, plr->getGuildId());
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetGuildRank(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->isInGuild())
        lua_pushinteger(L, plr->getGuildRankFromDB());
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::SetGuildRank(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    const uint32_t rank = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (plr->isInGuild())
        plr->setGuildRank(rank);
    return 0;
}

int LuaUnit::IsInGuild(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->isInGuild())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::SendGuildInvite(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const auto sender = dynamic_cast<Player*>(ptr);
    const auto invitedPlayer = CHECK_PLAYER(L, 1);
    if (invitedPlayer != nullptr)
        sender->getGuild()->sendGuildInvitePacket(sender->getSession(), invitedPlayer->getName());

    return 0;
}

int LuaUnit::DemoteGuildMember(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const Player* target = CHECK_PLAYER(L, 1);
    if (target && plr->getGuild())
        plr->getGuild()->handleUpdateMemberRank(plr->getSession(), target->getGuid(), true);

    return 0;
}

int LuaUnit::PromoteGuildMember(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const Player* target = CHECK_PLAYER(L, 1);
    if (target && plr->getGuild())
        plr->getGuild()->handleUpdateMemberRank(plr->getSession(), target->getGuid(), false);

    return 0;
}

int LuaUnit::SetGuildMotd(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const char* szNewMotd = luaL_checkstring(L, 1);
    if (plr->getGuild() && szNewMotd != nullptr)
        plr->getGuild()->handleSetMOTD(plr->getSession(), szNewMotd);
    return 0;
}

int LuaUnit::GetGuildMotd(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const Guild* guild = nullptr;
    if (luaL_optinteger(L, 1, -1) >= 0)
        guild = sGuildMgr.getGuildById(static_cast<uint32_t>(luaL_optinteger(L, 1, -1)));
    else
        guild = plr->getGuild();

    if (guild != nullptr)
        lua_pushstring(L, guild->getMOTDChar());
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::SetGuildInformation(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const char* gi = luaL_checkstring(L, 1);
    if (gi && plr->getGuild())
        plr->getGuild()->handleSetInfo(plr->getSession(), gi);
    return 0;
}

int LuaUnit::AddGuildMember(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const uint32_t g_id = CHECK_ULONG(L, 1);
    const uint8_t rank = static_cast<uint8_t>(luaL_optinteger(L, 2, -1));
    Guild* target = sGuildMgr.getGuildById(g_id);
    if (target)
        target->addMember(plr->getGuid(), rank);
    return 0;
}

int LuaUnit::RemoveGuildMember(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const Player* target = CHECK_PLAYER(L, 1);
    if (target && plr->getGuild())
        plr->getGuild()->deleteMember(target->getGuid(), false);

    return 0;
}

int LuaUnit::SetPublicNote(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const Player* target = CHECK_PLAYER(L, 1);
    const char* note = luaL_checkstring(L, 2);
    if (target && note && plr->getGuild())
        plr->getGuild()->handleSetMemberNote(plr->getSession(), note, target->getGuid(), true);

    return 0;
}

int LuaUnit::SetOfficerNote(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const Player* target = CHECK_PLAYER(L, 1);
    const char* note = luaL_checkstring(L, 2);
    if (target && note && plr->getGuild())
        plr->getGuild()->handleSetMemberNote(plr->getSession(), note, target->getGuid(), false);

    return 0;
}

int LuaUnit::DisbandGuild(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    Guild* guild = nullptr;
    if (luaL_optinteger(L, 1, -1) >= 0)
        guild = sGuildMgr.getGuildById(static_cast<uint32_t>(luaL_optinteger(L, 1, -1)));
    else
        guild = plr->getGuild();
    if (guild != nullptr)
        guild->disband();

    return 0;
}

int LuaUnit::ChangeGuildMaster(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const Player* target = CHECK_PLAYER(L, 1);
    if (target)
        plr->getGuild()->handleSetNewGuildMaster(plr->getSession(), target->getName());
    return 0;
}

int LuaUnit::SendGuildChatMessage(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const char* message = luaL_checkstring(L, 1);
    const bool officer = CHECK_BOOL(L, 2);
    if (plr->getGuild() != nullptr && message != nullptr)
        plr->getGuild()->broadcastToGuild(plr->getSession(), officer, message, 0);

    return 0;
}

int LuaUnit::SendGuildLog(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->getGuild() != nullptr)
        plr->getGuild()->sendLoginInfo(plr->getSession());

    return 0;
}

int LuaUnit::GuildBankDepositMoney(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const uint32_t amount = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (plr->getGuild() != nullptr)
        plr->getGuild()->handleMemberDepositMoney(plr->getSession(), amount, true);

    return 0;
}

int LuaUnit::GuildBankWithdrawMoney(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const uint32_t amount = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    if (plr->getGuild() != nullptr)
        plr->getGuild()->handleMemberWithdrawMoney(plr->getSession(), amount, false);

    return 0;
}

int LuaUnit::SetByteValue(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    /*uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint8_t index1 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    uint8_t value = static_cast<uint8_t>(luaL_checkinteger(L, 3));
    ptr->setByteValue(index, index1, value);*/
    return 0;
}

int LuaUnit::GetByteValue(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr)
        return 0;

    /*uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint8_t index1 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    lua_pushinteger(L, ptr->getByteValue(index, index1));*/
    return 1;
}

int LuaUnit::IsPvPFlagged(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    lua_pushboolean(L, dynamic_cast<Player*>(ptr)->isPvpFlagSet() ? 1 : 0);
    return 1;
}

int LuaUnit::IsFFAPvPFlagged(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    lua_pushboolean(L, dynamic_cast<Player*>(ptr)->isFfaPvpFlagSet() ? 1 : 0);
    return 1;
}

int LuaUnit::GetGuildLeader(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Guild* pGuild = dynamic_cast<Player*>(ptr)->getGuild();
    if (pGuild != nullptr)
    {
        const Player* plr = sObjectMgr.getPlayer(uint32_t(pGuild->getLeaderGUID()));
        if (plr != nullptr)
            lua_pushstring(L, plr->getName().c_str());
        else
            lua_pushnil(L);
    }
    else
        lua_pushnil(L);

    return 1;
}

int LuaUnit::GetGuildMemberCount(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Guild* pGuild = dynamic_cast<Player*>(ptr)->getGuild();
    (pGuild != nullptr) ? lua_pushinteger(L, pGuild->getMembersCount()) : lua_pushnil(L);

    return 1;
}

int LuaUnit::IsFriendly(lua_State* L, Unit* ptr)
{
    Unit* obj = CHECK_UNIT(L, 1);
    if (!obj || !ptr)
        return 0;
    if (ptr->isFriendlyTo(obj))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::IsInChannel(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* channelName = luaL_checkstring(L, 1);
    if (!channelName)
        return 0;

    const auto* channel = sChannelMgr.getChannel(channelName, dynamic_cast<Player*>(ptr));
    // Channels: "General", "Trade", "LocalDefense", "GuildRecruitment", "LookingForGroup", (or any custom channel)
    if (channel->hasMember(dynamic_cast<Player*>(ptr)))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

int LuaUnit::JoinChannel(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* channelName = luaL_checkstring(L, 1);
    auto* const channel = sChannelMgr.getChannel(channelName, dynamic_cast<Player*>(ptr));
    if (!channel)
        return 0;

    const char* password = luaL_optstring(L, 2, channel->getChannelPassword().c_str());

    if (channel->hasMember(dynamic_cast<Player*>(ptr)))
        return 0;

    channel->attemptJoin(dynamic_cast<Player*>(ptr), password);

    return 1;
}

int LuaUnit::LeaveChannel(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* channelName = luaL_checkstring(L, 1);
    auto* const channel = sChannelMgr.getChannel(channelName, dynamic_cast<Player*>(ptr));
    if (!channelName || !channel || !channel->hasMember(dynamic_cast<Player*>(ptr)))
        return 0;

    channel->leaveChannel(dynamic_cast<Player*>(ptr), true);

    return 1;
}

int LuaUnit::SetChannelName(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* currentName = luaL_checkstring(L, 1);
    const char* newName = luaL_checkstring(L, 2);
    auto* const channel = sChannelMgr.getChannel(currentName, dynamic_cast<Player*>(ptr));
    if (!currentName || !newName || !channel || channel->getChannelName() == newName)
        return 0;

    channel->setChannelName(newName);
    return 1;
}

int LuaUnit::SetChannelPassword(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* channelName = luaL_checkstring(L, 1);
    const char* password = luaL_checkstring(L, 2);
    auto* const channel = sChannelMgr.getChannel(channelName, dynamic_cast<Player*>(ptr));
    if (!password || !channel || channel->getChannelPassword() == password)
        return 0;

    channel->setPassword(dynamic_cast<Player*>(ptr), password);
    return 1;
}

int LuaUnit::GetChannelPassword(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* channelName = luaL_checkstring(L, 1);
    const auto* channel = sChannelMgr.getChannel(channelName, dynamic_cast<Player*>(ptr));
    if (!channel)
        return 0;

    lua_pushstring(L, channel->getChannelPassword().c_str());

    return 1;
}

int LuaUnit::KickFromChannel(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* channelName = luaL_checkstring(L, 1);
    Player* player = dynamic_cast<Player*>(ptr);
    auto* const channel = sChannelMgr.getChannel(channelName, player);
    if (!channel)
        return 0;

    channel->kickOrBanPlayer(player, player, false);
    return 1;
}

int LuaUnit::BanFromChannel(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* channelName = luaL_checkstring(L, 1);
    Player* player = dynamic_cast<Player*>(ptr);
    auto* const channel = sChannelMgr.getChannel(channelName, player);
    if (!channel)
        return 0;

    channel->kickOrBanPlayer(player, player, true);
    return 1;
}

int LuaUnit::UnbanFromChannel(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* channelName = luaL_checkstring(L, 1);
    Player* player = dynamic_cast<Player*>(ptr);
    auto* const channel = sChannelMgr.getChannel(channelName, player);
    if (!channel)
        return 0;

    channel->unBanPlayer(player, player->getPlayerInfo());
    return 1;
}

int LuaUnit::GetChannelMemberCount(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const char* channelName = luaL_checkstring(L, 1);
    if (!channelName)
        return 0;

    lua_pushnumber(L, static_cast<lua_Number>(sChannelMgr.getChannel(channelName, dynamic_cast<Player*>(ptr))->getMemberCount()));
    return 1;
}

int LuaUnit::GetPlayerMovementVector(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    const MovementInfo* movement_info = plr->getMovementInfo();
    if (movement_info != nullptr)
    {
        lua_newtable(L);

        lua_pushstring(L, "x");
        lua_pushnumber(L, movement_info->getPosition()->x);
        lua_rawset(L, -3);
        lua_pushstring(L, "y");
        lua_pushnumber(L, movement_info->getPosition()->y);
        lua_rawset(L, -3);
        lua_pushstring(L, "z");
        lua_pushnumber(L, movement_info->getPosition()->z);
        lua_rawset(L, -3);
        lua_pushstring(L, "o");
        lua_pushnumber(L, movement_info->getPosition()->o);
        lua_rawset(L, -3);
    }
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetPlayerMovementFlags(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
        return 0;

    const MovementInfo* move_info = dynamic_cast<Player*>(ptr)->getMovementInfo();
    if (move_info != nullptr)
        lua_pushnumber(L, move_info->flags);
    else
        lua_pushnil(L);

    return 1;
}

int LuaUnit::Repop(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->isDead())
        plr->repopRequest();
    return 0;
}

int LuaUnit::SetMovementFlags(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    const int movetype = static_cast<int>(luaL_checkinteger(L, 1)); //0: walk, 1: run, 2: fly.
    if (movetype == 2)
    {
        ptr->setMoveCanFly(ptr->IsFlying());
    }
    else if (movetype == 1)
    {
        ptr->setMoveWalk(false);
    }
    else
    {
        ptr->setMoveWalk(true);
    }
    return 0;
}

int LuaUnit::GetSpawnId(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    Creature* cre = dynamic_cast<Creature*>(ptr);
    lua_pushnumber(L, cre->GetSQL_id());
    return 1;
}

int LuaUnit::ResetTalents(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Player* plr = dynamic_cast<Player*>(ptr);
    plr->resetTalents();
    return 0;
}

int LuaUnit::SetTalentPoints(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const auto forBothSpecs = CHECK_BOOL(L, 1);
    const auto points = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    dynamic_cast<Player*>(ptr)->setTalentPoints(points, forBothSpecs);
    return 0;
}

int LuaUnit::GetTalentPoints(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t spec = static_cast<uint32_t>(luaL_checkinteger(L, 1)); //0 or 1
#ifdef FT_DUAL_SPEC
    PlayerSpec plrSpec = dynamic_cast<Player*>(ptr)->m_specs[spec];
#else
    PlayerSpec plrSpec = static_cast<Player*>(ptr)->m_spec;
#endif
    //uint32_t Lvl = static_cast<Player*>(ptr)->getLevel();
    const uint32_t FreePoints = plrSpec.getTalentPoints();

    lua_pushnumber(L, FreePoints);
    return 1;
}

int LuaUnit::EventChat(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    const uint8_t typ = static_cast<uint8_t>(luaL_checkinteger(L, 1));
    const uint32_t lang = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    const char* message = luaL_checkstring(L, 3);
    const uint32_t delay = static_cast<uint32_t>(luaL_checkinteger(L, 4));
    if (message != nullptr && delay)
        ptr->sendChatMessage(typ, lang, message, delay);
    return 0;
}

int LuaUnit::GetEquippedItemBySlot(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const int16_t slot = static_cast<int16_t>(luaL_checkinteger(L, 1));
    const Player* plr = dynamic_cast<Player*>(ptr);
    Item* pItem = plr->getItemInterface()->GetInventoryItem(slot);
    if (pItem)
        PUSH_ITEM(L, pItem);
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetGuildMembers(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const Player* plr = dynamic_cast<Player*>(ptr);
    const Guild* pGuild = plr->getGuild();
    uint32_t count = 0;
    lua_newtable(L);
    if (pGuild != nullptr)
    {
        for (const auto& member : pGuild->getMemberNameList())
        {
            count++;
            lua_pushinteger(L, count);
            //Paroxysm : Why do we push player names are opposed to objects?
            //hyper: because guild members might not be logged in
            //ret = (*itr).first->m_loggedInPlayer;
            //PUSH_UNIT(L, ((Unit*)ret), false);
            lua_pushstring(L, member.c_str());
            lua_rawset(L, -3);
        }
    }
    else
        lua_pushnil(L);

    return 1;
}

int LuaUnit::AddAchievement(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

#if VERSION_STRING > TBC
    const int32_t achievementID = static_cast<int32_t>(luaL_checkinteger(L, 1));
    Player* plr = dynamic_cast<Player*>(ptr);
    if (plr->getAchievementMgr()->gmCompleteAchievement(nullptr, achievementID))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
#endif
    return 1;
}

int LuaUnit::RemoveAchievement(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

#if VERSION_STRING > TBC
    const int32_t achievementID = static_cast<int32_t>(luaL_checkinteger(L, 1));
    dynamic_cast<Player*>(ptr)->getAchievementMgr()->gmResetAchievement(achievementID);
#endif
    return 0;
}

int LuaUnit::HasAchievement(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

#if VERSION_STRING > TBC
    const uint32_t achievementID = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    lua_pushboolean(L, dynamic_cast<Player*>(ptr)->getAchievementMgr()->hasCompleted(achievementID) ? 1 : 0);
#endif
    return 1;
}

int LuaUnit::GetAreaId(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0); return 1;
    }
    const auto area = ptr->GetArea();
    RET_NUMBER(area ? area->id : -1);
}

int LuaUnit::ResetPetTalents(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    Pet* pet = dynamic_cast<Player*>(ptr)->getPet();
    if (pet != nullptr)
    {
        pet->WipeTalents();
#if VERSION_STRING == WotLK || VERSION_STRING == Cata
        pet->setPetTalentPoints(pet->getTotalTalentPoints());
        pet->SendTalentsToOwner();
#endif
    }
    return 0;
}

int LuaUnit::IsDazed(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0); return 1;
    }
    lua_pushboolean(L, (ptr->isDazed()) ? 1 : 0);
    return 1;
}

int LuaUnit::IsRooted(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0); return 1;
    }
    if (ptr->isRooted())
        RET_BOOL(true)
        RET_BOOL(false)
}

int LuaUnit::HasAuraWithMechanic(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0); return 1;
    }
    const auto mechanic = static_cast<SpellMechanic>(CHECK_ULONG(L, 1));
    if (mechanic && ptr->hasAuraWithMechanic(mechanic))
        RET_BOOL(true)
        RET_BOOL(false)
}

int LuaUnit::HasNegativeAura(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0); return 1;
    }
    for (uint16_t x = AuraSlots::NEGATIVE_SLOT_START; x < AuraSlots::NEGATIVE_SLOT_END; ++x)
    {
        if (ptr->getAuraWithAuraSlot(x))
            RET_BOOL(true)
    }
    RET_BOOL(false)
}

int LuaUnit::HasPositiveAura(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    for (uint16_t x = AuraSlots::POSITIVE_SLOT_START; x < AuraSlots::POSITIVE_SLOT_END; ++x)
    {
        if (ptr->getAuraWithAuraSlot(x))
            RET_BOOL(true)
    }
    RET_BOOL(false)
}

int LuaUnit::GetClosestEnemy(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    float closest_dist = 99999.99f;
    float current_dist = 0;
    Unit* ret = nullptr;
    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        if (!itr || !itr->isCreatureOrPlayer() || !ptr->isHostileTo(itr))
            continue;

        current_dist = ptr->GetDistance2dSq(itr);
        if (current_dist < closest_dist)
        {
            closest_dist = current_dist;
            ret = dynamic_cast<Unit*>(itr);
        }
    }
    PUSH_UNIT(L, ret);
    return 1;
}

int LuaUnit::GetClosestFriend(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    float closest_dist = 99999.99f;
    float current_dist = 0.0f;
    Unit* ret = nullptr;
    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        if (!itr || !itr->isCreatureOrPlayer() || itr->isHostileTo(ptr))
            continue;

        current_dist = itr->getDistanceSq(ptr);
        if (current_dist < closest_dist)
        {
            closest_dist = current_dist;
            ret = dynamic_cast<Unit*>(itr);
        }
    }
    PUSH_UNIT(L, ret);
    return 1;
}

int LuaUnit::GetClosestUnit(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    float closest_dist = 99999.99f;
    float current_dist = 0;
    Unit* ret = nullptr;
    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        if (!itr || !itr->isCreatureOrPlayer())
            continue;

        current_dist = ptr->GetDistance2dSq(itr);
        if (current_dist < closest_dist)
        {
            closest_dist = current_dist;
            ret = dynamic_cast<Unit*>(itr);
        }
    }
    PUSH_UNIT(L, ret);
    return 1;
}

int LuaUnit::GetObjectType(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    if (ptr->isPlayer())
        lua_pushstring(L, "Player");
    else
        lua_pushstring(L, "Unit");
    return 1;
}

int LuaUnit::DisableMelee(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        lua_pushboolean(L, 0); return 1;
    }
    const bool disable = CHECK_BOOL(L, 1);
    dynamic_cast<Creature*>(ptr)->getAIInterface()->setMeleeDisabled(disable);
    RET_BOOL(true)
}
int LuaUnit::DisableSpells(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        lua_pushboolean(L, 0); return 1;
    }
    const bool disable = CHECK_BOOL(L, 1);
    dynamic_cast<Creature*>(ptr)->getAIInterface()->setCastDisabled(disable);
    RET_BOOL(true)
}
int LuaUnit::DisableRanged(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        lua_pushboolean(L, 0); return 1;
    }
    const bool disable = CHECK_BOOL(L, 1);
    dynamic_cast<Creature*>(ptr)->getAIInterface()->setRangedDisabled(disable);
    RET_BOOL(true)
}
int LuaUnit::DisableCombat(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        lua_pushboolean(L, 0); return 1;
    }
    const bool disable = CHECK_BOOL(L, 1);
    dynamic_cast<Creature*>(ptr)->getAIInterface()->setCombatDisabled(disable);
    RET_BOOL(true)
}
int LuaUnit::DisableTargeting(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        lua_pushboolean(L, 0); return 1;
    }
    const bool disable = CHECK_BOOL(L, 1);
    dynamic_cast<Creature*>(ptr)->getAIInterface()->setTargetingDisabled(disable);
    RET_BOOL(true)
}
int LuaUnit::IsInGroup(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    if (dynamic_cast<Player*>(ptr)->isInGroup())
        RET_BOOL(true)
        RET_BOOL(false)
}
int LuaUnit::GetLocation(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    lua_pushnumber(L, ptr->GetPositionX());
    lua_pushnumber(L, ptr->GetPositionY());
    lua_pushnumber(L, ptr->GetPositionZ());
    lua_pushnumber(L, ptr->GetOrientation());
    return 4;
}

int LuaUnit::GetByte(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    /*uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint8_t index2 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    uint8_t value = ptr->getByteValue(index, index2);
    RET_INT(value);*/
    RET_INT(0)
}

int LuaUnit::SetByte(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0); return 1;
    }
    /*uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint8_t index2 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    uint8_t value = static_cast<uint8_t>(luaL_checkinteger(L, 3));
    ptr->setByteValue(index, index2, value);*/
    RET_BOOL(true)
}

int LuaUnit::GetSpawnLocation(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    lua_pushnumber(L, ptr->GetSpawnX());
    lua_pushnumber(L, ptr->GetSpawnY());
    lua_pushnumber(L, ptr->GetSpawnZ());
    lua_pushnumber(L, ptr->GetSpawnO());
    return 4;
}

int LuaUnit::GetObjectByGuid(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    const uint64_t guid = CHECK_GUID(L, 1);
    Object* obj = ptr->getWorldMap()->getObject(guid);
    if (obj != nullptr && obj->isCreatureOrPlayer())
        PUSH_UNIT(L, obj);
    else if (obj != nullptr && obj->isGameObject())
        PUSH_GO(L, obj);
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::GetSecondHated(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    PUSH_UNIT(L, ptr->getThreatManager().getSecondMostHated());
    return 1;
}

int LuaUnit::UseAI(lua_State* L, Unit* ptr)
{
    const bool check = CHECK_BOOL(L, 1);
    ptr->setAItoUse(check);
    return 0;
}

int LuaUnit::FlagFFA(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    const bool set = CHECK_BOOL(L, 1);
    if (set)
        ptr->setFfaPvpFlag();
    else
        ptr->removeFfaPvpFlag();
    return 0;
}

int LuaUnit::TeleportCreature(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreature())
    {
        return 0;
    }
    const float x = CHECK_FLOAT(L, 1);
    const float y = CHECK_FLOAT(L, 2);
    const float z = CHECK_FLOAT(L, 3);
    ptr->SetPosition(x, y, z, ptr->GetOrientation());
    WorldPacket data(SMSG_MONSTER_MOVE, 50);
    data << ptr->GetNewGUID();
    data << uint8_t(0);
    data << ptr->GetPositionX();
    data << ptr->GetPositionY();
    data << ptr->GetPositionZ();
    data << Util::getMSTime();
    data << uint8_t(0x0);
    data << uint32_t(0x100);
    data << uint32_t(1);
    data << uint32_t(1);
    data << x;
    data << y;
    data << z;
    ptr->sendMessageToSet(&data, false);
    return 0;
}

int LuaUnit::IsInDungeon(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0); return 1;
    }
    if (ptr->getWorldMap()->getBaseMap()->getMapInfo() && ptr->getWorldMap()->getBaseMap()->getMapInfo()->isMultimodeDungeon())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::IsInRaid(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0); return 1;
    }
    if (ptr->getWorldMap()->getBaseMap()->getMapInfo() && ptr->getWorldMap()->getBaseMap()->getMapInfo()->isRaid())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit::IsHostile(lua_State* L, Unit* ptr)
{
    Object* B = CHECK_OBJECT(L, 1);
    lua_pushboolean(L, ptr->isHostileTo(B));
    return 1;
}

int LuaUnit::IsAttackable(lua_State* L, Unit* ptr)
{
    Object* B = CHECK_OBJECT(L, 1);
    lua_pushboolean(L, ptr->isValidTarget(B));
    return 1;
}

int LuaUnit::GetQuestLogSlot(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t entry = CHECK_ULONG(L, 1);

    if (const auto* questLog = dynamic_cast<Player*>(ptr)->getQuestLogByQuestId(entry))
        lua_pushnumber(L, questLog->getSlot());
    else
        RET_NUMBER(-1);

    return 1;
}

int LuaUnit::GetAuraStackCount(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        lua_pushboolean(L, 0); return 1;
    }
    const uint32_t id = CHECK_ULONG(L, 1);
    RET_NUMBER(ptr->getAuraCountForId(id));
}

int LuaUnit::AddAuraObject(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    // TODO: possibly needs rewrite of LuaEngine to handle unique_ptr<T> -Appled

    /*Aura* aura = CHECK_AURA(L, 1);
    if (!aura)
        return 0;
    ptr->addAura(aura);*/
    return 0;
}

int LuaUnit::GetAuraObjectById(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    const uint32_t id = CHECK_ULONG(L, 1);
    PUSH_AURA(L, ptr->getAuraWithId(id));
    return 1;
}

int LuaUnit::StopPlayerAttack(lua_State* /*L*/, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    ptr->smsg_AttackStop(ptr->getWorldMap()->getUnit(dynamic_cast<Player*>(ptr)->getTargetGuid()));
    return 0;
}

int LuaUnit::GetQuestObjectiveCompletion(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    const uint32_t questid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint8_t objective = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    const Player* player = dynamic_cast<Player*>(ptr);

    if (const auto* questLog = player->getQuestLogByQuestId(questid))
        lua_pushnumber(L, questLog->getMobCountByIndex(objective));
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit::IsOnVehicle(lua_State* L, Unit* ptr)
{
#ifdef FT_VEHICLES
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    if ((ptr->getVehicleKit() != nullptr) || (ptr->isPlayer() && ptr->isVehicle()))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
#endif
    return 1;
}

int LuaUnit::SpawnAndEnterVehicle(lua_State* L, Unit* ptr)
{
#ifdef FT_VEHICLES
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    uint32_t creature_entry = 0;
    uint32_t delay = 0;

    if (lua_gettop(L) != 2)
        return 0;

    creature_entry = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    delay = static_cast<uint32_t>(luaL_checkinteger(L, 2));

    if (delay < 1 * 1000)
        delay = 1 * 1000;

    if (creature_entry == 0)
        return 0;

    if ((ptr->getVehicleKit() != nullptr) && (!ptr->isPlayer() || !ptr->isVehicle()))
        return 0;

    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(creature_entry);
    if (cp == nullptr)
        return 0;

    const Player* p = nullptr;
    if (ptr->isPlayer())
        p = dynamic_cast<Player*>(ptr);

    if ((cp->vehicleid == 0) && (p == nullptr) && (!p->getSession()->HasGMPermissions()))
        return 0;

    const LocationVector v(ptr->GetPosition());

    Creature* c = ptr->getWorldMap()->createCreature(cp->Id);
    c->Load(cp, v.x, v.y, v.z, v.o);
    c->removeNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
    c->PushToWorld(ptr->getWorldMap());
    c->callEnterVehicle(ptr);
#endif
    return 0;
}

int LuaUnit::DismissVehicle(lua_State* /*L*/, Unit* ptr)
{
#ifdef FT_VEHICLES
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    Vehicle* v = nullptr;
    if (ptr->getVehicleKit() != nullptr)
    {
        v = ptr->getVehicleKit();
    }
    else
    {
        if (ptr->isPlayer() && (ptr->getVehicle() != nullptr))
            v = ptr->getVehicle();
    }

    if (v == nullptr)
        return 0;

    v->removeAllPassengers();

    Unit* o = v->getBase();

    if (o->isPlayer())
        o->removeAllAurasByAuraEffect(SPELL_AURA_MOUNTED);
    else
        o->Delete();
#endif
    return 0;
}

int LuaUnit::AddVehiclePassenger(lua_State* L, Unit* ptr)
{
#ifdef FT_VEHICLES
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    const Vehicle* v = nullptr;

    if (ptr->getVehicleKit() != nullptr)
        v = ptr->getVehicleKit();
    else
        if (ptr->isPlayer() && (ptr->getVehicle() != nullptr))
            v = ptr->getVehicle();

    if (v == nullptr)
        return 0;

    if (!v->hasEmptySeat())
        return 0;

    if (lua_gettop(L) != 1)
        return 0;

    const uint32_t creature_entry = static_cast<uint32_t>(luaL_checkinteger(L, 1));

    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(creature_entry);
    if (cp == nullptr)
        return 0;

    Unit* u = v->getBase();

    Creature* c = u->getWorldMap()->createCreature(creature_entry);
    c->Load(cp, u->GetPositionX(), u->GetPositionY(), u->GetPositionZ(), u->GetOrientation());
    c->PushToWorld(u->getWorldMap());
    c->callEnterVehicle(u);
#endif
    return 0;
}

int LuaUnit::HasEmptyVehicleSeat(lua_State* L, Unit* ptr)
{
#ifdef FT_VEHICLES
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    const Vehicle* v = nullptr;

    if (ptr->getVehicleKit() != nullptr)
        v = ptr->getVehicleKit();
    else
        if (ptr->isPlayer() && (ptr->getVehicle() != nullptr))
            v = ptr->getVehicle();

    if (v == nullptr)
        return 0;

    if (v->hasEmptySeat())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
#endif
    return 1;
}

int LuaUnit::EnterVehicle(lua_State* L, Unit* ptr)
{
#ifdef FT_VEHICLES
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    if (lua_gettop(L) != 2)
        return 0;

    const uint64_t guid = CHECK_GUID(L, 1);

    Unit* _unit = ptr->getWorldMapUnit(guid);
    if (_unit)
        _unit->callEnterVehicle(ptr);
#endif
    return 0;
}

int LuaUnit::ExitVehicle(lua_State* /*L*/, Unit* ptr)
{
#ifdef FT_VEHICLES
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    if (ptr->getVehicleKit() != nullptr)
    {
        ptr->callExitVehicle();
    }
    else
    {
        if (ptr->isPlayer() && ptr->getVehicle() != nullptr)
            ptr->removeAllAurasByAuraEffect(SPELL_AURA_MOUNTED);
    }
#endif
    return 0;
}

int LuaUnit::GetVehicleBase(lua_State* L, Unit* ptr)
{
#ifdef FT_VEHICLES
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    Unit* u = ptr->getVehicleBase();

    if (u != nullptr)
        PUSH_UNIT(L, u);
    else
        lua_pushnil(L);
#endif
    return 1;
}

int LuaUnit::EjectAllVehiclePassengers(lua_State* /*L*/, Unit* ptr)
{
#ifdef FT_VEHICLES
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    const Unit* u = ptr->getVehicleBase();
    if (u == nullptr)
        return 0;

    u->getVehicle()->removeAllPassengers();
#endif
    return 0;
}

int LuaUnit::EjectVehiclePassengerFromSeat(lua_State* L, Unit* ptr)
{
#ifdef FT_VEHICLES
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    const Unit* u = ptr->getVehicleBase();
    if (u == nullptr)
        return 0;

    if (u->getVehicle() == nullptr)
        return 0;

    if (lua_gettop(L) != 1)
        return 0;

    const int8_t seat = static_cast<int8_t>(luaL_checkinteger(L, 1));

    if (Unit* passenger = u->getVehicle()->getPassenger(seat))
        passenger->callExitVehicle();
#endif
    return 0;
}

int LuaUnit::MoveVehiclePassengerToSeat(lua_State* L, Unit* ptr)
{
#ifdef FT_VEHICLES
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    const Unit* u = ptr->getVehicleBase();
    if (u == nullptr)
        return 0;

    if (lua_gettop(L) != 2)
        return 0;

    Unit* passenger = CHECK_UNIT(L, 1);
    const uint32_t seat = static_cast<uint32_t>(luaL_checkinteger(L, 2));

    if (passenger == nullptr)
        return 0;

    passenger->callChangeSeat(seat);
#endif
    return 0;
}

int LuaUnit::SendCinematic(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    const uint32_t id = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    dynamic_cast<Player*>(ptr)->sendCinematicCamera(id);
    return 0;
}

int LuaUnit::GetWorldStateForZone(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    if (lua_gettop(L) != 1)
        return 0;

    const uint32_t field = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint32_t zoneId;
    uint32_t areaId;
    uint32_t entry = 0;

    ptr->getWorldMap()->getZoneAndAreaId(ptr->GetPhase(), zoneId, areaId, ptr->GetPosition());

    if (zoneId == 0)
        entry = areaId;
    else
        entry = zoneId;

    if (entry == 0)
        return 0;

    const uint32_t value = ptr->getWorldMap()->getWorldStatesHandler().GetWorldStateForZone(entry, 0, field);

    lua_pushinteger(L, value);
    return 1;
}

int LuaUnit::SetWorldStateForZone(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isCreatureOrPlayer())
    {
        return 0;
    }

    if (lua_gettop(L) != 2)
        return 0;

    const uint32_t field = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));

    uint32_t zoneId;
    uint32_t areaId;
    uint32_t entry = 0;

    ptr->getWorldMap()->getZoneAndAreaId(ptr->GetPhase(), zoneId, areaId, ptr->GetPosition());

    if (zoneId == 0)
        entry = areaId;
    else
        entry = zoneId;

    if (entry == 0)
        return 0;

    ptr->getWorldMap()->getWorldStatesHandler().SetWorldStateForZone(entry, 0, field, value);
    return 0;
}

int LuaUnit::SetWorldStateForPlayer(lua_State* L, Unit* ptr)
{
    if (ptr == nullptr || !ptr->IsInWorld() || !ptr->isPlayer())
    {
        return 0;
    }

    if (lua_gettop(L) != 2)
        return 0;

    const uint32_t field = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    const uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));

    dynamic_cast<Player*>(ptr)->sendWorldStateUpdate(field, value);
    return 0;
}
