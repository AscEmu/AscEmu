/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <Management/Gossip/Gossip.h>
#include <Server/EventMgr.h>
#include <Server/Script/ScriptMgr.h>

#include <set>
#include "LuaMacros.h"
#include "LuaGlobal.h"
#include "LuaHelpers.h"

#ifdef DEBUG
#define LUA_USE_APICHECK
#endif

extern "C"
{
// we're C++, and LUA is C, so the compiler needs to know to use C function names.
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
};

#include <sys/stat.h>
#include <sys/types.h>

class LuaEngine;
class LuaCreature;
class LuaGameObjectScript;
class LuaQuest;
class LuaInstance;
class LuaGossip;
class ArcLuna;
//
//#ifdef WIN32
//#include <Windows.h>
//HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
//#endif

#define RegisterHook(evt, _func) { \
    if(LuaGlobal::instance()->EventAsToFuncName[(evt)].size() > 0 && !(LuaGlobal::instance()->luaEngine()->HookInfo.hooks[(evt)])) { \
        LuaGlobal::instance()->luaEngine()->HookInfo.hooks[(evt)] = true; \
        m_scriptMgr->register_hook( (ServerHookEvents)(evt), (_func) ); } }

enum QuestEvents
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //RegisterQuestEvent
    // Quest callbacks are made by using the function RegisterQuestEvent (QuestId, EventId, function)

    QUEST_EVENT_ON_ACCEPT                       = 1,  // -- (pPlayer, QuestId)
    QUEST_EVENT_ON_COMPLETE                     = 2,  // -- (pPlayer, QuestId)
    QUEST_EVENT_ON_CANCEL                       = 3,  // -- (pPlayer)
    QUEST_EVENT_GAMEOBJECT_ACTIVATE             = 4,  // -- (GameObjectId, pPlayer, QuestId)
    QUEST_EVENT_ON_CREATURE_KILL                = 5,  // -- (CreatureId, pPlayer, QuestId)
    QUEST_EVENT_ON_EXPLORE_AREA                 = 6,  // -- (AreaTriggerId, pPlayer, QuestId)
    QUEST_EVENT_ON_PLAYER_ITEMPICKUP            = 7,  // -- (ItemId, Count, pPlayer, QuestId)
    QUEST_EVENT_COUNT
};

enum CreatureEvents
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //RegisterUnitEvent
    // Unit callbacks are made by using the function RegisterUnitEvent(UnitId, EventId, function)

    CREATURE_EVENT_ON_ENTER_COMBAT              = 1,  // -- (pUnit, event, pAttacker)
    CREATURE_EVENT_ON_LEAVE_COMBAT              = 2,  // -- (pUnit, event, pLastTarget)
    CREATURE_EVENT_ON_TARGET_DIED               = 3,  // -- (pUnit, event, pDied)
    CREATURE_EVENT_ON_DIED                      = 4,  // -- (pUnit, event, pLastTarget)
    CREATURE_EVENT_ON_TARGET_PARRIED            = 5,  // -- (pUnit, event, pTarget)
    CREATURE_EVENT_ON_TARGET_DODGED             = 6,  // -- (pUnit, event, pTarget)
    CREATURE_EVENT_ON_TARGET_BLOCKED            = 7,  // -- (pUnit, event, pTarget, pAmount)
    CREATURE_EVENT_ON_TARGET_CRIT_HIT           = 8,  // -- (pUnit, event, pTarget, pAmount)
    CREATURE_EVENT_ON_PARRY                     = 9,  // -- (pUnit, event, pTarget)
    CREATURE_EVENT_ON_DODGED                    = 10, // -- (pUnit, event, pTarget)
    CREATURE_EVENT_ON_BLOCKED                   = 11, // -- (pUnit, event, pTarget, pAmount)
    CREATURE_EVENT_ON_CRIT_HIT                  = 12, // -- (pUnit, event, pTarget, pAmount)
    CREATURE_EVENT_ON_HIT                       = 13, // -- (pUnit, event, pTarget, pAmount)
    CREATURE_EVENT_ON_ASSIST_TARGET_DIED        = 14, // -- (pUnit, event, pAssistTarget)
    CREATURE_EVENT_ON_FEAR                      = 15, // -- (pUnit, event, pTarget, pSpell)
    CREATURE_EVENT_ON_FLEE                      = 16, // -- (pUnit, event, pTarget)
    CREATURE_EVENT_ON_CALL_FOR_HELP             = 17, // -- (pUnit, event)
    CREATURE_EVENT_ON_LOAD                      = 18, // -- (pUnit, event)
    CREATURE_EVENT_ON_REACH_WP                  = 19, // -- (pUnit, event, pWaypointId, pForwards)
    CREATURE_EVENT_ON_LOOT_TAKEN                = 20, // -- (pUnit, event, pPlayer, pItemId)
    CREATURE_EVENT_ON_AIUPDATE                  = 21, // -- (pUnit, event)
    CREATURE_EVENT_ON_EMOTE                     = 22, // -- (pUnit, event, pPlayer, pEmote)
    CREATURE_EVENT_ON_DAMAGE_TAKEN              = 23, // -- (pUnit, event, pAttacker, pAmount)
    CREATURE_EVENT_ON_ENTER_VEHICLE             = 24, // -- (pUnit)
    CREATURE_EVENT_ON_EXIT_VEHICLE              = 25, // -- (pUnit)
    CREATURE_EVENT_ON_FIRST_PASSENGER_ENTERED   = 26, // -- (pUnit, Passenger)
    CREATURE_EVENT_ON_VEHICLE_FULL              = 27, // -- (pUnit)
    CREATURE_EVENT_ON_LAST_PASSENGER_LEFT       = 28, // -- (pUnit, Passenger)
    CREATURE_EVENT_COUNT
};

