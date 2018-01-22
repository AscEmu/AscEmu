# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server/Packets/Movement)

set(SRC_PACKET_MOVEMENT_FILES
   ${PATH_PREFIX}/CreatureMovement.cpp
   ${PATH_PREFIX}/CreatureMovement.h
)

source_group(Server\\Packets\\Movement FILES ${SRC_PACKET_MOVEMENT_FILES})
unset(PATH_PREFIX)
