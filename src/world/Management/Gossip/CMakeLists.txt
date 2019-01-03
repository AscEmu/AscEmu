# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management/Gossip)

set(SRC_MANAGEMENT_GOSSIP_FILES
   ${PATH_PREFIX}/Gossip.cpp
   ${PATH_PREFIX}/Gossip.h
   ${PATH_PREFIX}/GossipDefines.hpp
)

source_group(Management\\Gossip FILES ${SRC_MANAGEMENT_GOSSIP_FILES})
unset(PATH_PREFIX)
