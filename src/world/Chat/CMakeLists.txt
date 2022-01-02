# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Chat)

set(SRC_CHAT_FILES
    ${PATH_PREFIX}/Channel.cpp
    ${PATH_PREFIX}/Channel.hpp
    ${PATH_PREFIX}/ChannelDefines.hpp
    ${PATH_PREFIX}/ChannelMgr.cpp
    ${PATH_PREFIX}/ChannelMgr.hpp
    ${PATH_PREFIX}/ChatHandler.cpp
    ${PATH_PREFIX}/ChatHandler.hpp
    ${PATH_PREFIX}/ChatCommand.hpp
    ${PATH_PREFIX}/ChatDefines.hpp
    ${PATH_PREFIX}/CommandTableStorage.cpp
    ${PATH_PREFIX}/CommandTableStorage.hpp
)

source_group(Chat FILES ${SRC_CHAT_FILES})
unset(PATH_PREFIX)
