# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Northrend/VioletHold)

set(SRC_NORTHREND_VIOLETHOLD_FILES
   ${PATH_PREFIX}/VioletHold.cpp
   ${PATH_PREFIX}/VioletHold.h
)

source_group(Northrend\\VioletHold FILES ${SRC_NORTHREND_VIOLETHOLD_FILES})
unset(PATH_PREFIX)
