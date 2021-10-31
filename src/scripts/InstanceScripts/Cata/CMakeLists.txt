# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Cata)

if("${ASCEMU_NUMBER}" GREATER_EQUAL 3)
set(SRC_CATA_SCRIPTS_FILES
)
endif()

source_group(Cata FILES ${SRC_CATA_SCRIPTS_FILES})
unset(PATH_PREFIX)