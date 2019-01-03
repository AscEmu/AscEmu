# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Northrend/Nexus/Nexus)

set(SRC_NORTHREND_NX_NEXUS_FILES
   ${PATH_PREFIX}/Nexus.cpp
   ${PATH_PREFIX}/Nexus.h
)

source_group(Northrend\\Nexus\\Nexus FILES ${SRC_NORTHREND_NX_NEXUS_FILES})
unset(PATH_PREFIX)