enum GameObjectEvents
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //RegisterGameObjectEvent
    // GameObject callbacks are made by using the function RegisterGameObjectEvent(GameObjectId, EventId, function)

    GAMEOBJECT_EVENT_ON_CREATE                  = 1,  // -- (pGameObject)
    GAMEOBJECT_EVENT_ON_SPAWN                   = 2,  // -- (pGameObject)
    GAMEOBJECT_EVENT_ON_LOOT_TAKEN              = 3,  // -- (pGameObject, event, pLooter, ItemId)
    GAMEOBJECT_EVENT_ON_USE                     = 4,  // -- (pGameObject, event, pPlayer)
    GAMEOBJECT_EVENT_AIUPDATE                   = 5,  // -- (pGameObject)
    GAMEOBJECT_EVENT_ON_DESPAWN                 = 6,  // -- No arguments passed.
    GAMEOBJECT_EVENT_ON_DAMAGED                 = 7,  // -- (pGameObject, damage)
    GAMEOBJECT_EVENT_ON_DESTROYED               = 8,  // -- (pGameObject)
    GAMEOBJECT_EVENT_COUNT
};

enum GossipEvents
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //RegisterUnitGossipEvent
    // Gossip Event callbacks can be made using any of the following functions. Note that the pUnit in the arguments of these functions variate depending on the register you use. 

    // RegisterUnitGossipEvent(UnitId, EventId, function) (Applies to Creatures only) 
    // RegisterGOGossipEvent(GameObjectId, EventId, function) 
    // RegisterItemGossipEvent(ItemId, EventId, function)

    GOSSIP_EVENT_ON_TALK                        = 1,  // -- (pUnit, event, pPlayer)
    GOSSIP_EVENT_ON_SELECT_OPTION               = 2,  // -- (pUnit, event, pPlayer, id, intid, code)
    GOSSIP_EVENT_ON_END                         = 3,  // -- (pUnit, event)
    GOSSIP_EVENT_COUNT
};

enum RandomFlags
{
    RANDOM_ANY = 0,
    RANDOM_IN_SHORTRANGE = 1,
    RANDOM_IN_MIDRANGE = 2,
    RANDOM_IN_LONGRANGE = 3,
    RANDOM_WITH_MANA = 4,
    RANDOM_WITH_RAGE = 5,
    RANDOM_WITH_ENERGY = 6,
    RANDOM_NOT_MAINTANK = 7,
    RANDOM_COUNT
};

