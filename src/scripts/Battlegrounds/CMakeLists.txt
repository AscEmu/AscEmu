# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME Battlegrounds)

file(GLOB SRC_ALTERAC_VALLEY AlteracValley/*.cpp AlteracValley/*.h)
file(GLOB SRC_ARATHI_BASIN ArathiBasin/*.cpp ArathiBasin/*.h)
file(GLOB SRC_CIRCLE_OF_BLOOD CircleOfBlood/*.cpp CircleOfBlood/*.h)
file(GLOB SRC_DALARAN_SEWERS DalaranSewers/*.cpp DalaranSewers/*.h)
file(GLOB SRC_EYE_OF_THE_STORM EyeOfTheStorm/*.cpp EyeOfTheStorm/*.h)
file(GLOB SRC_ISLE_OF_CONQUEST IsleOfConquest/*.cpp IsleOfConquest/*.h)
file(GLOB SRC_RING_OF_TRIALS RingOfTrials/*.cpp RingOfTrials/*.h)
file(GLOB SRC_RING_OF_VALOR RingOfValor/*.cpp RingOfValor/*.h)
file(GLOB SRC_RUINS_OF_LORDAERON RuinsOfLordaeron/*.cpp RuinsOfLordaeron/*.h)
file(GLOB SRC_STRAND_OF_THE_ANCIENT StrandOfTheAncient/*.cpp StrandOfTheAncient/*.h)
file(GLOB SRC_WARSONG_GULCH WarsongGulch/*.cpp WarsongGulch/*.h)
file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

# required for linker.
set(SRC_BATTLEGROUNDS
    ../../world/Server/ServerState.cpp
    ../../world/Spell/SpellCastTargets.cpp
)

set(BATTLEGROUNDS_SRCS
    ${SRC_ALTERAC_VALLEY}
    ${SRC_ARATHI_BASIN}
    ${SRC_CIRCLE_OF_BLOOD}
    ${SRC_DALARAN_SEWERS}
    ${SRC_EYE_OF_THE_STORM}
    ${SRC_ISLE_OF_CONQUEST}
    ${SRC_RING_OF_TRIALS}
    ${SRC_RING_OF_VALOR}
    ${SRC_RUINS_OF_LORDAERON}
    ${SRC_STRAND_OF_THE_ANCIENT}
    ${SRC_WARSONG_GULCH}
    ${SRC_BATTLEGROUNDS}
    ${SRC_LOCAL_DIR}
)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} SHARED ${BATTLEGROUNDS_SRCS})

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
)

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    world
)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${LIBRARY_NAME}")
    target_precompile_headers(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchBattleGrounds.hpp)
endif ()

install(TARGETS ${LIBRARY_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

unset(BATTLEGROUNDS_SRCS)
