# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Utilities)

set(SRC_UTILITIES_FILES
    ${PATH_PREFIX}/Strings.cpp
    ${PATH_PREFIX}/Strings.hpp
    ${PATH_PREFIX}/Util.cpp
    ${PATH_PREFIX}/Util.hpp
)

source_group(Utilities FILES ${SRC_UTILITIES_FILES})
unset(PATH_PREFIX)
