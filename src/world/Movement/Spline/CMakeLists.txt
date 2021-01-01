# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Movement/Spline)

set(SRC_MOVEMENT_SPLINE_FILES
   ${PATH_PREFIX}/MovementSpline.cpp
   ${PATH_PREFIX}/MovementSpline.hpp
   ${PATH_PREFIX}/MovementSplineDefines.hpp
   ${PATH_PREFIX}/SplineFlags.cpp
   ${PATH_PREFIX}/SplineFlags.hpp
   ${PATH_PREFIX}/MonsterMoveFaceType.h
   ${PATH_PREFIX}/New/Spline.cpp
   ${PATH_PREFIX}/New/Spline.h
   ${PATH_PREFIX}/New/SplineImpl.h
   ${PATH_PREFIX}/New/MovementTypedefs.h
   ${PATH_PREFIX}/New/MoveSplineInitArgs.h
   ${PATH_PREFIX}/New/MoveSplineFlag.h
   ${PATH_PREFIX}/New/MovementUtil.cpp
   ${PATH_PREFIX}/New/MoveSpline.cpp
   ${PATH_PREFIX}/New/MoveSpline.h
   ${PATH_PREFIX}/New/MoveSplineFlag.h
   ${PATH_PREFIX}/New/SplineChain.h
)

source_group(Movement\\Spline FILES ${SRC_MOVEMENT_SPLINE_FILES})
unset(PATH_PREFIX)
