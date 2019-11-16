# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(Battlegrounds CXX)

include(AlteracValley/CMakeLists.txt)
include(ArathiBasin/CMakeLists.txt)
include(CircleOfBlood/CMakeLists.txt)
include(DalaranSewers/CMakeLists.txt)
include(EyeOfTheStorm/CMakeLists.txt)
include(IsleOfConquest/CMakeLists.txt)
include(RingOfTrials/CMakeLists.txt)
include(RingOfValor/CMakeLists.txt)
include(RuinsOfLordaeron/CMakeLists.txt)
include(StrandOfTheAncient/CMakeLists.txt)
include(WarsongGulch/CMakeLists.txt)

set(SRC_BATTLEGROUNDS
    Setup.cpp
    Setup.h
    # Required for linker
    ../../world/Server/ServerState.cpp
    ../../world/Spell/SpellCastTargets.cpp
)

source_group(Battlegrounds FILES ${SRC_BATTLEGROUNDS})

set(SRC_BGS
    ${SRC_AlteracValley}
    ${SRC_ArathiBasin}
    ${SRC_CircleOfBlood}
    ${SRC_DalaranSewers}
    ${SRC_EyeOfTheStorm}
    ${SRC_IsleOfConquest}
    ${SRC_RingOfTrials}
    ${SRC_RingOfValor}
    ${SRC_RuinsOfLordaeron}
    ${SRC_StrandOfTheAncient}
    ${SRC_WarsongGulch}
    ${SRC_BATTLEGROUNDS}
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
)

add_library(${PROJECT_NAME} SHARED ${SRC_BGS})

target_link_libraries(${PROJECT_NAME} world)

if(USE_PCH)
    gen_cxx_pch(${PROJECT_NAME} Setup.h ${CMAKE_CURRENT_SOURCE_DIR}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
unset(sources)
unset(headers)
