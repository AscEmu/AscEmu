# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

#set path to our config files
if(WIN32)
   set(ASCEMU_CONFIGSFILE_PATH configs CACHE PATH "Path to AscEmu configs.")
else()
   set(ASCEMU_CONFIGSFILE_PATH etc CACHE PATH "Path to AscEmu configs.")
endif()

set(ASCEMU_SCRIPTLIB_PATH "modules" CACHE PATH "The directory for AscEmu modules.")
set(BUILD_ASCEMUSCRIPTS TRUE CACHE BOOL "Build AscEmu modules.")
set(BUILD_TOOLS TRUE CACHE BOOL "Build AscEmu tools.")
set(BUILD_EXTRAS FALSE CACHE BOOL "Build AscEmu extra.")

#set compiler heap limit.
set(VISUALSTUDIO_COMPILERHEAPLIMIT 460 CACHE STRING "Visual Studio compiler heap limit. Ignore on darwin and unix platforms.")
