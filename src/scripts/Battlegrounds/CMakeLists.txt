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
    Setup.cpp
    StrandOfTheAncient.cpp
    WarsongGulch.cpp
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
    Setup.h
    StrandOfTheAncient.h
    WarsongGulch.h
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

add_library(${PROJECT_NAME} SHARED ${sources} ${headers})
#add_dependencies(${PROJECT_NAME} ${script_dependencies})
add_definitions(-DSCRIPTLIB -DSKIP_ALLOCATOR_SHARING)
target_link_libraries(${PROJECT_NAME} world)

if(MSVC)
    include(PCHSupport)
    ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
unset(sources)
unset(headers)