# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Northrend/Ulduar/HallsOfLightning)

set(SRC_NORTHREND_UL_HALLSOFLIGHTNING_FILES
   ${PATH_PREFIX}/HallsOfLightning.cpp
   ${PATH_PREFIX}/HallsOfLightning.h
)

source_group(Northrend\\Ulduar\\HallsOfLightning FILES ${SRC_NORTHREND_UL_HALLSOFLIGHTNING_FILES})
unset(PATH_PREFIX)
