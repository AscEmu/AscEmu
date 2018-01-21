# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Spell/Definitions)

set(SRC_SPELL_DEFINITIONS_FILES
    ${PATH_PREFIX}/AuraInterruptFlags.h
    ${PATH_PREFIX}/CastInterruptFlags.h
    ${PATH_PREFIX}/ChannelInterruptFlags.h
    ${PATH_PREFIX}/DiminishingGroup.h
    ${PATH_PREFIX}/DispelType.h
    ${PATH_PREFIX}/LockTypes.h
    ${PATH_PREFIX}/PowerType.h
    ${PATH_PREFIX}/PreventionType.h
    ${PATH_PREFIX}/ProcFlags.h
    ${PATH_PREFIX}/School.h
    ${PATH_PREFIX}/Spec.h
    ${PATH_PREFIX}/SpellCastTargetFlags.h
    ${PATH_PREFIX}/SpellClickSpell.h
    ${PATH_PREFIX}/SpellCoefficientsFlags.h
    ${PATH_PREFIX}/SpellCustomFlags.h
    ${PATH_PREFIX}/SpellDamageType.h
    ${PATH_PREFIX}/SpellDidHitResult.h
    ${PATH_PREFIX}/SpellEffectTarget.h
    ${PATH_PREFIX}/SpellGoFlags.h
    ${PATH_PREFIX}/SpellInFrontStatus.h
    ${PATH_PREFIX}/SpellIsFlags.h
    ${PATH_PREFIX}/SpellLog.h
    ${PATH_PREFIX}/SpellMechanics.h
    ${PATH_PREFIX}/SpellModifierType.h
    ${PATH_PREFIX}/SpellRanged.h
    ${PATH_PREFIX}/SpellSchoolConversionTable.h
    ${PATH_PREFIX}/SpellState.h
    ${PATH_PREFIX}/SpellTargetMod.h
    ${PATH_PREFIX}/SpellTargetType.h
    ${PATH_PREFIX}/SpellTypes.h
    ${PATH_PREFIX}/SummonControlTypes.h
    ${PATH_PREFIX}/SummonTypes.h
    ${PATH_PREFIX}/TeleportCoords.h
    ${PATH_PREFIX}/TeleportEffectCustomFlags.h
)

source_group(Spell\\Definitions FILES ${SRC_SPELL_DEFINITIONS_FILES})
unset(PATH_PREFIX)