enum InstanceHooks
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //RegisterInstanceEvent
    // Instance Hook callbacks can be made by using the function RegisterInstanceEvent(MapId, EventId, function)
    
    INSTANCE_EVENT_ON_PLAYER_DEATH              = 1,  // -- (InstanceID, pPlayer, pKiller)
    INSTANCE_EVENT_ON_PLAYER_ENTER              = 2,  // -- (InstanceID, pPlayer)
    INSTANCE_EVENT_ON_AREA_TRIGGER              = 3,  // -- (InstanceID, pPlayer, nAreaId)
    INSTANCE_EVENT_ON_ZONE_CHANGE               = 4,  // -- (InstanceID, pPlayer, nNewZone, nOldZone)
    INSTANCE_EVENT_ON_CREATURE_DEATH            = 5,  // -- (InstanceID, pVictim, pKiller)
    INSTANCE_EVENT_ON_CREATURE_PUSH             = 6,  // -- (InstanceID, pUnit) {AKA "OnSpawn" but for within an instance}
    INSTANCE_EVENT_ON_GO_ACTIVATE               = 7,  // -- (InstanceID, pGo, pPlayer)
    INSTANCE_EVENT_ON_GO_PUSH                   = 8,  // -- (InstanceID, pGo) {AKA "OnSpawn" but for within an instance}
    INSTANCE_EVENT_ONLOAD                       = 9,  // -- (InstanceID) {When the instance is created}
    INSTANCE_EVENT_DESTROY                      = 10, // -- (InstanceID) {When the instance is destroyed, happens when the instance resets.}
    INSTANCE_EVENT_COUNT
};

//Nice thing about this is that we can ignore any new core events(as long as they are added in order), it will automatically update.
enum CustomLuaEvenTypes
{
    LUA_EVENT_START = NUM_EVENT_TYPES, // Used as a placeholder
    EVENT_LUA_TIMED,
    EVENT_LUA_CREATURE_EVENTS,
    EVENT_LUA_GAMEOBJ_EVENTS,
    LUA_EVENTS_END
};

struct LUALoadScripts
{
    std::set<std::string> luaFiles;
};

struct EventInfoHolder
{
    const char* funcName;
    TimedEvent* te;
};

struct LuaObjectBinding
{
    uint16 m_functionReferences[CREATURE_EVENT_COUNT];
};

class LuaEngine
{
    lua_State* lu;  // main state.
    Mutex call_lock;
    Mutex co_lock;

    typedef std::unordered_map<uint32, LuaObjectBinding> LuaObjectBindingMap;

    std::set<int> m_pendingThreads;
    std::set<int> m_functionRefs;
    std::map< uint64, std::set<int> > m_objectFunctionRefs;

    //maps to creature, & go script interfaces
    std::multimap<uint32, LuaCreature*> m_cAIScripts;
    std::multimap<uint32, LuaGameObjectScript*> m_gAIScripts;
    std::unordered_map<uint32, LuaQuest*> m_qAIScripts;
    std::unordered_map<uint32, LuaInstance*> m_iAIScripts;

    std::unordered_map<uint32, LuaGossip*> m_unitgAIScripts;
    std::unordered_map<uint32, LuaGossip*> m_itemgAIScripts;
    std::unordered_map<uint32, LuaGossip*> m_gogAIScripts;

    LuaObjectBindingMap m_unitBinding;
    LuaObjectBindingMap m_questBinding;
    LuaObjectBindingMap m_gameobjectBinding;
    LuaObjectBindingMap m_instanceBinding;
    LuaObjectBindingMap m_unit_gossipBinding;
    LuaObjectBindingMap m_item_gossipBinding;
    LuaObjectBindingMap m_go_gossipBinding;

public:

    LuaEngine();
    ~LuaEngine(){}
    void Startup();
    void LoadScripts();
    void Restart();

