# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

project(map_extractor CXX)
file(GLOB source *.cpp *.h)

set(source
    ${source}
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
)
include_directories(
  ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
  ${CMAKE_SOURCE_DIR}/dep/dbcfile 
  ${CMAKE_SOURCE_DIR}/dep/loadlib
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(${PROJECT_NAME} ${source})
target_link_libraries(${PROJECT_NAME} dbcfile libmpq)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})

unset(source)
