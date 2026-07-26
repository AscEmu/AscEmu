# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME shared)

file(GLOB SRC_CONFIG_FILES Config/*.cpp Config/*.hpp)
file(GLOB SRC_CRYPTOGRAPHY_FILES Cryptography/*.cpp Cryptography/*.hpp)
file(GLOB SRC_DATABASE_FILES Database/*.cpp Database/*.hpp)

file(GLOB SRC_DEBUGGING_FILES Debugging/*.cpp Debugging/*.hpp)
file(GLOB SRC_EXCEPTIONS_FILES Exceptions/*.cpp Exceptions/*.hpp)
file(GLOB SRC_LOGGING_FILES Logging/*.cpp Logging/*.hpp)

file(GLOB_RECURSE SRC_NETWORK_FILES Network/*.cpp Network/*.hpp)

file(GLOB SRC_REGISTRY_FILES Registry/*.cpp Registry/*.hpp)
file(GLOB SRC_THREADING_FILES Threading/*.cpp Threading/*.hpp)
file(GLOB SRC_UTILITIES_FILES Utilities/*.cpp Utilities/*.hpp)
file(GLOB SRC_PLATFORM_FILES Platform/*.cpp Platform/*.hpp)
file(GLOB SRC_LOCAL_DIR *.cpp *.hpp)

set(SHARED_SRCS
    ${SRC_CONFIG_FILES}
    ${SRC_CRYPTOGRAPHY_FILES}
    ${SRC_DATABASE_FILES}
    ${SRC_DEBUGGING_FILES}
    ${SRC_EXCEPTIONS_FILES}
    ${SRC_LOGGING_FILES}
    ${SRC_NETWORK_FILES}
    ${SRC_REGISTRY_FILES}
    ${SRC_THREADING_FILES}
    ${SRC_UTILITIES_FILES}
    ${SRC_PLATFORM_FILES}
    ${SRC_LOCAL_DIR}
)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} STATIC ${SHARED_SRCS})

# Select exactly one zlib target. Prefer AscEmu's in-tree static target and
# use CMake's imported target only as a fallback.
if (TARGET zlibstatic)
    set(ASCEMU_ZLIB_TARGET zlibstatic)
elseif (TARGET zlib)
    set(ASCEMU_ZLIB_TARGET zlib)
elseif (TARGET ZLIB::ZLIB)
    set(ASCEMU_ZLIB_TARGET ZLIB::ZLIB)
else ()
    message(FATAL_ERROR
        "No zlib CMake target was found. Add the bundled zlib target or call "
        "find_package(ZLIB REQUIRED) before adding the shared target."
    )
endif ()

message(STATUS "${LIBRARY_NAME}: using zlib target '${ASCEMU_ZLIB_TARGET}'")

target_include_directories(${LIBRARY_NAME}
    SYSTEM PUBLIC
        ${OPENSSL_INCLUDE_DIR}
        ${MYSQL_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/dep/utf8cpp
)

target_include_directories(${LIBRARY_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        fmt::fmt

    PRIVATE
        ${ASCEMU_ZLIB_TARGET}
        ${MYSQL_LIBRARY}
        ${OPENSSL_LIBRARIES}
        ${EXTRA_LIBS}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BuildInfo.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/BuildInfo.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AEVersion.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/AEVersion.hpp)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${LIBRARY_NAME}")
    target_precompile_headers(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchShared.hpp)
endif ()

#Enable this if you want to use clang-tidy for static code analysis in MS Visual Studio. You need to have clang-tidy installed and available in your PATH.
#enable_clang_tidy(shared)

unset(ASCEMU_ZLIB_TARGET)
unset(SHARED_SRCS)
