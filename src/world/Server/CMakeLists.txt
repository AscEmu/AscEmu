# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server)

set(SRC_SERVER_FILES
   ${PATH_PREFIX}/CharacterErrors.h
   ${PATH_PREFIX}/BroadcastMgr.cpp
   ${PATH_PREFIX}/BroadcastMgr.h
   ${PATH_PREFIX}/Definitions.h
   ${PATH_PREFIX}/EventableObject.cpp
   ${PATH_PREFIX}/EventableObject.h
   ${PATH_PREFIX}/EventMgr.cpp
   ${PATH_PREFIX}/EventMgr.h
   ${PATH_PREFIX}/IUpdatable.h
   ${PATH_PREFIX}/UpdateFieldInclude.h
   ${PATH_PREFIX}/UpdateMask.h
   ${PATH_PREFIX}/Main.cpp
   ${PATH_PREFIX}/MainServerDefines.h
   ${PATH_PREFIX}/Master.cpp
   ${PATH_PREFIX}/Master.h
   ${PATH_PREFIX}/ServerState.cpp
   ${PATH_PREFIX}/ServerState.h
   ${PATH_PREFIX}/World.cpp
   ${PATH_PREFIX}/World.h
   ${PATH_PREFIX}/World.Legacy.cpp
   ${PATH_PREFIX}/World.Legacy.h
   ${PATH_PREFIX}/WorldConfig.cpp
   ${PATH_PREFIX}/WorldConfig.h
   ${PATH_PREFIX}/WorldRunnable.cpp
   ${PATH_PREFIX}/WorldRunnable.h
   ${PATH_PREFIX}/WorldSession.cpp
   ${PATH_PREFIX}/WorldSession.h
   ${PATH_PREFIX}/WorldSocket.cpp
   ${PATH_PREFIX}/WorldSocket.h
)

source_group(Server FILES ${SRC_SERVER_FILES})
unset(PATH_PREFIX)
