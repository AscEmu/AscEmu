# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server/Packets/Movement)

set(SRC_PACKET_MOVEMENT_FILES
   ${PATH_PREFIX}/CreatureMovement.cpp
   ${PATH_PREFIX}/CreatureMovement.hpp
   ${PATH_PREFIX}/SmsgMonsterMove.hpp
)

source_group(Server\\Packets\\Movement FILES ${SRC_PACKET_MOVEMENT_FILES})
unset(PATH_PREFIX)
