# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Debugging)

set(SRC_DEBUGGING_FILES
    ${PATH_PREFIX}/CrashHandler.cpp
    ${PATH_PREFIX}/CrashHandler.h
)

source_group(Debugging FILES ${SRC_DEBUGGING_FILES})
unset(PATH_PREFIX)
