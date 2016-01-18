# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
project(Ascemu)
cmake_minimum_required(VERSION 2.8)

# cmake options
include(${CMAKE_SOURCE_DIR}/cmake/Options.cmake)

# perform system tasks
include(${CMAKE_SOURCE_DIR}/cmake/CheckSystem.cmake)

#add dependecies
add_subdirectory(dep)

#add executables
add_subdirectory(src)

#add modules
add_subdirectory(src/scripts)

# install libraries for windows build (libsql.dll, libey.dll)
if(WIN32)
    install(FILES ${INSTALLED_DEPENDENCIES} DESTINATION .)
endif()