    void RegisterEvent(uint8, uint32, uint32, uint16);
    void ResumeLuaThread(int);
    void BeginCall(uint16);
    void HyperCallFunction(const char*, int);
    void CallFunctionByReference(int);
    void DestroyAllLuaEvents();
    inline bool ExecuteCall(uint8 params = 0, uint8 res = 0);
    inline void EndCall(uint8 res = 0);
    // Wrappers
    inline Unit* CheckUnit(lua_State* L, int narg)
    {
        if (L == NULL)
            return ArcLuna<Unit>::check(lu, narg);
        else
            return ArcLuna<Unit>::check(L, narg);
    }
    inline GameObject* CheckGo(lua_State* L, int narg)
    {
        if (L == NULL)
            return ArcLuna<GameObject>::check(lu, narg);
        else
            return ArcLuna<GameObject>::check(L, narg);
    }
    inline Item* CheckItem(lua_State* L, int narg)
    {
        if (L == NULL)
            return ArcLuna<Item>::check(lu, narg);
        else
            return ArcLuna<Item>::check(L, narg);
    }
    inline WorldPacket* CheckPacket(lua_State* L, int narg)
    {
        if (L == NULL)
            return ArcLuna<WorldPacket>::check(lu, narg);
        else
            return ArcLuna<WorldPacket>::check(L, narg);
    }
    inline uint64 CheckGuid(lua_State* L, int narg)
    {
        if (L == NULL)
            return GUID_MGR::check(lu, narg);
        else
            return GUID_MGR::check(L, narg);
    }
    inline Object* CheckObject(lua_State* L, int narg)
    {
        if (L == NULL)
            return ArcLuna<Object>::check(lu, narg);
        else
            return ArcLuna<Object>::check(L, narg);
    }
    inline TaxiPath* CheckTaxiPath(lua_State* L, int narg)
    {
        if (L == NULL)
            return ArcLuna<TaxiPath>::check(lu, narg);
        else
            return ArcLuna<TaxiPath>::check(L, narg);
    }
    inline Spell* CheckSpell(lua_State* L, int narg)
    {
        if (L == NULL)
            return ArcLuna<Spell>::check(lu, narg);
        else
            return ArcLuna<Spell>::check(L, narg);
    }
    inline Aura* CheckAura(lua_State* L, int narg)
    {
        if (L == NULL)
            return ArcLuna<Aura>::check(lu, narg);
        else
            return ArcLuna<Aura>::check(L, narg);
    }
    bool CheckBool(lua_State* L, int narg)
    {
        // first try with bool type
        if (lua_isboolean(L, narg))
            return lua_toboolean(L, narg) > 0;
        // then try with integer type
        else if (lua_isnumber(L, narg))
            return lua_tonumber(L, narg) > 0;
        // then return true by default
        else
            return true;
    }

    void PushUnit(Object* unit, lua_State* L = NULL);
    void PushGo(Object* go, lua_State* L = NULL);
    void PushItem(Object* item, lua_State* L = NULL);
    void PushGuid(uint64 guid, lua_State* L = NULL);
    void PushPacket(WorldPacket* packet, lua_State* L = NULL);
    void PushTaxiPath(TaxiPath* tp, lua_State* L = NULL);
    void PushSpell(Spell* sp, lua_State* L = NULL);
    void PushSqlField(Field* field, lua_State* L = NULL);
    void PushSqlResult(QueryResult* res, lua_State* L = NULL);
    void PushAura(Aura* aura, lua_State* L = NULL);

    inline void PUSH_BOOL(bool bewl)
    {
        if (bewl)
            lua_pushboolean(lu, 1);
        else
            lua_pushboolean(lu, 0);
    }
    inline void PUSH_NIL(lua_State* L = NULL)
    {
        if (L == NULL)
            lua_pushnil(lu);
        else
            lua_pushnil(L);
    }
    inline void PUSH_INT(int32 value)
    {
        lua_pushinteger(lu, value);
    }
    inline void PUSH_UINT(uint32 value)
    {
        lua_pushnumber(lu, value);
    }
    inline void PUSH_FLOAT(float value)
    {
        lua_pushnumber(lu, value);
    }
    inline void PUSH_STRING(const char* str)
    {
        lua_pushstring(lu, str);
    }
    void RegisterCoreFunctions();

    inline Mutex & getLock() { return call_lock; }
    inline Mutex & getcoLock() { return co_lock; }
    inline lua_State* getluState() { return lu; }

