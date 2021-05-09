# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Database)

set(SRC_DATABASE_FILES
   ${PATH_PREFIX}/CreateInterface.cpp
   ${PATH_PREFIX}/Database.cpp
   ${PATH_PREFIX}/Database.h
   ${PATH_PREFIX}/DatabaseCommon.hpp
   ${PATH_PREFIX}/DatabaseUpdater.cpp
   ${PATH_PREFIX}/DatabaseUpdater.hpp
   ${PATH_PREFIX}/Field.hpp
   ${PATH_PREFIX}/MySQLDatabase.cpp
   ${PATH_PREFIX}/MySQLDatabase.h
)

source_group(Database FILES ${SRC_DATABASE_FILES})
unset(PATH_PREFIX)
