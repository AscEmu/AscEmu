# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Data)

set(SRC_DATA_FILES
   ${PATH_PREFIX}/WoWContainer.h
   ${PATH_PREFIX}/WoWCorpse.h
   ${PATH_PREFIX}/WoWDynamicObject.h
   ${PATH_PREFIX}/WoWGameObject.h
   ${PATH_PREFIX}/WoWItem.h
   ${PATH_PREFIX}/WoWObject.h
   ${PATH_PREFIX}/WoWPlayer.h
   ${PATH_PREFIX}/WoWUnit.h
)

source_group(Data FILES ${SRC_DATA_FILES})
unset(PATH_PREFIX)
