/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <map>
#include <memory>
#include <vector>

class GossipMenu;
class LuaEngine;

class LuaGlobal
{
    static std::unique_ptr<LuaGlobal> s_instance;
    LuaGlobal();

    std::unique_ptr<LuaEngine> s_luaEngine;
public:
    static std::unique_ptr<LuaGlobal>& instance();

    GossipMenu* m_menu;
    std::vector<uint32_t> m_onLoadInfo;
    std::vector<uint16_t> EventAsToFuncName[33]; //NUM_SERVER_HOOKS
    std::map<uint32_t, uint16_t> m_luaDummySpells;

    std::unique_ptr<LuaEngine>& luaEngine();
};