    LuaObjectBinding* getUnitBinding(uint32 Id)
    {
        LuaObjectBindingMap::iterator itr = m_unitBinding.find(Id);
        return (itr == m_unitBinding.end()) ? NULL : &itr->second;
    }
    LuaObjectBinding* getQuestBinding(uint32 Id)
    {
        LuaObjectBindingMap::iterator itr = m_questBinding.find(Id);
        return (itr == m_questBinding.end()) ? NULL : &itr->second;
    }
    LuaObjectBinding* getGameObjectBinding(uint32 Id)
    {
        LuaObjectBindingMap::iterator itr = m_gameobjectBinding.find(Id);
        return (itr == m_gameobjectBinding.end()) ? NULL : &itr->second;
    }
    LuaObjectBinding* getInstanceBinding(uint32 Id)
    {
        LuaObjectBindingMap::iterator itr = m_instanceBinding.find(Id);
        return (itr == m_instanceBinding.end()) ? NULL : &itr->second;
    }
    LuaObjectBinding* getLuaUnitGossipBinding(uint32 Id)
    {
        LuaObjectBindingMap::iterator itr = m_unit_gossipBinding.find(Id);
        return (itr == m_unit_gossipBinding.end()) ? NULL : &itr->second;
    }
    LuaObjectBinding* getLuaItemGossipBinding(uint32 Id)
    {
        LuaObjectBindingMap::iterator itr = m_item_gossipBinding.find(Id);
        return (itr == m_item_gossipBinding.end()) ? NULL : &itr->second;
    }
    LuaObjectBinding* getLuaGOGossipBinding(uint32 Id)
    {
        LuaObjectBindingMap::iterator itr = m_go_gossipBinding.find(Id);
        return (itr == m_go_gossipBinding.end()) ? NULL : &itr->second;
    }
    LuaQuest* getLuaQuest(uint32 id)
    {
    std::unordered_map<uint32, LuaQuest*>::iterator itr = m_qAIScripts.find(id);
        return (itr == m_qAIScripts.end()) ? NULL : itr->second;
    }
        /*int getPendingThread(lua_State * threadtosearch) {
            set<lua_State*>::iterator itr = m_pendingThreads.find(threadtosearch);
            return (itr == m_pendingThreads.end() )? NULL : (*itr);
            }*/
    LuaGossip* getUnitGossipInterface(uint32 id)
    {
        std::unordered_map<uint32, LuaGossip*>::iterator itr = m_unitgAIScripts.find(id);
        return (itr == m_unitgAIScripts.end()) ? NULL : itr->second;
    }
    LuaGossip* getItemGossipInterface(uint32 id)
    {
        std::unordered_map<uint32, LuaGossip*>::iterator itr = m_itemgAIScripts.find(id);
        return (itr == m_itemgAIScripts.end()) ? NULL : itr->second;
    }
    LuaGossip* getGameObjectGossipInterface(uint32 id)
    {
        std::unordered_map<uint32, LuaGossip*>::iterator itr = m_gogAIScripts.find(id);
        return (itr == m_gogAIScripts.end()) ? NULL : itr->second;
    }
    inline std::multimap<uint32, LuaCreature*> & getLuCreatureMap() { return m_cAIScripts; }
    inline std::multimap<uint32, LuaGameObjectScript*> & getLuGameObjectMap() { return m_gAIScripts; }
    inline std::unordered_map<uint32, LuaQuest*> & getLuQuestMap() { return m_qAIScripts; }
    inline std::unordered_map<uint32, LuaInstance*> & getLuInstanceMap() { return m_iAIScripts; }
    inline std::unordered_map<uint32, LuaGossip*> & getUnitGossipInterfaceMap() { return m_unitgAIScripts; }
    inline std::unordered_map<uint32, LuaGossip*> & getItemGossipInterfaceMap() { return m_itemgAIScripts; }
    inline std::unordered_map<uint32, LuaGossip*> & getGameObjectGossipInterfaceMap() { return m_gogAIScripts; }
    inline std::set<int> & getThreadRefs() { return m_pendingThreads; }
    inline std::set<int> & getFunctionRefs() { return m_functionRefs; }
    inline std::map< uint64, std::set<int> > & getObjectFunctionRefs() { return m_objectFunctionRefs; }

    std::unordered_map<int, EventInfoHolder*> m_registeredTimedEvents;

    struct _ENGINEHOOKINFO
    {
        bool hooks[NUM_SERVER_HOOKS];
        std::vector<uint32> dummyHooks;
        _ENGINEHOOKINFO()
        {
            for (int i = 0; i < NUM_SERVER_HOOKS; ++i)
                hooks[i] = false;
        }
    } HookInfo;

