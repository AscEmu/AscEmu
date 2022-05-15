# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Classic/Stratholme)

set(SRC_CLASSIC_STRATHOLME_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_Stratholme.cpp
)

source_group(Classic/Stratholme FILES ${SRC_CLASSIC_STRATHOLME_SCRIPTS_FILES})
unset(PATH_PREFIX)
