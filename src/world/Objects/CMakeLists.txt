# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Objects)

set(SRC_OBJECTS_FILES
   ${PATH_PREFIX}/CObjectFactory.cpp
   ${PATH_PREFIX}/CObjectFactory.h
   ${PATH_PREFIX}/DynamicObject.cpp
   ${PATH_PREFIX}/DynamicObject.h
   ${PATH_PREFIX}/GameObject.cpp
   ${PATH_PREFIX}/GameObject.h
   ${PATH_PREFIX}/Object.cpp
   ${PATH_PREFIX}/Object.h
)

source_group(Objects FILES ${SRC_OBJECTS_FILES})
unset(PATH_PREFIX)
