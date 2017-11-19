/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2007 Moon++ <http://www.moonplusplus.info/>
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

#ifndef UNITFUNCTIONS_H
#define UNITFUNCTIONS_H

#include "Units/Unit.h"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Units/Summons/SummonHandler.h"
#include "Units/Creatures/Vehicle.h"
#include "Units/Creatures/Creature.h"
#include "Units/Summons/Summon.h"
#include "Management/Item.h"
#include "Management/Container.h"
#include "Map/MapMgr.h"
#include "Units/Stats.h"
#include "Management/ChannelMgr.h"
#include "Management/Channel.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Management/Group.h"
#include "Objects/Faction.h"
#include "Spell/SpellAuras.h"
#include "Server/WorldSession.h"
#include "Objects/Object.h"
#include "LuaGlobal.h"
#include <Spell/Definitions/PowerType.h>
#include <Map/MapScriptInterface.h>
#include <Units/Creatures/Pet.h>

class LuaUnit
{
    public:
    static int GetDisplay(lua_State* L, Unit* ptr)
    {
        if (ptr == NULL)
            lua_pushinteger(L, 0);
        else
            lua_pushinteger(L, ptr->GetDisplayId());

        return 1;
    }

    static int GetNativeDisplay(lua_State* L, Unit* ptr)
    {
        if (ptr == NULL)
            lua_pushinteger(L, 0);
        else
            lua_pushinteger(L, ptr->GetNativeDisplayId());

        return 1;
    }

    static int GossipCreateMenu(lua_State * L, Unit * ptr)
    {
        int text_id = static_cast<int>(luaL_checkinteger(L, 1));
        Player *plr = CHECK_PLAYER(L, 2);
        int autosend = static_cast<int>(luaL_checkinteger(L, 3));

        if (plr == NULL)
            return 0;

        if (LuaGlobal::instance()->m_menu != NULL)
            delete LuaGlobal::instance()->m_menu;

        LuaGlobal::instance()->m_menu = new Arcemu::Gossip::Menu(ptr->GetGUID(), text_id);

        if (autosend != 0)
            LuaGlobal::instance()->m_menu->Send(plr);

        return 0;
    }

    static int GossipMenuAddItem(lua_State * L, Unit * /*ptr*/)
    {
        uint8_t icon = static_cast<uint8_t>(luaL_checkinteger(L, 1));
        const char * menu_text = luaL_checkstring(L, 2);
        int IntId = static_cast<int>(luaL_checkinteger(L, 3));
        bool coded = (luaL_checkinteger(L, 4) ? true : false);
        const char * boxmessage = luaL_optstring(L, 5, "");
        uint32 boxmoney = static_cast<uint32>(luaL_optinteger(L, 6, 0));

        if (LuaGlobal::instance()->m_menu == NULL){
            LOG_ERROR("There is no menu to add items to!");
            return 0;
        }

        LuaGlobal::instance()->m_menu->AddItem(icon, menu_text, IntId, boxmoney, boxmessage, coded);

        return 0;
    }

    static int GossipSendMenu(lua_State * L, Unit * /*ptr*/)
    {
        Player* plr = CHECK_PLAYER(L, 1);

        if (LuaGlobal::instance()->m_menu == NULL){
            LOG_ERROR("There is no menu to send!");
            return 0;
        }

        if (plr != NULL)
            LuaGlobal::instance()->m_menu->Send(plr);

        return 0;
    }

    static int GossipSendPOI(lua_State * L, Unit * ptr)
    {
        TEST_PLAYER()
            Player * plr = static_cast<Player*>(ptr);
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        int icon = static_cast<int>(luaL_checkinteger(L, 3));
        int flags = static_cast<int>(luaL_checkinteger(L, 4));
        int data = static_cast<int>(luaL_checkinteger(L, 5));
        const char * name = luaL_checkstring(L, 6);

        plr->Gossip_SendPOI(x, y, icon, flags, data, name);

        return 0;
    }

    static int GossipSendQuickMenu(lua_State *L, Unit *ptr){
        TEST_UNIT()

        uint32 text_id = static_cast<uint32>(luaL_checkinteger(L, 1));
        Player *player = CHECK_PLAYER(L, 2);
        uint32 itemid = static_cast<uint32>(luaL_checkinteger(L, 3));
        uint8 itemicon = CHECK_UINT8(L, 4);
        const char *itemtext = luaL_checkstring(L, 5);
        uint32 requiredmoney = CHECK_ULONG(L, 6);
        const char *moneytext = luaL_checkstring(L, 7);
        uint8 extra = CHECK_UINT8(L, 8);

        if (player == NULL)
            return 0;

        Arcemu::Gossip::Menu::SendQuickMenu(ptr->GetGUID(), text_id, player, itemid, itemicon, itemtext, requiredmoney, moneytext, extra);

        return 0;
    }

    static int GossipAddQuests(lua_State *L, Unit *ptr){
        TEST_UNIT()

            if (LuaGlobal::instance()->m_menu == NULL){
                LOG_ERROR("There's no menu to fill quests into.");
                return 0;
            }

        Player *player = CHECK_PLAYER(L, 1);

        sQuestMgr.FillQuestMenu(static_cast< Creature* >(ptr), player, *LuaGlobal::instance()->m_menu);

        return 0;
    }


    static int GossipComplete(lua_State * /*L*/, Unit * ptr)
    {
        TEST_PLAYER()
            Player * plr = static_cast<Player*>(ptr);

        if (LuaGlobal::instance()->m_menu == nullptr)
        {
            LOG_ERROR("There is no menu to complete!");
            return 0;
        }

        LuaGlobal::instance()->m_menu->Complete(plr);

        return 0;
    }

    static int IsPlayer(lua_State* L, Unit* ptr)
    {
        if (!ptr)
        {
            lua_pushboolean(L, 0);
            return 1;
        }

        if (ptr->IsPlayer())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);

