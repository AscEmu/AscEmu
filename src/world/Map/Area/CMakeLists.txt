# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Map/Area)

set(SRC_MAP_AREA_FILES
   ${PATH_PREFIX}/AreaManagementGlobals.hpp
   ${PATH_PREFIX}/AreaStorage.cpp
   ${PATH_PREFIX}/AreaStorage.hpp
)

source_group(Map\\Area FILES ${SRC_MAP_AREA_FILES})
unset(PATH_PREFIX)
