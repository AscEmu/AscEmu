# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(Battlegrounds)

set(sources
    AlteracValley.cpp
    ArathiBasin.cpp
    CircleOfBlood.cpp
    DalaranSewers.cpp
    EyeOfTheStorm.cpp
    IsleOfConquest.cpp
    RingOfTrials.cpp
    RingOfValor.cpp
    RuinsOfLordaeron.cpp
    StrandOfTheAncient.cpp
    WarsongGulch.cpp
    Setup.cpp
)

set(headers
    AlteracValley.h
    ArathiBasin.h
    CircleOfBlood.h
    DalaranSewers.h
    EyeOfTheStorm.h
    IsleOfConquest.h
    RingOfTrials.h
    RingOfValor.h
    RuinsOfLordaeron.h
    StrandOfTheAncient.h
    WarsongGulch.h
    Setup.h
)

include_directories(
   ${OPENSSL_INCLUDE_DIR}
   ${PCRE_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
   ${CMAKE_SOURCE_DIR}/dep/vmaplib/include
   ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
   ${CMAKE_SOURCE_DIR}/src/shared
   ${CMAKE_SOURCE_DIR}/src/world
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${ZLIB_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} SHARED ${sources} ${headers})

# special case for project linking
# on some operating systems linking is a bit strange
# OS X can't link executable, MSVC can't link world library... weird
if(WIN32)
    add_dependencies(${PROJECT_NAME} worldserver)
    target_link_libraries(${PROJECT_NAME} worldserver)
elseif(APPLE)
    add_dependencies(${PROJECT_NAME} world)
    target_link_libraries(${PROJECT_NAME} world)
endif()

if(USE_PCH AND PCHSupport_FOUND)
    ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h ${CMAKE_CURRENT_SOURCE_DIR}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
unset(sources)
unset(headers)
