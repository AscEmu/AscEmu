# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management/ObjectUpdates)

set(SRC_MANAGEMENT_OBJECTUPDATES_FILES
    ${PATH_PREFIX}/UpdateManager.cpp
    ${PATH_PREFIX}/UpdateManager.hpp
)

source_group(Management\\ObjectUpdates FILES ${SRC_MANAGEMENT_OBJECTUPDATES_FILES})
unset(PATH_PREFIX)
