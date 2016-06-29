# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server/Console)

set(SRC_SERVER_CONSOLE_FILES
   ${PATH_PREFIX}/BaseConsole.h
   ${PATH_PREFIX}/CConsole.cpp
   ${PATH_PREFIX}/CConsole.h
   ${PATH_PREFIX}/ConsoleCommands.cpp
   ${PATH_PREFIX}/ConsoleCommands.h
   ${PATH_PREFIX}/ConsoleListener.cpp
)

source_group(Server\\Console FILES ${SRC_SERVER_CONSOLE_FILES})
unset(PATH_PREFIX)
