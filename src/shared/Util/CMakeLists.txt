# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Util)

set(SRC_UTIL_FILES
    ${PATH_PREFIX}/Strings.cpp
    ${PATH_PREFIX}/Strings.hpp
)

source_group(Util FILES ${SRC_UTIL_FILES})
unset(PATH_PREFIX)
