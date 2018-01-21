# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

if("${ASCEMU_VERSION}" STREQUAL "Classic")
   set(ASC_VERSION_MAX_LEVEL "60")
   set(ASC_GAME_BUILD "5875")
elseif("${ASCEMU_VERSION}" STREQUAL "TBC")
   set(ASC_VERSION_MAX_LEVEL "70")
   set(ASC_GAME_BUILD "8606")
elseif("${ASCEMU_VERSION}" STREQUAL "WotLK")
   set(ASC_VERSION_MAX_LEVEL "80")
   set(ASC_GAME_BUILD "12340")
elseif("${ASCEMU_VERSION}" STREQUAL "Cata")
   set(ASC_VERSION_MAX_LEVEL "85")
   set(ASC_GAME_BUILD "15595")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/configs/logon.conf.in ${CMAKE_SOURCE_DIR}/configs/logon.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/configs/world.conf.in ${CMAKE_SOURCE_DIR}/configs/world.conf)
