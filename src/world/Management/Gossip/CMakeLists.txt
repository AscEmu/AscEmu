# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management/Gossip)

set(SRC_MANAGEMENT_GOSSIP_FILES
    ${PATH_PREFIX}/GossipDefines.hpp
    ${PATH_PREFIX}/GossipMenu.cpp
    ${PATH_PREFIX}/GossipMenu.hpp
    ${PATH_PREFIX}/GossipScript.cpp
    ${PATH_PREFIX}/GossipScript.hpp
)

source_group(Management\\Gossip FILES ${SRC_MANAGEMENT_GOSSIP_FILES})
unset(PATH_PREFIX)
