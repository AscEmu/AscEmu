# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX LogonCommClient)

set(SRC_LOGON_COMMUNICATION_CLIENT_FILES
   ${PATH_PREFIX}/LogonCommClient.cpp
   ${PATH_PREFIX}/LogonCommClient.h
   ${PATH_PREFIX}/LogonCommHandler.cpp
   ${PATH_PREFIX}/LogonCommHandler.h
)

source_group(LogonCommClient FILES ${SRC_LOGON_COMMUNICATION_CLIENT_FILES})
unset(PATH_PREFIX)
