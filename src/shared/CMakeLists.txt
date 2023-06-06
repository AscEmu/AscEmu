# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(shared CXX)

file(GLOB SRC_CONFIG_FILES Config/*.cpp Config/*.h Config/*.hpp)
file(GLOB SRC_CRYPTOGRAPHY_FILES Cryptography/*.cpp Cryptography/*.h Cryptography/*.hpp)
file(GLOB SRC_DATABASE_FILES Database/*.cpp Database/*.h Database/*.hpp)
file(GLOB SRC_DEBUGGING_FILES Debugging/*.cpp Debugging/*.h Debugging/*.hpp)
file(GLOB SRC_EXCEPTIONS_FILES Exceptions/*.cpp Exceptions/*.h Exceptions/*.hpp)
file(GLOB SRC_LOGGING_FILES Logging/*.cpp Logging/*.h Logging/*.hpp)

include(Network/CMakeLists.txt) # SCH: coming soon

file(GLOB SRC_THREADING_FILES Threading/*.cpp Threading/*.h Threading/*.hpp)
file(GLOB SRC_UTILITIES_FILES Utilities/*.cpp Utilities/*.h Utilities/*.hpp)
file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

set(SHARED_SRCS
    ${SRC_CONFIG_FILES}
    ${SRC_CRYPTOGRAPHY_FILES}
    ${SRC_DATABASE_FILES}
    ${SRC_DEBUGGING_FILES}
    ${SRC_EXCEPTIONS_FILES}
    ${SRC_LOGGING_FILES}
    ${SRC_NETWORK_FILES}
    ${SRC_THREADING_FILES}
    ${SRC_UTILITIES_FILES}
    ${SRC_LOCAL_DIR}
)

include_directories(
    ${OPENSSL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/utf8cpp
)

# source group.
source_group(Config FILES ${SRC_CONFIG_FILES})
source_group(Cryptography FILES ${SRC_CRYPTOGRAPHY_FILES})
source_group(Database FILES ${SRC_DATABASE_FILES})
source_group(Debugging FILES ${SRC_DEBUGGING_FILES})
source_group(Exceptions FILES ${SRC_EXCEPTIONS_FILES})
source_group(Logging FILES ${SRC_LOGGING_FILES})
source_group(Network FILES ${SRC_NETWORK_FILES})
source_group(Threading FILES ${SRC_THREADING_FILES})
source_group(Utilities FILES ${SRC_UTILITIES_FILES})

add_library(${PROJECT_NAME} STATIC ${SHARED_SRCS})

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
    ${MYSQL_LIBRARY}
    ${OPENSSL_LIBRARIES}
    ${EXTRA_LIBS}
)

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

unset(SHARED_SRCS)
