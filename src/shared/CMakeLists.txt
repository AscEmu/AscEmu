# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(shared CXX)

set(sources
    Auth/BigNumber.cpp
    Auth/MD5.cpp
    Auth/Sha1.cpp
    Auth/WowCrypt.cpp

    Config/Config.cpp

    Database/CreateInterface.cpp
    Database/Database.cpp
    Database/MySQLDatabase.cpp

    Network/CircularBuffer.cpp
    Network/Socket.cpp

    Threading/ConditionVariable.cpp
    Threading/Mutex.cpp
    Threading/LegacyThreadPool.cpp

    Threading/AEThread.cpp
    Threading/AEThreadPool.cpp

    CrashHandler.cpp
    crc32.cpp
    CThreads.cpp
    Database/DatabaseUpdater.cpp
    DynLib.cpp
    LocationVector.cpp
    Log.cpp
    PerformanceCounter.cpp
    SysInfo.cpp
    Util.cpp
)

set(headers
    AscemuServerDefines.hpp
    AuthCodes.h
    ByteBuffer.h
    ByteConverter.h
    CallBack.h
    CircularQueue.h
    Common.Legacy.h
    CommonDefines.hpp
    CommonHelpers.hpp
    CommonTypes.hpp
    CrashHandler.h
    crc32.h
    CThreads.h
    Database/DatabaseUpdater.hpp
    DynLib.hpp
    Errors.h
    FastQueue.h
    LocationVector.h
    LogonCommDefines.h
    PerformanceCounter.hpp
    PreallocatedQueue.h
    Database/DatabaseCommon.hpp
    RC4Engine.h
    SysInfo.hpp
    TLSObject.h
    WorldPacket.h
    WoWGuid.h
    Auth/BigNumber.h
    Auth/MD5.h
    Auth/Sha1.h
    Auth/WowCrypt.hpp
    Config/Config.h
    Database/Database.h
    Database/Field.hpp
    Database/MySQLDatabase.h
    Exceptions/Exceptions.hpp
    Exceptions/PlayerExceptions.hpp
    Network/CircularBuffer.h
    Network/ListenSocketFreeBSD.h
    Network/ListenSocketLinux.h
    Network/ListenSocketWin32.h
    Network/Network.h
    Network/NetworkIncludes.hpp
    Network/Socket.h
    Network/SocketMgrFreeBSD.h
    Network/SocketMgrLinux.h
    Network/SocketMgrWin32.h
    Network/SocketOps.h
    Network/SocketDefines.h
    Threading/ConditionVariable.h
    Threading/LegacyThreading.h
    Threading/LegacyThreadPool.h
    Threading/LegacyThreadBase.h
    Threading/LockedQueue.h
    Threading/Mutex.h
    Threading/Queue.h

    Threading/AEThread.h
    Threading/AEThreadPool.h
    Threading/ThreadState.h
    
    Common.hpp
    Log.hpp
    LogDefines.hpp
    Util.hpp
)

if(WIN32)
   set(sources ${sources} Network/SocketMgrWin32.cpp Network/SocketOpsWin32.cpp Network/SocketWin32.cpp)
elseif((CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD") OR APPLE)
   set(sources ${sources} Network/SocketMgrFreeBSD.cpp Network/SocketFreeBSD.cpp Network/SocketOpsFreeBSD.cpp)
else()
   set(sources ${sources} Network/SocketMgrLinux.cpp Network/SocketLinux.cpp Network/SocketOpsLinux.cpp)
endif()

include_directories(
    ${OPENSSL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/utf8cpp
)

add_library(${PROJECT_NAME} STATIC ${sources} ${headers})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${MYSQL_LIBRARY} ${OPENSSL_LIBRARIES} ${EXTRA_LIBS})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Common.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/Common.hpp)

if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()

unset(sources)
unset(headers)