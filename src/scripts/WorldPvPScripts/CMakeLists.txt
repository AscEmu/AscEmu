# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

if (NOT BUILD_WORLD_PVP_SCRIPTS)
    message("\tWorldPvPScripts was skipped because BUILD_WORLD_PVP_SCRIPTS is unchecked.")
    return()
endif ()

# set up our project name.
set(LIBRARY_NAME WorldPvPScripts)

file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

# required for linker.
set(SRC_BUILD_WORLD_PVP_SCRIPTS
    ../../world/Server/ServerState.cpp
)

set(BUILD_WORLD_PVP_SCRIPTS_SRCS
    ${SRC_BUILD_WORLD_PVP_SCRIPTS}
    ${SRC_LOCAL_DIR}
)

add_library(${LIBRARY_NAME} SHARED ${BUILD_WORLD_PVP_SCRIPTS_SRCS})

target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    world
)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${LIBRARY_NAME}")
    target_precompile_headers(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchWorldPvPScripts.hpp)
endif ()

install(TARGETS ${LIBRARY_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

unset(BUILD_WORLD_PVP_SCRIPTS_SRCS)
