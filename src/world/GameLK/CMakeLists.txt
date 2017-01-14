# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameLK)

set(SRC_GAMELK_FILES
	${PATH_PREFIX}/Network/Opcodes.cpp
)

source_group(GameLK FILES ${SRC_GAMELK_FILES})
unset(PATH_PREFIX)
