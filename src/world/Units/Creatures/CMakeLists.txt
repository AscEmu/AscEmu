# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Units/Creatures)

set(SRC_UNITS_CREATURES_FILES
   ${PATH_PREFIX}/Corpse.cpp
   ${PATH_PREFIX}/Corpse.h
   ${PATH_PREFIX}/Creature.cpp
   ${PATH_PREFIX}/Creature.h
   ${PATH_PREFIX}/CreatureDefines.hpp
   ${PATH_PREFIX}/Pet.cpp
   ${PATH_PREFIX}/Pet.h
   ${PATH_PREFIX}/Vehicle.cpp
   ${PATH_PREFIX}/Vehicle.h
)

source_group(Units\\Creatures FILES ${SRC_UNITS_CREATURES_FILES})
unset(PATH_PREFIX)
