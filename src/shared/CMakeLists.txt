# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(shared CXX)

include(Auth/CMakeLists.txt)
include(Config/CMakeLists.txt)
include(Database/CMakeLists.txt)
include(Exceptions/CMakeLists.txt)
include(Logging/CMakeLists.txt)
include(Network/CMakeLists.txt)
include(Threading/CMakeLists.txt)
include(Util/CMakeLists.txt)

set(sources
    ${SRC_AUTH_FILES}
    ${SRC_CONFIG_FILES}
    ${SRC_DATABASE_FILES}
    ${SRC_EXCEPTIONS_FILES}
    ${SRC_LOGGING_FILES}
    ${SRC_NETWORK_FILES}
    ${SRC_THREADING_FILES}
    ${SRC_UTIL_FILES}

    CrashHandler.cpp
    crc32.cpp
    CThreads.cpp
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
    DynLib.hpp
    Errors.h
    FastQueue.h
    LocationVector.h
    LogonCommDefines.h
    PerformanceCounter.hpp
    PreallocatedQueue.h
    RC4Engine.h
    SysInfo.hpp
    TLSObject.h
    WorldPacket.h
    WoWGuid.h

    Common.hpp
    Log.hpp
    LogDefines.hpp
    Util.hpp
    FactoryHolder.h
    ObjectRegistry.h
)

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