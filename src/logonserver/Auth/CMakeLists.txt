# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Auth)

set(SRC_AUTH_FILES
   ${PATH_PREFIX}/AccountCache.cpp
   ${PATH_PREFIX}/AccountCache.TBC.cpp
   ${PATH_PREFIX}/AccountCache.h
   ${PATH_PREFIX}/AuthSocket.Legacy.cpp
   ${PATH_PREFIX}/AuthSocket.cpp
   ${PATH_PREFIX}/AuthSocket.h
   ${PATH_PREFIX}/AuthStructs.h
   ${PATH_PREFIX}/AutoPatcher.cpp
   ${PATH_PREFIX}/AutoPatcher.h
)

source_group(Auth FILES ${SRC_AUTH_FILES})
unset(PATH_PREFIX)
