# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server/Packets)

set(SRC_PACKET_FILES
   ${PATH_PREFIX}/ManagedPacket.cpp
   ${PATH_PREFIX}/ManagedPacket.hpp
   ${PATH_PREFIX}/Opcode.h
)

source_group(Server\\Packets FILES ${SRC_PACKET_FILES})
unset(PATH_PREFIX)
