# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Spell/Customization)

set(SRC_SPELL_CUSTOM_FILES
   ${PATH_PREFIX}/HackFixes.cpp
   ${PATH_PREFIX}/SpellCustomizations.cpp
   ${PATH_PREFIX}/SpellCustomizations.hpp
)

source_group(Spell\\Customization FILES ${SRC_SPELL_CUSTOM_FILES})
unset(PATH_PREFIX)
