# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME SpellHandlers)

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

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} SHARED ${sources} ${headers})

target_include_directories(${LIBRARY_NAME} PUBLIC 
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

add_compile_options(-DSCRIPTLIB -DSKIP_ALLOCATOR_SHARING)

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    world
)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${LIBRARY_NAME}")
    target_precompile_headers(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchSpellHandlers.hpp)
endif ()

install(TARGETS ${LIBRARY_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

unset(sources)
unset(headers)
