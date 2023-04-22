# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Data)

set(SRC_DATA_FILES
    ${PATH_PREFIX}/Flags.hpp
    ${PATH_PREFIX}/GuidData.hpp
    ${PATH_PREFIX}/WoWContainer.hpp
    ${PATH_PREFIX}/WoWCorpse.hpp
    ${PATH_PREFIX}/WoWDynamicObject.hpp
    ${PATH_PREFIX}/WoWGameObject.hpp
    ${PATH_PREFIX}/WoWItem.hpp
    ${PATH_PREFIX}/WoWObject.hpp
    ${PATH_PREFIX}/WoWPlayer.hpp
    ${PATH_PREFIX}/WoWUnit.hpp
)
if ("${ASCEMU_VERSION}" STREQUAL "Mop" OR "${ASCEMU_VERSION}" STREQUAL "Cata")
    set(SRC_DATA_FILES
    ${SRC_DATA_FILES}
    ${PATH_PREFIX}/WoWAreaTrigger.hpp)
endif ()
