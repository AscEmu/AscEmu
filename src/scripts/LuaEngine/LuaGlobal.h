/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <memory>
#include <Management/Gossip/GossipScript.h>
#include <Server/Script/ScriptMgr.h>
#include "WoWGuid.h"
#include "Management/Gossip/GossipMenu.h"

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
    std::vector<uint16_t> EventAsToFuncName[NUM_SERVER_HOOKS];
    std::map<uint32_t, uint16_t> m_luaDummySpells;

    std::unique_ptr<LuaEngine>& luaEngine();
};
