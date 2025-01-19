# - Find mysqlclient
# Find the native MySQL includes and library
#
#  MYSQL_INCLUDE_DIR - where to find mysql.h, etc.
#  MYSQL_LIBRARY     - List of libraries when using MySQL.
#  MYSQL_FOUND       - True if MySQL found.
#  MYSQL_DLL         - Full path to copy libmysql.dll to your AE folder

# NOTE: You can't build AE with Win32 and use MySQL x64 libs and otherwise. This will result in LNK errors!

# MySQL 32/64 bit handling
if (WIN32)
    LIST(APPEND MY_INCLUDE_PATHS
        # MySQL
        "$ENV{ProgramFiles}/MySQL/*/include"
        "$ENV{SystemDrive}/MySQL/*/include"
        "$ENV{MYSQL_ROOT}/include"
        "C:/Program Files/MySQL/*/include"
        "D:/Program Files/MySQL/*/include"
        "${PROGRAM_FILES_32}/MySQL/*/include"
        "${PROGRAM_FILES_32}/MySQL/include"
        "${PROGRAM_FILES_64}/MySQL/*/include"
        "${PROGRAM_FILES_64}/MySQL/include"
        "C:/Program Files (x86)/MySQL/*/include"
        "D:/Program Files (x86)/MySQL/*/include"

        # MariaDB
        "$ENV{ProgramFiles}/MariaDB/*/include"
        "$ENV{SystemDrive}/MariaDB/*/include"
        "$ENV{MARIADB_ROOT}/include"
        "C:/Program Files/MariaDB/*/include"
        "D:/Program Files/MariaDB/*/include"
        "${PROGRAM_FILES_32}/MariaDB/*/include"
        "${PROGRAM_FILES_32}/MariaDB/include"
        "${PROGRAM_FILES_64}/MariaDB/*/include"
        "${PROGRAM_FILES_64}/MariaDB/include"
        "C:/Program Files (x86)/MariaDB/*/include"
        "D:/Program Files (x86)/MariaDB/*/include"
    )
else()
    LIST(APPEND MY_INCLUDE_PATHS
        # Generic
        "/usr/include"
        "/usr/local/include"

        # MySQL
        "/usr/include/mysql"
        "/usr/local/include/mysql"
        "/usr/local/mysql/include"

        # MariaDB
        "/usr/include/mariadb"
        "/usr/local/include/mariadb"
        "/usr/local/mariadb/include"
    )
endif()

find_path(MYSQL_INCLUDE_DIR
    NAMES "mysql.h"
    PATHS ${MY_INCLUDE_PATHS})

if (WIN32)
    LIST(APPEND MY_LIB_PATHS
        # MySQL
        "${PROGRAM_FILES_64}/MySQL/*/lib"
        "${PROGRAM_FILES_32}/MySQL/*/lib"
        "$ENV{ProgramFiles}/MySQL/*/lib"
        "$ENV{SystemDrive}/MySQL/*/lib"
        "C:/Program Files (x86)/MySQL/*/lib"
        "D:/Program Files (x86)/MySQL/*/lib"
        "C:/Program Files/MySQL/*/lib"
        "D:/Program Files/MySQL/*/lib"

        # MariaDB
        "${PROGRAM_FILES_64}/MariaDB/*/lib"
        "${PROGRAM_FILES_32}/MariaDB/*/lib"
        "$ENV{ProgramFiles}/MariaDB/*/lib"
        "$ENV{SystemDrive}/MariaDB/*/lib"
        "C:/Program Files (x86)/MariaDB/*/lib"
        "D:/Program Files (x86)/MariaDB/*/lib"
        "C:/Program Files/MariaDB/*/lib"
        "D:/Program Files/MariaDB/*/lib"
        )

    find_library(MYSQL_LIBRARY
        NAMES "libmysql" "libmariadb"
        PATHS ${MY_LIB_PATHS})

else()
    find_library(MYSQL_LIBRARY
        NAMES "mysqlclient" "mysqlclient_r" "mariadbclient"
        PATHS
            # Generic
            "/usr/lib"
            "/usr/local/lib"

            # MySQL
            "/usr/lib/mysql"
            "/usr/local/mysql/lib"
            "/usr/local/lib/mysql"
            "/opt/local/lib/mysql5/mysql"

            # MariaDB
            "/usr/lib/mariadb"
            "/usr/local/mariadb/lib"
            "/usr/local/lib/mariadb"
            "/opt/local/lib/mariadb/mysql")
endif()

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
else ()
    message(FATAL_ERROR "Could not find the MySQL libraries! Please install the development libraries and headers")
endif ()
