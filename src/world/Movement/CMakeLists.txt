# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Movement)

set(SRC_MOVEMENT_FILES
    ${PATH_PREFIX}/MovementManager.cpp
    ${PATH_PREFIX}/MovementManager.h
    ${PATH_PREFIX}/MovementGenerator.cpp
    ${PATH_PREFIX}/MovementGenerator.h
    ${PATH_PREFIX}/MovementDefines.cpp
    ${PATH_PREFIX}/MovementDefines.h
    ${PATH_PREFIX}/AbstractFollower.cpp
    ${PATH_PREFIX}/AbstractFollower.h
    ${PATH_PREFIX}/PathGenerator.cpp
    ${PATH_PREFIX}/PathGenerator.h
    ${PATH_PREFIX}/WaypointDefines.h
    ${PATH_PREFIX}/WaypointManager.cpp
    ${PATH_PREFIX}/WaypointManager.h
)

source_group(Movement FILES ${SRC_MOVEMENT_FILES})
unset(PATH_PREFIX)
