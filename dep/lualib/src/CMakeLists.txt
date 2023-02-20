# Angle bracket includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
project(lualib C)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

file(GLOB lua_headers
    "lua/*.h"
)

file(GLOB lua_sources
    "lua/*.c"
)

# lua static library
add_library (${PROJECT_NAME} STATIC ${lua_sources})
