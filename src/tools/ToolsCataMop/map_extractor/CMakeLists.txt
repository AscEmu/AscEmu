# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

project(map_extractor CXX)
file(GLOB source *.cpp *.h)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/StormLib/src
    ${CMAKE_CURRENT_SOURCE_DIR}/loadlib
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
)

add_executable(${PROJECT_NAME} ${source})
target_link_libraries(${PROJECT_NAME} storm)
add_dependencies(${PROJECT_NAME} storm)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})

unset(source)
