# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

add_library(utf8cpp STATIC
  utf8.h
  utf8/checked.h
  utf8/core.h
  utf8/unchecked.h
)

target_link_libraries(utf8cpp)
set_target_properties(utf8cpp PROPERTIES LINKER_LANGUAGE C)
