# Angle bracket includes

# set up our project name.
project(lualib C)

file(GLOB LUALIB_HEADERS
    "lua/*.h"
)

file(GLOB LUALIB_SOURCES
    "lua/*.c"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# lua static library
add_library (${PROJECT_NAME} STATIC ${LUALIB_SOURCES})
