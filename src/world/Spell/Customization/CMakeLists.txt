# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Spell/Customization)

set(SRC_SPELL_CUSTOM_FILES
   ${PATH_PREFIX}/HackFixes.cpp
   ${PATH_PREFIX}/SpellCustomizations.cpp
)

source_group(Spell\\Customization FILES ${SRC_SPELL_CUSTOM_FILES})
unset(PATH_PREFIX)
