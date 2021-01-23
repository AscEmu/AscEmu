# - Find mysqlclient
# Find the native MySQL includes and library
#
#  MYSQL_INCLUDE_DIR - where to find mysql.h, etc.
#  MYSQL_LIBRARIES   - List of libraries when using MySQL.
#  MYSQL_FOUND       - True if MySQL found.

find_path(MYSQL_INCLUDE_DIR
    NAMES "mysql.h"
    PATHS "$ENV{ProgramFiles}/MySQL/*/include"
            "$ENV{ProgramW6432}/MySQL/*/include"
            "$ENV{SystemDrive}/MySQL/*/include"
            "/usr/include/mysql"
            "/usr/local/include/mysql"
            "/usr/mysql/include/mysql")
	
find_library(MYSQL_LIBRARY
    NAMES "libmysql" "libmysql_r"
    PATHS "$ENV{ProgramFiles}/MySQL/*/lib"
            "$ENV{ProgramW6432}/MySQL/*/lib"
            "$ENV{SystemDrive}/MySQL/*/lib"
            "/usr/lib"
            "/usr/lib/mysql"
            "/usr/local/lib"
            "/usr/mysql/lib/mysql")

if (MYSQL_LIBRARY)
    if (MYSQL_INCLUDE_DIR)
        set(MYSQL_FOUND 1)
        message(STATUS "Found MySQL library: ${MYSQL_LIBRARY}")
        message(STATUS "Found MySQL headers: ${MYSQL_INCLUDE_DIR}")
        if (WIN32)
            set(MYSQL_DLL ${MYSQL_LIBRARY})
            STRING(REPLACE ".lib" ".dll" MYSQL_DLL ${MYSQL_DLL})
            message(STATUS "Found MySQL dll: ${MYSQL_DLL}")
        endif (WIN32)
    else (MYSQL_INCLUDE_DIR)
        message(FATAL_ERROR "Could not find MySQL headers! Please install the development libraries and headers")
    endif (MYSQL_INCLUDE_DIR)
    mark_as_advanced(MYSQL_FOUND MYSQL_LIBRARY)
else (MYSQL_LIBRARY)
    message(FATAL_ERROR "Could not find the MySQL libraries! Please install the development libraries and headers")
endif (MYSQL_LIBRARY)