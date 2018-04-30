# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Storage/DBC)

set(SRC_STORAGE_DBC_FILES
   ${PATH_PREFIX}/DBCGlobals.hpp
   ${PATH_PREFIX}/DBCLoader.cpp
   ${PATH_PREFIX}/DBCLoader.hpp
   ${PATH_PREFIX}/DBCRecord.cpp
   ${PATH_PREFIX}/DBCRecord.hpp
   ${PATH_PREFIX}/DBCSQL.cpp
   ${PATH_PREFIX}/DBCSQL.hpp
   ${PATH_PREFIX}/DBCStorage.hpp
   ${PATH_PREFIX}/DBCStores.h
   ${PATH_PREFIX}/DBCStructures.hpp
)

source_group(Storage\\DBC FILES ${SRC_STORAGE_DBC_FILES})
unset(PATH_PREFIX)
