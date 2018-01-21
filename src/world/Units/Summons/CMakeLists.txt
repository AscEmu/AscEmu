# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Units/Summons)

set(SRC_UNITS_SUMMONS_FILES
   ${PATH_PREFIX}/Summon.cpp
   ${PATH_PREFIX}/Summon.h
   ${PATH_PREFIX}/SummonHandler.cpp
   ${PATH_PREFIX}/SummonHandler.h
   ${PATH_PREFIX}/CompanionSummon.cpp
   ${PATH_PREFIX}/CompanionSummon.h
   ${PATH_PREFIX}/SummonDefines.hpp
   ${PATH_PREFIX}/GuardianSummon.cpp
   ${PATH_PREFIX}/GuardianSummon.h
   ${PATH_PREFIX}/PossessedSummon.cpp
   ${PATH_PREFIX}/PossessedSummon.h
   ${PATH_PREFIX}/TotemSummon.cpp
   ${PATH_PREFIX}/TotemSummon.h
   ${PATH_PREFIX}/WildSummon.cpp
   ${PATH_PREFIX}/WildSummon.h
)

source_group(Units\\Summons FILES ${SRC_UNITS_SUMMONS_FILES})
unset(PATH_PREFIX)