    class luEventMgr : public EventableObject
    {
    public:
        bool HasEvent(int ref)
        {
            std::unordered_map<int, EventInfoHolder*>::iterator itr = LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.find(ref);
            return (itr != LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.end());
        }
        bool HasEventInTable(const char* table)
        {
            std::unordered_map<int, EventInfoHolder*>::iterator itr = LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.begin();
            for (; itr != LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.end(); ++itr)
            {
                if (strncmp(itr->second->funcName, table, strlen(table)) == 0)
                {
                    return true;
                }
            }
            return false;
        }
        bool HasEventWithName(const char* name)
        {
            std::unordered_map<int, EventInfoHolder*>::iterator itr = LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.begin();
            for (; itr != LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.end(); ++itr)
            {
                if (strcmp(itr->second->funcName, name) == 0)
                {
                    return true;
                }
            }
                return false;
        }
        void RemoveEventsInTable(const char* table)
        {
            std::unordered_map<int, EventInfoHolder*>::iterator itr = LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.begin(), itr2;
            for (; itr != LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.end();)
            {
                itr2 = itr++;
                if (strncmp(itr2->second->funcName, table, strlen(table)) == 0)
                {
                    event_RemoveByPointer(itr2->second->te);
                    free((void*)itr2->second->funcName);
                    luaL_unref(LuaGlobal::instance()->luaEngine()->getluState(), LUA_REGISTRYINDEX, itr2->first);
                    LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.erase(itr2);
                }
            }
        }
        void RemoveEventsByName(const char* name)
        {
            std::unordered_map<int, EventInfoHolder*>::iterator itr = LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.begin(), itr2;
            for (; itr != LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.end();)
            {
                itr2 = itr++;
                if (strcmp(itr2->second->funcName, name) == 0)
                {
                    event_RemoveByPointer(itr2->second->te);
                    free((void*)itr2->second->funcName);
                    luaL_unref(LuaGlobal::instance()->luaEngine()->getluState(), LUA_REGISTRYINDEX, itr2->first);
                    LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.erase(itr2);
                }
            }
        }
        void RemoveEventByRef(int ref)
        {
            std::unordered_map<int, EventInfoHolder*>::iterator itr = LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.find(ref);
            if (itr != LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.end())
            {
                event_RemoveByPointer(itr->second->te);
                free((void*)itr->second->funcName);
                luaL_unref(LuaGlobal::instance()->luaEngine()->getluState(), LUA_REGISTRYINDEX, itr->first);
                LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.erase(itr);
            }
        }
        void RemoveEvents()
        {
            event_RemoveEvents(EVENT_LUA_TIMED);
            std::unordered_map<int, EventInfoHolder*>::iterator itr = LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.begin();
            for (; itr != LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.end(); ++itr)
            {
                free((void*)itr->second->funcName);
                luaL_unref(LuaGlobal::instance()->luaEngine()->getluState(), LUA_REGISTRYINDEX, itr->first);
            }
            LuaGlobal::instance()->luaEngine()->m_registeredTimedEvents.clear();
        }
    } LuaEventMgr;

protected:

    //Hidden methods
    void Unload();
    void ScriptLoadDir(char* Dirname, LUALoadScripts* pak);

template <typename T>
class ArcLuna
{
public:

    typedef int(*mfp)(lua_State* L, T* ptr);
    typedef struct { const char* name; mfp mfunc; } RegType;

    static void Register(lua_State* L)
    {
        lua_newtable(L);
        int methods = lua_gettop(L);

        luaL_newmetatable(L, GetTClassName<T>());
        int metatable = lua_gettop(L);

        luaL_newmetatable(L, "DO NOT TRASH");
        lua_pop(L, 1);

        // store method table in globals so that
        // scripts can add functions written in Lua.
        lua_pushvalue(L, methods);
        lua_setglobal(L, GetTClassName<T>());

        // hide metatable from Lua getmetatable()
        lua_pushvalue(L, methods);
        lua_setfield(L, metatable, "__metatable");

        lua_pushcfunction(L, index);
        lua_setfield(L, metatable, "__index");

        lua_pushcfunction(L, tostring_T);
        lua_setfield(L, metatable, "__tostring");

        lua_pushcfunction(L, gc_T);
        lua_setfield(L, metatable, "__gc");

        lua_newtable(L);                // mt for method table
        lua_setmetatable(L, methods);

        // fill method table with methods from class T
        for (RegType* l = ((RegType*)GetMethodTable<T>()); l->name; l++)
        {
            lua_pushstring(L, l->name);
            lua_pushlightuserdata(L, (void*)l);
            lua_pushcclosure(L, thunk, 1);
            lua_settable(L, methods);
        }
        lua_pop(L, 2);  // drop metatable and method table
    }