        return 1;
    }

    static int IsCreature(lua_State* L, Unit* ptr)
    {
        if (!ptr)
        {
            lua_pushboolean(L, 0);
            return 1;
        }

        if (ptr->IsCreature())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);

        return 1;
    }

    static int Emote(lua_State* L, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32 emote_id = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 time = static_cast<uint32>(luaL_checkinteger(L, 2));
        if (emote_id == 0)
            return 0;
        if (time > 0)
            ptr->EventAddEmote((EmoteType)emote_id, time);
        else
            ptr->Emote((EmoteType)emote_id);
        return 1;
    }

    static int GetName(lua_State* L, Unit* ptr)
    {
        if (!ptr)
            return 0;

        switch (ptr->GetTypeId())
        {
            case TYPEID_UNIT:
                lua_pushstring(L, static_cast<Creature*>(ptr)->GetCreatureProperties() ? static_cast<Creature*>(ptr)->GetCreatureProperties()->Name.c_str() : "Unknown");
                break;

            case TYPEID_PLAYER:
                lua_pushstring(L, static_cast<Player*>(ptr)->GetName());
                break;

            default:
                lua_pushstring(L, "Unknown");
                break;
        }

        return 1;
    }

    static int PhaseSet(lua_State* L, Unit* ptr)
    {
        uint32 newphase = CHECK_ULONG(L, 1);
        bool Save = (luaL_optinteger(L, 2, false) > 0 ? true : false);
        Creature* crt = NULL;
        Player* p_target = NULL;
        //Save is only for creatures. if you want to save to DB with players, use your own query (security purposes).
        //Lua: CharDBQuery("UPDATE `characters` SET `phase`='"..phase.."' WHERE (`name`='"..player:GetName().."'",0)
        if (!ptr)
            return 0;

        switch (ptr->GetTypeId())
        {
            case TYPEID_UNIT:
                crt = static_cast<Creature*>(ptr);
                crt->Phase(PHASE_SET, newphase);
                if (crt->m_spawn)
                    crt->m_spawn->phase = newphase;
                if (Save)
                {
                    crt->SaveToDB();
                    crt->m_loadedFromDB = true;
                }
                break;

            case TYPEID_PLAYER:
                p_target = static_cast<Player*>(ptr);
                p_target->Phase(PHASE_SET, newphase);
                break;

            default:
                break;
        }
        return 0;
    }

    static int PhaseAdd(lua_State* L, Unit* ptr)
    {
        uint32 newphase = CHECK_ULONG(L, 1);
        bool Save = (luaL_optinteger(L, 2, false) > 0 ? true : false);
        Creature* crt = NULL;
        Player* p_target = NULL;
        //Save is only for creatures. if you want to save to DB with players, use your own query (security purposes).
        //Lua: CharDBQuery("UPDATE `characters` SET `phase`='"..player:GetPhase().."' WHERE (`name`='"..player:GetName().."'",0)
        if (!ptr)
            return 0;

        switch (ptr->GetTypeId())
        {
            case TYPEID_UNIT:
                crt = static_cast<Creature*>(ptr);
                crt->Phase(PHASE_ADD, newphase);
                if (crt->m_spawn)
                    crt->m_spawn->phase |= newphase;
                if (Save)
                {
                    crt->SaveToDB();
                    crt->m_loadedFromDB = true;
                }
                break;

            case TYPEID_PLAYER:
                p_target = static_cast<Player*>(ptr);
                p_target->Phase(PHASE_ADD, newphase);
                break;

            default:
                break;
        }
        return 0;
    }

    static int PhaseDelete(lua_State* L, Unit* ptr)
    {
        uint32 newphase = CHECK_ULONG(L, 1);
        bool Save = (luaL_checkinteger(L, 2) > 0 ? true : false);
        Creature* crt = NULL;
        Player* p_target = NULL;
        //Save is only for creatures. if you want to save to DB with players, use your own query (security purposes).
        //Lua: CharDBQuery("UPDATE `characters` SET `phase`='"..player:GetPhase().."' WHERE (`name`='"..player:GetName().."'",0)
        if (!ptr)
            return 0;

        switch (ptr->GetTypeId())
        {
            case TYPEID_UNIT:
                crt = static_cast<Creature*>(ptr);
                crt->Phase(PHASE_DEL, newphase);
                if (crt->m_spawn)
                    crt->m_spawn->phase &= ~newphase;
                if (Save)
                {
                    crt->SaveToDB();
                    crt->m_loadedFromDB = true;
                }
                break;

            case TYPEID_PLAYER:
                p_target = static_cast<Player*>(ptr);
                p_target->Phase(PHASE_DEL, newphase);
                break;

            default:
                break;
        }
        return 0;
    }

    static int GetPhase(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        lua_pushnumber(L, ptr->m_phase);
        return 1;
    }

    static int SendChatMessage(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            uint8_t typ = static_cast<uint8_t>(CHECK_ULONG(L, 1));
        uint32 lang = CHECK_ULONG(L, 2);
        const char* message = luaL_checklstring(L, 3, NULL);
        if (message == NULL)
            return 0;

        ptr->SendChatMessage(typ, lang, message);
        return 0;
    }

    static int PlayerSendChatMessage(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            uint32 type = CHECK_ULONG(L, 1);
        uint32 lang = CHECK_ULONG(L, 2);
        const char* msg = luaL_checklstring(L, 3, NULL);
        Player* plr = static_cast<Player*>(ptr);
        if (msg == NULL || !plr)
            return 0;
        WorldPacket* data = sChatHandler.FillMessageData(type, lang, msg, plr->GetGUID(), 0);
        plr->GetSession()->SendChatPacket(data, 1, lang, plr->GetSession());
        for (std::set< Object* >::iterator itr = plr->GetInRangePlayerSetBegin(); itr != plr->GetInRangePlayerSetEnd(); ++itr)
        {
            (static_cast< Player* >(*itr))->GetSession()->SendChatPacket(data, 1, lang, plr->GetSession());
        }
        return 0;
    }

    static int AggroWithInRangeFriends(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            // If Pointer isn't in combat skip everything
            if (!ptr->CombatStatus.IsInCombat())
                return 0;

        Unit* pTarget = ptr->GetAIInterface()->getNextTarget();
        if (!pTarget)
            return 0;

        Unit* pUnit = NULL;
        for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
        {
            Object* obj = *itr;
            // Object Isn't a Unit, Unit is Dead
            if (!obj->IsUnit() || static_cast<Unit*>(obj)->IsDead())
                continue;

            if (!isFriendly(obj, ptr))
                continue;

            if (ptr->GetDistance2dSq(obj) > 10 * 10) // 10yrd range?
                continue;

            pUnit = static_cast<Unit*>(obj);

            pUnit->GetAIInterface()->setNextTarget(pTarget);
            pUnit->GetAIInterface()->AttackReaction(pTarget, 1, 0);
        }
        return 0;
    }

    static int MoveTo(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float z = CHECK_FLOAT(L, 3);
        //float o = CHECK_FLOAT(L, 4);

        ptr->GetAIInterface()->MoveTo(x, y, z);
        return 0;
    }

    static int MoveRandomArea(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            float x1 = CHECK_FLOAT(L, 1);
        float y1 = CHECK_FLOAT(L, 2);
        float z1 = CHECK_FLOAT(L, 3);
        float x2 = CHECK_FLOAT(L, 4);
        float y2 = CHECK_FLOAT(L, 5);
        float z2 = CHECK_FLOAT(L, 6);
        //float o2 = CHECK_FLOAT(L, 7);

        ptr->GetAIInterface()->MoveTo(x1 + (RandomFloat(x2 - x1)), y1 + (RandomFloat(y2 - y1)), z1 + (RandomFloat(z2 - z1)));
        return 0;
    }

    static int SetMovementType(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            uint32 typ = CHECK_ULONG(L, 1);
        ptr->GetAIInterface()->setWaypointScriptType((Movement::WaypointMovementScript)typ);
        return 0;
    }

    static int GetX(lua_State* L, Unit* ptr)
    {
        if (ptr != NULL)
            lua_pushnumber(L, ptr->GetPositionX());
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetY(lua_State* L, Unit* ptr)
    {
        if (ptr != NULL)
            lua_pushnumber(L, ptr->GetPositionY());
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetZ(lua_State* L, Unit* ptr)
    {
        if (ptr != NULL)
            lua_pushnumber(L, ptr->GetPositionZ());
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetO(lua_State* L, Unit* ptr)
    {
        if (ptr != NULL)
            lua_pushnumber(L, ptr->GetOrientation());
        else
            lua_pushnil(L);
        return 1;
    }

    static int CastSpell(lua_State* L, Unit* ptr)
    {
        uint32 sp = CHECK_ULONG(L, 1);
        if (sp && ptr)
            ptr->CastSpell(ptr, sSpellCustomizations.GetSpellInfo(sp), true);
        return 0;
    }

    static int FullCastSpell(lua_State* L, Unit* ptr)
    {
        uint32 sp = CHECK_ULONG(L, 1);
        if (sp && ptr)
            ptr->CastSpell(ptr, sSpellCustomizations.GetSpellInfo(sp), false);
        return 0;
    }
    static int FullCastSpellOnTarget(lua_State* L, Unit* ptr)
    {
        if (ptr != NULL)
        {
            uint32 sp = CHECK_ULONG(L, 1);
            Object* target = CHECK_OBJECT(L, 2);
            if (sp && target != NULL)
                ptr->CastSpell(target->GetGUID(), sp, false);
        }
        return 0;
    }
    static int CastSpellOnTarget(lua_State* L, Unit* ptr)
    {
        uint32 sp = CHECK_ULONG(L, 1);
        Object* target = CHECK_OBJECT(L, 2);
        if (ptr != NULL && sp && target != NULL)
            ptr->CastSpell(target->GetGUID(), sp, true);
        return 0;
    }
    static int SpawnCreature(lua_State* L, Unit* ptr)
    {
        if (ptr == NULL) return 0;
        uint32 entry = CHECK_ULONG(L, 1);
        float x = CHECK_FLOAT(L, 2);
        float y = CHECK_FLOAT(L, 3);
        float z = CHECK_FLOAT(L, 4);
        float o = CHECK_FLOAT(L, 5);
        uint32 faction = CHECK_ULONG(L, 6);
        uint32 duration = CHECK_ULONG(L, 7);
        uint32 equip1 = static_cast<uint32>(luaL_optinteger(L, 8, 1));
        uint32 equip2 = static_cast<uint32>(luaL_optinteger(L, 9, 1));
        uint32 equip3 = static_cast<uint32>(luaL_optinteger(L, 10, 1));
        uint32 phase = static_cast<uint32>(luaL_optinteger(L, 11, ptr->m_phase));
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
        pCreature->SetFaction(faction);
        pCreature->SetEquippedItem(MELEE, equip1);
        pCreature->SetEquippedItem(OFFHAND, equip2);
        pCreature->SetEquippedItem(RANGED, equip3);
        pCreature->Phase(PHASE_SET, phase);
        pCreature->m_noRespawn = true;
        pCreature->AddToWorld(ptr->GetMapMgr());
        if (duration)
            pCreature->Despawn(duration, 0);
        if (save)
            pCreature->SaveToDB();
        PUSH_UNIT(L, pCreature);
        return 1;
    }
    static int SpawnGameObject(lua_State* L, Unit* ptr)
    {
        if (ptr == NULL) return 0;
        uint32 entry_id = CHECK_ULONG(L, 1);
        float x = CHECK_FLOAT(L, 2);
        float y = CHECK_FLOAT(L, 3);
        float z = CHECK_FLOAT(L, 4);
        float o = CHECK_FLOAT(L, 5);
        uint32 duration = CHECK_ULONG(L, 6);
        float scale = (float)(luaL_optinteger(L, 7, 100) / 100.0f);
        uint32 phase = static_cast<uint32>(luaL_optinteger(L, 8, ptr->m_phase));
        bool save = luaL_optinteger(L, 9, 0) ? true : false;
        if (entry_id)
        {
            GameObjectProperties const* info = sMySQLStore.getGameObjectProperties(entry_id);
            if (info == nullptr)
            {
                LOG_ERROR("Lua script tried to spawn a gameobject that doesn't exist ( %u ). Aborting.", entry_id);
                lua_pushnil(L);
                return 1;
            }

            GameObject* go = ptr->GetMapMgr()->CreateGameObject(entry_id);
            uint32 mapid = ptr->GetMapId();
            go->CreateFromProto(entry_id, mapid, x, y, z, o);
            go->Phase(PHASE_SET, phase);
            go->SetScale(scale);
            go->AddToWorld(ptr->GetMapMgr());

            if (duration)
                sEventMgr.AddEvent(go, &GameObject::ExpireAndDelete, EVENT_GAMEOBJECT_UPDATE, duration, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            if (save)
                go->SaveToDB();
            PUSH_GO(L, go);
        }
        else
            lua_pushnil(L);
        return 1;
    }
    static int RegisterEvent(lua_State* L, Unit* ptr)
    {
        TEST_UNIT();
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
            Creature* creature = static_cast<Creature*>(ptr);
            sEventMgr.AddEvent(creature, &Creature::TriggerScriptEvent, functionRef, EVENT_LUA_CREATURE_EVENTS, delay, repeats, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            std::map< uint64, std::set<int> > & objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
            std::map< uint64, std::set<int> >::iterator itr = objRefs.find(ptr->GetGUID());
            if (itr == objRefs.end())
            {
                std::set<int> refs;
                refs.insert(functionRef);
                objRefs.insert(make_pair(ptr->GetGUID(), refs));
            }
            else
            {
                std::set<int> & refs = itr->second;
                refs.insert(functionRef);
            }
        }
        return 0;
    }
    /* This one just simply calls the function directly w/o any arguments,
    the trick to arguments is done Lua side through closures(function that
    calls the wanted function  with the wanted arguments */
    static int CreateLuaEvent(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();

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
            TimedEvent* ev = TimedEvent::Allocate(ptr, new CallbackP1<LuaEngine, int>(LuaGlobal::instance()->luaEngine().get(), &LuaEngine::CallFunctionByReference, functionRef), EVENT_LUA_CREATURE_EVENTS, delay, repeats);
            ptr->event_AddEvent(ev);
            std::map< uint64, std::set<int> > & objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
            std::map< uint64, std::set<int> >::iterator itr = objRefs.find(ptr->GetGUID());
            if (itr == objRefs.end())
            {
                std::set<int> refs;
                refs.insert(functionRef);
                objRefs.insert(make_pair(ptr->GetGUID(), refs));
            }
            else
            {
                std::set<int> & refs = itr->second;
                refs.insert(functionRef);
            }
        }
        return 0;
    }
    static int RemoveEvents(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER();
        sEventMgr.RemoveEvents(ptr, EVENT_LUA_CREATURE_EVENTS);
        //Unref all contained references
        std::map< uint64, std::set<int> > & objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
        std::map< uint64, std::set<int> >::iterator itr = objRefs.find(ptr->GetGUID());
        if (itr != objRefs.end())
        {
            std::set<int> & refs = itr->second;
            for (std::set<int>::iterator it = refs.begin(); it != refs.end(); ++it)
                luaL_unref(L, LUA_REGISTRYINDEX, (*it));
            refs.clear();
        }
        return 0;
    }

    static int SetFaction(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
        int faction = static_cast<int>(luaL_checkinteger(L, 1));
        if (!faction)
            return 0;

        ptr->SetFaction(faction);
        return 0;
    }
    static int GetNativeFaction(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET()
            if (ptr->IsPlayer())
            {
                RET_INT(static_cast<Player*>(ptr)->GetInitialFactionId());
            }
            else
            {
                if (static_cast<Creature*>(ptr)->GetCreatureProperties())
                {
                    RET_INT(static_cast<Creature*>(ptr)->GetCreatureProperties()->Faction);
                }
                else
                {
                    RET_INT(ptr->GetFaction());
                }
            }
    }
    static int SetStandState(lua_State* L, Unit* ptr)   //states 0..8
    {
        if (!ptr)
            return 0;
        uint8_t state = static_cast<uint8_t>(luaL_checkinteger(L, 1));
        if (state < 0)
            return 0;
        ptr->SetStandState(state);
        return 0;
    }
    static int IsInCombat(lua_State* L, Unit* ptr)
    {
        if (ptr == NULL || !ptr->IsInWorld())
            RET_NIL()
            if (ptr->CombatStatus.IsInCombat())
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);
        return 1;
    }

    static int SetScale(lua_State* L, Unit* ptr)
    {
        float scale = CHECK_FLOAT(L, 1);
        if (scale && ptr)
            ptr->setFloatValue(OBJECT_FIELD_SCALE_X, (float)scale);
        else
            RET_BOOL(false)
            RET_BOOL(true)
    }

    static int SetModel(lua_State* L, Unit* ptr)
    {
        uint32 model = CHECK_ULONG(L, 1);
        if (ptr != NULL)
            ptr->SetDisplayId(model);
        else
            RET_BOOL(false)
            RET_BOOL(true)
    }

    static int SetNPCFlags(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        int flags = static_cast<int>(luaL_checkinteger(L, 1));
        ptr->setUInt32Value(UNIT_NPC_FLAGS, flags);
        return 0;
    }
    static int SetMount(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        uint32 DsplId = CHECK_ULONG(L, 1);
        ptr->SetMount(DsplId);
        return 0;
    }

    static int DestroyCustomWaypointMap(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            static_cast<Creature*>(ptr)->DestroyCustomWaypointMap();
        return 0;
    }

    static int CreateCustomWaypointMap(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            Creature* pCreature = static_cast<Creature*>(ptr);
        if (pCreature->m_custom_waypoint_map)
        {
            pCreature->GetAIInterface()->SetWaypointMap(nullptr);
        }

        pCreature->m_custom_waypoint_map = new Movement::WayPointMap;
        pCreature->GetAIInterface()->SetWaypointMap(pCreature->m_custom_waypoint_map);
        return 0;
    }

    static int CreateWaypoint(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float z = CHECK_FLOAT(L, 3);
        float o = CHECK_FLOAT(L, 4);
        int waittime = static_cast<int>(luaL_checkinteger(L, 5));
        int flags = static_cast<int>(luaL_checkinteger(L, 6));
        int modelid = static_cast<int>(luaL_checkinteger(L, 7));

        Creature* pCreature = static_cast<Creature*>(ptr);
        if (!pCreature->m_custom_waypoint_map)
        {
            pCreature->m_custom_waypoint_map = new Movement::WayPointMap;
            pCreature->GetAIInterface()->SetWaypointMap(pCreature->m_custom_waypoint_map);
        }

        if (!modelid)
            modelid = pCreature->GetDisplayId();

        Movement::WayPoint* wp = new Movement::WayPoint;
        wp->id = (uint32)pCreature->m_custom_waypoint_map->size() + 1;
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->o = o;
        wp->flags = flags;
        wp->backwardskinid = modelid;
        wp->forwardskinid = modelid;
        wp->backwardemoteid = wp->forwardemoteid = 0;
        wp->backwardemoteoneshot = wp->forwardemoteoneshot = false;
        wp->waittime = waittime;
        if (pCreature->GetAIInterface()->addWayPointUnsafe(wp))
            pCreature->m_custom_waypoint_map->push_back(wp);
        else
        {
            LogDetail("WayPoint created by a Lua script for Creature ID %u wasn't added due to an error occurred in CreateWaypoint()", pCreature->GetCreatureProperties()->Id);
            delete wp;
        }
        return 0;
    }
    static int CreateCustomWaypoint(lua_State* L, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;
        else
        {
            Creature* crc = static_cast<Creature*>(ptr);
            uint32 id = CHECK_ULONG(L, 1);
            float x = CHECK_FLOAT(L, 2);
            float y = CHECK_FLOAT(L, 3);
            float z = CHECK_FLOAT(L, 4);
            float o = CHECK_FLOAT(L, 5);
            uint32 waitime = CHECK_ULONG(L, 6);
            uint32 flags = CHECK_ULONG(L, 7);
            uint32 model = static_cast<uint32>(luaL_optinteger(L, 8, 0));
            Movement::WayPoint* wp = new Movement::WayPoint;
            wp->id = id;
            wp->x = x;
            wp->y = y;
            wp->z = z;
            wp->o = o;
            wp->waittime = waitime;
            wp->flags = flags;
            wp->backwardskinid = model;
            wp->forwardskinid = model;
            crc->GetAIInterface()->addWayPoint(wp);
        }
        return 0;
    }
    static int DeleteAllWaypoints(lua_State* /*L*/, Unit* ptr)
    {
        if (ptr != nullptr && ptr->IsCreature())
            ptr->GetAIInterface()->deleteAllWayPoints();
        return 0;
    }

    static int MoveToWaypoint(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        int id = static_cast<int>(luaL_checkinteger(L, 1));
        if (id)
        {
            ptr->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            ptr->GetAIInterface()->setWayPointToMove(id);
        }
        return 0;
    }

    static int RemoveItem(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 id = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 count = static_cast<uint32>(luaL_checkinteger(L, 2));

        static_cast<Player*>(ptr)->GetItemInterface()->RemoveItemAmt(id, count);
        return 0;
    }

    static int AddItem(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 id = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 count = static_cast<uint32>(luaL_checkinteger(L, 2));

        auto player = static_cast<Player*>(ptr);
        ItemProperties const* item_proto = sMySQLStore.getItemProperties(id);
        if (item_proto == nullptr)
            return 0;

        auto item_add = player->GetItemInterface()->FindItemLessMax(id, count, false);
        if (item_add == nullptr)
        {
            item_add = objmgr.CreateItem(id, player);
            if (item_add == nullptr)
                return 0;

            item_add->SetStackCount(count);
            if (player->GetItemInterface()->AddItemToFreeSlot(item_add))
                player->SendItemPushResult(false, true, false, true, player->GetItemInterface()->LastSearchItemBagSlot(),
                player->GetItemInterface()->LastSearchItemSlot(), count, item_add->GetEntry(), item_add->GetItemRandomSuffixFactor(),
                item_add->GetItemRandomPropertyId(), item_add->GetStackCount());
        }
        else
        {
            item_add->ModStackCount(count);
            item_add->SetDirty();
            player->SendItemPushResult(false, true, false, false,
                                       static_cast<uint8>(player->GetItemInterface()->GetBagSlotByGuid(item_add->GetGUID())), 0xFFFFFFFF,
                                       count, item_add->GetEntry(), item_add->GetItemRandomSuffixFactor(), item_add->GetItemRandomPropertyId(), item_add->GetStackCount());
        }
        PUSH_ITEM(L, item_add);
        return 1;
    }
    static int GetInstanceID(lua_State* L, Unit* ptr)
    {
        //TEST_UNIT()
        if (!ptr || ptr->GetMapMgr() == NULL || ptr->GetMapMgr()->GetMapInfo()->type == INSTANCE_NULL)
            lua_pushnil(L);
        else
            lua_pushinteger(L, ptr->GetInstanceID());
        return 1;
    }

    static int GetClosestPlayer(lua_State* L, Unit* ptr)
    {
        if (!ptr)
            return 0;

        float dist = 0;
        float d2 = 0;
        Player* ret = NULL;

        for (std::set< Object* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
        {
            d2 = (*itr)->getDistanceSq(ptr);
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
    static int GetRandomPlayer(lua_State* L, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;

        int flag = static_cast<int>(luaL_checkinteger(L, 1));
        Player* ret = nullptr;
        std::vector<Player*> players;
        switch (flag)
        {
            case RANDOM_ANY:
            {
                uint32 count = (uint32)ptr->GetInRangePlayersCount();
                uint32 r = RandomUInt(count - 1);
                count = 0;
                for (std::set< Object* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
                {
                    if (count == r)
                    {
                        ret = static_cast<Player*>(*itr);
                        break;
                    }
                    ++count;
                }
            }
            break;
            case RANDOM_IN_SHORTRANGE:

            {
                for (std::set< Object* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
                {
                    Player* obj = static_cast<Player*>(*itr);
                    if (obj && obj->CalcDistance(obj, ptr) <= 8)
                        players.push_back(obj);
                }
                if (players.size())
                    ret = players[RandomUInt(static_cast<uint32>(players.size() - 1))];
            }
            break;
            case RANDOM_IN_MIDRANGE:
            {
                for (std::set< Object* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
                {
                    Player* obj = static_cast<Player*>(*itr);
                    float distance = obj->CalcDistance(obj, ptr);
                    if (distance < 20 && distance > 8)
                        players.push_back(obj);
                }
                if (players.size())
                    ret = players[RandomUInt(static_cast<uint32>(players.size() - 1))];
            }
            break;
            case RANDOM_IN_LONGRANGE:
            {
                for (std::set< Object* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
                {
                    Player* obj = static_cast<Player*>(*itr);
                    if (obj && obj->CalcDistance(obj, ptr) >= 20)
                        players.push_back(obj);
                }
                if (players.size())
                    ret = players[RandomUInt(static_cast<uint32>(players.size() - 1))];
            }
            break;
            case RANDOM_WITH_MANA:
            {
                for (std::set< Object* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
                {
                    Player* obj = static_cast<Player*>(*itr);
                    if (obj && obj->GetPowerType() == POWER_TYPE_MANA)
                        players.push_back(obj);
                }
                if (players.size())
                    ret = players[RandomUInt(static_cast<uint32>(players.size() - 1))];
            }
            break;
            case RANDOM_WITH_ENERGY:
            {
                for (std::set< Object* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
                {
                    Player* obj = static_cast<Player*>(*itr);
                    if (obj && obj->GetPowerType() == POWER_TYPE_ENERGY)
                        players.push_back(obj);
                }
                if (players.size())
                    ret = players[RandomUInt(static_cast<uint32>(players.size() - 1))];
            }
            break;
            case RANDOM_WITH_RAGE:
            {
                for (std::set< Object* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
                {
                    Player* obj = static_cast<Player*>(*itr);
                    if (obj && obj->GetPowerType() == POWER_TYPE_RAGE)
                        players.push_back(obj);
                }
                if (players.size())
                    ret = players[RandomUInt(static_cast<uint32>(players.size() - 1))];
            }
            break;
            case RANDOM_NOT_MAINTANK:
            {
                Unit* mt = ptr->GetAIInterface()->GetMostHated();
                if (mt == nullptr || !mt->IsPlayer())
                    return 0;

                for (std::set< Object* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
                {
                    Player* obj = static_cast<Player*>(*itr);
                    if (obj != mt)
                        players.push_back(obj);
                }
                if (players.size())
                    ret = players[RandomUInt(static_cast<uint32>(players.size() - 1))];
            }
            break;
        }

        if (ret == nullptr)
            lua_pushnil(L);
        else
            PUSH_UNIT(L, ret);

        return 1;
    }
    static int GetRandomFriend(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER();

        std::vector<Object*> allies;

        for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
        {
            Object* obj = *itr;
            if (obj->IsUnit() && isFriendly(obj, ptr))
                allies.push_back(obj);
        }
        if (allies.size())
            PUSH_UNIT(L, allies[RandomUInt(static_cast<uint32>(allies.size() - 1))]);
        else
            lua_pushnil(L);
        return 1;
    }
    static int GetRandomEnemy(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER();

        std::vector<Object*> enemies;

        for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
        {
            Object* obj = *itr;
            if (obj->IsUnit() && isHostile(ptr, obj))
                enemies.push_back(obj);
        }
        if (enemies.size())
            PUSH_UNIT(L, enemies[RandomUInt(static_cast<uint32>(enemies.size() - 1))]);
        else
            lua_pushnil(L);
        return 1;
    }

    static int StopMovement(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        uint32 tim = static_cast<uint32>(luaL_checkinteger(L, 1));
        ptr->GetAIInterface()->StopMovement(tim);
        return 0;
    }

    static int RemoveAura(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
        uint32 auraid = static_cast<uint32>(luaL_checkinteger(L, 1));
        ptr->RemoveAura(auraid);
        return 0;
    }

    static int CanAttack(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        Unit* target = CHECK_UNIT(L, 1);
        if (!target) return 0;
        if (isAttackable(ptr, target))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int PlaySoundToSet(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER();
        uint32 soundid = static_cast<uint32>(luaL_checkinteger(L, 1));
        ptr->PlaySoundToSet(soundid);
        return 0;
    }

    static int PlaySoundToPlayer(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER();
        uint32 soundid = static_cast<uint32>(luaL_checkinteger(L, 1));
        Player* plr = static_cast<Player*>(ptr);
        plr->PlaySoundToPlayer(plr->GetGUID(), soundid);
        return 0;
    }

    static int GetUnitBySqlId(lua_State* L, Unit* ptr)
    {
        uint32 sqlid = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (!ptr || !sqlid)
            return 0;
        PUSH_UNIT(L, ptr->GetMapMgr()->GetSqlIdCreature(sqlid));
        return 1;
    }

    static int GetInventoryItem(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        int8 containerslot = static_cast<int8>(luaL_checkinteger(L, 1));
        int16 slot = static_cast<int16>(luaL_checkinteger(L, 2));
        Player* plr = static_cast<Player*>(ptr);
        PUSH_ITEM(L, plr->GetItemInterface()->GetInventoryItem(containerslot, slot));
        return 1;
    }

    static int GetInventoryItemById(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            uint32 entry = CHECK_ULONG(L, 1);
        Player* plr = static_cast<Player*>(ptr);
        int16 slot = plr->GetItemInterface()->GetInventorySlotById(entry);
        if (slot == -1)  //check bags
        {
            for (uint8 contslot = INVENTORY_SLOT_BAG_START; contslot != INVENTORY_SLOT_BAG_END; contslot++)
            {
                Container* bag = static_cast< Container* >(plr->GetItemInterface()->GetInventoryItem(contslot));
                if (bag == NULL)
                    continue;
                for (uint8 bslot = 0; bslot != bag->GetNumSlots(); bslot++)
                {
                    if (bag->GetItem(bslot) && bag->GetItem(bslot)->GetEntry() == entry)
                    {
                        PUSH_ITEM(L, bag->GetItem(bslot));
                        return 1;
                    }
                }
            }
        }
        PUSH_ITEM(L, plr->GetItemInterface()->GetInventoryItem(slot));
        return 1;
    }

    static int SetZoneWeather(lua_State* L, Unit* /*ptr*/)
    {
        /*
        WEATHER_TYPE_NORMAL            = 0, // NORMAL (SUNNY)
        WEATHER_TYPE_FOG               = 1, // FOG
        WEATHER_TYPE_RAIN              = 2, // RAIN
        WEATHER_TYPE_HEAVY_RAIN        = 4, // HEAVY_RAIN
        WEATHER_TYPE_SNOW              = 8, // SNOW
        WEATHER_TYPE_SANDSTORM         = 16 // SANDSTORM
        */
        uint32 zone_id = CHECK_ULONG(L, 1);
        uint32 type = CHECK_ULONG(L, 2);
        float Density = CHECK_FLOAT(L, 3); //min: 0.30 max: 2.00
        if (Density < 0.30f || Density > 2.0f || !zone_id || !type)
            return 0;

        uint32 sound;
        if (Density <= 0.30f)
            sound = 0;

        switch (type)
        {
            case 2:                                             //rain
            case 4:
                if (Density < 0.40f)
                    sound = 8533;
                else if (Density < 0.70f)
                    sound = 8534;
                else
                    sound = 8535;
                break;
            case 8:                                             //snow
                if (Density < 0.40f)
                    sound = 8536;
                else if (Density < 0.70f)
                    sound = 8537;
                else
                    sound = 8538;
                break;
            case 16:                                             //storm
                if (Density < 0.40f)
                    sound = 8556;
                else if (Density < 0.70f)
                    sound = 8557;
                else
                    sound = 8558;
                break;
            default:                                            //no sound
                sound = 0;
                break;
        }
        WorldPacket data(SMSG_WEATHER, 9);
        data.Initialize(SMSG_WEATHER);
        if (type == 0)  // set all parameter to 0 for sunny.
        {
            data << uint32(0);
            data << float(0);
            data << uint32(0);
            data << uint8(0);
        }
        else if (type == 1)  // No sound/density for fog
        {
            data << type;
            data << float(0);
            data << uint32(0);
            data << uint8(0);
        }
        else
        {
            data << type;
            data << Density;
            data << sound;
            data << uint8(0);
        }

        sWorld.sendZoneMessage(&data, zone_id);

        return 0;
    }

    static int SetPlayerWeather(lua_State* L, Unit* ptr)
    {
        /*
        WEATHER_TYPE_NORMAL            = 0, // NORMAL (SUNNY)
        WEATHER_TYPE_FOG               = 1, // FOG
        WEATHER_TYPE_RAIN              = 2, // RAIN
        WEATHER_TYPE_HEAVY_RAIN        = 4, // HEAVY_RAIN
        WEATHER_TYPE_SNOW              = 8, // SNOW
        WEATHER_TYPE_SANDSTORM         = 16 // SANDSTORM
        */
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        uint32 type = CHECK_ULONG(L, 1);
        float Density = CHECK_FLOAT(L, 2); //min: 0.30 max: 2.00
        if (Density < 0.30f || Density > 2.0f || !type)
            return 0;

        uint32 sound;
        if (Density <= 0.30f)
            sound = 0;

        switch (type)
        {
            case 2:                                             //rain
            case 4:
                if (Density < 0.40f)
                    sound = 8533;
                else if (Density < 0.70f)
                    sound = 8534;
                else
                    sound = 8535;
                break;
            case 8:                                             //snow
                if (Density < 0.40f)
                    sound = 8536;
                else if (Density < 0.70f)
                    sound = 8537;
                else
                    sound = 8538;
                break;
            case 16:                                             //storm
                if (Density < 0.40f)
                    sound = 8556;
                else if (Density < 0.70f)
                    sound = 8557;
                else
                    sound = 8558;
                break;
            default:                                            //no sound
                sound = 0;
                break;
        }
        WorldPacket data(SMSG_WEATHER, 9);
        data.Initialize(SMSG_WEATHER);
        if (type == 0)  // set all parameter to 0 for sunny.
        {
            data << uint32(0);
            data << float(0);
            data << uint32(0);
            data << uint8(0);
        }
        else if (type == 1)  // No sound/density for fog
        {
            data << type;
            data << float(0);
            data << uint32(0);
            data << uint8(0);
        }
        else
        {
            data << type;
            data << Density;
            data << sound;
            data << uint8(0);
        }

        plr->GetSession()->SendPacket(&data);

        return 0;
    }

    static int Despawn(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        uint32 delay = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 respawntime = static_cast<uint32>(luaL_checkinteger(L, 2));
        static_cast<Creature*>(ptr)->Despawn(delay, respawntime);
        return 0;
    }
    static int GetInRangeFriends(lua_State* L, Unit* ptr)
    {
        if (ptr == NULL)
            return 0;
        Object* pC = NULL;
        uint32 count = 0;
        lua_newtable(L);
        for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
        {
            if ((*itr)->IsUnit() && isFriendly(ptr, (*itr)))
            {
                count++,
                    pC = *itr;
                lua_pushinteger(L, count);
                PUSH_UNIT(L, pC);
                lua_rawset(L, -3);
            }
        }
        return 1;
    }

    static int GetInRangeEnemies(lua_State* L, Unit* ptr)
    {
        if (ptr == NULL)
            return 0;
        uint32 count = 0;
        lua_newtable(L);
        for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
        {
            if ((*itr)->IsUnit() && !isFriendly(ptr, (*itr)))
            {
                count++,
                    lua_pushinteger(L, count);
                PUSH_UNIT(L, *itr);
                lua_rawset(L, -3);
            }
        }
        return 1;
    }

    static int GetInRangeUnits(lua_State* L, Unit* ptr)
    {
        if (ptr == NULL)
            return 0;
        uint32 count = 0;
        lua_newtable(L);
        for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
        {
            if ((*itr)->IsUnit())
            {
                ++count;
                lua_pushinteger(L, count);
                PUSH_UNIT(L, *itr);
                lua_rawset(L, -3);
            }
        }
        return 1;
    }

    static int GetHealthPct(lua_State* L, Unit* ptr)
    {
        if (!ptr)
            lua_pushinteger(L, 0);
        else
            lua_pushinteger(L, ptr->GetHealthPct());
        return 1;
    }

    static int SetHealthPct(lua_State* L, Unit* ptr)
    {
        uint32 val = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (val && ptr)
            ptr->SetHealthPct(val);
        return 0;
    }

    static int GetItemCount(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 itemid = static_cast<uint32>(luaL_checkinteger(L, 1));
        lua_pushinteger(L, static_cast<Player*>(ptr)->GetItemInterface()->GetItemCount(itemid, false));
        return 1;
    }

    static int GetPrimaryCombatTarget(lua_State* L, Unit* ptr)
    {
        //should use now instead of GetTarget
        TEST_PLAYER()
            if (!ptr->CombatStatus.IsInCombat())
            {
                lua_pushinteger(L, 0);
                return 1;
            }
            else
                PUSH_UNIT(L, ptr->GetMapMgr()->GetUnit(static_cast<Player*>(ptr)->CombatStatus.GetPrimaryAttackTarget()));
        return 1;
    }

    static int GetMainTank(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Unit* ret = ptr->GetAIInterface()->GetMostHated();
        if (!ret)
            lua_pushnil(L);
        else
            PUSH_UNIT(L, ret);
        return 1;
    }

    static int GetAddTank(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Unit* ret = ptr->GetAIInterface()->GetSecondHated();
        if (ret == NULL)
            lua_pushnil(L);
        else
            PUSH_UNIT(L, ret);
        return 1;
    }

    static int ClearThreatList(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            ptr->ClearHateList();
        return 0;
    }

    static int GetTauntedBy(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            if (!ptr->GetAIInterface()->getTauntedBy())
                lua_pushnil(L);
            else
                PUSH_UNIT(L, ptr->GetAIInterface()->getTauntedBy());
        return 1;
    }

    static int SetTauntedBy(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Unit* target = CHECK_UNIT(L, 1);
        if (!target || ptr->GetAIInterface()->GetIsTaunted() || target == ptr)
            return 0;
        else
            ptr->GetAIInterface()->taunt(target);
        return 0;
    }

    static int ModThreat(lua_State* L, Unit* ptr)
    {
        Unit* target = CHECK_UNIT(L, 1);
        int32 amount = static_cast<int32>(luaL_checkinteger(L, 2));
        if (ptr && target && amount)
            ptr->GetAIInterface()->modThreatByPtr(target, amount);
        return 0;
    }

    static int GetThreatByPtr(lua_State* L, Unit* ptr)
    {
        Unit* target = CHECK_UNIT(L, 1);
        if (ptr && target)
            lua_pushnumber(L, ptr->GetAIInterface()->getThreatByPtr(target));
        return 1;
    }

    static int GetSoulLinkedWith(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            if (!ptr->GetAIInterface()->GetIsSoulLinked())
                lua_pushnil(L);
            else
                PUSH_UNIT(L, ptr->GetAIInterface()->getSoullinkedWith());
        return 1;
    }

    static int SetSoulLinkedWith(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Unit* target = CHECK_UNIT(L, 1);
        if (!target || ptr->GetAIInterface()->GetIsSoulLinked() || target == ptr)
            return 0;
        else
            ptr->GetAIInterface()->SetSoulLinkedWith(ptr);
        return 1;
    }

    static int ChangeTarget(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Unit* target = CHECK_UNIT(L, 1);
        if (!target || !isHostile(ptr, target) || ptr == target)
            return 0;
        else
            ptr->GetAIInterface()->setNextTarget(target);
        return 0;
    }

    static int HasFinishedQuest(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER_RET()
        uint32 questid = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (static_cast<Player*>(ptr)->HasFinishedQuest(questid))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int FinishQuest(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER_RET()
        uint32 quest_id = static_cast<uint32>(luaL_checkinteger(L, 1));
        Player* plr = static_cast<Player*>(ptr);

        QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
        if (qst)
        {
            if (plr->HasFinishedQuest(quest_id))
            {
                lua_pushnumber(L, 0);
                return 1;
            }
            else
            {
                QuestLogEntry* IsPlrOnQuest = plr->GetQuestLogForEntry(quest_id);
                if (IsPlrOnQuest)
                {
                    sQuestMgr.GenerateQuestXP(plr, qst);
                    sQuestMgr.BuildQuestComplete(plr, qst);

                    IsPlrOnQuest->Finish();
                    plr->AddToFinishedQuests(quest_id);
                    lua_pushnumber(L, 1);
                    return 1;
                }
                else
                {
                    lua_pushnumber(L, 2);
                    return 1;
                }
            }
        }
        else
            return 0;
    }

    static int StartQuest(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER_RET()
        uint32 quest_id = static_cast<uint32>(luaL_checkinteger(L, 1));
        Player* plr = static_cast<Player*>(ptr);

        QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
        if (qst)
        {
            if (plr->HasFinishedQuest(quest_id))
            {
                lua_pushnumber(L, 0);
                return 1;
            }
            else
            {
                QuestLogEntry* IsPlrOnQuest = plr->GetQuestLogForEntry(quest_id);
                if (IsPlrOnQuest)
                {
                    lua_pushnumber(L, 1);
                    return 1;
                }
                else
                {
                    int32 open_slot = plr->GetOpenQuestSlot();

                    if (open_slot == -1)
                    {
                        sQuestMgr.SendQuestLogFull(plr);
                        lua_pushnumber(L, 2);
                        return 1;
                    }
                    else
                    {
                        QuestLogEntry* qle = new QuestLogEntry();
                        qle->Init(qst, plr, (uint32)open_slot);
                        qle->UpdatePlayerFields();

                        // If the quest should give any items on begin, give them the items.
                        for (uint8 i = 0; i < 4; ++i)
                        {
                            if (qst->receive_items[i])
                            {
                                Item* item = objmgr.CreateItem(qst->receive_items[i], plr);
                                if (item == NULL)
                                    return false;

                                if (!plr->GetItemInterface()->AddItemToFreeSlot(item))
                                    item->DeleteMe();
                            }
                        }

                        if (qst->srcitem && qst->srcitem != qst->receive_items[0])
                        {
                            Item* item = objmgr.CreateItem(qst->srcitem, plr);
                            if (item)
                            {
                                item->SetStackCount(qst->srcitemcount ? qst->srcitemcount : 1);
                                if (!plr->GetItemInterface()->AddItemToFreeSlot(item))
                                    item->DeleteMe();
                            }
                        }

                        sHookInterface.OnQuestAccept(plr, qst, NULL);

                        lua_pushnumber(L, 3);
                        return 1;
                    }
                }
            }
        }

        return 0;

    } //StartQuest

    static int UnlearnSpell(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 spellid = static_cast<uint32>(luaL_checkinteger(L, 1));
        static_cast<Player*>(ptr)->removeSpell(spellid, false, false, 0);
        return 0;
    }

    static int LearnSpell(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 spellid = static_cast<uint32>(luaL_checkinteger(L, 1));
        static_cast<Player*>(ptr)->addSpell(spellid);
        return 0;
    }
    static int LearnSpells(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER_RET()
            if (!strcmp("table", luaL_typename(L, 1)))
            {
                int table = lua_gettop(L);
                lua_pushnil(L);
                while (lua_next(L, table) != 0)
                {
                    if (lua_isnumber(L, -1))
                        static_cast<Player*>(ptr)->addSpell(CHECK_ULONG(L, -1));
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

    static int MarkQuestObjectiveAsComplete(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 questid = static_cast<uint32>(luaL_checkinteger(L, 1));
        int objective = static_cast<int>(luaL_checkinteger(L, 2));
        Player* pl = static_cast<Player*>(ptr);
        if (!pl->HasFinishedQuest(questid))
        {
            auto questlog_entry = pl->GetQuestLogForEntry(questid);
            if (questlog_entry != nullptr)
            {
                questlog_entry->SetMobCount(objective, questlog_entry->GetQuest()->required_mob_or_go[objective]);
                questlog_entry->SendUpdateAddKill(objective);
                if (questlog_entry->CanBeFinished())
                {
                    questlog_entry->SendQuestComplete();
                    questlog_entry->UpdatePlayerFields();
                }
            }
        }
        return 0;
    }

    static int SendAreaTriggerMessage(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* msg = luaL_checkstring(L, 1);
        if (!msg) return 0;
        static_cast<Player*>(ptr)->SendAreaTriggerMessage(msg);
        return 0;
    }

    static int SendBroadcastMessage(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* msg = luaL_checkstring(L, 1);
        if (!msg) return 0;
        static_cast<Player*>(ptr)->BroadcastMessage(msg);
        return 0;
    }

    static int TeleportUnit(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            uint32 mapId = CHECK_ULONG(L, 1);
        float posX = CHECK_FLOAT(L, 2);
        float posY = CHECK_FLOAT(L, 3);
        float posZ = CHECK_FLOAT(L, 4);
        float Orientation = CHECK_FLOAT(L, 5);
        if (!posX || !posY || !posZ || !mapId)
        {
            if (mapId)
            {
                LogNotice("LuaEngineMgr : LUATeleporter ERROR - Wrong Coordinates given (Map, X, Y, Z) :: Map%f%s%f%s%f%s%u", mapId, " X", posX, " Y", posY, " Z", posZ);
                return 0;
            }
            else
                mapId = 0; //MapId is false reported as empty if you use Eastern Kingdoms (0) So lets override it IF it is reported as empty.
        }
        LocationVector vec(posX, posY, posZ, Orientation);
        static_cast<Player*>(ptr)->SafeTeleport(mapId, 0, vec);
        return 0;
    }

    static int GetHealth(lua_State* L, Unit* ptr)
    {
        if (!ptr)
            lua_pushinteger(L, 0);
        else
            lua_pushinteger(L, ptr->getUInt32Value(UNIT_FIELD_HEALTH));
        return 1;
    }

    static int GetMaxHealth(lua_State* L, Unit* ptr)
    {
        if (!ptr)
            lua_pushinteger(L, 0);
        else
            lua_pushinteger(L, ptr->getUInt32Value(UNIT_FIELD_MAXHEALTH));

        return 1;
    }

    static int SetHealth(lua_State* L, Unit* ptr)
    {
        uint32 val = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (ptr != nullptr && val > 0)
        {
            if (val > ptr->getUInt32Value(UNIT_FIELD_MAXHEALTH))
                ptr->SetHealth(ptr->getUInt32Value(UNIT_FIELD_MAXHEALTH));
            else
                ptr->SetHealth(val);
        }
        return 0;
    }

    static int SetMaxHealth(lua_State* L, Unit* ptr)
    {
        uint32 val = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (ptr != nullptr && val > 0)
        {
            if (val < ptr->getUInt32Value(UNIT_FIELD_HEALTH))
                ptr->SetHealth(val);
            ptr->setUInt32Value(UNIT_FIELD_MAXHEALTH, val);
        }
        return 0;
    }

    static int WipeHateList(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            ptr->WipeHateList();
        ptr->GetAIInterface()->WipeHateList();
        return 0;
    }

    static int WipeTargetList(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            ptr->GetAIInterface()->WipeTargetList();
        return 0;
    }

    static int WipeCurrentTarget(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            ptr->GetAIInterface()->WipeCurrentTarget();
        return 0;
    }

    static int GetPlayerClass(lua_State* L, Unit* ptr)
    {
        if (!ptr || !ptr->IsPlayer())
        {
            lua_pushstring(L, "Unknown");
            return 1;
        }
        int plrclass = static_cast<Player*>(ptr)->getClass();

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


    static int ClearHateList(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            ptr->ClearHateList();
        return 0;
    }

    static int SetMana(lua_State* L, Unit* ptr)
    {
        uint32 val = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (ptr != nullptr)
            ptr->setUInt32Value(UNIT_FIELD_POWER1, val);
        return 0;
    }

    static int SetMaxMana(lua_State* L, Unit* ptr)
    {
        uint32 val = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (ptr != nullptr && val > 0)
        {
            if (val < ptr->GetPower(POWER_TYPE_MANA))
                ptr->SetPower(POWER_TYPE_MANA, val);
            ptr->SetMaxPower(POWER_TYPE_MANA, val);
        }
        return 1;
    }

    static int GetPlayerRace(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER_RET()
            lua_pushinteger(L, static_cast<Player*>(ptr)->getRace());
        return 1;
    }

    static int SetFlying(lua_State* /*L*/, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;

        ptr->setMoveHover(true);
        ptr->GetAIInterface()->setMeleeDisabled(true);
        ptr->GetAIInterface()->setSplineFlying();
        ptr->Emote(EMOTE_ONESHOT_LIFTOFF);
        return 0;
    }

    static int Land(lua_State* /*L*/, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;

        ptr->setMoveHover(false);
        ptr->GetAIInterface()->unsetSplineFlying();
        ptr->GetAIInterface()->setMeleeDisabled(false);
        ptr->Emote(EMOTE_ONESHOT_LAND);
        return 0;
    }

    static int HasAura(lua_State* L, Unit* ptr)
    {
        uint32 spellid = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (!ptr || !spellid)
            return 0;
        else
        {
            if (ptr->HasAura(spellid))
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);
        }
        return 1;
    }

    static int ReturnToSpawnPoint(lua_State* /*L*/, Unit* ptr)
    {
        if (ptr == nullptr)
        {
            return 0;
        }

        float x = ptr->GetSpawnX();
        float y = ptr->GetSpawnY();
        float z = ptr->GetSpawnZ();
        float o = ptr->GetSpawnO();

        if (ptr->IsCreature())
        {
            ptr->GetAIInterface()->MoveTo(x, y, z);
            ptr->SetOrientation(o);
        }

        return 0;
    }

    static int GetGUID(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        PUSH_GUID(L, ptr->GetGUID());
        return 1;
    }

    static int GetDistance(lua_State* L, Unit* ptr)
    {
        if (!ptr)
            return 0;
        Object* target = CHECK_OBJECT(L, 1);
        lua_pushnumber(L, (float)ptr->GetDistance2dSq(target));
        return 1;
    }

    static int GetDistanceYards(lua_State* L, Unit* ptr)
    {
        Object* target = CHECK_OBJECT(L, 1);
        if (!ptr || !target)
            return 0;
        LocationVector vec = ptr->GetPosition();
        lua_pushnumber(L, (float)vec.Distance(target->GetPosition()));
        return 1;
    }

    static int GetDuelState(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        lua_pushnumber(L, plr->GetDuelState());
        /*Returns
          DUEL_STATE_REQUESTED = 0
          DUEL_STATE_STARTED = 1
          DUEL_STATE_FINISHED = 2 (Default)
          */
        return 1;
    }

    static int GetCreatureNearestCoords(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        uint32 entryid = CHECK_ULONG(L, 4);
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float z = CHECK_FLOAT(L, 3);
        PUSH_UNIT(L, ptr->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(x, y, z, entryid));
        return 1;
    }

    static int GetGameObjectNearestCoords(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        uint32 entryid = CHECK_ULONG(L, 4);
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float z = CHECK_FLOAT(L, 3);
        PUSH_GO(L, ptr->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(x, y, z, entryid));
        return 1;
    }

    static int SetPosition(lua_State* L, Unit* ptr)
    {
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float z = CHECK_FLOAT(L, 3);
        float o = CHECK_FLOAT(L, 4);
        ptr->SetFacing(o);
        ptr->SetOrientation(o);

        WorldPacket data(SMSG_MONSTER_MOVE, 50);
        data << ptr->GetNewGUID();
        data << uint8(0);
        data << ptr->GetPositionX();
        data << ptr->GetPositionY();
        data << ptr->GetPositionZ();
        data << Util::getMSTime();
        data << uint8(0x00);
        data << uint32(256);
        data << uint32(1);
        data << uint32(1);
        data << x << y << z;

        ptr->SendMessageToSet(&data, true);
        ptr->SetPosition(x, y, z, o, true);
        return 0;
    }

    static int GetLandHeight(lua_State* L, Unit* ptr)
    {
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        if (!ptr || !x || !y)
            return 0;
        float lH = ptr->GetMapMgr()->GetADTLandHeight(x, y);
        lua_pushnumber(L, lH);
        return 1;
    }

    static int IsInPhase(lua_State* L, Unit* ptr)
    {
        uint32 phase = static_cast<uint32>(luaL_checkinteger(L, 1));
        lua_pushboolean(L, ((ptr->m_phase & phase) != 0) ? 1 : 0);
        return 1;
    }

    static int HasFlag(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        uint32 index = CHECK_ULONG(L, 1);
        uint32 flag = CHECK_ULONG(L, 2);
        lua_pushboolean(L, ptr->HasFlag(index, flag) ? 1 : 0);
        return 1;
    }

    static int QuestAddStarter(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Creature* unit = static_cast<Creature*>(ptr);
        uint32 quest_id = (uint32)luaL_checknumber(L, 1);
        if (!unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
            unit->setUInt32Value(UNIT_NPC_FLAGS, unit->getUInt32Value(UNIT_NPC_FLAGS) + UNIT_NPC_FLAG_QUESTGIVER);
        if (!quest_id)
            return 0;

        QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
        if (!qst)
            return 0;

        uint32 quest_giver = unit->GetEntry();

        char my_query1[200];
        sprintf(my_query1, "SELECT id FROM creature_quest_starter WHERE id = %d AND quest = %d", quest_giver, quest_id);
        QueryResult* selectResult1 = WorldDatabase.Query(my_query1);
        if (selectResult1)
            delete selectResult1; //already has quest
        else
        {
            char my_insert1[200];
            sprintf(my_insert1, "INSERT INTO creature_quest_starter (id, quest) VALUES (%d,%d)", quest_giver, quest_id);
            WorldDatabase.Execute(my_insert1);
        }
        sQuestMgr.LoadExtraQuestStuff();

        QuestRelation* qstrel = new QuestRelation;
        qstrel->qst = qst;
        qstrel->type = QUESTGIVER_QUEST_START;

        uint8 qstrelid;
        if (unit->HasQuests())
        {
            qstrelid = (uint8)unit->GetQuestRelation(quest_id);
            unit->DeleteQuest(qstrel);
        }
        unit->_LoadQuests();
        return 0;
    }

    static int QuestAddFinisher(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Creature* unit = static_cast<Creature*>(ptr);
        uint32 quest_id = CHECK_ULONG(L, 1);
        if (!unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
            unit->setUInt32Value(UNIT_NPC_FLAGS, unit->getUInt32Value(UNIT_NPC_FLAGS) + UNIT_NPC_FLAG_QUESTGIVER);
        if (!quest_id)
            return 0;

        QuestProperties const* qst = sMySQLStore.getQuestProperties(quest_id);
        if (!qst)
            return 0;

        uint32 quest_giver = unit->GetEntry();

        char my_query1[200];
        sprintf(my_query1, "SELECT id FROM creature_quest_finisher WHERE id = %d AND quest = %d", quest_giver, quest_id);
        QueryResult* selectResult1 = WorldDatabase.Query(my_query1);
        if (selectResult1)
        {
            delete selectResult1; //already has quest
        }
        else
        {
            char my_insert1[200];
            sprintf(my_insert1, "INSERT INTO creature_quest_finisher (id, quest) VALUES (%d,%d)", quest_giver, quest_id);
            WorldDatabase.Execute(my_insert1);
        }
        sQuestMgr.LoadExtraQuestStuff();

        QuestRelation* qstrel = new QuestRelation;
        qstrel->qst = qst;
        qstrel->type = QUESTGIVER_QUEST_END;

        uint8 qstrelid;
        if (unit->HasQuests())
        {
            qstrelid = (uint8)unit->GetQuestRelation(quest_id);
            unit->DeleteQuest(qstrel);
        }
        unit->_LoadQuests();
        return 0;
    }

    static int CastSpellAoF(lua_State* L, Unit* ptr)
    {
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float z = CHECK_FLOAT(L, 3);
        uint32 sp = CHECK_ULONG(L, 4);
        if (!sp || !ptr)
            return 0;
        ptr->CastSpellAoF(LocationVector(x, y, z), sSpellCustomizations.GetSpellInfo(sp), true);
        return 0;
    }

    static int FullCastSpellAoF(lua_State* L, Unit* ptr)
    {
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float z = CHECK_FLOAT(L, 3);
        uint32 sp = CHECK_ULONG(L, 4);
        if (!sp || !ptr)
            return 0;
        ptr->CastSpellAoF(LocationVector(x, y, z), sSpellCustomizations.GetSpellInfo(sp), false);
        return 0;
    }

    static int SetInFront(lua_State* L, Unit* ptr)
    {
        Unit* target = CHECK_UNIT(L, 1);
        if (!target || !ptr)
            return 0;
        ptr->GetAIInterface()->setInFront(target);
        return 0;
    }

    static int RemoveAllAuras(lua_State* /*L*/, Unit* ptr)
    {
        if (!ptr)
            return 0;
        ptr->RemoveAllAuras();
        return 0;
    }

    static int CancelSpell(lua_State* /*L*/, Unit* ptr)
    {
        if (!ptr)
            return 0;
        ptr->interruptSpell();
        return 0;
    }

    static int IsAlive(lua_State* L, Unit* ptr)
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

    static int IsDead(lua_State* L, Unit* ptr)
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

    static int IsInWorld(lua_State* L, Unit* ptr)
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

    static int GetZoneId(lua_State* L, Unit* ptr)
    {
        if (!ptr)
            return 0;
        lua_pushinteger(L, (ptr->GetZoneId()));
        return 1;
    }

    static int Root(lua_State* /*L*/, Unit* ptr)
    {
        if (ptr)
            ptr->setMoveRoot(true);
        return 0;
    }

    static int Unroot(lua_State* /*L*/, Unit* ptr)
    {
        if (ptr)
            ptr->setMoveRoot(false);
        return 0;
    }

    static int IsCreatureMoving(lua_State* L, Unit* ptr)
    {
        if (ptr && ptr->IsCreature())
        {
            if (ptr->GetAIInterface()->isCreatureState(MOVING))
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);
        }
        return 1;
    }

    static int SetOutOfCombatRange(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        uint32 range = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (range)
            ptr->GetAIInterface()->setOutOfCombatRange(range);
        return 0;
    }

    static int ModifyRunSpeed(lua_State* L, Unit* ptr)
    {
        float Speed = CHECK_FLOAT(L, 1);
        if (ptr && Speed)
            ptr->setSpeedForType(TYPE_RUN, Speed);
        return 0;
    }

    static int ModifyWalkSpeed(lua_State* L, Unit* ptr)
    {
        float Speed = CHECK_FLOAT(L, 1);
        if (ptr && Speed)
            ptr->setSpeedForType(TYPE_WALK, Speed);
        return 0;
    }

    static int ModifyFlySpeed(lua_State* L, Unit* ptr)
    {
        float Speed = CHECK_FLOAT(L, 1);
        if (ptr && Speed)
            ptr->setSpeedForType(TYPE_FLY, Speed);
        return 0;
    }

    static int isFlying(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            if (ptr->GetAIInterface()->isFlying())
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);
        return 1;
    }

    static int SendAIReaction(lua_State* /*L*/, Unit* ptr)
    {
        ptr->SendAIReaction();
        return 0;
    }

    static int SetOrientation(lua_State* L, Unit* ptr)
    {
        float O = CHECK_FLOAT(L, 1);
        if (ptr)
            ptr->SetOrientation(O);
        return 0;
    }

    static int GetSpawnX(lua_State* L, Unit* ptr)
    {
        if (ptr)
            lua_pushnumber(L, ptr->GetSpawnX());
        return 1;
    }

    static int GetSpawnY(lua_State* L, Unit* ptr)
    {
        if (ptr)
            lua_pushnumber(L, ptr->GetSpawnY());
        return 1;
    }

    static int GetSpawnZ(lua_State* L, Unit* ptr)
    {
        if (ptr)
            lua_pushnumber(L, ptr->GetSpawnZ());
        return 1;
    }

    static int GetSpawnO(lua_State* L, Unit* ptr)
    {
        if (ptr)
            lua_pushnumber(L, ptr->GetSpawnO());
        return 1;
    }

    static int GetInRangePlayersCount(lua_State* L, Unit* ptr)
    {
        if (ptr)
            lua_pushnumber(L, static_cast<lua_Number>(ptr->GetInRangePlayersCount()));
        return 1;
    }

    static int GetEntry(lua_State* L, Unit* ptr)
    {
        if (ptr)
            lua_pushnumber(L, ptr->GetEntry());
        return 1;
    }

    static int HandleEvent(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        Unit* target = CHECK_UNIT(L, 1);
        uint32 event_id = static_cast<uint32>(luaL_checkinteger(L, 2));
        uint32 misc_1 = static_cast<uint32>(luaL_checkinteger(L, 3));
        ptr->GetAIInterface()->HandleEvent(event_id, target, misc_1);
        return 1;
    }

    static int GetCurrentSpellId(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        uint32_t spellId = 0;
        for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
        {
            if (ptr->getCurrentSpell(CurrentSpellType(i)) == nullptr)
                continue;
            spellId = ptr->getCurrentSpell(CurrentSpellType(i))->GetSpellInfo()->getId();
            break;
        }
        if (spellId != 0)
            lua_pushnumber(L, spellId);
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetCurrentSpell(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
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

    static int AddAssistTargets(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Unit* Friend = CHECK_UNIT(L, 1);
        if (Friend)
        {
            if (isFriendly(ptr, Friend))
                ptr->GetAIInterface()->addAssistTargets(Friend);
        }
        return 0;
    }

    static int GetAIState(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            lua_pushnumber(L, ptr->GetAIInterface()->getAiState());
        return 1;
    }

    static int GetFloatValue(lua_State* L, Unit* ptr)
    {
        uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
        if (ptr)
            lua_pushnumber(L, ptr->getFloatValue(field));
        return 1;
    }

    static int SendPacket(lua_State* L, Unit* ptr)
    {
        WorldPacket* data = CHECK_PACKET(L, 1);
        int self = lua_toboolean(L, 2);
        if (data && ptr)
            ptr->SendMessageToSet(data, (self > 0) ? true : false);
        return 0;
    }

    static int SendPacketToGroup(lua_State* L, Unit* ptr)
    {
        WorldPacket* data = CHECK_PACKET(L, 1);
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        if (!data) return 0;
        plr->GetGroup()->SendPacketToAll(data);
        return 0;
    }

    static int SendPacketToPlayer(lua_State* L, Unit* ptr)
    {
        WorldPacket* data = CHECK_PACKET(L, 1);
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        if (data)
            plr->GetSession()->SendPacket(data);
        return 0;
    }

    static int ModUInt32Value(lua_State* L, Unit* ptr)
    {
        uint32 field = static_cast<uint32>(luaL_checkinteger(L, 1));
        int32 value = static_cast<int32>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->ModSignedInt32Value(field, value);
        return 0;
    }

    static int ModFloatValue(lua_State* L, Unit* ptr)
    {
        uint32 field = static_cast<uint32>(luaL_checkinteger(L, 1));
        float value = CHECK_FLOAT(L, 2);
        if (ptr)
            ptr->ModFloatValue(field, value);
        return 0;
    }

    static int SetUInt32Value(lua_State* L, Unit* ptr)
    {
        uint16 field = static_cast<uint16>(luaL_checkinteger(L, 1));
        uint32 value = static_cast<uint32>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->setUInt32Value(field, value);
        return 0;
    }

    static int SetUInt64Value(lua_State* L, Unit* ptr)
    {
        uint16 field = static_cast<uint16>(CHECK_ULONG(L, 1));
        uint64 guid = CHECK_GUID(L, 2);
        if (ptr)
            ptr->setUInt64Value(field, guid);
        return 0;
    }

    static int RemoveFlag(lua_State* L, Unit* ptr)
    {
        uint32 field = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 value = static_cast<uint32>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->RemoveFlag(field, value);
        return 0;
    }

    static int SetFlag(lua_State* L, Unit* ptr)
    {
        uint32 field = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 value = static_cast<uint32>(luaL_checkinteger(L, 2));
        if (ptr)
            ptr->SetFlag(field, value);
        return 0;
    }

    static int SetFloatValue(lua_State* L, Unit* ptr)
    {
        uint16 field = static_cast<uint16>(luaL_checkinteger(L, 1));
        float value = CHECK_FLOAT(L, 2);
        if (ptr)
            ptr->setFloatValue(field, value);
        return 0;
    }

    static int GetUInt32Value(lua_State* L, Unit* ptr)
    {
        uint16 field = static_cast<uint16>(luaL_checkinteger(L, 1));
        if (ptr)
            lua_pushnumber(L, ptr->getUInt32Value(field));
        return 1;
    }

    static int GetUInt64Value(lua_State* L, Unit* ptr)
    {
        uint16 field = static_cast<uint16>(luaL_checkinteger(L, 1));
        if (ptr)
            PUSH_GUID(L, ptr->getUInt64Value(field));
        return 1;
    }

    static int AdvanceQuestObjective(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 questid = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 objective = static_cast<uint32>(luaL_checkinteger(L, 2));
        Player* pl = static_cast<Player*>(ptr);
        QuestLogEntry* qle = pl->GetQuestLogForEntry(questid);
        if (qle != nullptr)
        {
            qle->SetMobCount(objective, qle->GetMobCount(objective) + 1);
            qle->SendUpdateAddKill(objective);
            if (qle->CanBeFinished())
                qle->SendQuestComplete();

            qle->UpdatePlayerFields();
        }
        return 0;
    }

    static int Heal(lua_State* L, Unit* ptr)
    {
        Unit* target = CHECK_UNIT(L, 1);
        uint32 spellid = CHECK_ULONG(L, 2);
        uint32 amount = CHECK_ULONG(L, 3);
        if (!target || !spellid || !amount || !ptr)
            return 0;
        ptr->Heal(target, spellid, amount);
        return 0;
    }

    static int Energize(lua_State* L, Unit* ptr)
    {
        Unit* target = CHECK_UNIT(L, 1);
        uint32 spellid = CHECK_ULONG(L, 2);
        uint32 amount = CHECK_ULONG(L, 3);
        uint32 type = CHECK_ULONG(L, 4);
        if (!target || !spellid || !amount || !type || !ptr)
            return 0;
        ptr->Energize(target, spellid, amount, type);
        return 0;
    }

    static int SendChatMessageAlternateEntry(lua_State* L, Unit* ptr)
    {
        uint32 entry = CHECK_ULONG(L, 1);
        uint8 type = static_cast<uint8>(CHECK_ULONG(L, 2));
        uint32 lang = CHECK_ULONG(L, 3);
        const char* msg = luaL_checkstring(L, 4);
        if (!entry || !lang || !msg)
            return 0;
        ptr->SendChatMessageAlternateEntry(entry, type, lang, msg);
        return 0;
    }

    static int SendChatMessageToPlayer(lua_State* L, Unit* ptr)
    {
        uint8 type = static_cast<uint8>(CHECK_ULONG(L, 1));
        uint32 lang = CHECK_ULONG(L, 2);
        const char* msg = luaL_checkstring(L, 3);
        Player* plr = CHECK_PLAYER(L, 4);
        if (!plr || !msg || !ptr)
            return 0;
        ptr->SendChatMessageToPlayer(type, lang, msg, plr);
        return 0;
    }

    static int GetManaPct(lua_State* L, Unit* ptr)
    {
        if (!ptr)
            return 0;
        if (ptr->GetPowerType() == (uint8)POWER_TYPE_MANA)
            lua_pushnumber(L, (int)(ptr->GetPower(POWER_TYPE_MANA) * 100.0f / ptr->GetMaxPower(POWER_TYPE_MANA)));
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetPowerPct(lua_State* L, Unit* ptr)
    {
        if (!ptr)
        {
            lua_pushnil(L);
            return 1;
        }
        uint16 powertype;
        if (luaL_optinteger(L, 1, -1) == -1)
            powertype = ptr->GetPowerType();
        else
            powertype = static_cast<uint16>(luaL_optinteger(L, 1, -1));

        lua_pushnumber(L, static_cast<int>(ptr->GetPower(powertype) * 100.0f / ptr->GetMaxPower(powertype)));
        return 1;
    }

    static int GetMana(lua_State* L, Unit* ptr)
    {
        if (ptr == NULL)
            lua_pushinteger(L, 0);
        else
            lua_pushinteger(L, ptr->GetPower(POWER_TYPE_MANA));

        return 1;
    }

    static int GetPower(lua_State* L, Unit* ptr)
    {
        if (!ptr)
        {
            lua_pushnil(L);
            return 1;
        }
        uint16 powertype;
        if (luaL_optinteger(L, 1, -1) == -1)
            powertype = ptr->GetPowerType();
        else
            powertype = static_cast<uint16>(luaL_optinteger(L, 1, -1));

        lua_pushnumber(L, ptr->GetPower(powertype));
        return 1;
    }

    static int GetMaxMana(lua_State* L, Unit* ptr)
    {
        if (ptr == NULL)
            lua_pushinteger(L, 0);
        else
            lua_pushinteger(L, ptr->GetMaxPower(POWER_TYPE_MANA));

        return 1;
    }

    static int GetMaxPower(lua_State* L, Unit* ptr)
    {
        if (!ptr)
        {
            lua_pushnil(L);
            return 1;
        }
        uint16 powertype;
        if (luaL_optinteger(L, 1, -1) == -1)
            powertype = ptr->GetPowerType();
        else
            powertype = static_cast<uint16>(luaL_optinteger(L, 1, -1));

        lua_pushnumber(L, ptr->GetMaxPower(powertype));
        return 1;
    }


    static int SetPowerType(lua_State* L, Unit* ptr)
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

        uint8 powertype;
        if (luaL_optinteger(L, 1, -1) == -1)
            powertype = ptr->GetPowerType();
        else
            powertype = static_cast<uint8>(luaL_optinteger(L, 1, -1));

        ptr->SetPowerType(powertype);
        return 0;
    }

    static int SetMaxPower(lua_State* L, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32 amount = static_cast<uint32>(luaL_checkinteger(L, 1));

        uint16 powertype;
        if (luaL_optinteger(L, 2, -1) == -1)
            powertype = ptr->GetPowerType();
        else
            powertype = static_cast<uint16>(luaL_optinteger(L, 2, -1));

        ptr->SetMaxPower(powertype, amount);
        return 0;
    }

    static int SetPower(lua_State* L, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32 amount = static_cast<uint32>(luaL_checkinteger(L, 1));

        uint32 powertype;
        if (luaL_optinteger(L, 2, -1) == -1)
            powertype = ptr->GetPowerType();
        else
            powertype = static_cast<uint32>(luaL_optinteger(L, 2, -1));

        ptr->SetPower(powertype, amount);
        return 0;
    }

    static int SetPowerPct(lua_State* L, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint32 amount = static_cast<uint32>(luaL_checkinteger(L, 1));

        uint16 powertype;
        if (luaL_optinteger(L, 2, -1) == -1)
            powertype = ptr->GetPowerType();
        else
            powertype = static_cast<uint16>(luaL_optinteger(L, 2, -1));

        ptr->SetPower(powertype, static_cast<int>(amount / 100) * (ptr->GetMaxPower(powertype)));
        return 0;
    }


    static int GetPowerType(lua_State* L, Unit* ptr)
    {
        if (!ptr)
            return 0;

        lua_pushinteger(L, ptr->GetPowerType());
        return 1;
    }

    static int Strike(lua_State* L, Unit* ptr)
    {
        TEST_UNIT_RET();

        Unit* target = CHECK_UNIT(L, 1);
        uint32 weapon_damage_type = static_cast<uint32>(luaL_checkinteger(L, 2));
        uint32 sp = CHECK_ULONG(L, 3);
        int32 adddmg = static_cast<int32>(luaL_checkinteger(L, 4));
        uint32 exclusive_damage = CHECK_ULONG(L, 5);
        int32 pct_dmg_mod = static_cast<int32>(luaL_checkinteger(L, 6));

        if (!target)
            return 0;
        ptr->Strike(target, weapon_damage_type, sSpellCustomizations.GetSpellInfo(sp), adddmg, pct_dmg_mod, exclusive_damage, false, false);
        return 0;
    }

    static int SetAttackTimer(lua_State* L, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;

        int32 timer = static_cast<int32>(luaL_checkinteger(L, 1));
        bool offhand = CHECK_BOOL(L, 2);
        if (!timer)
            return 0;
        ptr->setAttackTimer(timer, offhand);
        return 0;
    }

    static int Kill(lua_State* L, Unit* ptr)
    {
        Unit* target = CHECK_UNIT(L, 1);
        if (!ptr || !target)
            return 0;
        ptr->DealDamage(target, target->getUInt32Value(UNIT_FIELD_HEALTH), 0, 0, 0, true);
        return 0;
    }

    static int DealDamage(lua_State* L, Unit* ptr)
    {
        Unit* target = CHECK_UNIT(L, 1);
        uint32 damage = CHECK_ULONG(L, 2);
        uint32 spellid = CHECK_ULONG(L, 3);
        if (!ptr || !target)
            return 0;
        ptr->DealDamage(target, damage, 0, 0, spellid, true);
        return 0;
    }

    static int SetNextTarget(lua_State* L, Unit* ptr)
    {
        Unit* target = CHECK_UNIT(L, 1);
        if (ptr && target)
            ptr->GetAIInterface()->setNextTarget(target);
        return 0;
    }

    static int GetNextTarget(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            PUSH_UNIT(L, ptr->GetAIInterface()->getNextTarget());
        return 1;
    }

    static int SetPetOwner(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Unit* owner = CHECK_UNIT(L, 1);
        if (owner)
            ptr->GetAIInterface()->SetPetOwner(owner);
        return 0;
    }

    static int DismissPet(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            ptr->GetAIInterface()->DismissPet();
        return 0;
    }

    static int IsPet(lua_State* L, Unit* ptr)
    {
        if (ptr)
        {
            if (ptr->IsPet())
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);
        }
        return 1;
    }

    static int GetPetOwner(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            PUSH_UNIT(L, ptr->GetAIInterface()->GetPetOwner());
        return 1;
    }

    static int SetUnitToFollow(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()

            Unit* target = CHECK_UNIT(L, 1);
        float dist = CHECK_FLOAT(L, 2);
        float angle = CHECK_FLOAT(L, 3);

        ptr->GetAIInterface()->SetUnitToFollow(target);
        ptr->GetAIInterface()->SetFollowDistance(dist);
        ptr->GetAIInterface()->SetUnitToFollowAngle(angle);
        return 0;
    }

    static int GetUnitToFollow(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            PUSH_UNIT(L, ptr->GetAIInterface()->getUnitToFollow());
        return 1;
    }

    static int IsInFront(lua_State* L, Unit* ptr)
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

    static int IsInBack(lua_State* L, Unit* ptr)
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

    static int IsPacified(lua_State* L, Unit* ptr)
    {
        if (ptr)
            lua_pushboolean(L, (ptr->IsPacified()) ? 1 : 0);
        return 1;
    }

    static int SetPacified(lua_State* L, Unit* ptr)
    {
        bool pacified = CHECK_BOOL(L, 1);
        if (!ptr)
            return 0;
        ptr->m_pacified = pacified ? 1 : 0;
        if (pacified)
            ptr->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
        else
            ptr->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
        return 0;
    }

    static int IsFeared(lua_State* L, Unit* ptr)
    {
        if (ptr)
            lua_pushboolean(L, (ptr->IsFeared()) ? 1 : 0);
        return 1;
    }

    static int IsStunned(lua_State* L, Unit* ptr)
    {
        if (ptr)
            lua_pushboolean(L, (ptr->IsStunned()) ? 1 : 0);
        return 1;
    }

    static int CreateGuardian(lua_State* L, Unit* ptr)
    {
        uint32 entry = CHECK_ULONG(L, 1);
        //uint32 duration = CHECK_ULONG(L, 2);
        float angle = CHECK_FLOAT(L, 3);
        uint32 lvl = CHECK_ULONG(L, 4);

        if ((ptr == NULL) || (entry == 0) || (lvl == 0))
            return 0;

        CreatureProperties const* cp = sMySQLStore.getCreatureProperties(entry);
        if (cp == nullptr)
            return 0;

        LocationVector v(ptr->GetPosition());
        v.x += (3 * (cosf(angle + v.o)));
        v.y += (3 * (sinf(angle + v.o)));

        Summon* guardian = ptr->GetMapMgr()->CreateSummon(entry, SUMMONTYPE_GUARDIAN);

        guardian->Load(cp, ptr, v, 0, -1);
        guardian->GetAIInterface()->SetUnitToFollowAngle(angle);
        guardian->PushToWorld(ptr->GetMapMgr());

        if (guardian == NULL)
            return 0;

        PUSH_UNIT(L, guardian);

        return 1;
    }

    static int IsInArc(lua_State* L, Unit* ptr)
    {
        Unit* target = CHECK_UNIT(L, 1);
        float degrees = CHECK_FLOAT(L, 2);
        if (!target || !ptr || !degrees)
            return 0;
        else
        {
            if (ptr->isInArc(target, degrees))
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);
        }
        return 1;
    }

    static int IsInWater(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            if (ptr)
            {
                if (static_cast<Player*>(ptr)->m_UnderwaterState)
                    lua_pushboolean(L, 1);
                else
                    lua_pushboolean(L, 0);
            }
        return 1;
    }

    static int GetAITargetsCount(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        lua_pushnumber(L, static_cast<lua_Number>(ptr->GetAIInterface()->getAITargetsCount()));
        return 1;
    }

    static int GetUnitByGUID(lua_State* L, Unit* ptr)
    {
        uint64 guid = CHECK_GUID(L, 1);
        if (ptr && guid)
            PUSH_UNIT(L, ptr->GetMapMgr()->GetUnit(guid));
        return 1;
    }

    static int GetAITargets(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Unit* ret = NULL;
        TargetMap::iterator itr;
        lua_newtable(L);
        int count = 0;
        for (itr = ptr->GetAIInterface()->GetAITargets()->begin(); itr != ptr->GetAIInterface()->GetAITargets()->end(); ++itr)
        {
            ret = ptr->GetMapMgr()->GetUnit(itr->first);
            count++;
            lua_pushvalue(L, count);
            PUSH_UNIT(L, ret);
            lua_rawset(L, -3);
        }
        return 1;
    }

    static int GetInRangeObjectsCount(lua_State* L, Unit* ptr)
    {
        if (ptr)
            lua_pushnumber(L, static_cast<lua_Number>(ptr->GetInRangeCount()));
        return 1;
    }

    static int GetInRangePlayers(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        uint32 count = 0;
        lua_newtable(L);
        for (std::set< Object* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
        {
            if ((*itr)->IsPlayer())
            {
                count++,
                    lua_pushinteger(L, count);
                PUSH_UNIT(L, *itr);
                lua_rawset(L, -3);
            }
        }
        return 1;
    }

    static int GetGroupPlayers(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* _player = static_cast<Player*>(ptr);
        Group* party = _player->GetGroup();
        uint32 count = 0;
        lua_newtable(L);
        if (party)
        {
            GroupMembersSet::iterator itr;
            SubGroup* sgrp;
            party->getLock().Acquire();
            for (uint32 i = 0; i < party->GetSubGroupCount(); i++)
            {
                sgrp = party->GetSubGroup(i);
                for (itr = sgrp->GetGroupMembersBegin(); itr != sgrp->GetGroupMembersEnd(); ++itr)
                {
                    if ((*itr)->m_loggedInPlayer && (*itr)->m_loggedInPlayer->GetZoneId() == _player->GetZoneId() && _player->GetInstanceID() == (*itr)->m_loggedInPlayer->GetInstanceID())
                    {
                        count++,
                            lua_pushinteger(L, count);
                        PUSH_UNIT(L, (*itr)->m_loggedInPlayer);
                        lua_rawset(L, -3);
                    }
                }
            }
            party->getLock().Release();
        }
        return 1;
    }

    static int GetDungeonDifficulty(lua_State* L, Unit* ptr)
    {
        /*
        MODE_NORMAL_10MEN   =   0,
        MODE_NORMAL_25MEN   =   1,
        MODE_HEROIC_10MEN   =   2,
        MODE_HEROIC_25MEN   =   3
        */
        if (ptr->IsPlayer())
        {
            Player* plr = static_cast<Player*>(ptr);
            if (plr->GetGroup())
            {
                if (plr->GetGroup()->GetGroupType() == GROUP_TYPE_PARTY)
                    lua_pushnumber(L, plr->GetGroup()->m_difficulty);
                else
                    lua_pushnumber(L, plr->GetGroup()->m_raiddifficulty);
            }
            else
            {
                if (!plr->IsInInstance())
                    return 0;
                Instance* pInstance = sInstanceMgr.GetInstanceByIds(plr->GetMapId(), plr->GetInstanceID());
                lua_pushinteger(L, pInstance->m_difficulty);
            }
            return 1;
        }
        else
        {
            if (!ptr->IsInInstance())
            {
                lua_pushboolean(L, 0);
                return 1;
            }
            Instance* pInstance = sInstanceMgr.GetInstanceByIds(ptr->GetMapId(), ptr->GetInstanceID());
            lua_pushinteger(L, pInstance->m_difficulty);
        }
        return 1;
    }

    static int GetInstanceOwner(lua_State* L, Unit* ptr)
    {
        if (!ptr)
            return 0;

        if (!ptr->IsInInstance())
        {
            lua_pushnil(L);
        }
        else
        {
            Instance* pInstance = sInstanceMgr.GetInstanceByIds(ptr->GetMapId(), ptr->GetInstanceID());
            if (pInstance->m_creatorGuid != 0)  // creator guid is 0 if its owned by a group.
            {
                Player* owner = pInstance->m_mapMgr->GetPlayer(pInstance->m_creatorGuid);
                PUSH_UNIT(L, owner);
            }
            else
            {
                uint32 gId = pInstance->m_creatorGroup;
                auto group_id = objmgr.GetGroupById(gId);

                if (group_id == nullptr)
                    return 0;

                PUSH_UNIT(L, group_id->GetLeader()->m_loggedInPlayer);
            }
        }
        return 1;
    }

    static int IsGroupFull(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        lua_pushboolean(L, plr->GetGroup()->IsFull() ? 1 : 0);
        return 1;
    }

    static int GetGroupLeader(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        PUSH_UNIT(L, plr->GetGroup()->GetLeader()->m_loggedInPlayer);
        return 1;
    }

    static int SetGroupLeader(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* _plr = CHECK_PLAYER(L, 1);
        bool silent = CHECK_BOOL(L, 2);
        Player* plr = static_cast<Player*>(ptr);
        plr->GetGroup()->SetLeader(_plr, silent);
        return 0;
    }

    static int AddGroupMember(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        Player* plr = static_cast<Player*>(ptr);
        Player* _plr = CHECK_PLAYER(L, 1);
        int32 subgroup = static_cast<int32>(luaL_optinteger(L, 2, -1));
        plr->GetGroup()->AddMember(_plr->getPlayerInfo(), subgroup);
        return 0;
    }

    static int SetDungeonDifficulty(lua_State* L, Unit* ptr)
    {
        /*
        MODE_NORMAL_10MEN   =   0,
        MODE_NORMAL_25MEN   =   1,
        MODE_HEROIC_10MEN   =   2,
        MODE_HEROIC_25MEN   =   3
        */
        uint8 difficulty = static_cast<uint8>(CHECK_ULONG(L, 1));
        if (!ptr)
            return 0;
        if (ptr->IsInInstance())
        {
            if (ptr->IsPlayer())
            {
                Player* plr = static_cast<Player*>(ptr);
                if (plr->GetGroup())
                    (difficulty > 1 ? plr->GetGroup()->m_difficulty : plr->GetGroup()->m_raiddifficulty) = difficulty;
                else
                {
                    Instance* pInstance = sInstanceMgr.GetInstanceByIds(plr->GetMapId(), plr->GetInstanceID());
                    pInstance->m_difficulty = difficulty;
                }
            }
            else
            {
                Instance* pInstance = sInstanceMgr.GetInstanceByIds(ptr->GetMapId(), ptr->GetInstanceID());
                pInstance->m_difficulty = difficulty;
            }
        }
        return 0;
    }

    static int ExpandToRaid(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        plr->GetGroup()->ExpandToRaid();
        return 0;
    }

    static int GetInRangeGameObjects(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        lua_newtable(L);
        uint32 count = 0;
        for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
        {
            if ((*itr)->IsGameObject())
            {
                count++,
                    lua_pushinteger(L, count);
                PUSH_GO(L, *itr);
                lua_rawset(L, -3);
            }
        }
        return 1;
    }

    static int HasInRangeObjects(lua_State* L, Unit* ptr)
    {
        if (ptr)
        {
            if (ptr->HasInRangeObjects())
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);
        }
        return 1;
    }

    static int SetFacing(lua_State* L, Unit* ptr)
    {
        float newo = CHECK_FLOAT(L, 1);
        if (!ptr)
            return 0;
        ptr->SetFacing(newo);
        return 0;
    }

    static int CalcToDistance(lua_State* L, Unit* ptr)
    {
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float z = CHECK_FLOAT(L, 3);
        if (!ptr || !x || !y || !z)
            return 0;
        lua_pushnumber(L, ptr->CalcDistance(x, y, z));
        return 1;
    }

    static int CalcAngle(lua_State* L, Unit* ptr)
    {
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float x2 = CHECK_FLOAT(L, 3);
        float y2 = CHECK_FLOAT(L, 4);
        if (!x || !y || !x2 || !y2 || !ptr)
            return 0;
        lua_pushnumber(L, ptr->calcAngle(x, y, x2, y2));
        return 1;
    }

    static int CalcRadAngle(lua_State* L, Unit* ptr)
    {
        float ang = 0;
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float dx = CHECK_FLOAT(L, 3);
        float dy = CHECK_FLOAT(L, 4);
        if (!x || !y || !dx || !dy || !ptr)
            return 0;
        else
        {
            ang = ptr->calcRadAngle(x, y, dx, dy);
            lua_pushnumber(L, ang);
        }
        return 1;
    }

    static int IsInvisible(lua_State* L, Unit* ptr)   //THIS IS NOT "IS" IT'S SET!
    {
        if (!ptr) return 0;
        bool enabled = CHECK_BOOL(L, 1);
        if (enabled)
        {
            ptr->m_invisFlag = INVIS_FLAG_TOTAL;
            ptr->m_invisible = true;
        }
        else
        {
            ptr->m_invisFlag = INVIS_FLAG_NORMAL;
            ptr->m_invisible = false;
        }
        return 0;
    }

    static int MoveFly(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            bool enabled = CHECK_BOOL(L, 1);
        if (enabled)
            ptr->GetAIInterface()->setSplineFlying();
        else
            ptr->GetAIInterface()->unsetSplineFlying();
        return 0;
    }

    static int IsInvincible(lua_State* L, Unit* ptr)   //THIS IS NOT "IS" IT'S SET!
    {
        bool enabled = CHECK_BOOL(L, 1);
        if (ptr)
            ptr->bInvincible = enabled;
        return 0;
    }

    static int ResurrectPlayer(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            static_cast<Player*>(ptr)->RemoteRevive();
        return 0;
    }

    static int KickPlayer(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 delay = static_cast<uint32>(luaL_checkinteger(L, 1));
        static_cast<Player*>(ptr)->Kick(delay);
        return 0;
    }

    static int CanCallForHelp(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            bool enabled = CHECK_BOOL(L, 1);
        ptr->GetAIInterface()->m_canCallForHelp = enabled;
        return 0;
    }

    static int CallForHelpHp(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            float hp = CHECK_FLOAT(L, 1);
        ptr->GetAIInterface()->m_CallForHelpHealth = hp;
        return 0;
    }

    static int SetDeathState(lua_State* L, Unit* ptr)
    {
        int state = static_cast<int>(luaL_checkinteger(L, 1));
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

    static int SetCreatureName(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            return 0;
    }

    static int SetBindPoint(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float z = CHECK_FLOAT(L, 3);
        uint32 map = CHECK_ULONG(L, 4);
        uint32 zone = CHECK_ULONG(L, 5);
        if (!x || !y || !z || !zone)
            return 0;
        plr->SetBindPoint(x, y, z, map, zone);
        return 0;
    }

    static int SoftDisconnect(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            static_cast<Player*>(ptr)->SoftDisconnect();
        return 0;
    }

    static int Possess(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Unit* target = CHECK_UNIT(L, 1);
        if (target)
            static_cast<Player*>(ptr)->Possess(target);
        return 0;
    }

    static int Unpossess(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            static_cast<Player*>(ptr)->UnPossess();
        return 0;
    }

    static int RemoveFromWorld(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            Creature* unit = static_cast<Creature*>(ptr);
        if (unit->IsInWorld())
        {
            if (unit->m_spawn)
            {
                uint32 cellx = uint32(((_maxX - unit->m_spawn->x) / _cellSize));
                uint32 celly = uint32(((_maxY - unit->m_spawn->y) / _cellSize));

                if (cellx <= _sizeX && celly <= _sizeY)
                {
                    CellSpawns* sp = unit->GetMapMgr()->GetBaseMap()->GetSpawnsList(cellx, celly);
                    if (sp != NULL)
                    {
                        for (CreatureSpawnList::iterator itr = sp->CreatureSpawns.begin(); itr != sp->CreatureSpawns.end(); ++itr)
                            if ((*itr) == unit->m_spawn)
                            {
                                sp->CreatureSpawns.erase(itr);
                                break;
                            }
                    }
                    delete unit->m_spawn;
                    unit->m_spawn = NULL;
                }
            }
            unit->RemoveFromWorld(false, true);
        }
        return 0;
    }

    static int GetFaction(lua_State* L, Unit* ptr)
    {
        if (ptr)
            lua_pushnumber(L, ptr->GetFaction());
        return 1;
    }

    static int SpellNonMeleeDamageLog(lua_State* L, Unit* ptr)
    {
        Unit* pVictim = CHECK_UNIT(L, 1);
        uint32 spellid = CHECK_ULONG(L, 2);
        uint32 damage = CHECK_ULONG(L, 3);
        bool allowproc = CHECK_BOOL(L, 4);
        bool static_dmg = CHECK_BOOL(L, 5);
        bool no_remove_auras = CHECK_BOOL(L, 6);
        if (pVictim && spellid && damage)
        {
            ptr->SpellNonMeleeDamageLog(pVictim, spellid, damage, allowproc, static_dmg, no_remove_auras);
        }
        return 0;
    }

    static int NoRespawn(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            bool enabled = CHECK_BOOL(L, 1);
        static_cast<Creature*>(ptr)->m_noRespawn = enabled;
        return 0;
    }

    static int GetMapId(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        lua_pushnumber(L, ptr->GetMapId());
        return 1;
    }

    static int AttackReaction(lua_State* L, Unit* ptr)
    {
        Unit* target = CHECK_UNIT(L, 1);
        uint32 damage = static_cast<uint32>(luaL_checkinteger(L, 2));
        uint32 spell = static_cast<uint32>(luaL_checkinteger(L, 3));
        if (ptr && target && damage)
            ptr->GetAIInterface()->AttackReaction(target, damage, spell);
        return 0;
    }

    static int EventCastSpell(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
            Unit* target = CHECK_UNIT(L, 1);
        uint32 sp = CHECK_ULONG(L, 2);
        uint32 delay = CHECK_ULONG(L, 3);
        uint32 repeats = CHECK_ULONG(L, 4);
        if (ptr && sp)
        {
            switch (ptr->GetTypeId())
            {
                case TYPEID_PLAYER:
                    sEventMgr.AddEvent(ptr, &Player::EventCastSpell, target, sSpellCustomizations.GetSpellInfo(sp), EVENT_PLAYER_UPDATE, delay, repeats, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                    break;
                case TYPEID_UNIT:
                    sEventMgr.AddEvent(ptr, &Unit::EventCastSpell, target, sSpellCustomizations.GetSpellInfo(sp), EVENT_CREATURE_UPDATE, delay, repeats, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                    break;
            }
        }
        return 0;
    }

    static int IsPlayerMoving(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER_RET()
            lua_pushboolean(L, (static_cast<Player*>(ptr)->m_isMoving) ? 1 : 0);
        return 1;
    }

    static int IsPlayerAttacking(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER_RET()
            if (static_cast<Player*>(ptr)->IsAttacking())
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);
        return 1;
    }

    static int GetFactionStanding(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 faction = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (faction)
        {
            switch (static_cast<Player*>(ptr)->GetStandingRank(faction))
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

    static int SetPlayerAtWar(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER_RET()
            uint32 faction = CHECK_ULONG(L, 1);
        bool set = CHECK_BOOL(L, 3);
        if (faction)
        {
            static_cast<Player*>(ptr)->SetAtWar(faction, set);
        }
        return 0;
    }

    static int SetPlayerStanding(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 faction = static_cast<uint32>(luaL_checkinteger(L, 1));
        int32 value = static_cast<int32>(luaL_checkinteger(L, 2));
        if (faction && value)
            static_cast<Player*>(ptr)->SetStanding(faction, value);
        return 0;
    }

    static int SetPlayerSpeed(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        float Speed = CHECK_FLOAT(L, 1);
        if (Speed < 1 || Speed > 255)
            return 0;
        plr->setSpeedForType(TYPE_RUN, Speed);
        plr->setSpeedForType(TYPE_SWIM, Speed);
        plr->setSpeedForType(TYPE_RUN_BACK, Speed / 2);
        plr->setSpeedForType(TYPE_FLY, Speed * 2);
        return 0;
    }

    static int GiveHonor(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        uint32 honor = CHECK_ULONG(L, 1);
        plr->AddHonor(honor, true);
        return 0;
    }

    static int TakeHonor(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        uint32 honor = CHECK_ULONG(L, 1);

        if (plr->m_honorPoints < honor)
            plr->m_honorPoints = 0;
        else
            plr->m_honorPoints -= honor;

        if (plr->m_honorToday < honor)
            plr->m_honorToday = 0;
        else
            plr->m_honorToday -= honor;

        plr->UpdateHonor();
        return 0;
    }

    static int GetStanding(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 faction = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (faction)
            lua_pushinteger(L, static_cast<Player*>(ptr)->GetStanding(faction));
        return 1;
    }

    static int RemoveThreatByPtr(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Unit* target = CHECK_UNIT(L, 1);
        if (target)
            ptr->GetAIInterface()->RemoveThreatByPtr(target);
        return 0;
    }

    static int HasItem(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 itemid = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (itemid)
        {
            if (static_cast<Player*>(ptr)->GetItemInterface()->GetItemCount(itemid, false) > 0)
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);
        }
        return 1;
    }

    static int PlaySpellVisual(lua_State* L, Unit* ptr)
    {
        uint64 guid = CHECK_GUID(L, 1);
        uint32 spell = static_cast<uint32>(luaL_checkinteger(L, 2));
        if (ptr && guid && spell)
        {
            ptr->playSpellVisual(guid, spell);
        }
        return 1;
    }

    static int GetLevel(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        lua_pushinteger(L, ptr->getLevel());
        return 1;
    }

    static int SetLevel(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        uint32 level = CHECK_ULONG(L, 1);
        if (level <= worldConfig.player.playerLevelCap && level > 0)
        {
            if (ptr->IsPlayer())
            {
                LevelInfo* Info = objmgr.GetLevelInfo(ptr->getRace(), ptr->getClass(), level);
                if (Info)
                    static_cast<Player*>(ptr)->ApplyLevelInfo(Info, level);
            }
            else
                ptr->setLevel(level);
        }
        return 0;
    }

    static int AddSkill(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 skill = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 current = static_cast<uint32>(luaL_checkinteger(L, 2));
        uint32 max = static_cast<uint32>(luaL_checkinteger(L, 3));
        Player* plr = static_cast<Player*>(ptr);
        if (!max)
            max = 475;
        if (current > max)
            return 0;
        plr->_AddSkillLine(skill, current, max);
        plr->_UpdateMaxSkillCounts();
        return 0;
    }

    static int RemoveSkill(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 skill = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (!ptr || !skill)
            return 0;
        Player* plr = static_cast<Player*>(ptr);
        plr->_RemoveSkillLine(skill);
        plr->_UpdateMaxSkillCounts();
        return 0;
    }

    static int FlyCheat(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            bool enabled = CHECK_BOOL(L, 1);
        static_cast<Player*>(ptr)->FlyCheat = enabled;
        return 0;
    }

    static int AdvanceSkill(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 skill = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 count = static_cast<uint32>(luaL_checkinteger(L, 2));
        Player* plr = static_cast<Player*>(ptr);
        if (skill && count)
        {
            if (plr->_HasSkillLine(skill))
                plr->_AdvanceSkillLine(skill, count);
        }
        return 0;
    }

    static int RemoveAurasByMechanic(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
        uint32 mechanic = static_cast<uint32>(luaL_checkinteger(L, 1));
        bool hostileonly = CHECK_BOOL(L, 2);
        if (ptr && mechanic)
            ptr->RemoveAllAurasByMechanic(mechanic, 0, hostileonly);
        return 0;
    }

    static int RemoveAurasType(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
        uint32 type = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (ptr && type)
            ptr->RemoveAllAuraType(type);
        return 0;
    }

    static int AddAura(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
        uint32 spellid = static_cast<uint32>(luaL_checkinteger(L, 1));
        int32 duration = static_cast<int32>(luaL_checkinteger(L, 2));
        bool temp = CHECK_BOOL(L, 3);
        if (ptr && spellid)
        {
            Aura* aura = sSpellFactoryMgr.NewAura(sSpellCustomizations.GetSpellInfo(spellid), duration, ptr, ptr, temp);
            ptr->AddAura(aura);
            lua_pushboolean(L, 1);
        }
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int SetAIState(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        uint32 state = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (state)
        {
            switch (state)
            {
                case 0:
                    ptr->GetAIInterface()->setAiState(AI_STATE_IDLE);
                    break;
                case 1:
                    ptr->GetAIInterface()->setAiState(AI_STATE_ATTACKING);
                    break;
                case 2:
                    ptr->GetAIInterface()->setAiState(AI_STATE_CASTING);
                    break;
                case 3:
                    ptr->GetAIInterface()->setAiState(AI_STATE_FLEEING);
                    break;
                case 4:
                    ptr->GetAIInterface()->setAiState(AI_STATE_FOLLOWING);
                    break;
                case 5:
                    ptr->GetAIInterface()->setAiState(AI_STATE_EVADE);
                    break;
                case 6:
                    ptr->GetAIInterface()->setAiState(AI_STATE_MOVEWP);
                    break;
                case 7:
                    ptr->GetAIInterface()->setAiState(AI_STATE_FEAR);
                    break;
                case 8:
                    ptr->GetAIInterface()->setAiState(AI_STATE_WANDER);
                    break;
                case 9:
                    ptr->GetAIInterface()->setAiState(AI_STATE_STOPPED);
                    break;
                case 10:
                    ptr->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                    break;
                case 11:
                    ptr->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                    break;
            }
        }
        return 0;
    }

    static int SetStealth(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        uint32 stealthlevel = CHECK_ULONG(L, 1);
        ptr->SetStealth(stealthlevel);
        return 0;
    }

    static int GetStealthLevel(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        lua_pushinteger(L, ptr->GetStealthLevel());
        return 1;
    }

    static int IsStealthed(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        if (ptr->IsStealth())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int RemoveStealth(lua_State* /*L*/, Unit* ptr)
    {
        if (!ptr) return 0;
        ptr->RemoveStealth();
        return 0;
    }

    static int InterruptSpell(lua_State* /*L*/, Unit* ptr)
    {
        if (!ptr) return 0;
        ptr->interruptSpell();
        return 0;
    }

    static int IsPoisoned(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        if (ptr->IsPoisoned())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int RegisterAIUpdateEvent(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        time_t time = static_cast<time_t>(luaL_checkinteger(L, 1));
        sEventMgr.AddEvent(static_cast<Creature*>(ptr), &Creature::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, time, 0, 0);
        return 0;
    }

    static int ModifyAIUpdateEvent(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        time_t newtime = static_cast<time_t>(luaL_checkinteger(L, 1));
        sEventMgr.ModifyEventTimeAndTimeLeft(ptr, EVENT_SCRIPT_UPDATE_EVENT, newtime);
        return 0;
    }

    static int RemoveAIUpdateEvent(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            sEventMgr.RemoveEvents(ptr, EVENT_SCRIPT_UPDATE_EVENT);
        return 0;
    }

    static int deleteWaypoint(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        uint32 wp = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (wp)
            static_cast<Creature*>(ptr)->GetAIInterface()->deleteWayPointById(wp);
        return 0;
    }

    static int DealGoldCost(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        Player* plr = static_cast<Player*>(ptr);
        int32 debt = static_cast<int32>(luaL_checkinteger(L, 1));
        if (debt < 0)
            return 0;
        if (!plr->HasGold(debt))
            lua_pushboolean(L, 0);
        else
        {
            plr->ModGold(-debt);
            lua_pushboolean(L, 1);
        }
        return 1;
    }

    static int DealGoldMerit(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            uint32 profit = CHECK_ULONG(L, 1);
        static_cast<Player*>(ptr)->ModGold(profit);
        return 0;
    }

    static int DeMorph(lua_State* /*L*/, Unit* ptr)
    {
        if (!ptr)
            return 0;
        ptr->DeMorph();
        return 0;
    }

    static int Attack(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Unit* target = CHECK_UNIT(L, 1);
        if (target)
        {
            ptr->GetAIInterface()->WipeTargetList();
            ptr->GetAIInterface()->ClearHateList();
            ptr->GetAIInterface()->WipeCurrentTarget();
            ptr->GetAIInterface()->taunt(target);
            lua_pushboolean(L, 1);
        }
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int CanUseCommand(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            char cmdlevel = (char)luaL_checkstring(L, 1)[0];
        Player* plr = static_cast<Player*>(ptr);
        if (plr->GetSession()->CanUseCommand(cmdlevel))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int GetTarget(lua_State* L, Unit* ptr)
    {
        LogNotice("LuaEngine : GetTarget is outdated. Please use GetPrimaryCombatTarget.");
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        Unit* target = plr->GetMapMgr()->GetUnit(plr->GetTarget());
        if (target != NULL)
            PUSH_UNIT(L, target);
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetSelection(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        Unit* selection = plr->GetMapMgr()->GetUnit(plr->GetSelection());
        if (selection)
            PUSH_UNIT(L, selection);
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetSelectedGO(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        PUSH_GO(L, plr->GetSelectedGo());
        return 1;
    }

    static int SetSelectedGO(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        GameObject* newsel = CHECK_GO(L, 1);
        if (!newsel)
            return 0;
        plr->m_GM_SelectedGO = newsel->GetGUID();
        return 0;
    }


    static int RepairAllPlayerItems(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER_RET()

        Player* plr = static_cast<Player*>(ptr);
        Item* pItem = NULL;
        Container* pContainer = NULL;
        uint16 j;
        uint16 i;

        for (i = 0; i < MAX_INVENTORY_SLOT; i++)
        {
            pItem = plr->GetItemInterface()->GetInventoryItem(i);
            if (pItem != NULL)
            {
                if (pItem->IsContainer())
                {
                    pContainer = static_cast< Container* >(pItem);
                    for (j = 0; j < pContainer->GetItemProperties()->ContainerSlots; ++j)
                    {
                        pItem = pContainer->GetItem(j);
                        if (pItem != NULL)
                        {
                            pItem->SetDurabilityToMax();
                        }
                    }
                }
                else
                {
                    if (pItem->GetItemProperties()->MaxDurability > 0 && i < INVENTORY_SLOT_BAG_END && pItem->GetDurability() <= 0)
                    {
                        pItem->SetDurabilityToMax();
                        plr->ApplyItemMods(pItem, i, true);
                    }
                    else
                    {
                        pItem->SetDurabilityToMax();
                    }
                }
            }
        }
        return 0;
    }

    static int SetKnownTitle(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        int title = static_cast<int>(luaL_checkinteger(L, 1));
        Player* plr = static_cast<Player*>(ptr);
        plr->SetKnownTitle(RankTitles(title), true);
        plr->SaveToDB(false);
        return 0;
    }

    static int UnsetKnownTitle(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        int title = static_cast<int>(luaL_checkinteger(L, 1));
        Player* plr = static_cast<Player*>(ptr);
        plr->SetKnownTitle(RankTitles(title), false);
        plr->SaveToDB(false);
        return 0;
    }

    static int LifeTimeKills(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        int kills = static_cast<int>(luaL_checkinteger(L, 1));
        const char* check = luaL_checklstring(L, 2, NULL);
        Player* plr = static_cast<Player*>(ptr);
        int killscheck = plr->getUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);
        if (check && strncmp(check, "add", 4) == 0 && kills > 0)
        {
            plr->setUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, killscheck + kills);
            plr->SaveToDB(false);
            return 0;
        }
        else if (check && strncmp(check, "del", 4) == 0 && killscheck >= kills)
        {
            plr->setUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, killscheck - kills);
            plr->SaveToDB(false);
            return 0;
        }
        else if (check && strncmp(check, "set", 4) == 0 && kills >= 0)
        {
            plr->setUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, kills);
            plr->SaveToDB(false);
            return 0;
        }
        else if (check == NULL || kills == 0)
        {
            lua_pushinteger(L, killscheck);
            return 1;
        }
        return 0;
    }

    static int HasTitle(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        int title = static_cast<int>(luaL_checkinteger(L, 1));
        Player* plr = static_cast<Player*>(ptr);
        if (plr->HasTitle(RankTitles(title)))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int GetMaxSkill(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 skill = static_cast<uint32>(luaL_checkinteger(L, 1));
        lua_pushinteger(L, static_cast<Player*>(ptr)->_GetSkillLineMax(skill));
        return 1;
    }

    static int GetCurrentSkill(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 skill = static_cast<uint32>(luaL_checkinteger(L, 1));
        lua_pushinteger(L, static_cast<Player*>(ptr)->_GetSkillLineCurrent(skill));
        return 1;
    }

    static int HasSkill(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 skill = static_cast<uint32>(luaL_checkinteger(L, 1));
        lua_pushboolean(L, (static_cast<Player*>(ptr)->_HasSkillLine(skill)) ? 1 : 0);
        return 1;
    }

    static int GetGuildName(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Guild* pGuild = objmgr.GetGuild(static_cast<Player*>(ptr)->GetGuildId());
        if (pGuild != NULL)
            lua_pushstring(L, pGuild->getGuildName());
        else
            lua_pushnil(L);
        return 1;
    }

    static int ClearCooldownForSpell(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        Player* plr = static_cast<Player*>(ptr);
        plr->ClearCooldownForSpell(static_cast<uint32>(luaL_checkinteger(L, 1)));
        return 0;
    }

    static int HasSpell(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            uint32 sp = CHECK_ULONG(L, 1);
        lua_pushboolean(L, (sp && static_cast<Player*>(ptr)->HasSpell(sp)) ? 1 : 0);
        return 1;
    }

    static int ClearAllCooldowns(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            static_cast<Player*>(ptr)->ResetAllCooldowns();
        return 0;
    }

    static int ResetAllTalents(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            static_cast<Player*>(ptr)->Reset_Talents();
        return 0;
    }

    static int GetAccountName(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* aName = static_cast<Player*>(ptr)->GetSession()->GetAccountNameS();
        lua_pushstring(L, aName);
        return 1;
    }

    static int GetGmRank(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* level = static_cast<Player*>(ptr)->GetSession()->GetPermissions();
        if (level != NULL)
            lua_pushstring(L, level);
        else
            lua_pushnil(L);
        return 1;
    }

    static int IsGm(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            if (static_cast<Player*>(ptr)->GetSession()->HasGMPermissions())
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);
        return 1;
    }

    static int SavePlayer(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            static_cast<Player*>(ptr)->SaveToDB(false);
        return 0;
    }

    static int HasQuest(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            uint32 quest_id = CHECK_ULONG(L, 1);
        if (quest_id && static_cast<Player*>(ptr)->HasQuest(quest_id))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int CreatureHasQuest(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Creature* ctr = static_cast<Creature*>(ptr);
        uint32 questid = CHECK_ULONG(L, 1);
        QuestProperties const* qst = sMySQLStore.getQuestProperties(questid);
        if (ctr->HasQuest(qst->id, qst->type))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int RemovePvPFlag(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        if (plr != NULL && plr->IsPvPFlagged())
            plr->RemovePvPFlag();
        return 0;
    }

    static int RemoveNegativeAuras(lua_State* /*L*/, Unit* ptr)
    {
        if (!ptr)
            return 0;
        ptr->RemoveNegativeAuras();
        return 0;
    }

    static int GossipMiscAction(lua_State* L, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;

        Player* plr = static_cast<Player*>(ptr);
        Creature* crc = static_cast<Creature*>(CHECK_UNIT(L, 2));
        uint32 miscint = static_cast<uint32>(luaL_checkinteger(L, 3));
        uint32 actionid = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (!crc && actionid < 9)
            return 0;
        if (actionid == 1) plr->GetSession()->SendInventoryList(crc);
        else if (actionid == 2) plr->GetSession()->SendTrainerList(crc);
        else if (actionid == 3) plr->GetSession()->SendInnkeeperBind(crc);
        else if (actionid == 4) plr->GetSession()->SendBankerList(crc);
        else if (actionid == 5) plr->GetSession()->SendBattlegroundList(crc, miscint);
        else if (actionid == 6) plr->GetSession()->SendAuctionList(crc);
        else if (actionid == 7) plr->GetSession()->SendTabardHelp(crc);
        else if (actionid == 8) plr->GetSession()->SendSpiritHealerRequest(crc);
        else if (actionid == 9) plr->SendTalentResetConfirm();
        else if (actionid == 10) plr->SendPetUntrainConfirm();
        return 0;
    }

    static int SendVendorWindow(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        Creature* object = static_cast<Creature*>(CHECK_UNIT(L, 1));  //NOT entry. The unit pointer.
        if (plr != NULL && object != NULL)
            plr->GetSession()->SendInventoryList(object);
        return 0;
    }

    static int SendTrainerWindow(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        Creature* crc = static_cast<Creature*>(CHECK_UNIT(L, 1));  //NOT entry. The unit pointer.
        if (crc != NULL)
            plr->GetSession()->SendTrainerList(crc);
        return 0;
    }

    static int SendInnkeeperWindow(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        Creature* crc = static_cast<Creature*>(CHECK_UNIT(L, 1));  //NOT entry. The unit pointer.
        if (crc != NULL)
            plr->GetSession()->SendInnkeeperBind(crc);
        return 0;
    }

    static int SendBankWindow(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        Creature* crc = static_cast<Creature*>(CHECK_UNIT(L, 1));  //NOT entry. The unit pointer.
        if (crc != NULL)
            plr->GetSession()->SendBankerList(crc);
        return 0;
    }

    static int SendAuctionWindow(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        Creature* crc = static_cast<Creature*>(CHECK_UNIT(L, 1));  //NOT entry. The unit pointer.
        if (crc != NULL)
            plr->GetSession()->SendAuctionList(crc);
        return 0;
    }

    static int SendBattlegroundWindow(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        Player* plr = static_cast<Player*>(ptr);
        Creature* crc = static_cast<Creature*>(CHECK_UNIT(L, 1));
        uint32 bgid = static_cast<uint32>(luaL_checkinteger(L, 2));
        if (bgid && crc != NULL)
            plr->GetSession()->SendBattlegroundList(crc, bgid); //player filler ftw
        return 0;
    }

    static int SendLootWindow(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            uint64 guid = CHECK_GUID(L, 1);
        uint8 loot_type = (uint8)luaL_checkinteger(L, 2);
        uint8 loot_type2 = 1;
        Player* plr = static_cast<Player*>(ptr);
        plr->SetLootGUID(guid);
        uint32 guidtype = GET_TYPE_FROM_GUID(guid);
        if (guidtype == HIGHGUID_TYPE_UNIT)
        {
            Unit* pUnit = plr->GetMapMgr()->GetUnit(guid);
            CreatureProperties const* creature_properties = static_cast<Creature*>(pUnit)->GetCreatureProperties();
            switch (loot_type)
            {
                default:
                    lootmgr.FillCreatureLoot(&pUnit->loot, pUnit->GetEntry(), pUnit->GetMapMgr() ? (pUnit->GetMapMgr()->iInstanceMode ? true : false) : false);
                    pUnit->loot.gold = creature_properties ? creature_properties->money : 0;
                    loot_type2 = 1;
                    break;
                case 2:
                    lootmgr.FillSkinningLoot(&pUnit->loot, pUnit->GetEntry());
                    loot_type2 = 2;
                    break;
                case 3:
                    lootmgr.FillPickpocketingLoot(&pUnit->loot, pUnit->GetEntry());
                    loot_type2 = 2;
                    break;
            }
        }
        else if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
        {
            GameObject* pGO = plr->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
            if (pGO != NULL && pGO->IsLootable())
            {
                GameObject_Lootable* lt = static_cast<GameObject_Lootable*>(pGO);
                switch (loot_type)
                {
                    default:
                        lootmgr.FillGOLoot(&lt->loot, pGO->GetEntry(), pGO->GetMapMgr() ? (pGO->GetMapMgr()->iInstanceMode ? true : false) : false);
                        loot_type2 = 1;
                        break;
                    case 5:
                        lootmgr.FillSkinningLoot(&lt->loot, pGO->GetEntry());
                        loot_type2 = 2;
                        break;
                }
            }
        }
        else if (guidtype == HIGHGUID_TYPE_ITEM)
        {
            Item* pItem = plr->GetItemInterface()->GetItemByGUID(guid);
            switch (loot_type)
            {
                case 6:
                    lootmgr.FillItemLoot(pItem->loot, pItem->GetEntry());
                    loot_type2 = 1;
                    break;
                default:
                    break;
            }
        }
        plr->SendLoot(guid, 2, plr->GetMapId());
        return 0;
    }

    static int AddLoot(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            if ((lua_gettop(L) != 3) || (lua_gettop(L) != 5))
                return 0;

        uint32 itemid = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 mincount = static_cast<uint32>(luaL_checkinteger(L, 2));
        uint32 maxcount = static_cast<uint32>(luaL_checkinteger(L, 3));
        bool perm = ((luaL_optinteger(L, 4, 0) == 1) ? true : false);
        if (perm)
        {
            float chance = CHECK_FLOAT(L, 5);
            QueryResult* result = WorldDatabase.Query("SELECT * FROM loot_creatures WHERE entryid = %u, itemid = %u", ptr->GetEntry(), itemid);
            if (!result)
                WorldDatabase.Execute("REPLACE INTO loot_creatures VALUES (%u, %u, %f, 0, 0, 0, %u, %u )", ptr->GetEntry(), itemid, chance, mincount, maxcount);
            delete result;
        }
        lootmgr.AddLoot(&ptr->loot, itemid, mincount, maxcount);
        return 0;
    }

    static int VendorAddItem(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        Creature* ctr = static_cast<Creature*>(ptr);
        uint32 itemid = (uint32)luaL_checknumber(L, 1);
        uint32 amount = (uint32)luaL_checknumber(L, 2);
        uint32 costid = (uint32)luaL_checknumber(L, 3);
#if VERSION_STRING != Cata
        auto item_extended_cost = (costid > 0) ? sItemExtendedCostStore.LookupEntry(costid) : NULL;
        if (itemid && amount)
            ctr->AddVendorItem(itemid, amount, item_extended_cost);
#endif
        return 0;
    }

    static int VendorRemoveItem(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Creature* ctr = static_cast<Creature*>(ptr);
        uint32 itemid = (uint32)luaL_checknumber(L, 1);
        int slot = ctr->GetSlotByItemId(itemid);
        if (itemid && slot > 0)
            ctr->RemoveVendorItem(itemid);
        return 0;
    }

    static int VendorRemoveAllItems(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNIT()
            Creature* ctr = static_cast<Creature*>(ptr);
        uint32 i = 0;
        if (ctr->HasItems())
        {
            uint32 creatureitemids[200];
            size_t count = ctr->GetSellItemCount();
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

    static int EquipWeapons(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        uint32 equip1 = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 equip2 = static_cast<uint32>(luaL_checkinteger(L, 2));
        uint32 equip3 = static_cast<uint32>(luaL_checkinteger(L, 3));
        ptr->SetEquippedItem(MELEE, equip1);
        ptr->SetEquippedItem(OFFHAND, equip2);
        ptr->SetEquippedItem(RANGED, equip3);
        return 0;
    }

    static int Dismount(lua_State* /*L*/, Unit* ptr)
    {
        if (!ptr)
            return 0;
        if (ptr->IsPlayer())
        {
            Player* plr = static_cast<Player*>(ptr);
            plr->RemoveAura(plr->m_MountSpellId);
            plr->setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
        }
        else
            ptr->setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
        return 0;
    }

    static int GiveXp(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        Player* pl = static_cast<Player*>(ptr);
        uint32 exp = static_cast<uint32>(luaL_checkinteger(L, 1));
        pl->GiveXP(exp, pl->GetGUID(), true);
        return 0;
    }

    static int AdvanceAllSkills(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        uint32 skillvalue = static_cast<uint32>(luaL_checkinteger(L, 1));
        plr->_AdvanceAllSkills(skillvalue);
        return 0;
    }

    static int GetTeam(lua_State* L, Unit* ptr)   //returns 0 for alliance, 1 for horde.
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        lua_pushinteger(L, plr->GetTeam());
        return 1;
    }

    static int StartTaxi(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        Player* plr = static_cast<Player*>(ptr);
        TaxiPath* tp = CHECK_TAXIPATH(L, 1);
        uint32 mount_id = static_cast<uint32>(luaL_checkinteger(L, 2));
        plr->TaxiStart(tp, mount_id, 0);
        return 0;
    }

    static int IsOnTaxi(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            lua_pushboolean(L, static_cast<Player*>(ptr)->GetTaxiState() ? 1 : 0);
        return 1;
    }

    static int GetTaxi(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            PUSH_TAXIPATH(L, static_cast<Player*>(ptr)->GetTaxiPath());
        return 1;
    }

    static int SetPlayerLock(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            bool lock = CHECK_BOOL(L, 1);
        if (lock)
        {
            ptr->m_pacified = 1;
            ptr->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
            WorldPacket data1(9);
            data1.Initialize(SMSG_CLIENT_CONTROL_UPDATE);
            data1 << ptr->GetNewGUID() << uint8(0x00);
            static_cast<Player*>(ptr)->GetSession()->SendPacket(&data1);
        }
        else
        {
            ptr->m_pacified = 0;
            ptr->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
            WorldPacket data1(9);
            data1.Initialize(SMSG_CLIENT_CONTROL_UPDATE);
            data1 << ptr->GetNewGUID() << uint8(0x01);
            static_cast<Player*>(ptr)->GetSession()->SendPacket(&data1);
        }
        return 0;
    }

    static int MovePlayerTo(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float z = CHECK_FLOAT(L, 3);
        float o = CHECK_FLOAT(L, 4);
        uint32 mov_flag = CHECK_ULONG(L, 5); //0 - walk, 256 - teleport, 4096 - run, 12288 - fly
        float moveSpeed = (float)luaL_optnumber(L, 6, 1.0f);
        if (moveSpeed == 1.0f)
        {
            if (mov_flag == 0)
                moveSpeed = 2.5f * 0.001f;
            else
                moveSpeed = 7.0f * 0.001f;
        }
        ptr->SetFacing(o);
        ptr->SetOrientation(o);
        float distance = ptr->CalcDistance(ptr->GetPositionX(), ptr->GetPositionY(), ptr->GetPositionZ(), x, y, z);
        uint32 moveTime = uint32(distance / moveSpeed);
        WorldPacket data(SMSG_MONSTER_MOVE, 50);
        data << ptr->GetNewGUID();
        data << uint8(0);
        data << ptr->GetPositionX();
        data << ptr->GetPositionY();
        data << ptr->GetPositionZ();
        data << Util::getMSTime();
        data << uint8(0x00);
        data << uint32(mov_flag);
        data << moveTime;
        data << uint32(1);
        data << x << y << z;

        ptr->SendMessageToSet(&data, true);
        ptr->SetPosition(x, y, z, o);
        return 0;
    }

    static int ChannelSpell(lua_State* L, Unit* ptr)
    {
        uint32 Csp = static_cast<uint32>(luaL_checkinteger(L, 1));
        Object* target = CHECK_OBJECT(L, 2);
        if (Csp && target != nullptr)
        {
            ptr->CastSpell(target->GetGUID(), sSpellCustomizations.GetSpellInfo(Csp), false);
            ptr->SetChannelSpellTargetGUID(target->GetGUID());
            ptr->SetChannelSpellId(Csp);
        }
        return 0;
    }

    static int StopChannel(lua_State* /*L*/, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;
        ptr->SetChannelSpellTargetGUID(0);
        ptr->SetChannelSpellId(0);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    // WORLDSTATES/WORLD PVP NOT SUPPORTED
    //////////////////////////////////////////////////////////////////////////
    /*
    static int SetWorldState(lua_State * L, Unit * ptr)
    {
    int zone = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);

    if(!zone || !index || !value)
    lua_pushnil(L);

    ptr->GetMapMgr()->SetWorldState(zone, index, value);
    lua_pushboolean(L, 1);
    return 1;
    }
    */

    static int EnableFlight(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        bool enable_fly = CHECK_BOOL(L, 1);
        ptr->setMoveCanFly(enable_fly);
        return 0;
    }

    static int GetCoinage(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        lua_pushinteger(L, plr->GetGold());
        return 1;
    }

    static int FlagPvP(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        plr->SetPvPFlag();
        return 0;
    }

    static int IsMounted(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        if (ptr->IsPlayer())
        {
            Player* plr = static_cast<Player*>(ptr);
            if (plr != NULL && plr->IsMounted())
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);
        }
        else
            lua_pushboolean(L, (ptr->GetMount() > 0) ? 1 : 0);
        return 1;
    }

    //credits to alvanaar for the following 9 functions:
    static int IsGroupedWith(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* target = CHECK_PLAYER(L, 1);
        if (ptr->GetGroup()->HasMember(target))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int GetGroupType(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        Group* group = plr->GetGroup();
        if (group != NULL)
            lua_pushinteger(L, group->GetGroupType());
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetTotalHonor(lua_State* L, Unit* ptr)   // I loathe typing "honour" like "honor".
    {
        TEST_PLAYER()
            lua_pushinteger(L, static_cast<Player*>(ptr)->m_honorPoints);
        return 1;
    }

    static int GetHonorToday(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            lua_pushinteger(L, static_cast<Player*>(ptr)->m_honorToday);
        return 1;
    }

    static int GetHonorYesterday(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            lua_pushinteger(L, static_cast<Player*>(ptr)->m_honorYesterday);
        return 1;
    }

    static int GetArenaPoints(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            lua_pushinteger(L, static_cast<Player*>(ptr)->m_arenaPoints);
        return 1;
    }

    static int AddArenaPoints(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 pnts = static_cast<uint32>(luaL_checkinteger(L, 1));
        Player* plr = static_cast<Player*>(ptr);
        if (pnts > 0)
        {
            plr->AddArenaPoints(pnts, true);
        }
        return 0;
    }

    static int RemoveArenaPoints(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 pnts = static_cast<uint32>(luaL_checkinteger(L, 1));
        Player* plr = static_cast<Player*>(ptr);
        int32 npts = plr->m_arenaPoints - pnts;
        if (npts >= 0)
        {
            plr->m_arenaPoints = npts;
        }
        else
        {
            plr->m_arenaPoints = 0;
        }
        plr->UpdateArenaPoints();
        return 0;
    }

    static int AddLifetimeKills(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 pnts = static_cast<uint32>(luaL_checkinteger(L, 1));
        Player* plr = static_cast<Player*>(ptr);
        plr->m_killsLifetime += pnts;
        return 0;
    }

    static int GetGender(lua_State* L, Unit* ptr)
    {
        if (!ptr) return 0;
        lua_pushinteger(L, ptr->getGender());
        return 1;
    }

    static int SetGender(lua_State* L, Unit* ptr)
    {
        if (!ptr)
            return 0;

        uint8 gender = static_cast<uint8>(luaL_checkinteger(L, 1));
        ptr->setGender(gender);
        return 0;
    }
    //next 5 credits: alvanaar
    static int SendPacketToGuild(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        WorldPacket* data = CHECK_PACKET(L, 1);
        Player* plr = static_cast<Player*>(ptr);
        Guild* guild = nullptr;
        if (luaL_optinteger(L, 2, -1) > 0)
        {
            guild = objmgr.GetGuild(static_cast<uint32>(luaL_optinteger(L, 2, 0)));
        }
        else
        {
            guild = plr->GetGuild();
        }
        if (data != NULL && guild != NULL)
            guild->sendPacket(data);
        return 0;
    }

    static int GetGuildId(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        if (plr->GetGuild() != NULL)
            lua_pushinteger(L, plr->GetGuildId());
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetGuildRank(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        if (plr->GetGuild() != NULL)
            lua_pushinteger(L, plr->GetGuildRank());
        else
            lua_pushnil(L);
        return 1;
    }

    static int SetGuildRank(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        Player* plr = static_cast<Player*>(ptr);
        uint32 rank = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (plr->IsInGuild())
            plr->SetGuildRank(rank);
        return 0;
    }

    static int IsInGuild(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        if (plr->IsInGuild())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int SendGuildInvite(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        Player* sender = static_cast<Player*>(ptr);
        Player* plyr = CHECK_PLAYER(L, 1);
        std::string inviteeName = plyr->GetName();

        Guild* pGuild = sender->GetGuild();
        if (!plyr)
        {
            Guild::sendCommandResult(sender->GetSession(), GC_TYPE_INVITE, GC_ERROR_PLAYER_NOT_FOUND_S, inviteeName.c_str());
        }
        else if (!pGuild)
        {
            Guild::sendCommandResult(sender->GetSession(), GC_TYPE_INVITE, GC_ERROR_PLAYER_NOT_IN_GUILD, "");
        }
        else if (plyr->GetGuildId())
        {
            Guild::sendCommandResult(sender->GetSession(), GC_TYPE_INVITE, GC_ERROR_ALREADY_IN_GUILD, plyr->GetName());
        }
#if VERSION_STRING != Cata
        else if (plyr->GetGuildInvitersGuid())
#else
        else if (plyr->GetGuildIdInvited())
#endif
        {
            Guild::sendCommandResult(sender->GetSession(), GC_TYPE_INVITE, GC_ERROR_ALREADY_INVITED_TO_GUILD, plyr->GetName());
        }
        else if (plyr->GetTeam() != sender->GetTeam() && sender->GetSession()->GetPermissionCount() == 0 && !worldConfig.player.isInterfactionGuildEnabled)
        {
            Guild::sendCommandResult(sender->GetSession(), GC_TYPE_INVITE, GC_ERROR_NOT_ALLIED, "");
        }
        else
        {
            Guild::sendCommandResult(sender->GetSession(), GC_TYPE_INVITE, GC_ERROR_SUCCESS, inviteeName.c_str());
            WorldPacket data(SMSG_GUILD_INVITE, 100);
            data << sender->GetName();
            data << pGuild->getGuildName();
            plyr->GetSession()->SendPacket(&data);
#if VERSION_STRING != Cata
            plyr->SetGuildInvitersGuid(sender->GetLowGUID());
#endif
        }

        return 0;
    }

    static int DemoteGuildMember(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Player* plr = static_cast<Player*>(ptr);
        Player* target = CHECK_PLAYER(L, 1);
        if (target && plr->GetGuild())
            plr->GetGuild()->DemoteGuildMember(target->getPlayerInfo(), plr->GetSession());
#endif
        return 0;
    }

    static int PromoteGuildMember(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Player* plr = static_cast<Player*>(ptr);
        Player* target = CHECK_PLAYER(L, 1);
        if (target && plr->GetGuild())
            plr->GetGuild()->PromoteGuildMember(target->getPlayerInfo(), plr->GetSession());
#endif
        return 0;
    }

    static int SetGuildMotd(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Player* plr = static_cast<Player*>(ptr);
        const char* szNewMotd = luaL_checkstring(L, 1);
        if (plr->GetGuild() && szNewMotd != NULL)
            plr->GetGuild()->SetMOTD(szNewMotd, plr->GetSession());
#endif
        return 0;
    }

    static int GetGuildMotd(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
        Player* plr = static_cast<Player*>(ptr);
        Guild* guild = nullptr;
        if (luaL_optinteger(L, 1, -1) >= 0)
            guild = objmgr.GetGuild(static_cast<uint32>(luaL_optinteger(L, 1, -1)));
        else
            guild = plr->GetGuild();
        if (guild != nullptr)
            lua_pushstring(L, guild->getMOTD());
        else
            lua_pushnil(L);
#endif
        return 1;
    }

    static int SetGuildInformation(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Player* plr = static_cast<Player*>(ptr);
        const char* gi = luaL_checkstring(L, 1);
        if (gi && plr->GetGuild())
            plr->GetGuild()->SetGuildInformation(gi, plr->GetSession());
#endif
        return 0;
    }

    static int AddGuildMember(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
        Player* plr = static_cast<Player*>(ptr);
        uint32 g_id = CHECK_ULONG(L, 1);
        int32 rank = static_cast<int32>(luaL_optinteger(L, 2, -1));
        Guild* target = objmgr.GetGuild(g_id);
        if (target)
            target->AddGuildMember(plr->getPlayerInfo(), NULL, rank);
#endif
        return 0;
    }

    static int RemoveGuildMember(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Player* plr = static_cast<Player*>(ptr);
        Player* target = CHECK_PLAYER(L, 1);
        if (target && plr->GetGuild())
            plr->GetGuild()->RemoveGuildMember(target->getPlayerInfo(), plr->GetSession());
#endif
        return 0;
    }

    static int SetPublicNote(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Player* plr = static_cast<Player*>(ptr);
        Player* target = CHECK_PLAYER(L, 1);
        const char* note = luaL_checkstring(L, 2);
        if (target && note && plr->GetGuild())
            plr->GetGuild()->SetPublicNote(target->getPlayerInfo(), note, plr->GetSession());
#endif
        return 0;
    }

    static int SetOfficerNote(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Player* plr = static_cast<Player*>(ptr);
        Player* target = CHECK_PLAYER(L, 1);
        const char* note = luaL_checkstring(L, 2);
        if (target && note && plr->GetGuild())
            plr->GetGuild()->SetOfficerNote(target->getPlayerInfo(), note, plr->GetSession());
#endif
        return 0;
    }

    static int DisbandGuild(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
        Player* plr = static_cast<Player*>(ptr);
        Guild* guild = nullptr;
        if (luaL_optinteger(L, 1, -1) >= 0)
            guild = objmgr.GetGuild(static_cast<uint32>(luaL_optinteger(L, 1, -1)));
        else
            guild = plr->GetGuild();
        if (guild != nullptr)
            guild->disband();
#endif
        return 0;
    }

    static int ChangeGuildMaster(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Player* plr = static_cast<Player*>(ptr);
        Player* target = CHECK_PLAYER(L, 1);
        if (target)
            plr->GetGuild()->ChangeGuildMaster(target->getPlayerInfo(), plr->GetSession());
#endif
        return 0;
    }

    static int SendGuildChatMessage(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Player* plr = static_cast<Player*>(ptr);
        const char* message = luaL_checkstring(L, 1);
        bool officer = CHECK_BOOL(L, 2);
        if (plr->GetGuild() != NULL && message != NULL)
            (officer) ? plr->GetGuild()->OfficerChat(message, plr->GetSession(), 0) : plr->GetGuild()->GuildChat(message, plr->GetSession(), 0);
#endif
        return 0;
    }

    static int SendGuildLog(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Player* plr = static_cast<Player*>(ptr);
        if (plr->GetGuild() != nullptr)
            plr->GetGuild()->SendGuildLog(plr->GetSession());
#endif
        return 0;
    }

    static int GuildBankDepositMoney(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
        Player* plr = static_cast<Player*>(ptr);
        uint32 amount = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (plr->GetGuild() != NULL)
            plr->GetGuild()->DepositMoney(plr->GetSession(), amount);
#endif
        return 0;
    }

    static int GuildBankWithdrawMoney(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
        Player* plr = static_cast<Player*>(ptr);
        uint32 amount = static_cast<uint32>(luaL_checkinteger(L, 1));
        if (plr->GetGuild() != NULL)
            plr->GetGuild()->WithdrawMoney(plr->GetSession(), amount);
#endif
        return 0;
    }

    static int SetByteValue(lua_State* L, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint16 index = static_cast<uint16>(luaL_checkinteger(L, 1));
        uint8 index1 = static_cast<uint8>(luaL_checkinteger(L, 2));
        uint8 value = static_cast<uint8>(luaL_checkinteger(L, 3));
        ptr->setByteValue(index, index1, value);
        return 0;
    }

    static int GetByteValue(lua_State* L, Unit* ptr)
    {
        if (ptr == nullptr)
            return 0;

        uint16 index = static_cast<uint16>(luaL_checkinteger(L, 1));
        uint8 index1 = static_cast<uint8>(luaL_checkinteger(L, 2));
        lua_pushinteger(L, ptr->getByteValue(index, index1));
        return 1;
    }

    static int IsPvPFlagged(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            lua_pushboolean(L, static_cast<Player*>(ptr)->IsPvPFlagged() ? 1 : 0);
        return 1;
    }

    static int IsFFAPvPFlagged(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            lua_pushboolean(L, static_cast<Player*>(ptr)->IsFFAPvPFlagged() ? 1 : 0);
        return 1;
    }

    static int GetGuildLeader(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Guild* pGuild = static_cast<Player*>(ptr)->GetGuild();
        if (pGuild != NULL)
        {
            Player* plr = objmgr.GetPlayer(pGuild->GetGuildLeader());
            if (plr != NULL)
                lua_pushstring(L, plr->GetName());
            else
                lua_pushnil(L);
        }
        else
            lua_pushnil(L);
#endif
        return 1;
    }

    static int GetGuildMemberCount(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Guild* pGuild = static_cast<Player*>(ptr)->GetGuild();
        (pGuild != NULL) ? lua_pushinteger(L, pGuild->GetNumMembers()) : lua_pushnil(L);
#endif
        return 1;
    }

    static int IsFriendly(lua_State* L, Unit* ptr)
    {
        Unit* obj = CHECK_UNIT(L, 1);
        if (!obj || !ptr) return 0;
        if (isFriendly(ptr, obj))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int IsInChannel(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* channel_name = luaL_checkstring(L, 1);
        if (!ptr || !channel_name)
            return 0;

        Channel* pChannel = channelmgr.GetChannel(channel_name, static_cast<Player*>(ptr));
        if (pChannel->HasMember(static_cast<Player*>(ptr))) // Channels: "General", "Trade", "LocalDefense", "GuildRecruitment", "LookingForGroup", (or any custom channel)
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }

    static int JoinChannel(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* channel_name = luaL_checkstring(L, 1);
        Channel* pChannel = channelmgr.GetChannel(channel_name, static_cast<Player*>(ptr));
        const char* pw = luaL_optstring(L, 2, pChannel->m_password.c_str());

        if (!ptr || !channel_name || pChannel->HasMember(static_cast<Player*>(ptr)) || !pChannel)
            return 0;
        else
            pChannel->AttemptJoin(static_cast<Player*>(ptr), pw);
        return 1;
    }

    static int LeaveChannel(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* channel_name = luaL_checkstring(L, 1);
        Channel* pChannel = channelmgr.GetChannel(channel_name, static_cast<Player*>(ptr));
        if (!ptr || !channel_name || !pChannel || !pChannel->HasMember(static_cast<Player*>(ptr)))
            return 0;
        else
            pChannel->Part(static_cast<Player*>(ptr), true);
        return 1;
    }

    static int SetChannelName(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* current_name = luaL_checkstring(L, 1);
        const char* new_name = luaL_checkstring(L, 2);
        Channel* pChannel = channelmgr.GetChannel(current_name, static_cast<Player*>(ptr));
        if (!current_name || !new_name || !ptr || !pChannel || pChannel->m_name == new_name)
            return 0;
        pChannel->m_name = new_name;
        return 1;
    }

    static int SetChannelPassword(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* channel_name = luaL_checkstring(L, 1);
        const char* pass = luaL_checkstring(L, 2);
        Channel* pChannel = channelmgr.GetChannel(channel_name, static_cast<Player*>(ptr));
        if (!pass || !ptr || pChannel->m_password == pass)
            return 0;
        pChannel->Password(static_cast<Player*>(ptr), pass);
        return 1;
    }

    static int GetChannelPassword(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* channel_name = luaL_checkstring(L, 1);
        Channel* pChannel = channelmgr.GetChannel(channel_name, static_cast<Player*>(ptr));
        if (!ptr)
            return 0;
        lua_pushstring(L, pChannel->m_password.c_str());
        return 1;
    }

    static int KickFromChannel(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* channel_name = luaL_checkstring(L, 1);
        Player* plr = static_cast<Player*>(ptr);
        Channel* pChannel = channelmgr.GetChannel(channel_name, plr);
        if (!plr || !pChannel)
            return 0;
        pChannel->Kick(plr, plr, false);
        return 1;
    }

    static int BanFromChannel(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* channel_name = luaL_checkstring(L, 1);
        Player* plr = static_cast<Player*>(ptr);
        Channel* pChannel = channelmgr.GetChannel(channel_name, plr);
        if (!plr || !pChannel)
            return 0;
        pChannel->Kick(plr, plr, true);
        return 1;
    }

    static int UnbanFromChannel(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* channel_name = luaL_checkstring(L, 1);
        Player* plr = static_cast<Player*>(ptr);
        Channel* pChannel = channelmgr.GetChannel(channel_name, plr);
        if (!plr || !pChannel)
            return 0;
        pChannel->Unban(plr, plr->getPlayerInfo());
        return 1;
    }

    static int GetChannelMemberCount(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            const char* channel_name = luaL_checkstring(L, 1);
        if (!ptr || !channel_name)
            return 0;
        lua_pushnumber(L, static_cast<lua_Number>(channelmgr.GetChannel(channel_name, static_cast<Player*>(ptr))->GetNumMembers()));
        return 1;
    }

    static int GetPlayerMovementVector(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        MovementInfo* movement_info = plr->GetSession()->GetMovementInfo();
        if (movement_info != NULL)
        {
            lua_newtable(L);
#if VERSION_STRING != Cata
            lua_pushstring(L, "x");
            lua_pushnumber(L, movement_info->position.x);
            lua_rawset(L, -3);
            lua_pushstring(L, "y");
            lua_pushnumber(L, movement_info->position.y);
            lua_rawset(L, -3);
            lua_pushstring(L, "z");
            lua_pushnumber(L, movement_info->position.z);
            lua_rawset(L, -3);
            lua_pushstring(L, "o");
            lua_pushnumber(L, movement_info->position.o);
            lua_rawset(L, -3);
#else
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
#endif
        }
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetPlayerMovementFlags(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
            MovementInfo* move_info = static_cast<Player*>(ptr)->GetSession()->GetMovementInfo();
        if (move_info != NULL)
#if VERSION_STRING != Cata
            lua_pushnumber(L, move_info->flags);
#else
            lua_pushnumber(L, move_info->getMovementFlags());
#endif
        else
            RET_NIL()
            return 1;
    }

    static int Repop(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        if (plr->IsDead())
            plr->RepopRequestedPlayer();
        return 0;
    }

    static int SetMovementFlags(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        int movetype = static_cast<int>(luaL_checkinteger(L, 1)); //0: walk, 1: run, 2: fly.
        if (movetype == 2)
        {
            ptr->GetAIInterface()->setSplineFlying();
        }
        else if (movetype == 1)
        {
            ptr->GetAIInterface()->setSplineRun();
        }
        else
        {
            ptr->GetAIInterface()->setSplineWalk();
        }
        return 0;
    }

    static int GetSpawnId(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            Creature* cre = static_cast<Creature*>(ptr);
        lua_pushnumber(L, cre->GetSQL_id());
        return 1;
    }

    static int ResetTalents(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            Player* plr = static_cast<Player*>(ptr);
        plr->Reset_Talents();
        return 0;
    }

    static int SetTalentPoints(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
        uint32 spec = static_cast<uint32>(luaL_checkinteger(L, 1)); //0 or 1
        uint32 points = static_cast<uint32>(luaL_checkinteger(L, 2));
        static_cast<Player*>(ptr)->m_specs[spec].SetTP(points);

        if (spec == static_cast<Player*>(ptr)->m_talentActiveSpec)
            static_cast<Player*>(ptr)->setUInt32Value(PLAYER_CHARACTER_POINTS1, points);

        static_cast<Player*>(ptr)->smsg_TalentsInfo(false);
#endif
        return 0;
    }

    static int GetTalentPoints(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        uint32 spec = static_cast<uint32>(luaL_checkinteger(L, 1)); //0 or 1
        PlayerSpec plrSpec = static_cast<Player*>(ptr)->m_specs[spec];
        //uint32 Lvl = static_cast<Player*>(ptr)->getLevel();
        uint32 FreePoints = plrSpec.GetTP();

        lua_pushnumber(L, FreePoints);
        return 1;
    }

    static int EventChat(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
        uint8 typ = static_cast<uint8>(luaL_checkinteger(L, 1));
        uint32 lang = static_cast<uint32>(luaL_checkinteger(L, 2));
        const char* message = luaL_checkstring(L, 3);
        uint32 delay = static_cast<uint32>(luaL_checkinteger(L, 4));
        if (message != NULL && delay)
            ptr->SendChatMessage(typ, lang, message, delay);
        return 0;
    }

    static int GetEquippedItemBySlot(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
        int16 slot = static_cast<int16>(luaL_checkinteger(L, 1));
        Player* plr = static_cast<Player*>(ptr);
        Item* pItem = plr->GetItemInterface()->GetInventoryItem(slot);
        if (pItem)
            PUSH_ITEM(L, pItem);
        else
            lua_pushnil(L);
        return 1;
    }

    static int GetGuildMembers(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING != Cata
            Player* plr = static_cast<Player*>(ptr);
        Guild* pGuild = plr->GetGuild();
        uint32 count = 0;
        lua_newtable(L);
        if (pGuild != NULL)
        {
            GuildMemberMap::iterator itr;
            pGuild->getLock().Acquire();
            for (itr = pGuild->GetGuildMembersBegin(); itr != pGuild->GetGuildMembersEnd(); ++itr)
            {
                count++;
                lua_pushinteger(L, count);
                //Paroxysm : Why do we push player names are opposed to objects?
                //hyper: because guild members might not be logged in
                //ret = (*itr).first->m_loggedInPlayer;
                //PUSH_UNIT(L, ((Unit*)ret), false);
                lua_pushstring(L, (*itr).first->name);
                lua_rawset(L, -3);
            }
            pGuild->getLock().Release();
        }
        else
            lua_pushnil(L);
#endif
        return 1;
    }

    static int AddAchievement(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING > TBC
        int32 achievementID = static_cast<int32>(luaL_checkinteger(L, 1));
        Player* plr = static_cast<Player*>(ptr);
        if(plr->GetAchievementMgr().GMCompleteAchievement(NULL, achievementID))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
#endif
        return 1;
    }

    static int RemoveAchievement(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING > TBC
        int32 achievementID = static_cast<int32>(luaL_checkinteger(L, 1));
        static_cast<Player*>(ptr)->GetAchievementMgr().GMResetAchievement(achievementID);
#endif
        return 0;
    }

    static int HasAchievement(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER()
#if VERSION_STRING > TBC
        uint32 achievementID = static_cast<uint32>(luaL_checkinteger(L, 1));
        lua_pushboolean(L, static_cast<Player*>(ptr)->GetAchievementMgr().HasCompleted(achievementID) ? 1 : 0);
#endif
        return 1;
    }

    static int GetAreaId(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        auto area = ptr->GetArea();
        RET_NUMBER(area ? area->id : -1);
    }

    static int ResetPetTalents(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER()
            Pet* pet = static_cast<Player*>(ptr)->GetSummon();
        if (pet != NULL)
        {
            pet->WipeTalents();
            pet->SetTPs(pet->GetTPsForLevel(pet->getLevel()));
            pet->SendTalentsToOwner();
        }
        return 0;
    }

    static int IsDazed(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        lua_pushboolean(L, (ptr->IsDazed()) ? 1 : 0);
        return 1;
    }

    static int GetAura(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        uint32 slot = CHECK_ULONG(L, 1);
        if (slot > MAX_TOTAL_AURAS_START && slot < MAX_TOTAL_AURAS_END)
            RET_NUMBER(ptr->m_auras[slot]->GetSpellId());
        RET_NIL()
    }

    static int GetAuraObject(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        uint32 slot = CHECK_ULONG(L, 1);
        if (slot > MAX_TOTAL_AURAS_START && slot < MAX_TOTAL_AURAS_END)
        {
            PUSH_AURA(L, ptr->m_auras[slot]);
            return 1;
        }
        RET_NIL()
    }

    static int IsRooted(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        if (ptr->isRooted())
            RET_BOOL(true)
            RET_BOOL(false)
    }

    static int HasAuraWithMechanic(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        uint32 mechanic = CHECK_ULONG(L, 1);
        if (mechanic && ptr->HasAuraWithMechanics(mechanic))
            RET_BOOL(true)
            RET_BOOL(false)
    }

    static int HasNegativeAura(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        for (uint32 x = MAX_NEGATIVE_VISUAL_AURAS_START; x < MAX_NEGATIVE_VISUAL_AURAS_END; ++x)
        {
            if (ptr->m_auras[x] && ptr->m_auras[x]->m_spellInfo)
                RET_BOOL(true)
        }
        RET_BOOL(false)
    }

    static int HasPositiveAura(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
            for (uint32 x = MAX_POSITIVE_VISUAL_AURAS_START; x < MAX_POSITIVE_VISUAL_AURAS_END; ++x)
            {
                if (ptr->m_auras[x] && ptr->m_auras[x]->m_spellInfo)
                    RET_BOOL(true)
            }
        RET_BOOL(false)
    }

    static int GetClosestEnemy(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
            float closest_dist = 99999.99f;
        float current_dist = 0;
        Object* closest_unit = NULL;
        Unit* ret = NULL;
        for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
        {
            closest_unit = (*itr);
            if (!closest_unit->IsUnit() || !isHostile(ptr, closest_unit))
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

    static int GetClosestFriend(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
            float closest_dist = 99999.99f;
        float current_dist = 0.0f;
        Object* closest_unit = NULL;
        Unit* ret = NULL;
        for (std::set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
        {
            closest_unit = (*itr);
            if (!closest_unit->IsUnit() || isHostile(closest_unit, ptr))
                continue;
            current_dist = closest_unit->getDistanceSq(ptr);
            if (current_dist < closest_dist)
            {
                closest_dist = current_dist;
                ret = static_cast<Unit*>(closest_unit);
            }
        }
        PUSH_UNIT(L, ret);
        return 1;
    }

    static int GetClosestUnit(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
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

    static int GetObjectType(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
            if (ptr->IsPlayer())
                lua_pushstring(L, "Player");
            else
                lua_pushstring(L, "Unit");
        return 1;
    }
    static int GetCurrentWaypoint(lua_State* L, Unit* ptr)
    {
        TEST_UNIT()
            RET_NUMBER(ptr->GetAIInterface()->getCurrentWayPointId());
    }
    static int DisableMelee(lua_State* L, Unit* ptr)
    {
        TEST_UNIT_RET()
            bool disable = CHECK_BOOL(L, 1);
        static_cast<Creature*>(ptr)->GetAIInterface()->setMeleeDisabled(disable);
        RET_BOOL(true)
    }
    static int DisableSpells(lua_State* L, Unit* ptr)
    {
        TEST_UNIT_RET()
            bool disable = CHECK_BOOL(L, 1);
        static_cast<Creature*>(ptr)->GetAIInterface()->setCastDisabled(disable);
        RET_BOOL(true)
    }
    static int DisableRanged(lua_State* L, Unit* ptr)
    {
        TEST_UNIT_RET()
            bool disable = CHECK_BOOL(L, 1);
        static_cast<Creature*>(ptr)->GetAIInterface()->setRangedDisabled(disable);
        RET_BOOL(true)
    }
    static int DisableCombat(lua_State* L, Unit* ptr)
    {
        TEST_UNIT_RET()
            bool disable = CHECK_BOOL(L, 1);
        static_cast<Creature*>(ptr)->GetAIInterface()->setCombatDisabled(disable);
        RET_BOOL(true)
    }
    static int DisableTargeting(lua_State* L, Unit* ptr)
    {
        TEST_UNIT_RET()
            bool disable = CHECK_BOOL(L, 1);
        static_cast<Creature*>(ptr)->GetAIInterface()->setTargetingDisabled(disable);
        RET_BOOL(true)
    }
    static int IsInGroup(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER_RET()
            if (static_cast<Player*>(ptr)->InGroup())
                RET_BOOL(true)
                RET_BOOL(false)
    }
    static int GetLocation(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
            lua_pushnumber(L, ptr->GetPositionX());
        lua_pushnumber(L, ptr->GetPositionY());
        lua_pushnumber(L, ptr->GetPositionZ());
        lua_pushnumber(L, ptr->GetOrientation());
        return 4;
    }
    static int GetByte(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER()
        uint16 index = static_cast<uint16>(luaL_checkinteger(L, 1));
        uint8 index2 = static_cast<uint8>(luaL_checkinteger(L, 2));
        uint8 value = ptr->getByteValue(index, index2);
        RET_INT(value);
    }
    static int SetByte(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        uint16 index = static_cast<uint16>(luaL_checkinteger(L, 1));
        uint8 index2 = static_cast<uint8>(luaL_checkinteger(L, 2));
        uint8 value = static_cast<uint8>(luaL_checkinteger(L, 3));
        ptr->setByteValue(index, index2, value);
        RET_BOOL(true)
    }

    static int GetSpawnLocation(lua_State* L, Unit* ptr)
    {
        TEST_UNIT();
        lua_pushnumber(L, ptr->GetSpawnX());
        lua_pushnumber(L, ptr->GetSpawnY());
        lua_pushnumber(L, ptr->GetSpawnZ());
        lua_pushnumber(L, ptr->GetSpawnO());
        return 4;
    }
    static int GetObject(lua_State* L, Unit* ptr)
    {
        TEST_UNIT();
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
    static int GetSecondHated(lua_State* L, Unit* ptr)
    {
        TEST_UNIT();
        PUSH_UNIT(L, ptr->GetAIInterface()->GetSecondHated());
        return 1;
    }
    static int SaveToInstance(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER();
        Instance* dungeon = sInstanceMgr.GetInstanceByIds(ptr->GetMapId(), ptr->GetInstanceID());
        dungeon->SaveToDB();
        sInstanceMgr.BuildRaidSavedInstancesForPlayer(static_cast<Player*>(ptr));
        return 0;
    }
    static int UseAI(lua_State* L, Unit* ptr)
    {
        bool check = CHECK_BOOL(L, 1);
        ptr->setAItoUse(check);
        return 0;
    }
    static int FlagFFA(lua_State* L, Unit* ptr)
    {

        TEST_UNITPLAYER();
        bool set = CHECK_BOOL(L, 1);
        if (set)
            ptr->SetFFAPvPFlag();
        else
            ptr->RemoveFFAPvPFlag();
        return 0;
    }
    static int TeleportCreature(lua_State* L, Unit* ptr)
    {
        TEST_UNIT();
        float x = CHECK_FLOAT(L, 1);
        float y = CHECK_FLOAT(L, 2);
        float z = CHECK_FLOAT(L, 3);
        ptr->SetPosition(x, y, z, ptr->GetOrientation());
        WorldPacket data(SMSG_MONSTER_MOVE, 50);
        data << ptr->GetNewGUID();
        data << uint8(0);
        data << ptr->GetPositionX();
        data << ptr->GetPositionY();
        data << ptr->GetPositionZ();
        data << Util::getMSTime();
        data << uint8(0x0);
        data << uint32(0x100);
        data << uint32(1);
        data << uint32(1);
        data << x;
        data << y;
        data << z;
        ptr->SendMessageToSet(&data, false);
        return 0;
    }
    static int IsInDungeon(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        if (ptr->GetMapMgr()->GetMapInfo() && ptr->GetMapMgr()->GetMapInfo()->type == INSTANCE_MULTIMODE)
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }
    static int IsInRaid(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        if (ptr->GetMapMgr()->GetMapInfo() && ptr->GetMapMgr()->GetMapInfo()->type == INSTANCE_RAID)
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
        return 1;
    }
    static int IsHostile(lua_State* L, Unit* ptr)
    {
        Object* B = CHECK_OBJECT(L, 1);
        lua_pushboolean(L, isHostile(ptr, B));
        return 1;
    }
    static int IsAttackable(lua_State* L, Unit* ptr)
    {
        Object* B = CHECK_OBJECT(L, 1);
        lua_pushboolean(L, isAttackable(ptr, B));
        return 1;
    }
    static int GetNumWaypoints(lua_State* L, Unit* ptr)
    {
        TEST_UNIT();
        RET_NUMBER(static_cast<lua_Number>(ptr->GetAIInterface()->getWayPointsCount()));
    }
    static int GetMovementType(lua_State* L, Unit* ptr)
    {
        TEST_UNIT();
        RET_NUMBER((uint32)ptr->GetAIInterface()->getWaypointScriptType());
    }
    static int GetQuestLogSlot(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER();
        uint32 entry = CHECK_ULONG(L, 1);
        QuestLogEntry* qle = static_cast<Player*>(ptr)->GetQuestLogForEntry(entry);
        if (!qle)
            RET_NUMBER(-1);
        lua_pushnumber(L, qle->GetSlot());
        return 1;
    }

    static int GetAuraStackCount(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER_RET();
        uint32 id = CHECK_ULONG(L, 1);
        RET_NUMBER(ptr->GetAuraStackCount(id));
    }

    static int AddAuraObject(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER();
        Aura* aura = CHECK_AURA(L, 1);
        if (!aura) return 0;
        ptr->AddAura(aura);
        return 0;
    }

    static int GetAuraObjectById(lua_State* L, Unit* ptr)
    {
        TEST_UNITPLAYER();
        uint32 id = CHECK_ULONG(L, 1);
        PUSH_AURA(L, ptr->getAuraWithId(id));
        return 1;
    }

    static int StopPlayerAttack(lua_State* /*L*/, Unit* ptr)
    {
        TEST_PLAYER();
        static_cast<Player*>(ptr)->smsg_AttackStop(static_cast<Player*>(ptr)->GetSelection());
        return 0;
    }
    static int GetQuestObjectiveCompletion(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER_RET()
        uint32 questid = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 objective = static_cast<uint32>(luaL_checkinteger(L, 2));
        Player* pl = static_cast<Player*>(ptr);
        QuestLogEntry* qle = pl->GetQuestLogForEntry(questid);
        if (qle != nullptr)
            lua_pushnumber(L, qle->GetMobCount(objective));
        else
            lua_pushnil(L);
        return 1;
    }
    static int IsOnVehicle(lua_State *L, Unit *ptr){
        TEST_UNITPLAYER()

            if ((ptr->GetCurrentVehicle() != NULL) || (ptr->IsPlayer() && ptr->IsVehicle()))
                lua_pushboolean(L, 1);
            else
                lua_pushboolean(L, 0);

        return 1;
    }
    static int SpawnAndEnterVehicle(lua_State *L, Unit *ptr)
    {
        TEST_UNITPLAYER()

        uint32 creature_entry = 0;
        uint32 delay = 0;

        if (lua_gettop(L) != 2)
            return 0;

        creature_entry = static_cast<uint32>(luaL_checkinteger(L, 1));
        delay = static_cast<uint32>(luaL_checkinteger(L, 2));

        if (delay < 1 * 1000)
            delay = 1 * 1000;

        if (creature_entry == 0)
            return 0;

        if ((ptr->GetCurrentVehicle() != NULL) && (!ptr->IsPlayer() || !ptr->IsVehicle()))
            return 0;

        CreatureProperties const* cp = sMySQLStore.getCreatureProperties(creature_entry);
        if (cp == nullptr)
            return 0;

        Player* p = nullptr;
        if (ptr->IsPlayer())
            p = static_cast<Player*>(ptr);

        if ((cp->vehicleid == 0) && (p == NULL) && (!p->GetSession()->HasGMPermissions()))
            return 0;

        LocationVector v(ptr->GetPosition());

        Creature* c = ptr->GetMapMgr()->CreateCreature(cp->Id);
        c->Load(cp, v.x, v.y, v.z, v.o);
        c->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        c->PushToWorld(ptr->GetMapMgr());

        // Need to delay this a bit since first the client needs to see the vehicle
        ptr->EnterVehicle(c->GetGUID(), delay);

        return 0;
    }
    static int DismissVehicle(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNITPLAYER()

        Vehicle* v = nullptr;
        if (ptr->GetCurrentVehicle() != nullptr)
            v = ptr->GetCurrentVehicle();
        else
            if (ptr->IsPlayer() && (ptr->GetVehicleComponent() != nullptr))
                v = ptr->GetVehicleComponent();

        if (v == nullptr)
            return 0;

        v->EjectAllPassengers();

        Unit* o = v->GetOwner();

        if (o->IsPlayer())
            o->RemoveAllAuraType(SPELL_AURA_MOUNTED);
        else
            o->Delete();

        return 0;
    }
    static int AddVehiclePassenger(lua_State *L, Unit *ptr)
    {
        TEST_UNITPLAYER()

            Vehicle *v = NULL;

        if (ptr->GetCurrentVehicle() != NULL)
            v = ptr->GetCurrentVehicle();
        else
            if (ptr->IsPlayer() && (ptr->GetVehicleComponent() != NULL))
                v = ptr->GetVehicleComponent();

        if (v == NULL)
            return 0;

        if (!v->HasEmptySeat())
            return 0;

        if (lua_gettop(L) != 1)
            return 0;

        uint32 creature_entry = static_cast<uint32>(luaL_checkinteger(L, 1));

        CreatureProperties const* cp = sMySQLStore.getCreatureProperties(creature_entry);
        if (cp == nullptr)
            return 0;

        Unit* u = v->GetOwner();

        Creature* c = u->GetMapMgr()->CreateCreature(creature_entry);
        c->Load(cp, u->GetPositionX(), u->GetPositionY(), u->GetPositionZ(), u->GetOrientation());
        c->PushToWorld(u->GetMapMgr());
        c->EnterVehicle(u->GetGUID(), 1);

        return 0;
    }
    static int HasEmptyVehicleSeat(lua_State *L, Unit *ptr)
    {
        TEST_UNITPLAYER();

        Vehicle *v = NULL;

        if (ptr->GetCurrentVehicle() != NULL)
            v = ptr->GetCurrentVehicle();
        else
            if (ptr->IsPlayer() && (ptr->GetVehicleComponent() != NULL))
                v = ptr->GetVehicleComponent();

        if (v == NULL)
            return 0;

        if (v->HasEmptySeat())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);

        return 1;
    }
    static int EnterVehicle(lua_State *L, Unit *ptr)
    {
        TEST_UNITPLAYER()

            if (lua_gettop(L) != 2)
                return 0;

        uint64 guid = CHECK_GUID(L, 1);
        uint32 delay = static_cast<uint32>(luaL_checkinteger(L, 2));

        ptr->EnterVehicle(guid, delay);

        return 0;
    }
    static int ExitVehicle(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNITPLAYER()

            if (ptr->GetCurrentVehicle() != nullptr)
                ptr->GetCurrentVehicle()->EjectPassenger(ptr);
            else
                if (ptr->IsPlayer() && ptr->GetVehicleComponent() != nullptr)
                    ptr->RemoveAllAuraType(SPELL_AURA_MOUNTED);

        return 0;
    }
    static int GetVehicleBase(lua_State *L, Unit *ptr)
    {
        TEST_UNITPLAYER();

        Unit *u = ptr->GetVehicleBase();

        if (u != NULL)
            PUSH_UNIT(L, u);
        else
            lua_pushnil(L);

        return 1;
    }
    static int EjectAllVehiclePassengers(lua_State* /*L*/, Unit* ptr)
    {
        TEST_UNITPLAYER();

        Unit* u = ptr->GetVehicleBase();
        if (u == nullptr)
            return 0;

        u->GetVehicleComponent()->EjectAllPassengers();

        return 0;
    }
    static int EjectVehiclePassengerFromSeat(lua_State *L, Unit *ptr)
    {
        TEST_UNITPLAYER();

        Unit *u = ptr->GetVehicleBase();
        if (u == NULL)
            return 0;

        if (lua_gettop(L) != 1)
            return 0;

        uint32 seat = static_cast<uint32>(luaL_checkinteger(L, 1));

        u->GetVehicleComponent()->EjectPassengerFromSeat(seat);

        return 0;
    }
    static int MoveVehiclePassengerToSeat(lua_State *L, Unit *ptr)
    {
        TEST_UNITPLAYER();

        Unit *u = ptr->GetVehicleBase();
        if (u == NULL)
            return 0;

        if (lua_gettop(L) != 2)
            return 0;

        Unit *passenger = CHECK_UNIT(L, 1);
        uint32 seat = static_cast<uint32>(luaL_checkinteger(L, 2));

        if (passenger == NULL)
            return 0;

        u->GetVehicleComponent()->MovePassengerToSeat(passenger, seat);

        return 0;
    }
    static int SendCinematic(lua_State* L, Unit* ptr)
    {
        TEST_PLAYER();
        uint32 id = static_cast<uint32>(luaL_checkinteger(L, 1));
        static_cast<Player*>(ptr)->SendCinematicCamera(id);
        return 0;
    }
    static int GetWorldStateForZone(lua_State *L, Unit *ptr)
    {
        TEST_UNITPLAYER();

        if (lua_gettop(L) != 1)
            return 0;

        uint32 field = static_cast<uint32>(luaL_checkinteger(L, 1));

        auto a = ptr->GetMapMgr()->GetArea(ptr->GetPositionX(), ptr->GetPositionY(), ptr->GetPositionZ());
        if (a == NULL)
            return 0;

        uint32 zone = a->zone;

        if (zone == 0)
            zone = a->id;

        if (zone == 0)
            return 0;

        uint32 value
            = ptr->GetMapMgr()->GetWorldStatesHandler().GetWorldStateForZone(zone, 0, field);

        lua_pushinteger(L, value);

        return 1;
    }

    static int SetWorldStateForZone(lua_State *L, Unit *ptr)
    {
        TEST_UNITPLAYER();

        if (lua_gettop(L) != 2)
            return 0;

        uint32 field = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 value = static_cast<uint32>(luaL_checkinteger(L, 2));

        auto a = ptr->GetMapMgr()->GetArea(ptr->GetPositionX(), ptr->GetPositionY(), ptr->GetPositionZ());
        if (a == NULL)
            return 0;

        uint32 zone = a->zone;

        if (zone == 0)
            zone = a->id;

        if (zone == 0)
            return 0;

        ptr->GetMapMgr()->GetWorldStatesHandler().SetWorldStateForZone(zone, 0, field, value);

        return 0;
    }

    static int SetWorldStateForPlayer(lua_State *L, Unit *ptr)
    {
        TEST_PLAYER();

        if (lua_gettop(L) != 2)
            return 0;

        uint32 field = static_cast<uint32>(luaL_checkinteger(L, 1));
        uint32 value = static_cast<uint32>(luaL_checkinteger(L, 2));

        static_cast<Player*>(ptr)->SendWorldStateUpdate(field, value);

        return 0;
    }
};
#endif
