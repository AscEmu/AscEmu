# Find pcre
# Find the native PCRE includes and library
#
# PCRE_INCLUDE_DIRS - where to find pcre.h, etc.
# PCRE_LIBRARIES - List of libraries when using pcre.
# PCRE_FOUND - True if pcre found.

if (PCRE_INCLUDE_DIRS)
    # Already in cache, be silent
    SET(PCRE_FIND_QUIETLY TRUE)
endif (PCRE_INCLUDE_DIRS)

find_path(PCRE_INCLUDE_DIR pcre.h)

set(PCRE_NAMES pcre)
find_library(PCRE_LIBRARY NAMES ${PCRE_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set PCRE_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PCRE DEFAULT_MSG PCRE_LIBRARY PCRE_INCLUDE_DIR)

if (PCRE_FOUND)
    set( PCRE_LIBRARIES ${PCRE_LIBRARY} )
    set( PCRE_INCLUDE_DIRS ${PCRE_INCLUDE_DIR} )
else (PCRE_FOUND)
    set( PCRE_LIBRARIES )
    set( PCRE_INCLUDE_DIRS )
endif (PCRE_FOUND)

MARK_AS_ADVANCED( PCRE_LIBRARIES PCRE_INCLUDE_DIRS )
