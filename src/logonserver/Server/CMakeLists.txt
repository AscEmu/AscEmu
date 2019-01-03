# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server)

set(SRC_SERVER_FILES
   ${PATH_PREFIX}/AccountMgr.cpp
   ${PATH_PREFIX}/AccountMgr.h
   ${PATH_PREFIX}/IpBanMgr.cpp
   ${PATH_PREFIX}/IpBanMgr.h
   ${PATH_PREFIX}/Logon.cpp
   ${PATH_PREFIX}/Logon.h
   ${PATH_PREFIX}/LogonConfig.cpp
   ${PATH_PREFIX}/LogonConfig.h
   ${PATH_PREFIX}/LogonServerDefines.hpp
   ${PATH_PREFIX}/Main.cpp
   ${PATH_PREFIX}/Master.cpp
   ${PATH_PREFIX}/Master.hpp
   ${PATH_PREFIX}/PeriodicFunctionCall_Thread.h
   ${PATH_PREFIX}/RealmsMgr.cpp
   ${PATH_PREFIX}/RealmsMgr.h
)

source_group(Server FILES ${SRC_SERVER_FILES})
unset(PATH_PREFIX)
