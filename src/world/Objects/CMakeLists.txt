# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Objects)

set(SRC_OBJECTS_FILES
    ${PATH_PREFIX}/Container.cpp
    ${PATH_PREFIX}/Container.h
    ${PATH_PREFIX}/DynamicObject.cpp
    ${PATH_PREFIX}/DynamicObject.h
    ${PATH_PREFIX}/GameObject.cpp
    ${PATH_PREFIX}/GameObject.h
    ${PATH_PREFIX}/GameObjectDefines.hpp
    ${PATH_PREFIX}/GameObjectProperties.hpp
    ${PATH_PREFIX}/Item.cpp
    ${PATH_PREFIX}/Item.hpp
    ${PATH_PREFIX}/ItemDefines.hpp
    ${PATH_PREFIX}/MovementDefines.h
    ${PATH_PREFIX}/MovementInfo.h
    ${PATH_PREFIX}/Object.cpp
    ${PATH_PREFIX}/Object.h
    ${PATH_PREFIX}/Transporter.cpp
    ${PATH_PREFIX}/Transporter.h
    ${PATH_PREFIX}/ObjectDefines.h
)

source_group(Objects FILES ${SRC_OBJECTS_FILES})
unset(PATH_PREFIX)
