# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(Battlegrounds)

set(prefix ${ROOT_PATH}/src/scripts/${PROJECT_NAME})

#sources -contains source files for our library.
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
 
foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)
foreach(src IN ITEMS ${headers})
   set(HEADERS ${HEADERS} ${prefix}/${src})
endforeach(src)

include_directories(${GLOBAL_INCLUDE_DIRS})
add_library(${PROJECT_NAME} SHARED ${SRCS} ${HEADERS})
add_dependencies(${PROJECT_NAME} ${script_dependencies})
add_definitions(-DSCRIPTLIB -DSKIP_ALLOCATOR_SHARING)
target_link_libraries(${PROJECT_NAME} ${script_linkages})

include(PCHSupport)
if(CMAKE_GENERATOR MATCHES Visual*)
   ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h ${prefix}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
