# Angle bracket includes

# set up our project name.
set(LIBRARY_NAME lualib)

file(GLOB LUALIB_HEADERS
    "lua/*.h"
)

file(GLOB LUALIB_SOURCES
    "lua/*.c"
)

set(LUALIB_SRCS
    ${LUALIB_SOURCES}
)

# lua static library
add_library (${LIBRARY_NAME} STATIC ${LUALIB_SRCS})

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

unset(LUALIB_SRCS)
