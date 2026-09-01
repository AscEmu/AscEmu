# Angle bracket includes

# set up our project name.
set(LIBRARY_NAME lualib)

file(GLOB LUALIB_HEADERS
    "lua/*.h"
)

file(GLOB LUALIB_SOURCES
    "lua/*.c"
)

# luac.c is the standalone "luac" bytecode-compiler CLI tool's source - it
# defines its own main() and was never meant to be part of the embeddable
# lua library, but the glob above picks it up anyway. Linking it into this
# static lib pulls that main() into every executable that links lualib,
# colliding with the executable's own main() (LNK4006 "already defined,
# second definition ignored").
list(REMOVE_ITEM LUALIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lua/luac.c")

set(LUALIB_SRCS
    ${LUALIB_SOURCES}
)

# lua static library
add_library (${LIBRARY_NAME} STATIC ${LUALIB_SRCS})

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

unset(LUALIB_SRCS)