    // push onto the Lua stack a userdata containing a pointer to T object
    static int push(lua_State* L, T* obj, bool gc = false)
    {
        if (!obj)
        {
            lua_pushnil(L); return lua_gettop(L);
        }

        luaL_getmetatable(L, GetTClassName<T>());  // lookup metatable in Lua registry
        if (lua_isnil(L, -1))
            luaL_error(L, "%s missing metatable", GetTClassName<T>());

        int mt = lua_gettop(L);
        T** ptrHold = (T**)lua_newuserdata(L, sizeof(T**));
        int ud = lua_gettop(L);
        if (ptrHold != NULL)
        {
            *ptrHold = obj;
            lua_pushvalue(L, mt);
            lua_setmetatable(L, -2);
            char name[32];
            tostring(name, obj);
            lua_getfield(L, LUA_REGISTRYINDEX, "DO NOT TRASH");
            if (lua_isnil(L, -1))
            {
                luaL_newmetatable(L, "DO NOT TRASH");
                lua_pop(L, 1);
            }
            lua_getfield(L, LUA_REGISTRYINDEX, "DO NOT TRASH");
            if (gc == false)
            {
                lua_pushboolean(L, 1);
                lua_setfield(L, -2, name);
            }
            lua_pop(L, 1);
        }
        lua_settop(L, ud);
        lua_replace(L, mt);
        lua_settop(L, mt);
        return mt; // index of userdata containing pointer to T object
    }

    // get userdata from Lua stack and return pointer to T object
    static T* check(lua_State* L, int narg)
    {
        T** ptrHold = static_cast<T**>(lua_touserdata(L, narg));
        if (ptrHold == NULL)
            return NULL;
        return *ptrHold;
    }

private:

    ArcLuna();  // hide default constructor

    // member function dispatcher
    static int thunk(lua_State* L)
    {
        // stack has userdata, followed by method args
        T* obj = check(L, 1);  // get 'self', or if you prefer, 'this'
        lua_remove(L, 1);  // remove self so member function args start at index 1
        // get member function from upvalue
        RegType* l = static_cast<RegType*>(lua_touserdata(L, lua_upvalueindex(1)));
        //return (obj->*(l->mfunc))(L);  // call member function
        return l->mfunc(L, obj);
    }

    // garbage collection metamethod
    static int gc_T(lua_State* L)
    {
        T* obj = check(L, 1);
        if (obj == NULL)
            return 0;

        lua_getfield(L, LUA_REGISTRYINDEX, "DO NOT TRASH");
        if (lua_istable(L, -1))
        {
            char name[32];
            tostring(name, obj);
            lua_getfield(L, -1, std::string(name).c_str());
            if (lua_isnil(L, -1))
            {
                delete obj;
                obj = NULL;
            }
        }
        lua_pop(L, 3);
        return 0;
    }
    static int tostring_T(lua_State* L)
    {
        char buff[32];
        T** ptrHold = (T**)lua_touserdata(L, 1);
        T* obj = *ptrHold;
        sprintf(buff, "%p", obj);
        lua_pushfstring(L, "%s (%s)", GetTClassName<T>(), buff);
        return 1;
    }
    inline static void tostring(char* buff, void* obj)
    {
        sprintf(buff, "%p", obj);
    }
    static int index(lua_State* L)
    {
        /*Paroxysm : the table obj and the missing key are currently on the stack(index 1 & 2) */
        lua_getglobal(L, GetTClassName<T>());
        // string form of the key.
        const char* key = lua_tostring(L, 2);
        if (lua_istable(L, -1))
        {
            lua_pushvalue(L, 2);
            lua_rawget(L, -2);
            // If the key were looking for is not in the table, retrieve its' metatables' index value.
            if (lua_isnil(L, -1))
            {
                lua_getmetatable(L, -2);
                if (lua_istable(L, -1))
                {
                    lua_getfield(L, -1, "__index");
                    if (lua_isfunction(L, -1))
                    {
                        lua_pushvalue(L, 1);
                        lua_pushvalue(L, 2);
                        lua_pcall(L, 2, 1, 0);
                    }
                    else if (lua_istable(L, -1))
                        lua_getfield(L, -1, key);
                    else
                        lua_pushnil(L);
                }
                else
                    lua_pushnil(L);
            }
            else if (lua_istable(L, -1))
            {
                lua_pushvalue(L, 2);
                lua_rawget(L, -2);
            }
        }
        else
            lua_pushnil(L);
    
        lua_insert(L, 1);
        lua_settop(L, 1);
        return 1;
    }
};


class GUID_MGR
{
    static const char* GetName() { return "WoWGUID"; }

public:

