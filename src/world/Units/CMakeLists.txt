# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Units)

set(SRC_UNITS_FILES
   ${PATH_PREFIX}/CombatStatus.h
   ${PATH_PREFIX}/CombatStatus.cpp
   ${PATH_PREFIX}/Stats.cpp
   ${PATH_PREFIX}/Stats.h
   ${PATH_PREFIX}/Unit.cpp
   ${PATH_PREFIX}/Unit.h
   ${PATH_PREFIX}/UnitDefines.hpp
)

source_group(Units FILES ${SRC_UNITS_FILES})
unset(PATH_PREFIX)
