# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX WarsongGulch)

set(SRC_WarsongGulch
    ${PATH_PREFIX}/WarsongGulch.cpp
    ${PATH_PREFIX}/WarsongGulch.h
    ${PATH_PREFIX}/WarsongGulchDefinitions.h
)

source_group(WarsongGulch FILES ${SRC_WarsongGulch})
unset(PATH_PREFIX)
