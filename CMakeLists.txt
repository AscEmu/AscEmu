# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
cmake_minimum_required(VERSION 3.22)

project(AscEmu)

# cmake options
include(cmake/Options.cmake)

# perform system tasks
include(cmake/CheckSystem.cmake)

# add dependencies
add_subdirectory(dep)

# add executables
add_subdirectory(src)

# add script libraries
add_subdirectory(src/scripts)

# install db updates
install(DIRECTORY ${INSTALL_DB_FILES} DESTINATION sql FILES_MATCHING PATTERN "*.sql")
