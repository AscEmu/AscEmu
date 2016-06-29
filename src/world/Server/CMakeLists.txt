# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server)

set(SRC_SERVER_FILES
   ${PATH_PREFIX}/Main.cpp
   ${PATH_PREFIX}/MainServerDefines.h
   ${PATH_PREFIX}/Master.cpp
   ${PATH_PREFIX}/Master.h
   ${PATH_PREFIX}/World.cpp
   ${PATH_PREFIX}/World.h
   ${PATH_PREFIX}/WorldRunnable.cpp
   ${PATH_PREFIX}/WorldRunnable.h
   ${PATH_PREFIX}/WorldSession.cpp
   ${PATH_PREFIX}/WorldSession.h
   ${PATH_PREFIX}/WorldSocket.cpp
   ${PATH_PREFIX}/WorldSocket.h

)

source_group(Server FILES ${SRC_SERVER_FILES})
unset(PATH_PREFIX)
