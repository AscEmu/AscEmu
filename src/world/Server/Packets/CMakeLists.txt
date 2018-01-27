# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server/Packets)

set(SRC_PACKET_FILES
   ${PATH_PREFIX}/CmsgNameQuery.h
   ${PATH_PREFIX}/CmsgSetActiveMover.h
   ${PATH_PREFIX}/ManagedPacket.h
   ${PATH_PREFIX}/Opcode.h
   ${PATH_PREFIX}/SmsgBindPointUpdate.h
   ${PATH_PREFIX}/SmsgClearExtraAuraInfo.h
   ${PATH_PREFIX}/SmsgInitialSpells.h
   ${PATH_PREFIX}/SmsgInstanceDifficulty.h
   ${PATH_PREFIX}/SmsgNameQueryResponse.h
   ${PATH_PREFIX}/SmsgSetExtraAuraInfo.h
   ${PATH_PREFIX}/SmsgSetProficiency.h
   ${PATH_PREFIX}/SmsgTutorialFlags.h
   ${PATH_PREFIX}/SmsgUpdateAuraDuration.h
)

source_group(Server\\Packets FILES ${SRC_PACKET_FILES})
unset(PATH_PREFIX)
