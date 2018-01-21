# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

#We have our own custom modules that we use. This tells cmakes where to find them.
set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/cmake/Modules)

# get git information
include(${CMAKE_SOURCE_DIR}/cmake/GitRevision.cmake)

# generally load PCH module
if(USE_PCH)
    include(PCHSupport)
endif()

# get architecture type
if(UNIX)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "[xX]64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "[xX]86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "[aA][mM][dD]64" )
        set(IS_64BIT TRUE)
    else()
        set(IS_64BIT FALSE)
    endif()
else()
    if(CMAKE_GENERATOR MATCHES Win64*)
        set(IS_64BIT TRUE)
    else()
        set(IS_64BIT FALSE)
    endif()
endif()

# set default architecture identifier
if(IS_64BIT)
    message(STATUS "Detected 64 bit system")
endif()

# default definitions
#-DPREFIX=\"${ASCEMU_SCRIPTLIB_PATH}\"
add_definitions(-DHAVE_CONFIG_H  )

mark_as_advanced(
    ZLIB_LIBRARIES
    ZLIB_INCLUDE_DIRS
    PCRE_LIBRARIES
    PCRE_INCLUDE_DIR
    OPENSSL_LIBRARIES
    OPENSSL_INCLUDE_DIR
    MYSQL_LIBRARIES
    MYSQL_INCLUDE_DIR
    BZIP2_LIBRARIES
    BZIP2_INCLUDE_DIRS
)

# apply system settings
if(WIN32)
    include(${CMAKE_SOURCE_DIR}/cmake/Systems/Windows.cmake)
elseif(UNIX)
    if(APPLE)
        include(${CMAKE_SOURCE_DIR}/cmake/Systems/Apple.cmake) 
    elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD") 
        include(${CMAKE_SOURCE_DIR}/cmake/Systems/FreeBSD.cmake) 
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux") 
        include(${CMAKE_SOURCE_DIR}/cmake/Systems/Linux.cmake) 
    else()
        message(FATAL_ERROR "System is not supported." ) 
    endif()
else()
    message(FATAL_ERROR "System is not supported." )
endif()

# apply config settings
include(${CMAKE_SOURCE_DIR}/cmake/Modules/GenerateConfigs.cmake)
