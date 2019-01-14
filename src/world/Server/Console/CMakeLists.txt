# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server/Console)

set(SRC_SERVER_CONSOLE_FILES
   ${PATH_PREFIX}/BaseConsole.h
   ${PATH_PREFIX}/ConsoleAuthMgr.cpp
   ${PATH_PREFIX}/ConsoleAuthMgr.h
   ${PATH_PREFIX}/ConsoleCommands.cpp
   ${PATH_PREFIX}/ConsoleCommands.h
   ${PATH_PREFIX}/ConsoleListener.cpp
   ${PATH_PREFIX}/ConsoleSocket.cpp
   ${PATH_PREFIX}/ConsoleSocket.h
   ${PATH_PREFIX}/ConsoleThread.cpp
   ${PATH_PREFIX}/ConsoleThread.h
)

source_group(Server\\Console FILES ${SRC_SERVER_CONSOLE_FILES})
unset(PATH_PREFIX)
