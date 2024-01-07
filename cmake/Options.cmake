# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

set(ASCEMU_CONFIGSFILE_PATH "configs" CACHE PATH "The directory for AscEmu configs.")

set(ASCEMU_SCRIPTLIB_PATH "modules" CACHE PATH "The directory for AscEmu modules.")

set(ASCEMU_TOOLS_PATH "tools" CACHE PATH "The directory for AscEmu tools.")

set(ASCEMU_VERSION "WotLK" CACHE STRING "Client Version")
set_property(CACHE ASCEMU_VERSION PROPERTY STRINGS Classic TBC WotLK Cata Mop)

option(BUILD_ASCEMUSCRIPTS      "Build AscEmu modules."                                                     ON)
option(BUILD_TOOLS              "Build AscEmu tools."                                                       OFF)
option(BUILD_EXTRAS             "Build AscEmu extra."                                                       OFF)
option(BUILD_EVENTSCRIPTS       "Build ascEventScripts."                                                    ON)
option(BUILD_INSTANCESCRIPTS    "Build ascInstanceScripts."                                                 ON)
option(BUILD_EXTRASCRIPTS       "Build ascExtraScripts."                                                    ON)
option(BUILD_GOSSIPSCRIPTS      "Build ascGossipScripts."                                                   ON)
option(BUILD_QUESTSCRIPTS       "Build ascQuestScripts."                                                    ON)
option(BUILD_MAPSCRIPTS         "Build ascMapScripts."                                                      ON)
option(BUILD_MISCSCRIPTS        "Build ascMiscScripts."                                                     ON)
option(BUILD_LUAENGINE          "Build LuaEngine."                                                          ON)
set(ASCEMU_TOOLS_PATH           "tools"                                                                     CACHE PATH "The directory where you want the tools installed.")
option(BUILD_WITH_WARNINGS      "Enable/Disable warnings on compilation"                                    ON)
option(USE_PCH                  "Enable precompiled headers - it will reduce compilation time"              ON)
option(TREAT_WARNINGS_AS_ERRORS "Treats warnings as errors"                                                 OFF)
