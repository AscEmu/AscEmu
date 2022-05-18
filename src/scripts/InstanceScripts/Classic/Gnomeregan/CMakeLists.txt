# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Classic/Gnomeregan)

set(SRC_CLASSIC_GNOME_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_Gnomeregan.cpp
)

source_group(Classic/Gnomeregan FILES ${SRC_CLASSIC_GNOME_SCRIPTS_FILES})
unset(PATH_PREFIX)
