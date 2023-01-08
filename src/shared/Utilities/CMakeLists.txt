# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Utilities)

set(SRC_UTIL_FILES
    ${PATH_PREFIX}/Strings.cpp
    ${PATH_PREFIX}/Strings.hpp
)

source_group(Utilities FILES ${SRC_UTIL_FILES})
unset(PATH_PREFIX)
