# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Northrend/Nexus/Oculus)

set(SRC_NORTHREND_NX_OCULUS_FILES
   ${PATH_PREFIX}/Oculus.cpp
   ${PATH_PREFIX}/Oculus.h
)

source_group(Northrend\\Nexus\\Oculus FILES ${SRC_NORTHREND_NX_OCULUS_FILES})
unset(PATH_PREFIX)
