# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

project(SpellHandlers CXX)

#sources -contains source files for our library.
set(sources
    DeathKnightSpells.cpp
    DruidSpells.cpp
    HunterSpells.cpp
    ItemSpells_1.cpp
    MageSpells.cpp
    PaladinSpells.cpp
    PetAISpells.cpp
    PriestSpells.cpp
    QIspells.cpp
    RogueSpells.cpp
    Setup.cpp
    ShamanSpells.cpp
    WarlockSpells.cpp
    WarriorSpells.cpp
    MiscSpells.cpp
    # Required for linker
    ../../world/Server/ServerState.cpp
    ../../world/Spell/SpellCastTargets.cpp
    ../../shared/Util.cpp
)

set(headers
    Setup.h
)

include_directories(
   ${OPENSSL_INCLUDE_DIR}
   ${PCRE_INCLUDE_DIR}
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

target_link_libraries(${PROJECT_NAME} world)

if(USE_PCH)
    gen_cxx_pch(${PROJECT_NAME} Setup.h ${CMAKE_CURRENT_SOURCE_DIR}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH} )

unset(sources)
unset(headers)
