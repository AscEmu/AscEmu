# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Network)

set(SRC_NETWORK_FILES
    ${PATH_PREFIX}/CircularBuffer.cpp
    ${PATH_PREFIX}/CircularBuffer.h
    ${PATH_PREFIX}/ListenSocketFreeBSD.h
    ${PATH_PREFIX}/ListenSocketLinux.h
    ${PATH_PREFIX}/ListenSocketWin32.h
    ${PATH_PREFIX}/Network.h
    ${PATH_PREFIX}/NetworkIncludes.hpp
    ${PATH_PREFIX}/Socket.cpp
    ${PATH_PREFIX}/Socket.h
    ${PATH_PREFIX}/SocketDefines.h
    ${PATH_PREFIX}/SocketMgrFreeBSD.h
    ${PATH_PREFIX}/SocketMgrLinux.h
    ${PATH_PREFIX}/SocketMgrWin32.h
    ${PATH_PREFIX}/SocketOps.h
)

if (WIN32)
    list(APPEND SRC_NETWORK_FILES ${sources} ${PATH_PREFIX}/SocketMgrWin32.cpp ${PATH_PREFIX}/SocketOpsWin32.cpp ${PATH_PREFIX}/SocketWin32.cpp)
elseif ((CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD") OR APPLE)
    list(APPEND SRC_NETWORK_FILES ${sources} ${PATH_PREFIX}/SocketMgrFreeBSD.cpp ${PATH_PREFIX}/SocketFreeBSD.cpp ${PATH_PREFIX}/SocketOpsFreeBSD.cpp)
else ()
    list(APPEND SRC_NETWORK_FILES ${sources} ${PATH_PREFIX}/SocketMgrLinux.cpp ${PATH_PREFIX}/SocketLinux.cpp ${PATH_PREFIX}/SocketOpsLinux.cpp)
endif ()

source_group(Network FILES ${SRC_NETWORK_FILES})
unset(PATH_PREFIX)
