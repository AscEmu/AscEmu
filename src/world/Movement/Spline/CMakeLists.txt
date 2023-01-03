# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Movement/Spline)

set(SRC_MOVEMENT_SPLINE_FILES
    ${PATH_PREFIX}/Spline.cpp
    ${PATH_PREFIX}/Spline.h
    ${PATH_PREFIX}/SplineImpl.h
    ${PATH_PREFIX}/MovementTypedefs.h
    ${PATH_PREFIX}/MoveSplineInitArgs.h
    ${PATH_PREFIX}/MoveSplineFlag.h
    ${PATH_PREFIX}/MovementUtil.cpp
    ${PATH_PREFIX}/MoveSpline.cpp
    ${PATH_PREFIX}/MoveSpline.h
    ${PATH_PREFIX}/MoveSplineFlag.h
    ${PATH_PREFIX}/SplineChain.h
    ${PATH_PREFIX}/MovementPacketBuilder.cpp
    ${PATH_PREFIX}/MovementPacketBuilder.h
    ${PATH_PREFIX}/MoveSplineInit.cpp
    ${PATH_PREFIX}/MoveSplineInit.h
)

source_group(Movement\\Spline FILES ${SRC_MOVEMENT_SPLINE_FILES})
unset(PATH_PREFIX)