    static void Register(lua_State* L)
    {
        luaL_newmetatable(L, GetName());
        int mt = lua_gettop(L);
        //Hide metatable.
        lua_pushnil(L);
        lua_setfield(L, mt, "__metatable");
        //nil gc method
        lua_pushnil(L);
        lua_setfield(L, mt, "__gc");
        //set our tostring method
        lua_pushcfunction(L, _tostring);
        lua_setfield(L, mt, "__tostring");
        //nil __index field
        lua_pushnil(L);
        lua_setfield(L, mt, "__index");
        //set __newindex method
        lua_pushcfunction(L, _newindex);
        lua_setfield(L, mt, "__newindex");
        //no call method
        lua_pushnil(L);
        lua_setfield(L, mt, "__call");
        // pop metatable
        lua_pop(L, 1);
    }
    static uint64 check(lua_State* L, int narg)
    {
        uint64 GUID = 0;
        uint64* ptrHold = (uint64*)lua_touserdata(L, narg);
        if (ptrHold != NULL)
            GUID = *ptrHold;
        return GUID;
    }
    static int push(lua_State* L, uint64 guid)
    {
        int index = 0;
        if (guid == 0)
        {
            lua_pushnil(L);
            index = lua_gettop(L);
        }
        else
        {
            luaL_getmetatable(L, GetName());
            if (lua_isnoneornil(L, -1))
                luaL_error(L, "%s metatable not found!. \n", GetName());
            else
            {
                int mt = lua_gettop(L);
                uint64* guidHold = (uint64*)lua_newuserdata(L, sizeof(uint64));
                int ud = lua_gettop(L);
                if (guidHold == NULL)
                    luaL_error(L, "Lua tried to allocate size %d of memory and failed! \n", sizeof(uint64*));
                else
                {
                    (*guidHold) = guid;
                    lua_pushvalue(L, mt);
                    lua_setmetatable(L, ud);
                    lua_replace(L, mt);
                    lua_settop(L, mt);
                    index = mt;
                }
            }
        }
        return index;
    }

    private:

        GUID_MGR() {}
        //This method prints formats the GUID in hexform and pushes to the stack.
        static int _tostring(lua_State* L)
        {
            uint64 GUID = GUID_MGR::check(L, 1);
            if (GUID == 0)
                lua_pushnil(L);
            else
            {
                char buff[32];
                sprintf(buff, I64FMT, GUID);
                lua_pushfstring(L, "%s", buff);
            }
            return 1;
        }
        static int _newindex(lua_State* L)
        {
            //Remove table, key, and value
            lua_remove(L, 1);
            lua_remove(L, 1);
            lua_remove(L, 1);
            luaL_error(L, "OPERATION PROHIBITED!\n");
            return 0;
        }
    };
};

static int RegisterServerHook(lua_State* L);
static int RegisterUnitEvent(lua_State* L);
static int RegisterQuestEvent(lua_State* L);
static int RegisterGameObjectEvent(lua_State* L);
static int RegisterUnitGossipEvent(lua_State* L);
static int RegisterItemGossipEvent(lua_State* L);
static int RegisterGOGossipEvent(lua_State* L);
static int SuspendLuaThread(lua_State* L);
static int RegisterTimedEvent(lua_State* L);
static int RemoveTimedEvents(lua_State* L);
static int RegisterDummySpell(lua_State* L);
static int RegisterInstanceEvent(lua_State* L);
void RegisterGlobalFunctions(lua_State*);