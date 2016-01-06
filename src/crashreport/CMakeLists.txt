# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

project(crashreport)

set(prefix ${ROOT_PATH}/src/${PROJECT_NAME})
set(SRCS
 ${prefix}/Main.c
 )

add_executable(${PROJECT_NAME} ${SRCS})
remove_definitions( -std=c++0x )
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
