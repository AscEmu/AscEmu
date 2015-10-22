/*
 * luabridge.cpp - Copyright (C) 2007 by Nathan Reed
 * Source for the luabridge library.
 */

#include "LUAEngine.h"
#include <cstdio>

/*#ifdef _MSC_VER
#    if (_MSC_VER >= 1400)
#        define snprintf _snprintf_s
#    else
#        define snprintf _snprintf
#    endif
#endif */

/*
 * Default name for unknown types
 */

const char* luabridge::classname_unknown = "(unknown type)";

/*
 * Class type checker.  Given the index of a userdata on the stack, makes
 * sure that it's an object of the given classname or a subclass thereof.
 * If yes, returns the address of the data; otherwise, throws an error.
 * Works like the luaL_checkudata function.
 */

void* luabridge::checkclass(lua_State* L, int idx, const char* tname,
                            bool exact)
{
    // If idx is relative to the top of the stack, convert it into an index
    // relative to the bottom of the stack, so we can push our own stuff
    if(idx < 0)
        idx += lua_gettop(L) + 1;

    int top = lua_gettop(L);

    // Check that the thing on the stack is indeed a userdata
    if(!lua_isuserdata(L, idx))
        luaL_argerror(L, idx, tname);

    // Lookup the given name in the registry
    luaL_getmetatable(L, tname);

    // Lookup the metatable of the given userdata
    lua_getmetatable(L, idx);

    // If exact match required, simply test for identity.
    if(exact)
    {
        // Ignore "const" for exact tests (which are used for destructors).
        if(!strncmp(tname, "const ", 6))
            tname += 6;

        if(lua_rawequal(L, -1, -2))
            return lua_touserdata(L, idx);
        else
        {
            // Generate an informative error message
            rawgetfield(L, -1, "__type");
            char buffer[256];
            snprintf(buffer, 256, "%s expected, got %s", tname,
                     lua_tostring(L, -1));
            // luaL_argerror does not return
            luaL_argerror(L, idx, buffer);
            return 0;
        }
    }

    // Navigate up the chain of parents if necessary
    while(!lua_rawequal(L, -1, -2))
    {
        // Check for equality to the const metatable
        rawgetfield(L, -1, "__const");
        if(!lua_isnil(L, -1))
        {
            if(lua_rawequal(L, -1, -3))
                break;
        }
        lua_pop(L, 1);

        // Look for the metatable's parent field
        rawgetfield(L, -1, "__parent");

        // No parent field?  We've failed; generate appropriate error
        if(lua_isnil(L, -1))
        {
            // Lookup the __type field of the original metatable, so we can
            // generate an informative error message
            lua_getmetatable(L, idx);
            rawgetfield(L, -1, "__type");

            char buffer[256];
            snprintf(buffer, 256, "%s expected, got %s", tname,
                     lua_tostring(L, -1));
            // luaL_argerror does not return
            luaL_argerror(L, idx, buffer);
            return 0;
        }

        // Remove the old metatable from the stack
        lua_remove(L, -2);
    }

    lua_settop(L, top);
    // Found a matching metatable; return the userdata
    return lua_touserdata(L, idx);
}

/*
 * Index metamethod for C++ classes exposed to Lua.  This searches the
 * metatable for the given key, but if it can't find it, it looks for a
 * __parent key and delegates the lookup to that.
 */

