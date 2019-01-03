# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Northrend/Naxxramas)

set(SRC_NORTHREND_NAXXRAMAS_FILES
   ${PATH_PREFIX}/Naxxramas.cpp
   ${PATH_PREFIX}/Naxxramas.h
)

source_group(Northrend\\Naxxramas FILES ${SRC_NORTHREND_NAXXRAMAS_FILES})
unset(PATH_PREFIX)
