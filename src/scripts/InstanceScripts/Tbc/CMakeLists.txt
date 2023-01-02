# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Tbc)

if ("${ASCEMU_NUMBER}" GREATER_EQUAL 1)
set(SRC_TBC_SCRIPTS_FILES

)
endif ()

source_group(Tbc FILES ${SRC_TBC_SCRIPTS_FILES})
unset(PATH_PREFIX)
