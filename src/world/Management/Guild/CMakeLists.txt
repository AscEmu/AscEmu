# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management/Guild)

set(SRC_MANAGEMENT_GUILD_FILES
   ${PATH_PREFIX}/GuildDefinitions.h
)

source_group(Management\\Guild FILES ${SRC_MANAGEMENT_GUILD_FILES})
unset(PATH_PREFIX)
