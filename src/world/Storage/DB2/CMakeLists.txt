# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Storage/DB2)

set(SRC_STORAGE_DB2_FILES
   ${PATH_PREFIX}/DB2Loader.cpp
   ${PATH_PREFIX}/DB2Loader.hpp
   ${PATH_PREFIX}/DB2Storage.hpp
   ${PATH_PREFIX}/DB2Stores.cpp
   ${PATH_PREFIX}/DB2Stores.hpp
   ${PATH_PREFIX}/DB2Structures.hpp
)

source_group(Storage\\DB2 FILES ${SRC_STORAGE_DB2_FILES})
unset(PATH_PREFIX)
