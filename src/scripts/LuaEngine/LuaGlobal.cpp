/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LuaGlobal.hpp"
#include "LUAEngine.hpp"

std::unique_ptr<LuaGlobal> LuaGlobal::s_instance;

LuaGlobal::LuaGlobal(): m_menu(nullptr)
{
}

std::unique_ptr<LuaGlobal>& LuaGlobal::instance()
{
    if (!s_instance)
        s_instance = std::unique_ptr<LuaGlobal>(new LuaGlobal);

    return s_instance;
}

std::unique_ptr<LuaEngine>& LuaGlobal::luaEngine()
{
    if (!s_luaEngine)
        s_luaEngine = std::make_unique<LuaEngine>();

    return s_luaEngine;
}