int luabridge::indexer(lua_State* L)
{
    lua_getmetatable(L, 1);

    do
    {
        // Look for the key in the metatable
        lua_pushvalue(L, 2);
        lua_rawget(L, -2);
        // Did we get a non-nil result?  If so, return it
        if(!lua_isnil(L, -1))
            return 1;
        ///pop the nil
        lua_pop(L, 1);
        //search the metatable of the object's metatable.
        if(lua_getmetatable(L, -1) && !lua_isnil(L, -1))
        {
            //to prevent calling this indexer method again, make sure the found metatable is not equal to the table's metatable.
            if(!lua_rawequal(L, -1, -2))
            {
                //store our current argument cnt so we can determine if any arguments were pushed
                ptrdiff_t base = lua_gettop(L), top;
                //push the method name being called
                lua_pushvalue(L, 2);
                //try to retrieve the key and allow any events to be called.
                lua_gettable(L, -2);
                //if there were any arguments, return those.
                top = lua_gettop(L);
                //check for multiple arguments.
                if(top > (base + 1))
                    return (top - base);
                //check if the returned object is nil
                else if(top == (base + 1) && !lua_isnil(L, -1))
                    return 1;
                else if(top == (base + 1) && lua_isnil(L, -1))
                    lua_pop(L, 1);
                //otherwise, proceed w/ other checks.
            }
            //pop the metatable's metatable.
            lua_pop(L, 1);
        }
        // Look for the key in the __propget metafield
        rawgetfield(L, -1, "__propget");
        if(!lua_isnil(L, -1))
        {
            lua_pushvalue(L, 2);
            lua_rawget(L, -2);
            // If we got a non-nil result, call it and return its value
            if(!lua_isnil(L, -1))
            {
                assert(lua_isfunction(L, -1));
                lua_pushvalue(L, 1);
                lua_call(L, 1, 1);
                return 1;
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        // Look for the key in the __const metafield
        rawgetfield(L, -1, "__const");
        if(!lua_isnil(L, -1))
        {
            lua_pushvalue(L, 2);
            lua_rawget(L, -2);
            if(!lua_isnil(L, -1))
                return 1;
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        // Look for a __parent metafield; if it doesn't exist, return nil;
        // otherwise, repeat the lookup on it.
        rawgetfield(L, -1, "__parent");
        if(lua_isnil(L, -1))
            return 1;
        lua_remove(L, -2);
    }
    while(true);

    // Control never gets here
    return 0;
}

/*
 * Newindex metamethod for supporting properties on modules and static
 * properties of classes.
 */

int luabridge::newindexer(lua_State* L)
{
    lua_getmetatable(L, 1);

    do
    {
        // Look for the key in the __propset metafield
        rawgetfield(L, -1, "__propset");
        if(!lua_isnil(L, -1))
        {
            lua_pushvalue(L, 2);
            lua_rawget(L, -2);
            // If we got a non-nil result, call it
            if(!lua_isnil(L, -1))
            {
                assert(lua_isfunction(L, -1));
                lua_pushvalue(L, 3);
                lua_call(L, 1, 0);
                return 0;
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        // Look for a __parent metafield; if it doesn't exist, error;
        // otherwise, repeat the lookup on it.
        rawgetfield(L, -1, "__parent");
        if(lua_isnil(L, -1))
        {
            return luaL_error(L, "attempt to set %s, which isn't a property",
                              lua_tostring(L, 2));
        }
        lua_remove(L, -2);
    }
    while(true);

    // Control never gets here
    return 0;
}

/*
 * Newindex variant for properties of objects, which passes the
 * object on which the property is to be set as the first parameter
 * to the setter method.
 */

int luabridge::m_newindexer(lua_State* L)
{
    lua_getmetatable(L, 1);

    do
    {
        // Look for the key in the __propset metafield
        rawgetfield(L, -1, "__propset");
        if(!lua_isnil(L, -1))
        {
            lua_pushvalue(L, 2);
            lua_rawget(L, -2);
            // If we got a non-nil result, call it
            if(!lua_isnil(L, -1))
            {
                assert(lua_isfunction(L, -1));
                lua_pushvalue(L, 1);
                lua_pushvalue(L, 3);
                lua_call(L, 2, 0);
                return 0;
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        // Look for a __parent metafield; if it doesn't exist, error;
        // otherwise, repeat the lookup on it.
        rawgetfield(L, -1, "__parent");
        if(lua_isnil(L, -1))
        {
            return luaL_error(L, "attempt to set %s, which isn't a property",
                              lua_tostring(L, 2));
        }
        lua_remove(L, -2);
    }
    while(true);

    // Control never gets here
    return 0;
}

template<>
int luabridge::metaevent_tostring< ObjectWrap<uint64> >(lua_State* L)
{
    typedef ObjectWrap<uint64> typ;
    uint64 guid = static_cast<typ*>(checkclass(L, 1, classname<typ>::name()))->value_;
    std::ostringstream stream;
    stream << std::hex << guid;
    lua_pushstring(L, stream.str().c_str());
    return 1;
}

template<>
int luabridge::metaevent_tostring< ObjectWrap<const uint64> >(lua_State* L)
{
    typedef ObjectWrap<const uint64> typ;
    const uint64 guid = static_cast<typ*>(checkclass(L, 1, classname<typ>::name()))->value_;
    std::ostringstream stream;
    stream << std::hex << guid;
    lua_pushstring(L, stream.str().c_str());
    return 1;
}

template<>
int luabridge::metaevent_tostring< ObjectWrap<uint64 &> >(lua_State* L)
{
    typedef ObjectWrap<uint64> typ;
    uint64 guid = static_cast<typ*>(checkclass(L, 1, classname<typ>::name()))->value_;
    std::ostringstream stream;
    stream << std::hex << guid;
    lua_pushstring(L, stream.str().c_str());
    return 1;
}

template<>
int luabridge::metaevent_tostring<const uint64 &>(lua_State* L)
{
    typedef ObjectWrap<const uint64> typ;
    uint64 guid = static_cast<typ*>(checkclass(L, 1, classname<typ>::name()))->value_;
    std::ostringstream stream;
    stream << std::hex << guid;
    lua_pushstring(L, stream.str().c_str());
    return 1;
}

template<>
int luabridge::metaevent_equal< ObjectWrap<uint64> >(lua_State* L)
{
    typedef ObjectWrap<uint64> typ;
    uint64 obj1, obj2;
    obj1 = static_cast<typ*>(checkclass(L, 1, classname<typ>::name()))->value_;
    obj2 =  static_cast<typ*>(checkclass(L, 2, classname<typ>::name()))->value_;
    if(obj1 == obj2)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

template<>
int luabridge::metaevent_equal< ObjectWrap<uint64 &> >(lua_State* L)
{
    typedef ObjectWrap<uint64> typ;
    typ & obj1 = *static_cast<typ*>(checkclass(L, 1, classname<typ>::name()));
    typ & obj2 =  *static_cast<typ*>(checkclass(L, 2, classname<typ>::name()));
    if(obj1 == obj2)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

template<>
int luabridge::metaevent_equal< ObjectWrap<const uint64> >(lua_State* L)
{
    typedef ObjectWrap< const uint64> typ;
    typ & obj1 = *static_cast<typ*>(checkclass(L, 1, classname<typ>::name()));
    typ & obj2 =  *static_cast<typ*>(checkclass(L, 2, classname<typ>::name()));
    if(obj1 == obj2)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

template<>
int luabridge::metaevent_equal< ObjectWrap<const uint64 &> >(lua_State* L)
{
    typedef ObjectWrap< const uint64> typ;
    typ & obj1 = *static_cast<typ*>(checkclass(L, 1, classname<typ>::name()));
    typ & obj2 =  *static_cast<typ*>(checkclass(L, 2, classname<typ>::name()));
    if(obj1 == obj2)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

