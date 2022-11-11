# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Tbc/HellfireCitadel/HellfireRamparts)

set(SRC_TBC_HELLFIRERAMPARTS_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_HellfireRamparts.cpp
    ${PATH_PREFIX}/Instance_HellfireRamparts.hpp
    ${PATH_PREFIX}/NazanAndVazruden.cpp
    ${PATH_PREFIX}/NazanAndVazruden.hpp
    ${PATH_PREFIX}/Omor.cpp
    ${PATH_PREFIX}/Omor.hpp
    ${PATH_PREFIX}/WatchkeeperGargolmar.cpp
    ${PATH_PREFIX}/WatchkeeperGargolmar.hpp
)

source_group(Tbc/HellfireCitadel/HellfireRamparts FILES ${SRC_TBC_HELLFIRERAMPARTS_SCRIPTS_FILES})
unset(PATH_PREFIX)
