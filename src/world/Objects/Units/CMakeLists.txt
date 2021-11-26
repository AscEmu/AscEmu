# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Objects/Units)

set(SRC_UNITS_FILES
    ${PATH_PREFIX}/Stats.cpp
    ${PATH_PREFIX}/Stats.h
    ${PATH_PREFIX}/Unit.cpp
    ${PATH_PREFIX}/Unit.h
    ${PATH_PREFIX}/Unit.Legacy.cpp
    ${PATH_PREFIX}/UnitDefines.hpp
    ${PATH_PREFIX}/ThreatHandler.cpp
    ${PATH_PREFIX}/ThreatHandler.h
)

source_group(Objects\\Units FILES ${SRC_UNITS_FILES})
unset(PATH_PREFIX)
