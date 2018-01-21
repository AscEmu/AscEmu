# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Movement/Spline)

set(SRC_MOVEMENT_SPLINE_FILES
   ${PATH_PREFIX}/MovementSpline.cpp
   ${PATH_PREFIX}/MovementSpline.hpp
   ${PATH_PREFIX}/MovementSplineDefines.hpp
   ${PATH_PREFIX}/SplineFlags.cpp
   ${PATH_PREFIX}/SplineFlags.hpp
   ${PATH_PREFIX}/MonsterMoveFaceType.h
)

source_group(Movement\\Spline FILES ${SRC_MOVEMENT_SPLINE_FILES})
unset(PATH_PREFIX)
