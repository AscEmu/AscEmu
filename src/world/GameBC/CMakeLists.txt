# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameBC)

set(SRC_GAMEBC_FILES
	${PATH_PREFIX}/Network/Opcodes.cpp
)

source_group(GameBC FILES ${SRC_GAMEBC_FILES})
unset(PATH_PREFIX)
