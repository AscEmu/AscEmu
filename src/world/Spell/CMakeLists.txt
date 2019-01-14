# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Spell)

set(SRC_SPELL_FILES
   ${PATH_PREFIX}/Spell.cpp
   ${PATH_PREFIX}/Spell.h
   ${PATH_PREFIX}/Spell.Legacy.cpp
   ${PATH_PREFIX}/Spell.Legacy.h
   ${PATH_PREFIX}/Spell_ClassScripts.cpp
   ${PATH_PREFIX}/SpellAuras.Legacy.cpp
   ${PATH_PREFIX}/SpellAuras.h
   ${PATH_PREFIX}/SpellCastTargets.h
   ${PATH_PREFIX}/SpellCastTargets.cpp
   ${PATH_PREFIX}/SpellDefines.hpp
   ${PATH_PREFIX}/SpellEffects.Legacy.cpp
   ${PATH_PREFIX}/SpellHelpers.h
   ${PATH_PREFIX}/SpellInfo.cpp
   ${PATH_PREFIX}/SpellInfo.hpp
   ${PATH_PREFIX}/SpellMgr.cpp
   ${PATH_PREFIX}/SpellMgr.h
   ${PATH_PREFIX}/SpellProc.Legacy.cpp
   ${PATH_PREFIX}/SpellProc.h
   ${PATH_PREFIX}/SpellProc_ClassScripts.cpp
   ${PATH_PREFIX}/SpellProc_Items.cpp
   ${PATH_PREFIX}/SpellTarget.Legacy.cpp
   ${PATH_PREFIX}/SpellTarget.h
   ${PATH_PREFIX}/SpellTargetConstraint.cpp
   ${PATH_PREFIX}/SpellTargetConstraint.h
)

source_group(Spell FILES ${SRC_SPELL_FILES})
unset(PATH_PREFIX)
