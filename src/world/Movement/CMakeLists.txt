# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Movement)

set(SRC_MOVEMENT_FILES
   ${PATH_PREFIX}/MovementCommon.cpp
   ${PATH_PREFIX}/MovementCommon.hpp
   ${PATH_PREFIX}/UnitMovementManager.cpp
   ${PATH_PREFIX}/UnitMovementManager.hpp
)

source_group(Movement FILES ${SRC_MOVEMENT_FILES})
unset(PATH_PREFIX)
