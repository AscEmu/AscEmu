# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

macro(ascemu_source_group dir)
  # skip this if WITH_SOURCE_TREE is not set (empty string).
  if (NOT ${WITH_SOURCE_TREE} STREQUAL "")
    # include all header and c files
    file(GLOB_RECURSE elements RELATIVE ${dir} *.h *.hpp *.c *.cpp *.cc)
    foreach(element ${elements})
      # extract filename and directory
      get_filename_component(element_name ${element} NAME)
      get_filename_component(element_dir ${element} DIRECTORY)
      if (NOT ${element_dir} STREQUAL "")
        # if the file is in a subdirectory use it as source group.
        if (${WITH_SOURCE_TREE} STREQUAL "flat")
          # build flat structure by using only the first subdirectory.
          string(FIND ${element_dir} "/" delemiter_pos)
          if (NOT ${delemiter_pos} EQUAL -1)
            string(SUBSTRING ${element_dir} 0 ${delemiter_pos} group_name)
            source_group("${group_name}" FILES ${dir}/${element})
          else()
            # build hierarchical structure.
            # file is in root directory.
            source_group("${element_dir}" FILES ${dir}/${element})
          endif()
        else()
          # use the full hierarchical structure to build source_groups.
          string(REPLACE "/" "\\" group_name ${element_dir})
          source_group("${group_name}" FILES ${dir}/${element})
        endif()
      else()
        # if the file is in the root directory, place it in the root source_group.
        source_group("\\" FILES ${dir}/${element})
      endif()
    endforeach()
  endif()
endmacro()
