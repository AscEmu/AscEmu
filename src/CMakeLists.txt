# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# add our executable related folders
add_subdirectory(shared)
add_subdirectory(logonserver)
add_subdirectory(world)
add_subdirectory(collision)

# if build tools is set, add the subdirectory
if (BUILD_TOOLS)
    add_subdirectory(tools)
endif ()

# if build extras is set, add the subdirectory
if (BUILD_EXTRAS)
    add_subdirectory(tools/extras)
endif ()
