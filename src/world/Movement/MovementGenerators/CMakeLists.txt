# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Movement/MovementGenerators)

set(SRC_MOVEMENT_MOVEMENTGENERATOR_FILES
    ${PATH_PREFIX}/ChaseMovementGenerator.cpp
    ${PATH_PREFIX}/ChaseMovementGenerator.h
    ${PATH_PREFIX}/ConfusedMovementGenerator.cpp
    ${PATH_PREFIX}/ConfusedMovementGenerator.h
    ${PATH_PREFIX}/FleeingMovementGenerator.cpp
    ${PATH_PREFIX}/FleeingMovementGenerator.h
    ${PATH_PREFIX}/FollowMovementGenerator.cpp
    ${PATH_PREFIX}/FollowMovementGenerator.h
    ${PATH_PREFIX}/FormationMovementGenerator.cpp
    ${PATH_PREFIX}/FormationMovementGenerator.h
    ${PATH_PREFIX}/GenericMovementGenerator.cpp
    ${PATH_PREFIX}/GenericMovementGenerator.h
    ${PATH_PREFIX}/HomeMovementGenerator.cpp
    ${PATH_PREFIX}/HomeMovementGenerator.h
    ${PATH_PREFIX}/IdleMovementGenerator.cpp
    ${PATH_PREFIX}/IdleMovementGenerator.h
    ${PATH_PREFIX}/PathMovementBase.h
    ${PATH_PREFIX}/PointMovementGenerator.cpp
    ${PATH_PREFIX}/PointMovementGenerator.h
    ${PATH_PREFIX}/RandomMovementGenerator.cpp
    ${PATH_PREFIX}/RandomMovementGenerator.h
    ${PATH_PREFIX}/SplineChainMovementGenerator.cpp
    ${PATH_PREFIX}/SplineChainMovementGenerator.h   
    ${PATH_PREFIX}/WaypointMovementGenerator.cpp
    ${PATH_PREFIX}/WaypointMovementGenerator.h   
)

source_group(Movement\\MovementGenerators FILES ${SRC_MOVEMENT_MOVEMENTGENERATOR_FILES})
unset(PATH_PREFIX)
