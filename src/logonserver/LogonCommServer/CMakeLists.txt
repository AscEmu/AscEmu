# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX LogonCommServer)

set(SRC_LOGON_COMM_SERVER_FILES
   ${PATH_PREFIX}/LogonCommServer.cpp
   ${PATH_PREFIX}/LogonCommServer.h
)

source_group(LogonCommServer FILES ${SRC_LOGON_COMM_SERVER_FILES})
unset(PATH_PREFIX)
