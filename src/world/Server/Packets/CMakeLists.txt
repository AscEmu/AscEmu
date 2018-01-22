# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server/Packets)

set(SRC_PACKET_FILES
   ${PATH_PREFIX}/ManagedPacket.h
   ${PATH_PREFIX}/Opcode.h
   ${PATH_PREFIX}/SmsgBindPointUpdate.h
   ${PATH_PREFIX}/SmsgSetProficiency.h
   ${PATH_PREFIX}/SmsgTutorialFlags.h
)

source_group(Server\\Packets FILES ${SRC_PACKET_FILES})
unset(PATH_PREFIX)
