# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management/ObjectUpdates)

set(SRC_MANAGEMENT_OBJECTUPDATES_FILES
   ${PATH_PREFIX}/SplineManager.cpp
   ${PATH_PREFIX}/SplineManager.h
   ${PATH_PREFIX}/UpdateManager.cpp
   ${PATH_PREFIX}/UpdateManager.h
)

source_group(Management\\ObjectUpdates FILES ${SRC_MANAGEMENT_OBJECTUPDATES_FILES})
unset(PATH_PREFIX)