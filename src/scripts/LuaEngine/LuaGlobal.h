/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <memory>
#include <Management/Gossip/Gossip.h>
#include <Server/Script/ScriptMgr.h>

class LuaEngine;

class LuaGlobal
{
    static std::unique_ptr<LuaGlobal> s_instance;
    LuaGlobal();

    std::unique_ptr<LuaEngine> s_luaEngine;
public:
    static std::unique_ptr<LuaGlobal>& instance();

    Arcemu::Gossip::Menu* m_menu;
    std::vector<uint32_t> m_onLoadInfo;
    std::vector<uint16> EventAsToFuncName[NUM_SERVER_HOOKS];
    std::map<uint32, uint16> m_luaDummySpells;

    std::unique_ptr<LuaEngine>& luaEngine();
};
