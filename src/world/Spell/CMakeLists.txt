# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Spell)

set(SRC_SPELL_FILES
   ${PATH_PREFIX}/Spell.cpp
   ${PATH_PREFIX}/Spell.h
   ${PATH_PREFIX}/Spell_ClassScripts.cpp
   ${PATH_PREFIX}/SpellDefines.hpp
   ${PATH_PREFIX}/SpellAuras.cpp
   ${PATH_PREFIX}/SpellAuras.h
   ${PATH_PREFIX}/SpellEffects.cpp
   ${PATH_PREFIX}/SpellFailure.h
   ${PATH_PREFIX}/SpellHandler.cpp
   ${PATH_PREFIX}/SpellMgr.cpp
   ${PATH_PREFIX}/SpellMgr.h
   ${PATH_PREFIX}/SpellNameHashes.h
   ${PATH_PREFIX}/SpellProc.cpp
   ${PATH_PREFIX}/SpellProc.h
   ${PATH_PREFIX}/SpellProc_ClassScripts.cpp
   ${PATH_PREFIX}/SpellProc_Items.cpp
   ${PATH_PREFIX}/SpellTarget.cpp
   ${PATH_PREFIX}/SpellTarget.h 
)

source_group(Spell FILES ${SRC_SPELL_FILES})
unset(PATH_PREFIX)
