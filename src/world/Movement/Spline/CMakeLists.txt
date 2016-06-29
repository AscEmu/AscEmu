# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Movement/Spline)

set(SRC_MOVEMENT_SPLINE_FILES
   ${PATH_PREFIX}/MovementSpline.cpp
   ${PATH_PREFIX}/MovementSpline.hpp
   ${PATH_PREFIX}/MovementSplineDefines.hpp
   ${PATH_PREFIX}/SplineFlags.cpp
   ${PATH_PREFIX}/SplineFlags.hpp
)

source_group(Movement\\Spline FILES ${SRC_MOVEMENT_SPLINE_FILES})
unset(PATH_PREFIX)
