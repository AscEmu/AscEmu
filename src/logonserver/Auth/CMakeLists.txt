# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Auth)

set(SRC_AUTH_FILES
   ${PATH_PREFIX}/AuthSocket.Legacy.cpp
   ${PATH_PREFIX}/AuthSocket.cpp
   ${PATH_PREFIX}/AuthSocket.h
   ${PATH_PREFIX}/AuthStructs.h
   ${PATH_PREFIX}/AutoPatcher.cpp
   ${PATH_PREFIX}/AutoPatcher.h
)

source_group(Auth FILES ${SRC_AUTH_FILES})
unset(PATH_PREFIX)
