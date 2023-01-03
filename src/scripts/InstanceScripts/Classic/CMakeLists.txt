# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Classic)

set(SRC_CLASSIC_SCRIPTS_FILES
	${PATH_PREFIX}/Raid_WorldBosses.cpp
    ${PATH_PREFIX}/Raid_WorldBosses.h
)

source_group(Classic FILES ${SRC_CLASSIC_SCRIPTS_FILES})
unset(PATH_PREFIX)
