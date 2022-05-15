# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Classic/BlackrockMountain/MoltenCore)

set(SRC_CLASSIC_MOLTENCORE_SCRIPTS_FILES
    ${PATH_PREFIX}/Raid_MoltenCore.cpp
)

source_group(Classic/BlackrockMountain/MoltenCore FILES ${SRC_CLASSIC_MOLTENCORE_SCRIPTS_FILES})
unset(PATH_PREFIX)
