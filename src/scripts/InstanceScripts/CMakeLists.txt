# Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>

if(NOT BUILD_INSTANCESCRIPTS )
  message("\tInstanceScripts was skipped because BUILD_INSTANCESCRIPTS is unchecked.")
  return()
endif()

# set up our project name
project(InstanceScripts CXX)

include(Classic/CMakeLists.txt)
include(Tbc/CMakeLists.txt)
include(Wotlk/CMakeLists.txt)

set(SRC_BASIC
   Setup.cpp
   Setup.h
   # Required for linker
   ../../world/Server/ServerState.cpp
   ../../shared/Util.cpp
)

#sources -contains source files for our library.
set(sources
    ${SRC_BASIC}
    ${SRC_CLASSIC_SCRIPTS_FILES}
    ${SRC_TBC_SCRIPTS_FILES}
    ${SRC_WOTLK_SCRIPTS_FILES}	
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

target_link_libraries(${PROJECT_NAME} world)

if(USE_PCH)
    gen_cxx_pch(${PROJECT_NAME} Setup.h ${CMAKE_CURRENT_SOURCE_DIR}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
unset(sources)
unset(headers)
