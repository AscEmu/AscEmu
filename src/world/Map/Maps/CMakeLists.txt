# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Map/Maps)

set(SRC_MAP_MAPS_FILES
    ${PATH_PREFIX}/BaseMap.cpp
    ${PATH_PREFIX}/BaseMap.hpp
    ${PATH_PREFIX}/InstanceMap.cpp
    ${PATH_PREFIX}/InstanceMap.hpp
    ${PATH_PREFIX}/InstanceDefines.hpp
    ${PATH_PREFIX}/InstanceMgr.cpp
    ${PATH_PREFIX}/InstanceMgr.hpp
    ${PATH_PREFIX}/MapScriptInterface.cpp
    ${PATH_PREFIX}/MapScriptInterface.h
    ${PATH_PREFIX}/WorldMap.cpp
    ${PATH_PREFIX}/WorldMap.hpp
    ${PATH_PREFIX}/BattleGroundMap.cpp
    ${PATH_PREFIX}/BattleGroundMap.hpp
)

source_group(Map\\Maps FILES ${SRC_MAP_MAPS_FILES})
unset(PATH_PREFIX)
