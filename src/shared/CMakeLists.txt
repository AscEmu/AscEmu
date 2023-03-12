# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(shared CXX)

include(Auth/CMakeLists.txt)
include(Config/CMakeLists.txt)
include(Database/CMakeLists.txt)
include(Debugging/CMakeLists.txt)
include(Exceptions/CMakeLists.txt)
include(Logging/CMakeLists.txt)
include(Network/CMakeLists.txt)
include(Threading/CMakeLists.txt)
include(Utilities/CMakeLists.txt)

set(sources
    ${SRC_AUTH_FILES}
    ${SRC_CONFIG_FILES}
    ${SRC_DATABASE_FILES}
    ${SRC_DEBUGGING_FILES}
    ${SRC_EXCEPTIONS_FILES}
    ${SRC_LOGGING_FILES}
    ${SRC_NETWORK_FILES}
    ${SRC_THREADING_FILES}
    ${SRC_UTILITIES_FILES}

    crc32.cpp
    CThreads.cpp
    DynLib.cpp
    LocationVector.cpp
    PerformanceCounter.cpp
    SysInfo.cpp
)

set(headers
    AuthCodes.h
    ByteBuffer.h
    ByteConverter.h
    CircularQueue.h
    Common.Legacy.h
    CommonDefines.hpp
    CommonHelpers.hpp
    CommonTypes.hpp
    crc32.h
    CThreads.h
    DynLib.hpp
    FastQueue.h
    LocationVector.h
    LogonCommDefines.h
    PerformanceCounter.hpp
    PreallocatedQueue.h
    RC4Engine.h
    SysInfo.hpp
    WorldPacket.h
    WoWGuid.h
    Common.hpp
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
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AEVersion.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/AEVersion.hpp)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} c++)
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
    target_link_libraries(${PROJECT_NAME} c++experimental)
endif ()

# generate precompiled header.
if (USE_PCH)
    message(STATUS "Generate PCH for ${PROJECT_NAME}")
    target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchShared.hpp)
endif ()

unset(sources)
unset(headers)
