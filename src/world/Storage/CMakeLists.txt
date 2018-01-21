# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Storage)

set(SRC_STORAGE_FILES
   ${PATH_PREFIX}/DatabaseCleaner.cpp
   ${PATH_PREFIX}/DatabaseCleaner.h
   ${PATH_PREFIX}/DayWatcherThread.cpp
   ${PATH_PREFIX}/DayWatcherThread.h
   ${PATH_PREFIX}/MySQLDataStore.cpp
   ${PATH_PREFIX}/MySQLDataStore.hpp
   ${PATH_PREFIX}/MySQLStructures.h
   ${PATH_PREFIX}/WorldStrings.h
)

source_group(Storage FILES ${SRC_STORAGE_FILES})
unset(PATH_PREFIX)
