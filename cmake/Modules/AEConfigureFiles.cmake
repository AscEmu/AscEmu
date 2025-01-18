# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

# ASCEMU_NUMBER - for including scripts
# ASC_VERSION_MAX_LEVEL - for setting the maximum level

if ("${ASCEMU_VERSION}" STREQUAL "Classic")
    set(ASCEMU_NUMBER 0)
    set(ASC_VERSION_MAX_LEVEL "60")
elseif ("${ASCEMU_VERSION}" STREQUAL "TBC")
    set(ASCEMU_NUMBER 1)
    set(ASC_VERSION_MAX_LEVEL "70")
elseif ("${ASCEMU_VERSION}" STREQUAL "WotLK")
    set(ASCEMU_NUMBER 2)
    set(ASC_VERSION_MAX_LEVEL "80")
elseif ("${ASCEMU_VERSION}" STREQUAL "Cata")
    set(ASCEMU_NUMBER 3)
    set(ASC_VERSION_MAX_LEVEL "85")
elseif ("${ASCEMU_VERSION}" STREQUAL "Mop")
    set(ASCEMU_NUMBER 4)
    set(ASC_VERSION_MAX_LEVEL "90")
endif ()

# generate Configs
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/configs/logon.conf.in ${CMAKE_SOURCE_DIR}/configs/logon.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/configs/world.conf.in ${CMAKE_SOURCE_DIR}/configs/world.conf)
