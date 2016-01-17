# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

project(SpellHandlers)

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
)

include_directories(
   ${OPENSSL_INCLUDE_DIR}
   ${PCRE_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
   ${CMAKE_SOURCE_DIR}/dep/vmaplib/include
   ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
   ${CMAKE_SOURCE_DIR}/src/shared
   ${CMAKE_SOURCE_DIR}/src/world
   ${ZLIB_INCLUDE_DIRS}
)
add_library(${PROJECT_NAME} SHARED ${sources})
#add_dependencies(${PROJECT_NAME} ${script_dependencies})
add_definitions(-DSCRIPTLIB -DSKIP_ALLOCATOR_SHARING)
target_link_libraries(${PROJECT_NAME} world)

IF(MSVC)
    include(PCHSupport)
	ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h Setup.cpp)
ENDIF()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH} )

unset(sources)