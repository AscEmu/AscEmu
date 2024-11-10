# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

if (NOT BUILD_LUAENGINE)
    message("\tLuaEngine was skipped because BUILD_LUAENGINE is unchecked.")
    return()
endif ()

# set up our project name.
set(EXECUTABLE_NAME LuaEngine)

file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

# required for linker.
set(SRC_LUAENGINE
    ../../world/Server/ServerState.cpp
    ../../world/Spell/SpellCastTargets.cpp
    ../../shared/Utilities/Util.cpp
)

set(LUAENGINE_SRCS
    ${SRC_LUAENGINE}
    ${SRC_LOCAL_DIR}
)

include_directories(
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
    ${CMAKE_SOURCE_DIR}/src/collision
    ${CMAKE_SOURCE_DIR}/src/collision/Management
    ${CMAKE_SOURCE_DIR}/src/collision/Maps
    ${CMAKE_SOURCE_DIR}/src/collision/Models
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/src/world
    ${CMAKE_SOURCE_DIR}/dep/lualib/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/dep/utf8cpp
)

add_library(${EXECUTABLE_NAME} SHARED ${LUAENGINE_SRCS})

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    world
    lualib
)

# generate precompiled header.
if (USE_PCH)
    message(STATUS "Generate PCH for ${EXECUTABLE_NAME}")
    target_precompile_headers(${EXECUTABLE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchLuaEngine.hpp)
endif ()

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

unset(LUAENGINE_SRCS)
