# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

include(GameCata/Units/CMakeLists.txt)
include(GameCata/Management/CMakeLists.txt)

set(PATH_PREFIX GameCata)

set(SRC_GAMECATA_FILES
    ${PATH_PREFIX}/Handlers/AddonHandler.cpp
    ${PATH_PREFIX}/Handlers/AuctionHandler.cpp
    ${PATH_PREFIX}/Handlers/BattlegroundHandler.cpp
    ${PATH_PREFIX}/Handlers/ChannelHandler.cpp
    ${PATH_PREFIX}/Handlers/CharacterHandler.cpp
    ${PATH_PREFIX}/Handlers/DuelHandler.cpp
    ${PATH_PREFIX}/Handlers/GroupHandler.cpp
    ${PATH_PREFIX}/Handlers/ItemHandler.cpp
    ${PATH_PREFIX}/Handlers/LfgHandler.cpp
    ${PATH_PREFIX}/Handlers/LootHandler.cpp
    ${PATH_PREFIX}/Handlers/MailHandler.cpp
    ${PATH_PREFIX}/Handlers/MiscellaneousHandler.cpp
    ${PATH_PREFIX}/Handlers/MovementHandler.cpp
    ${PATH_PREFIX}/Handlers/NPCHandler.cpp
    ${PATH_PREFIX}/Handlers/QueryHandler.cpp
    ${PATH_PREFIX}/Handlers/QuestHandler.cpp
    ${PATH_PREFIX}/Handlers/ReportHandler.cpp
    ${PATH_PREFIX}/Handlers/SkillHandler.cpp
    ${PATH_PREFIX}/Handlers/TaxiHandler.cpp
    ${PATH_PREFIX}/Handlers/TradeHandler.cpp
    ${PATH_PREFIX}/Movement/MovementDefines.h
    ${PATH_PREFIX}/Movement/MovementStructures.h
    ${PATH_PREFIX}/Network/Opcodes.cpp
    ${PATH_PREFIX}/Network/Opcodes.h
    ${PATH_PREFIX}/Network/WorldPacketHandlers.cpp
    ${PATH_PREFIX}/Storage/DBCStores.cpp
    ${PATH_PREFIX}/Storage/DBCStores.h
    ${PATH_PREFIX}/Storage/DBCStructures.cpp
    ${PATH_PREFIX}/Storage/DBCStructures.h
    ${PATH_PREFIX}/UpdateFields.h
)

source_group(GameCata FILES ${SRC_GAMECATA_FILES})
unset(PATH_PREFIX)
