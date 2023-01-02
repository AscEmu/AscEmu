# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Map)

set(SRC_MAP_FILES
    ${PATH_PREFIX}/AreaBoundary.cpp
    ${PATH_PREFIX}/AreaBoundary.hpp
    ${PATH_PREFIX}/RecastIncludes.hpp
    ${PATH_PREFIX}/SpawnGroups.hpp
)

source_group(Map FILES ${SRC_MAP_FILES})
unset(PATH_PREFIX)
