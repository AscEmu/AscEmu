# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Storage)

set(SRC_STORAGE_FILES
   ${PATH_PREFIX}/MySQLDataStore.cpp
   ${PATH_PREFIX}/MySQLDataStore.hpp
)

source_group(Storage FILES ${SRC_STORAGE_FILES})
unset(PATH_PREFIX)
