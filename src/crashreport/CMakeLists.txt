# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

project(crashreport C)

set(SRCS Main.c)

add_executable(${PROJECT_NAME} ${SRCS})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
unset(SRCS)
