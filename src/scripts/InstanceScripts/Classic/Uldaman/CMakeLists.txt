# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Classic/Uldaman)

set(SRC_CLASSIC_ULDAMAN_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_Uldaman.cpp
)

source_group(Classic/Uldaman FILES ${SRC_CLASSIC_ULDAMAN_SCRIPTS_FILES})
unset(PATH_PREFIX)
