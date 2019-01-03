# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Chat/Commands)

set(SRC_COMMAND_FILES
   ${PATH_PREFIX}/AccountCommands.cpp
   ${PATH_PREFIX}/AchievementCommands.cpp
   ${PATH_PREFIX}/AdminCommands.cpp
   ${PATH_PREFIX}/ArenaCommands.cpp
   ${PATH_PREFIX}/BGCommands.cpp
   ${PATH_PREFIX}/CharacterCommands.cpp
   ${PATH_PREFIX}/CheatCommands.cpp
   ${PATH_PREFIX}/debugcmds.cpp
   ${PATH_PREFIX}/DebugCommands.cpp
   ${PATH_PREFIX}/GameEventMgrCommands.cpp
   ${PATH_PREFIX}/GameMasterCommands.cpp
   ${PATH_PREFIX}/GameObjectCommands.cpp
   ${PATH_PREFIX}/GMTicketCommands.cpp
   ${PATH_PREFIX}/GuildCommands.cpp
   ${PATH_PREFIX}/InstanceCommands.cpp
   ${PATH_PREFIX}/LookupCommands.cpp
   ${PATH_PREFIX}/MiscCommands.cpp
   ${PATH_PREFIX}/ModifyCommands.cpp
   ${PATH_PREFIX}/NpcCommands.cpp
   ${PATH_PREFIX}/PetCommands.cpp
   ${PATH_PREFIX}/QuestCommands.cpp
   ${PATH_PREFIX}/RecallCommands.cpp
   ${PATH_PREFIX}/ServerCommands.cpp
   ${PATH_PREFIX}/TransporterCommands.cpp
   ${PATH_PREFIX}/TicketCommands.cpp
   ${PATH_PREFIX}/VehicleCommands.cpp
   ${PATH_PREFIX}/WaypointCommands.cpp
)

source_group(Chat\\Commands FILES ${SRC_COMMAND_FILES})
unset(PATH_PREFIX)
