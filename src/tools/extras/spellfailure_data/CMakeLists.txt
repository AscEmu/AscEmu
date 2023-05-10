# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

file(GLOB SPELLFAILURE_DATA_SOURCES *.cpp)

set(SRC_SPELLFAILURE_DATA
    ${SPELLFAILURE_DATA_SOURCES}
)

add_executable(spellfailure_data ${SRC_SPELLFAILURE_DATA})
target_link_libraries(spellfailure_data ${ZLIB_LIBRARIES})
install(TARGETS spellfailure_data RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
