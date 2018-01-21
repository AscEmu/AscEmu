# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server)

set(SRC_SERVER_FILES
   ${PATH_PREFIX}/LogonServerDefines.hpp
   ${PATH_PREFIX}/Main.cpp
   ${PATH_PREFIX}/Master.cpp
   ${PATH_PREFIX}/Master.hpp
   ${PATH_PREFIX}/PeriodicFunctionCall_Thread.h
)

source_group(Server FILES ${SRC_SERVER_FILES})
unset(PATH_PREFIX)
