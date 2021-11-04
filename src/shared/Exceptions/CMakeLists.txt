# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Exceptions)

set(SRC_EXCEPTIONS_FILES
    ${PATH_PREFIX}/Exceptions.hpp
    ${PATH_PREFIX}/PlayerExceptions.hpp
)

source_group(Exceptions FILES ${SRC_EXCEPTIONS_FILES})
unset(PATH_PREFIX)
