# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Spell/Definitions)

set(SRC_SPELL_DEFINITIONS_FILES
    ${PATH_PREFIX}/AuraInterruptFlags.h
    ${PATH_PREFIX}/CastInterruptFlags.h
    ${PATH_PREFIX}/ChannelInterruptFlags.h
    ${PATH_PREFIX}/ProcFlags.h
    ${PATH_PREFIX}/School.h
    ${PATH_PREFIX}/SpellCastTargetFlags.h
    ${PATH_PREFIX}/SpellClickSpell.h
    ${PATH_PREFIX}/SpellCustomFlags.h
    ${PATH_PREFIX}/SpellDamageType.h
    ${PATH_PREFIX}/SpellGoFlags.h
    ${PATH_PREFIX}/SpellInFrontStatus.h
    ${PATH_PREFIX}/SpellLog.h
    ${PATH_PREFIX}/SpellModifierType.h
    ${PATH_PREFIX}/SpellSchoolConversionTable.h
    ${PATH_PREFIX}/SpellTargetType.h
)

source_group(Spell\\Definitions FILES ${SRC_SPELL_DEFINITIONS_FILES})
unset(PATH_PREFIX)
