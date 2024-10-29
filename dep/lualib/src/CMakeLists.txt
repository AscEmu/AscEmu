# Angle bracket includes

# set up our project name.
set(EXECUTABLE_NAME lualib)

file(GLOB LUALIB_HEADERS
    "lua/*.h"
)

file(GLOB LUALIB_SOURCES
    "lua/*.c"
)

set(LUALIB_SRCS
    ${LUALIB_SOURCES}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# lua static library
add_library (${EXECUTABLE_NAME} STATIC ${LUALIB_SRCS})

unset(LUALIB_SRCS)
