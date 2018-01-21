# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameCata/Spell)

set(SRC_GAMECATA_SPELL_FILES
  ${PATH_PREFIX}/Spell.cpp
  ${PATH_PREFIX}/Spell.h
)

source_group(GameCata\\Spell FILES ${SRC_GAMECATA_SPELL_FILES})
unset(PATH_PREFIX)
