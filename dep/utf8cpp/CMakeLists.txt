# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

file(GLOB utf8cpp_SRCS utf8/*.h)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/utf8.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utf8/checked.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utf8/core.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utf8/unchecked.h
)

add_library(utf8cpp STATIC ${utf8cpp_SRCS})
target_link_libraries(utf8cpp ${utf8cpp_LIBRARIES})
set_target_properties(utf8cpp PROPERTIES LINKER_LANGUAGE CXX)

unset(utf8cpp_SRCS)
