# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Classic/TheStockade)

set(SRC_CLASSIC_STOCKADE_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_TheStockade.cpp
)

source_group(Classic/TheStockade FILES ${SRC_CLASSIC_STOCKADE_SCRIPTS_FILES})
unset(PATH_PREFIX)
