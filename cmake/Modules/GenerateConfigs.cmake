# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

if("${ASCEMU_VERSION}" STREQUAL "Classic")
   set(ASC_VERSION_MAX_LEVEL "60")
elseif("${ASCEMU_VERSION}" STREQUAL "TBC")
   set(ASC_VERSION_MAX_LEVEL "70")
elseif("${ASCEMU_VERSION}" STREQUAL "WotLK")
   set(ASC_VERSION_MAX_LEVEL "80")
elseif("${ASCEMU_VERSION}" STREQUAL "Cata")
   set(ASC_VERSION_MAX_LEVEL "85")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/configs/logon.conf.in ${CMAKE_SOURCE_DIR}/configs/logon.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/configs/world.conf.in ${CMAKE_SOURCE_DIR}/configs/world.conf)
