# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management/Gossip)

set(SRC_MANAGEMENT_GOSSIP_FILES
   ${PATH_PREFIX}/GossipDefines.hpp
   ${PATH_PREFIX}/GossipMenu.cpp
   ${PATH_PREFIX}/GossipMenu.h
   ${PATH_PREFIX}/GossipScript.cpp
   ${PATH_PREFIX}/GossipScript.h
)

source_group(Management\\Gossip FILES ${SRC_MANAGEMENT_GOSSIP_FILES})
unset(PATH_PREFIX)
