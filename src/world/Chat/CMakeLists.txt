# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Chat)

set(SRC_CHAT_FILES
   ${PATH_PREFIX}/Chat.cpp
   ${PATH_PREFIX}/Chat.h
)

source_group(Chat FILES ${SRC_CHAT_FILES})
unset(PATH_PREFIX)
