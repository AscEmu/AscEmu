# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Objects/Units/Creatures/Summons)

set(SRC_UNITS_SUMMONS_FILES
    ${PATH_PREFIX}/Summon.cpp
    ${PATH_PREFIX}/Summon.hpp
    ${PATH_PREFIX}/SummonDefines.hpp
    ${PATH_PREFIX}/SummonHandler.cpp
    ${PATH_PREFIX}/SummonHandler.hpp
)

source_group(Objects\\Units\\Creatures\\Summons FILES ${SRC_UNITS_SUMMONS_FILES})
unset(PATH_PREFIX)
