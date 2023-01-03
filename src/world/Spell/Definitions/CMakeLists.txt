# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Spell/Definitions)

set(SRC_SPELL_DEFINITIONS_FILES
    ${PATH_PREFIX}/AuraEffects.hpp
    ${PATH_PREFIX}/AuraInterruptFlags.hpp
    ${PATH_PREFIX}/AuraRemoveMode.hpp
    ${PATH_PREFIX}/AuraSlots.hpp
    ${PATH_PREFIX}/AuraStates.hpp
    ${PATH_PREFIX}/CastInterruptFlags.hpp
    ${PATH_PREFIX}/ChannelInterruptFlags.hpp
    ${PATH_PREFIX}/DiminishingGroup.hpp
    ${PATH_PREFIX}/DispelType.hpp
    ${PATH_PREFIX}/LockTypes.hpp
    ${PATH_PREFIX}/PowerType.hpp
    ${PATH_PREFIX}/PreventionType.hpp
    ${PATH_PREFIX}/ProcFlags.hpp
    ${PATH_PREFIX}/School.hpp
    ${PATH_PREFIX}/Spec.hpp
    ${PATH_PREFIX}/SpellCastTargetFlags.hpp
    ${PATH_PREFIX}/SpellDamageType.hpp
    ${PATH_PREFIX}/SpellDidHitResult.hpp
    ${PATH_PREFIX}/SpellEffects.hpp
    ${PATH_PREFIX}/SpellEffectTarget.hpp
    ${PATH_PREFIX}/SpellFailure.hpp
    ${PATH_PREFIX}/SpellFamily.hpp
    ${PATH_PREFIX}/SpellInFrontStatus.hpp
    ${PATH_PREFIX}/SpellIsFlags.hpp
    ${PATH_PREFIX}/SpellLog.hpp
    ${PATH_PREFIX}/SpellMechanics.hpp
    ${PATH_PREFIX}/SpellModifierType.hpp
    ${PATH_PREFIX}/SpellPacketFlags.hpp
    ${PATH_PREFIX}/SpellRanged.hpp
    ${PATH_PREFIX}/SpellSchoolConversionTable.hpp
    ${PATH_PREFIX}/SpellState.hpp
    ${PATH_PREFIX}/SpellTargetMod.hpp
    ${PATH_PREFIX}/SpellTargetType.hpp
    ${PATH_PREFIX}/SpellTypes.hpp
    ${PATH_PREFIX}/SummonControlTypes.hpp
    ${PATH_PREFIX}/SummonTypes.hpp
    ${PATH_PREFIX}/TeleportCoords.hpp
    ${PATH_PREFIX}/TeleportEffectCustomFlags.hpp
)

source_group(Spell\\Definitions FILES ${SRC_SPELL_DEFINITIONS_FILES})
unset(PATH_PREFIX)
