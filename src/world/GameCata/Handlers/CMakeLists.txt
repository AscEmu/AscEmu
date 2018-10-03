# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameCata/Handlers)

set(SRC_GAMECATA_HANDLER_FILES
    ${PATH_PREFIX}/AddonHandler.cpp
    ${PATH_PREFIX}/BattlegroundHandler.cpp
    ${PATH_PREFIX}/GMTicketHandler.cpp
    ${PATH_PREFIX}/GroupHandler.cpp
    ${PATH_PREFIX}/ItemHandler.cpp
    ${PATH_PREFIX}/LfgHandler.cpp
    ${PATH_PREFIX}/LootHandler.cpp
    ${PATH_PREFIX}/MovementHandler.cpp
    ${PATH_PREFIX}/ReportHandler.cpp
    ${PATH_PREFIX}/SkillHandler.cpp
    ${PATH_PREFIX}/SpellHandler.cpp
)

source_group(GameCata\\Handlers FILES ${SRC_GAMECATA_HANDLER_FILES})
unset(PATH_PREFIX)
