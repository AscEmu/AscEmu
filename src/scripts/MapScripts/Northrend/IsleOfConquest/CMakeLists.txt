# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Northrend/IsleOfConquest)

set(SRC_NORTHREND_ISLEOFCONQUEST_FILES
   ${PATH_PREFIX}/IsleOfConquest.cpp
   ${PATH_PREFIX}/IsleOfConquest.h
)

source_group(Northrend\\IsleOfConquest FILES ${SRC_NORTHREND_ISLEOFCONQUEST_FILES})
unset(PATH_PREFIX)
