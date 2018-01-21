# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Northrend/IceCrownCitadel)

set(SRC_NORTHREND_ICC_FILES
   ${PATH_PREFIX}/Icecrown_Citadel.cpp
   ${PATH_PREFIX}/Icecrown_Citadel.h
)

source_group(Northrend\\IceCrownCitadel FILES ${SRC_NORTHREND_ICC_FILES})
unset(PATH_PREFIX)
