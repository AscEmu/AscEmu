# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Northrend/DraktharonKeep)

set(SRC_NORTHREND_DRAKTHARONKEEP_FILES
   ${PATH_PREFIX}/DraktharonKeep.cpp
   ${PATH_PREFIX}/DraktharonKeep.h
)

source_group(Northrend\\DraktharonKeep FILES ${SRC_NORTHREND_DRAKTHARONKEEP_FILES})
unset(PATH_PREFIX)
