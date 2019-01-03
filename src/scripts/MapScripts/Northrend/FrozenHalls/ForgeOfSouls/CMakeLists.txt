# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Northrend/FrozenHalls/ForgeOfSouls)

set(SRC_NORTHREND_FH_FORGEOFSOULS_FILES
   ${PATH_PREFIX}/ForgeOfSouls.cpp
   ${PATH_PREFIX}/ForgeOfSouls.h
)

source_group(Northrend\\FrozenHalls\\ForgeOfSouls FILES ${SRC_NORTHREND_FH_FORGEOFSOULS_FILES})
unset(PATH_PREFIX)
