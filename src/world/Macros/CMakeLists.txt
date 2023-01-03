# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Macros)

set(SRC_MACROS_FILES
    ${PATH_PREFIX}/AIInterfaceMacros.hpp
    ${PATH_PREFIX}/CorpseMacros.hpp
    ${PATH_PREFIX}/CreatureMacros.hpp
    ${PATH_PREFIX}/GuildMacros.hpp
    ${PATH_PREFIX}/ItemMacros.hpp
    ${PATH_PREFIX}/LFGMacros.hpp
    ${PATH_PREFIX}/PetMacros.hpp
    ${PATH_PREFIX}/PlayerMacros.hpp
    ${PATH_PREFIX}/ScriptMacros.hpp
    ${PATH_PREFIX}/LootMacros.hpp
    ${PATH_PREFIX}/MapsMacros.hpp
    ${PATH_PREFIX}/UnitMacros.hpp
)

source_group(Macros FILES ${SRC_MACROS_FILES})
unset(PATH_PREFIX)
