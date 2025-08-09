# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME Battlegrounds)

file(GLOB_RECURSE SRC_LOCAL_DIR *.cpp *.h *.hpp)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

# add shared library.
add_library(${LIBRARY_NAME} STATIC ${SRC_LOCAL_DIR})

# include directories.
set(BATTLEGROUNDS_INCLUDES
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
    ${CMAKE_SOURCE_DIR}/src/collision
    ${CMAKE_SOURCE_DIR}/src/collision/Management
    ${CMAKE_SOURCE_DIR}/src/collision/Maps
    ${CMAKE_SOURCE_DIR}/src/collision/Models
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
    ${CMAKE_SOURCE_DIR}/dep/lualib/src
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/src/world
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)
target_include_directories(${LIBRARY_NAME} PUBLIC ${BATTLEGROUNDS_INCLUDES})

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PRIVATE
    collision
    shared
    world
)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${LIBRARY_NAME}")
    target_precompile_headers(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchBattleGrounds.hpp)
endif ()

# install library.
install(TARGETS ${LIBRARY_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

