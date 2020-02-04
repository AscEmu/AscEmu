# Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX LegacyFiles)

set(SRC_SPELL_SCRIPTS_LEGACY_FILES
   ${PATH_PREFIX}/DeathKnightSpells.cpp
   ${PATH_PREFIX}/DruidSpells.cpp
   ${PATH_PREFIX}/HunterSpells.cpp
   ${PATH_PREFIX}/ItemSpells_1.cpp
   ${PATH_PREFIX}/MageSpells.cpp
   ${PATH_PREFIX}/PaladinSpells.cpp
   ${PATH_PREFIX}/PetAISpells.cpp
   ${PATH_PREFIX}/PriestSpells.cpp
   ${PATH_PREFIX}/QIspells.cpp
   ${PATH_PREFIX}/RogueSpells.cpp
   ${PATH_PREFIX}/ShamanSpells.cpp
   ${PATH_PREFIX}/WarlockSpells.cpp
   ${PATH_PREFIX}/WarriorSpells.cpp
   ${PATH_PREFIX}/MiscSpells.cpp
)

source_group(LegacyFiles FILES ${SRC_SPELL_SCRIPTS_LEGACY_FILES})
unset(PATH_PREFIX)
