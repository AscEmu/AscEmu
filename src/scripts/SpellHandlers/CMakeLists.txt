# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(SpellHandlers CXX)

include(Scripts/CMakeLists.txt)
include(LegacyFiles/CMakeLists.txt)

# sources - contains source files for our library.
set(sources
    ${SRC_SPELL_SCRIPTS_FILES}
    ${SRC_SPELL_SCRIPTS_LEGACY_FILES}

    Setup.cpp
    # required for linker
    ../../world/Server/ServerState.cpp
    ../../world/Spell/SpellCastTargets.cpp
    ../../shared/Utilities/Util.cpp
)

set(headers
    Setup.h
    pchSpellHandlers.hpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/dep/utf8cpp
)
add_library(${PROJECT_NAME} SHARED ${sources} ${headers})

add_definitions(-DSCRIPTLIB -DSKIP_ALLOCATOR_SHARING)

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    world
)

# generate precompiled header.
if (USE_PCH)
    message(STATUS "Generate PCH for ${PROJECT_NAME}")
    target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchSpellHandlers.hpp)
endif ()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH} )

unset(sources)
unset(headers)
