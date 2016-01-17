# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# general
set(ASCEMU_SCRIPTLIB_PATH "modules" CACHE PATH "The directory for AscEmu modules." )
option(BUILD_ASCEMUSCRIPTS "Build AscEmu modules." ON)
option(BUILD_TOOLS "Build AscEmu tools." ON)
option(BUILD_EXTRAS "Build AscEmu extra." OFF)
option(BUILD_EVENTSCRIPTS "Build ascEventScripts." ON)
option(BUILD_INSTANCESCRIPTS "Build ascInstanceScripts." ON)
option(BUILD_EXTRASCRIPTS "Build ascExtraScripts." ON)
option(BUILD_GOSSIPSCRIPTS "Build ascGossipScripts." ON)
option(BUILD_QUESTSCRIPTS "Build ascQuestScripts." ON)
option(BUILD_MISCSCRIPTS "Build ascMiscScripts." ON)
option(BUILD_LUAENGINE "Build LuaEngine." ON)
set(ASCEMU_TOOLS_PATH "tools" CACHE PATH "The directory where you want the tools installed.")
option(BUILD_TOOLS_MAP_EXTRACTOR "Build DBC and Map extractors." OFF)
option(BUILD_TOOLS_VMAPS "Build VMAP extractor tools." OFF)
option(BUILD_TOOLS_MMAPS_GENERATOR "Build MMAPS generator tools." OFF)

# platform specific
if(WIN32)
    set(VISUALSTUDIO_COMPILERHEAPLIMIT 460 CACHE STRING "Visual Studio compiler heap limit. Ignore on darwin and unix platforms.")
    set(ASCEMU_CONFIGSFILE_PATH configs CACHE PATH "Path to AscEmu configs." )
else()
    set(ASCEMU_CONFIGSFILE_PATH etc CACHE PATH "Path to AscEmu configs." )
endif()