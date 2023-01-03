# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk)

if ("${ASCEMU_NUMBER}" GREATER_EQUAL 2)
set(SRC_WOTLK_SCRIPTS_FILES

)
endif ()

source_group(Wotlk FILES ${SRC_WOTLK_SCRIPTS_FILES})
unset(PATH_PREFIX)
