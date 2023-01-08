# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Logging)

set(SRC_LOGGING_FILES
    ${PATH_PREFIX}/LoggerDefines.hpp
    ${PATH_PREFIX}/Log.hpp
    ${PATH_PREFIX}/Log.cpp
    ${PATH_PREFIX}/Logger.cpp
    ${PATH_PREFIX}/Logger.hpp
)

source_group(Logging FILES ${SRC_LOGGING_FILES})
unset(PATH_PREFIX)
