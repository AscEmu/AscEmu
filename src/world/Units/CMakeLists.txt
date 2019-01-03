# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Units)

set(SRC_UNITS_FILES
   ${PATH_PREFIX}/Stats.cpp
   ${PATH_PREFIX}/Stats.h
   ${PATH_PREFIX}/Unit.cpp
   ${PATH_PREFIX}/Unit.h
   ${PATH_PREFIX}/Unit.Legacy.cpp
   ${PATH_PREFIX}/UnitDefines.hpp
)

source_group(Units FILES ${SRC_UNITS_FILES})
unset(PATH_PREFIX)
