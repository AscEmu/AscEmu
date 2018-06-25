# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server/Packets/Handlers)

set(SRC_PACKET_HANDLER_FILES
   ${PATH_PREFIX}/AreaTriggerHandler.cpp
   ${PATH_PREFIX}/AreaTrigger.Legacy.cpp
   ${PATH_PREFIX}/AuctionHandler.cpp
   ${PATH_PREFIX}/BattlegroundHandler.cpp
   ${PATH_PREFIX}/CalendarHandler.cpp
   ${PATH_PREFIX}/ChannelHandler.cpp
   ${PATH_PREFIX}/ChannelHandler.Legacy.cpp
   ${PATH_PREFIX}/CharacterHandler.cpp 
   ${PATH_PREFIX}/CharacterHandler.Legacy.cpp 
   ${PATH_PREFIX}/ChatHandler.cpp
   ${PATH_PREFIX}/ChatHandler.Legacy.cpp
   ${PATH_PREFIX}/CombatHandler.cpp
   ${PATH_PREFIX}/CombatHandler.Legacy.cpp
   ${PATH_PREFIX}/DuelHandler.cpp
   ${PATH_PREFIX}/DuelHandler.Legacy.cpp
   ${PATH_PREFIX}/GMTicketHandler.cpp
   
   ${PATH_PREFIX}/GroupHandler.cpp
   ${PATH_PREFIX}/GroupHandler.Legacy.cpp
   ${PATH_PREFIX}/GuildHandler.cpp
   ${PATH_PREFIX}/GuildHandler.Legacy.cpp
   ${PATH_PREFIX}/HonorHandler.cpp
   ${PATH_PREFIX}/HonorHandler.Legacy.cpp
   ${PATH_PREFIX}/HonorHandler.h
   ${PATH_PREFIX}/LfgHandler.cpp
   ${PATH_PREFIX}/LfgHandler.Legacy.cpp
   ${PATH_PREFIX}/ItemHandler.cpp
   ${PATH_PREFIX}/ItemHandler.Legacy.cpp
   ${PATH_PREFIX}/MailHandler.cpp
   ${PATH_PREFIX}/MailHandler.Legacy.cpp
   ${PATH_PREFIX}/MiscHandler.cpp
   ${PATH_PREFIX}/MiscHandler.Legacy.cpp
   
   ${PATH_PREFIX}/NPCHandler.cpp
   ${PATH_PREFIX}/NPCHandler.Legacy.cpp
   
   ${PATH_PREFIX}/NPCHandler.h
   ${PATH_PREFIX}/MovementHandler.cpp
   ${PATH_PREFIX}/MovementHandler.Legacy.cpp
   ${PATH_PREFIX}/PetHandler.cpp
   ${PATH_PREFIX}/PetHandler.Legacy.cpp
   
   ${PATH_PREFIX}/PlayerCache.cpp
   ${PATH_PREFIX}/PlayerCache.h
   ${PATH_PREFIX}/PlayerPacketWrapper.cpp
   ${PATH_PREFIX}/QueryHandler.cpp
   
   ${PATH_PREFIX}/QuestHandler.cpp
   ${PATH_PREFIX}/QuestHandler.Legacy.cpp
   ${PATH_PREFIX}/RaidHandler.cpp
   ${PATH_PREFIX}/RaidHandler.Legacy.cpp
   ${PATH_PREFIX}/ReputationHandler.cpp
   ${PATH_PREFIX}/ReputationHandler.Legacy.cpp
   
   ${PATH_PREFIX}/SocialHandler.cpp
   ${PATH_PREFIX}/SpellHandler.cpp
   
   ${PATH_PREFIX}/TaxiHandler.cpp
   ${PATH_PREFIX}/TaxiHandler.Legacy.cpp
   ${PATH_PREFIX}/TradeHandler.cpp
   ${PATH_PREFIX}/TradeHandler.Legacy.cpp
   ${PATH_PREFIX}/VehicleHandler.cpp
   ${PATH_PREFIX}/VehicleHandler.Legacy.cpp
   #${PATH_PREFIX}/VoiceChatHandler.Legacy.cpp //Not used!
)

source_group(Server\\Packets\\Handlers FILES ${SRC_PACKET_HANDLER_FILES})
unset(PATH_PREFIX)
