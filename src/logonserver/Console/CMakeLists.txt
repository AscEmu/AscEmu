# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Console)

set(SRC_CONSOLE_FILES
   ${PATH_PREFIX}/LogonConsole.cpp
   ${PATH_PREFIX}/LogonConsole.h
)

source_group(Console FILES ${SRC_CONSOLE_FILES})
unset(PATH_PREFIX)
