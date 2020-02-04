# Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Scripts)

set(SRC_SPELL_SCRIPTS_FILES
   ${PATH_PREFIX}/DeathKnight.cpp
   ${PATH_PREFIX}/Druid.cpp
   ${PATH_PREFIX}/Hunter.cpp
   ${PATH_PREFIX}/Items.cpp
   ${PATH_PREFIX}/Mage.cpp
   ${PATH_PREFIX}/Misc.cpp
   ${PATH_PREFIX}/Monk.cpp
   ${PATH_PREFIX}/Paladin.cpp
   ${PATH_PREFIX}/Pets.cpp
   ${PATH_PREFIX}/Priest.cpp
   ${PATH_PREFIX}/Quests.cpp
   ${PATH_PREFIX}/Rogue.cpp
   ${PATH_PREFIX}/Shaman.cpp
   ${PATH_PREFIX}/Warlock.cpp
   ${PATH_PREFIX}/Warrior.cpp
)

source_group(Scripts FILES ${SRC_SPELL_SCRIPTS_FILES})
unset(PATH_PREFIX)
