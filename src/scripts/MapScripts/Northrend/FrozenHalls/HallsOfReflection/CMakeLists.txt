# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Northrend/FrozenHalls/HallsOfReflection)

set(SRC_NORTHREND_FH_HALLSOFREFLECTION_FILES
   ${PATH_PREFIX}/HallsOfReflection.cpp
   ${PATH_PREFIX}/HallsOfReflection.h
)

source_group(Northrend\\FrozenHalls\\HallsOfReflection FILES ${SRC_NORTHREND_FH_HALLSOFREFLECTION_FILES})
unset(PATH_PREFIX)
