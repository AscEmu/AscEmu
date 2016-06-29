# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Packets/Handlers)

set(SRC_PACKET_HANDLER_FILES
   ${PATH_PREFIX}/BattlegroundHandler.cpp
   ${PATH_PREFIX}/CalendarHandler.cpp
   ${PATH_PREFIX}/ChannelHandler.cpp
   ${PATH_PREFIX}/CharacterHandler.cpp 
   ${PATH_PREFIX}/ChatHandler.cpp
   ${PATH_PREFIX}/CombatHandler.cpp
   ${PATH_PREFIX}/GroupHandler.cpp
   ${PATH_PREFIX}/GMTicket
   ${PATH_PREFIX}/GuildHandler.cpp
   ${PATH_PREFIX}/HonorHandler.cpp
   ${PATH_PREFIX}/HonorHandler.h
   ${PATH_PREFIX}/LfgHandler.cpp
   ${PATH_PREFIX}/ItemHandler.cpp
   ${PATH_PREFIX}/MailHandler.cpp
   ${PATH_PREFIX}/MiscHandler.cpp
   ${PATH_PREFIX}/MiscHandler.h
   ${PATH_PREFIX}/NPCHandler.cpp
   ${PATH_PREFIX}/NPCHandler.h
   ${PATH_PREFIX}/MovementHandler.cpp
   ${PATH_PREFIX}/PetHandler.cpp
   ${PATH_PREFIX}/QueryHandler.cpp
   ${PATH_PREFIX}/QuestHandler.cpp
   ${PATH_PREFIX}/RaidHandler.cpp
   ${PATH_PREFIX}/ReputationHandler.cpp
   ${PATH_PREFIX}/SocialHandler.cpp
   ${PATH_PREFIX}/TaxiHandler.cpp
   ${PATH_PREFIX}/TradeHandler.cpp
   ${PATH_PREFIX}/VehicleHandler.cpp
   #${PATH_PREFIX}/VoiceChatHandler.cpp //Not used!
)

source_group(Packets\\Handlers FILES ${SRC_PACKET_HANDLER_FILES})
unset(PATH_PREFIX)
