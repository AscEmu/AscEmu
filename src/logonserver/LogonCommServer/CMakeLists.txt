# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX LogonCommServer)

set(SRC_LOGON_COMM_SERVER_FILES
   ${PATH_PREFIX}/LogonCommServer.cpp
   ${PATH_PREFIX}/LogonCommServer.h
   ${PATH_PREFIX}/LogonRealmOpcodes.hpp
)

source_group(LogonCommServer FILES ${SRC_LOGON_COMM_SERVER_FILES})
unset(PATH_PREFIX)
