/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../world/Server/WorldSession.h"

void WorldSession::loadSpecificHandlers()
{
    // Login
    WorldPacketHandlers[CMSG_CHAR_ENUM].handler = &WorldSession::handleCharEnumOpcode;
    WorldPacketHandlers[CMSG_CHAR_ENUM].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_CREATE].handler = &WorldSession::handleCharCreateOpcode;
    WorldPacketHandlers[CMSG_CHAR_CREATE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_DELETE].handler = &WorldSession::handleCharDeleteOpcode;
    WorldPacketHandlers[CMSG_CHAR_DELETE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_RENAME].handler = &WorldSession::handleCharRenameOpcode;
    WorldPacketHandlers[CMSG_CHAR_RENAME].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].handler = &WorldSession::handleCharCustomizeLooksOpcode;
    WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].handler = &WorldSession::handleCharFactionOrRaceChange;
    WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].handler = &WorldSession::handleCharFactionOrRaceChange;
    WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].status = STATUS_AUTHED;

    // declined names (Cyrillic client)
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].handler = &WorldSession::handleDeclinedPlayerNameOpcode;
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_PLAYER_LOGIN].handler = &WorldSession::handlePlayerLoginOpcode;
    WorldPacketHandlers[CMSG_PLAYER_LOGIN].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_REALM_SPLIT].handler = &WorldSession::handleRealmSplitOpcode;
    WorldPacketHandlers[CMSG_REALM_SPLIT].status = STATUS_AUTHED;

    // Queries
    WorldPacketHandlers[MSG_CORPSE_QUERY].handler = &WorldSession::handleCorpseQueryOpcode;
    WorldPacketHandlers[CMSG_NAME_QUERY].handler = &WorldSession::handleNameQueryOpcode;
    WorldPacketHandlers[CMSG_QUERY_TIME].handler = &WorldSession::handleQueryTimeOpcode;
    WorldPacketHandlers[CMSG_CREATURE_QUERY].handler = &WorldSession::handleCreatureQueryOpcode;
    WorldPacketHandlers[CMSG_GAMEOBJECT_QUERY].handler = &WorldSession::handleGameObjectQueryOpcode;
    WorldPacketHandlers[CMSG_PAGE_TEXT_QUERY].handler = &WorldSession::handlePageTextQueryOpcode;
    WorldPacketHandlers[CMSG_ITEM_NAME_QUERY].handler = &WorldSession::handleItemNameQueryOpcode;
    WorldPacketHandlers[CMSG_QUERY_INSPECT_ACHIEVEMENTS].handler = &WorldSession::handleAchievmentQueryOpcode;

    // Movement
    WorldPacketHandlers[MSG_MOVE_HEARTBEAT].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_WORLDPORT_ACK].handler = &WorldSession::handleMoveWorldportAckOpcode;
    WorldPacketHandlers[MSG_MOVE_JUMP].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_FORWARD].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_BACKWARD].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_FACING].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_STRAFE_LEFT].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_STRAFE_RIGHT].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_STRAFE].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_TURN_LEFT].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_TURN_RIGHT].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_TURN].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_PITCH_UP].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_PITCH_DOWN].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_PITCH].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_RUN_MODE].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_WALK_MODE].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_PITCH].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_SWIM].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_SWIM].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_FALL_LAND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_SET_FLY].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_NOT_ACTIVE_MOVER].handler = &WorldSession::handleMoveNotActiveMoverOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIVE_MOVER].handler = &WorldSession::handleSetActiveMoverOpcode;
    WorldPacketHandlers[CMSG_MOVE_CHNG_TRANSPORT].handler = &WorldSession::handleMovementOpcodes;

    // ACK
    WorldPacketHandlers[MSG_MOVE_TELEPORT_ACK].handler = &WorldSession::handleMoveTeleportAckOpcode;
    WorldPacketHandlers[CMSG_FORCE_WALK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_FEATHER_FALL_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_WATER_WALK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_TURN_RATE_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_ROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_UNROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_KNOCK_BACK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_HOVER_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_SET_CAN_FLY_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_DESCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;

    // Action Buttons
    WorldPacketHandlers[CMSG_SET_ACTION_BUTTON].handler = &WorldSession::handleSetActionButtonOpcode;
    WorldPacketHandlers[CMSG_REPOP_REQUEST].handler = &WorldSession::handleRepopRequestOpcode;

    // Loot
    WorldPacketHandlers[CMSG_AUTOSTORE_LOOT_ITEM].handler = &WorldSession::handleAutostoreLootItemOpcode;
    WorldPacketHandlers[CMSG_LOOT_MONEY].handler = &WorldSession::handleLootMoneyOpcode;
    WorldPacketHandlers[CMSG_LOOT].handler = &WorldSession::handleLootOpcode;
    WorldPacketHandlers[CMSG_LOOT_RELEASE].handler = &WorldSession::handleLootReleaseOpcode;
    WorldPacketHandlers[CMSG_LOOT_ROLL].handler = &WorldSession::handleLootRollOpcode;
    WorldPacketHandlers[CMSG_LOOT_MASTER_GIVE].handler = &WorldSession::handleLootMasterGiveOpcode;

    // Player Interaction
    WorldPacketHandlers[CMSG_WHO].handler = &WorldSession::handleWhoOpcode;
    WorldPacketHandlers[CMSG_WHOIS].handler = &WorldSession::handleWhoIsOpcode;
    WorldPacketHandlers[CMSG_LOGOUT_REQUEST].handler = &WorldSession::handleLogoutRequestOpcode;
    WorldPacketHandlers[CMSG_PLAYER_LOGOUT].handler = &WorldSession::handlePlayerLogoutOpcode;
    WorldPacketHandlers[CMSG_LOGOUT_CANCEL].handler = &WorldSession::handleLogoutCancelOpcode;
    // WorldPacketHandlers[CMSG_LOGOUT_CANCEL].status = STATUS_LOGGEDIN_RECENTLY_LOGGOUT;

    WorldPacketHandlers[CMSG_ZONEUPDATE].handler = &WorldSession::handleZoneupdate;
    // WorldPacketHandlers[CMSG_SET_TARGET_OBSOLETE].handler = &WorldSession::HandleSetTargetOpcode;
    WorldPacketHandlers[CMSG_SET_SELECTION].handler = &WorldSession::handleSetSelectionOpcode;
    WorldPacketHandlers[CMSG_STANDSTATECHANGE].handler = &WorldSession::handleStandStateChangeOpcode;
    WorldPacketHandlers[CMSG_CANCEL_MOUNT_AURA].handler = &WorldSession::handleDismountOpcode;

    // Friends
    WorldPacketHandlers[CMSG_CONTACT_LIST].handler = &WorldSession::handleFriendListOpcode;
    WorldPacketHandlers[CMSG_ADD_FRIEND].handler = &WorldSession::handleAddFriendOpcode;
    WorldPacketHandlers[CMSG_DEL_FRIEND].handler = &WorldSession::handleDelFriendOpcode;
    WorldPacketHandlers[CMSG_ADD_IGNORE].handler = &WorldSession::handleAddIgnoreOpcode;
    WorldPacketHandlers[CMSG_DEL_IGNORE].handler = &WorldSession::handleDelIgnoreOpcode;
    WorldPacketHandlers[CMSG_BUG].handler = &WorldSession::handleBugOpcode;
    WorldPacketHandlers[CMSG_SET_CONTACT_NOTES].handler = &WorldSession::handleSetFriendNote;

    // Areatrigger
    WorldPacketHandlers[CMSG_AREATRIGGER].handler = &WorldSession::handleAreaTriggerOpcode;

    // Account Data
    WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].handler = &WorldSession::handleUpdateAccountData;
    WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].status = STATUS_AUTHED;
    WorldPacketHandlers[CMSG_REQUEST_ACCOUNT_DATA].handler = &WorldSession::handleRequestAccountData;
    WorldPacketHandlers[CMSG_TOGGLE_PVP].handler = &WorldSession::handleTogglePVPOpcode;

    // Faction / Reputation
    WorldPacketHandlers[CMSG_SET_FACTION_ATWAR].handler = &WorldSession::handleSetFactionAtWarOpcode;
    WorldPacketHandlers[CMSG_SET_WATCHED_FACTION].handler = &WorldSession::handleSetWatchedFactionIndexOpcode;
    WorldPacketHandlers[CMSG_SET_FACTION_INACTIVE].handler = &WorldSession::handleSetFactionInactiveOpcode;

    // Player Interaction
    WorldPacketHandlers[CMSG_GAMEOBJ_USE].handler = &WorldSession::handleGameObjectUse;
    WorldPacketHandlers[CMSG_PLAYED_TIME].handler = &WorldSession::handlePlayedTimeOpcode;
    WorldPacketHandlers[CMSG_SETSHEATHED].handler = &WorldSession::handleSetSheathedOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_EMOTE].handler = &WorldSession::handleEmoteOpcode;
    WorldPacketHandlers[CMSG_TEXT_EMOTE].handler = &WorldSession::handleTextEmoteOpcode;
    WorldPacketHandlers[CMSG_INSPECT].handler = &WorldSession::handleInspectOpcode;
    // clearly wrong naming!
    //WorldPacketHandlers[SMSG_BARBER_SHOP_RESULT].handler = &WorldSession::handleBarberShopResult;

    // Channels
    WorldPacketHandlers[CMSG_JOIN_CHANNEL].handler = &WorldSession::handleChannelJoin;
    WorldPacketHandlers[CMSG_LEAVE_CHANNEL].handler = &WorldSession::handleChannelLeave;
    WorldPacketHandlers[CMSG_CHANNEL_LIST].handler = &WorldSession::handleChannelList;
    WorldPacketHandlers[CMSG_CHANNEL_PASSWORD].handler = &WorldSession::handleChannelPassword;
    WorldPacketHandlers[CMSG_CHANNEL_SET_OWNER].handler = &WorldSession::handleChannelSetOwner;
    WorldPacketHandlers[CMSG_CHANNEL_OWNER].handler = &WorldSession::handleChannelOwner;
    WorldPacketHandlers[CMSG_CHANNEL_MODERATOR].handler = &WorldSession::handleChannelModerator;
    WorldPacketHandlers[CMSG_CHANNEL_UNMODERATOR].handler = &WorldSession::handleChannelUnmoderator;
    WorldPacketHandlers[CMSG_CHANNEL_MUTE].handler = &WorldSession::handleChannelMute;
    WorldPacketHandlers[CMSG_CHANNEL_UNMUTE].handler = &WorldSession::handleChannelUnmute;
    WorldPacketHandlers[CMSG_CHANNEL_INVITE].handler = &WorldSession::handleChannelInvite;
    WorldPacketHandlers[CMSG_CHANNEL_KICK].handler = &WorldSession::handleChannelKick;
    WorldPacketHandlers[CMSG_CHANNEL_BAN].handler = &WorldSession::handleChannelBan;
    WorldPacketHandlers[CMSG_CHANNEL_UNBAN].handler = &WorldSession::handleChannelUnban;
    WorldPacketHandlers[CMSG_CHANNEL_ANNOUNCEMENTS].handler = &WorldSession::handleChannelAnnounce;
    WorldPacketHandlers[CMSG_CHANNEL_MODERATE].handler = &WorldSession::handleChannelModerate;
    WorldPacketHandlers[CMSG_GET_CHANNEL_MEMBER_COUNT].handler = &WorldSession::handleGetChannelMemberCount;
    WorldPacketHandlers[CMSG_CHANNEL_DISPLAY_LIST].handler = &WorldSession::handleChannelRosterQuery;

    // Groups / Raids
    WorldPacketHandlers[CMSG_GROUP_INVITE].handler = &WorldSession::handleGroupInviteOpcode;
    //WorldPacketHandlers[CMSG_GROUP_CANCEL].handler = &WorldSession::HandleGroupCancelOpcode;
    WorldPacketHandlers[CMSG_GROUP_ACCEPT].handler = &WorldSession::handleGroupAcceptOpcode;
    WorldPacketHandlers[CMSG_GROUP_DECLINE].handler = &WorldSession::handleGroupDeclineOpcode;
    WorldPacketHandlers[CMSG_GROUP_UNINVITE].handler = &WorldSession::handleGroupUninviteOpcode;
    WorldPacketHandlers[CMSG_GROUP_UNINVITE_GUID].handler = &WorldSession::handleGroupUninviteGuidOpcode;
    WorldPacketHandlers[CMSG_GROUP_SET_LEADER].handler = &WorldSession::handleGroupSetLeaderOpcode;
    WorldPacketHandlers[CMSG_GROUP_DISBAND].handler = &WorldSession::handleGroupDisbandOpcode;
    WorldPacketHandlers[CMSG_LOOT_METHOD].handler = &WorldSession::handleLootMethodOpcode;
    WorldPacketHandlers[MSG_MINIMAP_PING].handler = &WorldSession::handleMinimapPingOpcode;
    WorldPacketHandlers[CMSG_GROUP_RAID_CONVERT].handler = &WorldSession::handleConvertGroupToRaidOpcode;
    WorldPacketHandlers[CMSG_GROUP_CHANGE_SUB_GROUP].handler = &WorldSession::handleGroupChangeSubGroup;
    WorldPacketHandlers[CMSG_GROUP_ASSISTANT_LEADER].handler = &WorldSession::handleGroupAssistantLeader;
    WorldPacketHandlers[CMSG_REQUEST_RAID_INFO].handler = &WorldSession::handleRequestRaidInfoOpcode;
    WorldPacketHandlers[MSG_RAID_READY_CHECK].handler = &WorldSession::handleReadyCheckOpcode;
    WorldPacketHandlers[MSG_RAID_TARGET_UPDATE].handler = &WorldSession::handleSetPlayerIconOpcode;
    WorldPacketHandlers[CMSG_REQUEST_PARTY_MEMBER_STATS].handler = &WorldSession::handlePartyMemberStatsOpcode;
    WorldPacketHandlers[MSG_PARTY_ASSIGNMENT].handler = &WorldSession::handleGroupPromote;

    // LFG System
    WorldPacketHandlers[CMSG_SET_LFG_COMMENT].handler = &WorldSession::handleLfgSetCommentOpcode;
    WorldPacketHandlers[CMSG_LFG_JOIN].handler = &WorldSession::handleLfgJoinOpcode;
    WorldPacketHandlers[CMSG_LFG_LEAVE].handler = &WorldSession::handleLfgLeaveOpcode;
    WorldPacketHandlers[CMSG_SEARCH_LFG_JOIN].handler = &WorldSession::handleLfgSearchOpcode;
    WorldPacketHandlers[CMSG_SEARCH_LFG_LEAVE].handler = &WorldSession::handleLfgSearchLeaveOpcode;
    WorldPacketHandlers[CMSG_LFG_PROPOSAL_RESULT].handler = &WorldSession::handleLfgProposalResultOpcode;
    WorldPacketHandlers[CMSG_LFG_SET_ROLES].handler = &WorldSession::handleLfgSetRolesOpcode;
    WorldPacketHandlers[CMSG_LFG_SET_BOOT_VOTE].handler = &WorldSession::handleLfgSetBootVoteOpcode;
    WorldPacketHandlers[CMSG_LFD_PLAYER_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgPlayerLockInfoRequestOpcode;
    WorldPacketHandlers[CMSG_LFG_TELEPORT].handler = &WorldSession::handleLfgTeleportOpcode;
    WorldPacketHandlers[CMSG_LFD_PARTY_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgPartyLockInfoRequestOpcode;

    // Taxi / NPC Interaction
    WorldPacketHandlers[CMSG_ENABLETAXI].handler = &WorldSession::handleEnabletaxiOpcode;
    WorldPacketHandlers[CMSG_TAXINODE_STATUS_QUERY].handler = &WorldSession::handleTaxiNodeStatusQueryOpcode;
    WorldPacketHandlers[CMSG_TAXIQUERYAVAILABLENODES].handler = &WorldSession::handleTaxiQueryAvaibleNodesOpcode;
    WorldPacketHandlers[CMSG_ACTIVATETAXI].handler = &WorldSession::handleActivateTaxiOpcode;
    WorldPacketHandlers[MSG_TABARDVENDOR_ACTIVATE].handler = &WorldSession::handleTabardVendorActivateOpcode;
    WorldPacketHandlers[CMSG_BANKER_ACTIVATE].handler = &WorldSession::handleBankerActivateOpcode;
    WorldPacketHandlers[CMSG_BUY_BANK_SLOT].handler = &WorldSession::handleBuyBankSlotOpcode;
    WorldPacketHandlers[CMSG_TRAINER_LIST].handler = &WorldSession::handleTrainerListOpcode;
    WorldPacketHandlers[CMSG_TRAINER_BUY_SPELL].handler = &WorldSession::handleTrainerBuySpellOpcode;
    WorldPacketHandlers[CMSG_PETITION_SHOWLIST].handler = &WorldSession::handleCharterShowListOpcode;
    WorldPacketHandlers[MSG_AUCTION_HELLO].handler = &WorldSession::handleAuctionHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_HELLO].handler = &WorldSession::handleGossipHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_SELECT_OPTION].handler = &WorldSession::handleGossipSelectOptionOpcode;
    WorldPacketHandlers[CMSG_SPIRIT_HEALER_ACTIVATE].handler = &WorldSession::handleSpiritHealerActivateOpcode;
    WorldPacketHandlers[CMSG_NPC_TEXT_QUERY].handler = &WorldSession::handleNpcTextQueryOpcode;
    WorldPacketHandlers[CMSG_BINDER_ACTIVATE].handler = &WorldSession::handleBinderActivateOpcode;
    WorldPacketHandlers[CMSG_ACTIVATETAXIEXPRESS].handler = &WorldSession::handleMultipleActivateTaxiOpcode;
    // Item / Vendors
    WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler = &WorldSession::handleSwapInvItemOpcode;
    WorldPacketHandlers[CMSG_SWAP_ITEM].handler = &WorldSession::handleSwapItemOpcode;
    WorldPacketHandlers[CMSG_DESTROYITEM].handler = &WorldSession::handleDestroyItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler = &WorldSession::handleAutoEquipItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM_SLOT].handler = &WorldSession::handleAutoEquipItemSlotOpcode;
    WorldPacketHandlers[CMSG_ITEM_QUERY_SINGLE].handler = &WorldSession::handleItemQuerySingleOpcode;
    WorldPacketHandlers[CMSG_SELL_ITEM].handler = &WorldSession::handleSellItemOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM_IN_SLOT].handler = &WorldSession::handleBuyItemInSlotOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM].handler = &WorldSession::handleBuyItemOpcode;
    WorldPacketHandlers[CMSG_LIST_INVENTORY].handler = &WorldSession::handleListInventoryOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler = &WorldSession::handleAutoStoreBagItemOpcode;
    WorldPacketHandlers[CMSG_SET_AMMO].handler = &WorldSession::handleAmmoSetOpcode;
    WorldPacketHandlers[CMSG_BUYBACK_ITEM].handler = &WorldSession::handleBuyBackOpcode;
    WorldPacketHandlers[CMSG_SPLIT_ITEM].handler = &WorldSession::handleSplitOpcode;
    WorldPacketHandlers[CMSG_READ_ITEM].handler = &WorldSession::handleReadItemOpcode;
    WorldPacketHandlers[CMSG_REPAIR_ITEM].handler = &WorldSession::handleRepairItemOpcode;
    WorldPacketHandlers[CMSG_AUTOBANK_ITEM].handler = &WorldSession::handleAutoBankItemOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BANK_ITEM].handler = &WorldSession::handleAutoStoreBankItemOpcode;
    WorldPacketHandlers[CMSG_CANCEL_TEMP_ENCHANTMENT].handler = &WorldSession::handleCancelTemporaryEnchantmentOpcode;
    WorldPacketHandlers[CMSG_SOCKET_GEMS].handler = &WorldSession::handleInsertGemOpcode;
    WorldPacketHandlers[CMSG_WRAP_ITEM].handler = &WorldSession::handleWrapItemOpcode;
    WorldPacketHandlers[CMSG_ITEMREFUNDINFO].handler = &WorldSession::handleItemRefundInfoOpcode;
    WorldPacketHandlers[CMSG_ITEMREFUNDREQUEST].handler = &WorldSession::handleItemRefundRequestOpcode;

    WorldPacketHandlers[CMSG_EQUIPMENT_SET_SAVE].handler = &WorldSession::handleEquipmentSetSave;
    WorldPacketHandlers[CMSG_EQUIPMENT_SET_USE].handler = &WorldSession::handleEquipmentSetUse;
    WorldPacketHandlers[CMSG_EQUIPMENT_SET_DELETE].handler = &WorldSession::handleEquipmentSetDelete;

    // Spell System / Talent System
    WorldPacketHandlers[CMSG_USE_ITEM].handler = &WorldSession::handleUseItemOpcode;
    WorldPacketHandlers[CMSG_CAST_SPELL].handler = &WorldSession::handleCastSpellOpcode;
    WorldPacketHandlers[CMSG_SPELLCLICK].handler = &WorldSession::handleSpellClick;
    WorldPacketHandlers[CMSG_CANCEL_CAST].handler = &WorldSession::handleCancelCastOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AURA].handler = &WorldSession::handleCancelAuraOpcode;
    WorldPacketHandlers[CMSG_CANCEL_CHANNELLING].handler = &WorldSession::handleCancelChannellingOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AUTO_REPEAT_SPELL].handler = &WorldSession::handleCancelAutoRepeatSpellOpcode;
    WorldPacketHandlers[CMSG_TOTEM_DESTROYED].handler = &WorldSession::handleCancelTotem;
    WorldPacketHandlers[CMSG_LEARN_TALENT].handler = &WorldSession::handleLearnTalentOpcode;
    WorldPacketHandlers[CMSG_LEARN_TALENTS_MULTIPLE].handler = &WorldSession::handleLearnMultipleTalentsOpcode;
    WorldPacketHandlers[CMSG_UNLEARN_TALENTS].handler = &WorldSession::handleUnlearnTalents;
    WorldPacketHandlers[MSG_TALENT_WIPE_CONFIRM].handler = &WorldSession::handleUnlearnTalents;
    WorldPacketHandlers[CMSG_UPDATE_PROJECTILE_POSITION].handler = &WorldSession::handleUpdateProjectilePosition;
    // Combat / Duel
    WorldPacketHandlers[CMSG_ATTACKSWING].handler = &WorldSession::handleAttackSwingOpcode;
    WorldPacketHandlers[CMSG_ATTACKSTOP].handler = &WorldSession::handleAttackStopOpcode;
    WorldPacketHandlers[CMSG_DUEL_ACCEPTED].handler = &WorldSession::handleDuelAccepted;
    WorldPacketHandlers[CMSG_DUEL_CANCELLED].handler = &WorldSession::handleDuelCancelled;

    // Trade
    WorldPacketHandlers[CMSG_INITIATE_TRADE].handler = &WorldSession::handleInitiateTradeOpcode;
    WorldPacketHandlers[CMSG_BEGIN_TRADE].handler = &WorldSession::handleBeginTradeOpcode;
    WorldPacketHandlers[CMSG_BUSY_TRADE].handler = &WorldSession::handleBusyTrade;
    WorldPacketHandlers[CMSG_IGNORE_TRADE].handler = &WorldSession::handleIgnoreTrade;
    WorldPacketHandlers[CMSG_ACCEPT_TRADE].handler = &WorldSession::handleAcceptTrade;
    WorldPacketHandlers[CMSG_UNACCEPT_TRADE].handler = &WorldSession::handleUnacceptTrade;
    WorldPacketHandlers[CMSG_CANCEL_TRADE].handler = &WorldSession::handleCancelTrade;
    WorldPacketHandlers[CMSG_SET_TRADE_ITEM].handler = &WorldSession::handleSetTradeItem;
    WorldPacketHandlers[CMSG_CLEAR_TRADE_ITEM].handler = &WorldSession::handleClearTradeItem;
    WorldPacketHandlers[CMSG_SET_TRADE_GOLD].handler = &WorldSession::handleSetTradeGold;

    // Quest System
    WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
    WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_QUERY].handler = &WorldSession::handleQuestgiverStatusQueryOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_HELLO].handler = &WorldSession::handleQuestgiverHelloOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_ACCEPT_QUEST].handler = &WorldSession::handleQuestgiverAcceptQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_CANCEL].handler = &WorldSession::handleQuestgiverCancelOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_CHOOSE_REWARD].handler = &WorldSession::handleQuestgiverChooseRewardOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_REQUEST_REWARD].handler = &WorldSession::handleQuestgiverRequestRewardOpcode;
    WorldPacketHandlers[CMSG_QUEST_QUERY].handler = &WorldSession::handleQuestQueryOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_QUERY_QUEST].handler = &WorldSession::handleQuestGiverQueryQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_COMPLETE_QUEST].handler = &WorldSession::handleQuestgiverCompleteQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTLOG_REMOVE_QUEST].handler = &WorldSession::handleQuestlogRemoveQuestOpcode;
    WorldPacketHandlers[CMSG_RECLAIM_CORPSE].handler = &WorldSession::handleCorpseReclaimOpcode;
    WorldPacketHandlers[CMSG_RESURRECT_RESPONSE].handler = &WorldSession::handleResurrectResponse;
    WorldPacketHandlers[CMSG_PUSHQUESTTOPARTY].handler = &WorldSession::handlePushQuestToPartyOpcode;
    WorldPacketHandlers[MSG_QUEST_PUSH_RESULT].handler = &WorldSession::handleQuestPushResultOpcode;
    WorldPacketHandlers[CMSG_QUEST_POI_QUERY].handler = &WorldSession::handleQuestPOIQueryOpcode;

    // Auction System
    WorldPacketHandlers[CMSG_AUCTION_LIST_ITEMS].handler = &WorldSession::handleAuctionListItems;
    WorldPacketHandlers[CMSG_AUCTION_LIST_BIDDER_ITEMS].handler = &WorldSession::handleAuctionListBidderItems;
    WorldPacketHandlers[CMSG_AUCTION_SELL_ITEM].handler = &WorldSession::handleAuctionSellItem;
    WorldPacketHandlers[CMSG_AUCTION_LIST_OWNER_ITEMS].handler = &WorldSession::handleAuctionListOwnerItems;
    WorldPacketHandlers[CMSG_AUCTION_PLACE_BID].handler = &WorldSession::handleAuctionPlaceBid;
    WorldPacketHandlers[CMSG_AUCTION_REMOVE_ITEM].handler = &WorldSession::handleCancelAuction;
    WorldPacketHandlers[CMSG_AUCTION_LIST_PENDING_SALES].handler = &WorldSession::handleAuctionListPendingSales;

    // Mail System
    WorldPacketHandlers[CMSG_GET_MAIL_LIST].handler = &WorldSession::handleGetMailOpcode;
    WorldPacketHandlers[CMSG_ITEM_TEXT_QUERY].handler = &WorldSession::handleItemTextQueryOpcode;
    WorldPacketHandlers[CMSG_SEND_MAIL].handler = &WorldSession::handleSendMailOpcode;
    WorldPacketHandlers[CMSG_MAIL_TAKE_MONEY].handler = &WorldSession::handleTakeMoneyOpcode;
    WorldPacketHandlers[CMSG_MAIL_TAKE_ITEM].handler = &WorldSession::handleTakeItemOpcode;
    WorldPacketHandlers[CMSG_MAIL_MARK_AS_READ].handler = &WorldSession::handleMarkAsReadOpcode;
    WorldPacketHandlers[CMSG_MAIL_RETURN_TO_SENDER].handler = &WorldSession::handleReturnToSenderOpcode;
    WorldPacketHandlers[CMSG_MAIL_DELETE].handler = &WorldSession::handleMailDeleteOpcode;
    WorldPacketHandlers[MSG_QUERY_NEXT_MAIL_TIME].handler = &WorldSession::handleMailTimeOpcode;
    WorldPacketHandlers[CMSG_MAIL_CREATE_TEXT_ITEM].handler = &WorldSession::handleMailCreateTextItemOpcode;

    // Guild Query (called when not logged in sometimes)
    WorldPacketHandlers[CMSG_GUILD_QUERY].handler = &WorldSession::handleGuildQuery;
    WorldPacketHandlers[CMSG_GUILD_QUERY].status = STATUS_AUTHED;

    // Guild System
    //WorldPacketHandlers[CMSG_GUILD_CREATE].handler = &WorldSession::HandleCreateGuild;
    WorldPacketHandlers[CMSG_GUILD_INVITE].handler = &WorldSession::handleInviteToGuild;
    WorldPacketHandlers[CMSG_GUILD_ACCEPT].handler = &WorldSession::handleGuildAccept;
    WorldPacketHandlers[CMSG_GUILD_DECLINE].handler = &WorldSession::handleGuildDecline;
    WorldPacketHandlers[CMSG_GUILD_INFO].handler = &WorldSession::handleGuildInfo;
    WorldPacketHandlers[CMSG_GUILD_ROSTER].handler = &WorldSession::handleGuildRoster;
    WorldPacketHandlers[CMSG_GUILD_PROMOTE].handler = &WorldSession::handleGuildPromote;
    WorldPacketHandlers[CMSG_GUILD_DEMOTE].handler = &WorldSession::handleGuildDemote;
    WorldPacketHandlers[CMSG_GUILD_LEAVE].handler = &WorldSession::handleGuildLeave;
    WorldPacketHandlers[CMSG_GUILD_REMOVE].handler = &WorldSession::handleGuildRemove;
    WorldPacketHandlers[CMSG_GUILD_DISBAND].handler = &WorldSession::handleGuildDisband;
    WorldPacketHandlers[CMSG_GUILD_LEADER].handler = &WorldSession::handleGuildLeader;
    WorldPacketHandlers[CMSG_GUILD_MOTD].handler = &WorldSession::handleGuildMotd;
    WorldPacketHandlers[CMSG_GUILD_SET_RANK].handler = &WorldSession::handleGuildSetRank;
    WorldPacketHandlers[CMSG_GUILD_ADD_RANK].handler = &WorldSession::handleGuildAddRank;
    WorldPacketHandlers[CMSG_GUILD_DEL_RANK].handler = &WorldSession::handleGuildDelRank;
    WorldPacketHandlers[CMSG_GUILD_SET_PUBLIC_NOTE].handler = &WorldSession::handleGuildSetPublicNote;
    WorldPacketHandlers[CMSG_GUILD_SET_OFFICER_NOTE].handler = &WorldSession::handleGuildSetOfficerNote;
    WorldPacketHandlers[CMSG_PETITION_BUY].handler = &WorldSession::handleCharterBuy;
    WorldPacketHandlers[CMSG_PETITION_SHOW_SIGNATURES].handler = &WorldSession::handleCharterShowSignatures;
    WorldPacketHandlers[CMSG_TURN_IN_PETITION].handler = &WorldSession::handleCharterTurnInCharter;
    WorldPacketHandlers[CMSG_PETITION_QUERY].handler = &WorldSession::handleCharterQuery;
    WorldPacketHandlers[CMSG_OFFER_PETITION].handler = &WorldSession::handleCharterOffer;
    WorldPacketHandlers[CMSG_PETITION_SIGN].handler = &WorldSession::handleCharterSign;
    WorldPacketHandlers[MSG_PETITION_DECLINE].handler = &WorldSession::handleCharterDecline;
    WorldPacketHandlers[MSG_PETITION_RENAME].handler = &WorldSession::handleCharterRename;
    WorldPacketHandlers[MSG_SAVE_GUILD_EMBLEM].handler = &WorldSession::handleSaveGuildEmblem;
    WorldPacketHandlers[CMSG_GUILD_INFO_TEXT].handler = &WorldSession::handleSetGuildInfo;
    WorldPacketHandlers[MSG_QUERY_GUILD_BANK_TEXT].handler = &WorldSession::handleGuildBankQueryText;
    WorldPacketHandlers[CMSG_SET_GUILD_BANK_TEXT].handler = &WorldSession::handleSetGuildBankText;
    WorldPacketHandlers[MSG_GUILD_EVENT_LOG_QUERY].handler = &WorldSession::handleGuildLog;
    WorldPacketHandlers[CMSG_GUILD_BANKER_ACTIVATE].handler = &WorldSession::handleGuildBankerActivate;
    WorldPacketHandlers[CMSG_GUILD_BANK_BUY_TAB].handler = &WorldSession::handleGuildBankBuyTab;
    WorldPacketHandlers[MSG_GUILD_BANK_MONEY_WITHDRAWN].handler = &WorldSession::handleGuildBankMoneyWithdrawn;
    WorldPacketHandlers[CMSG_GUILD_BANK_UPDATE_TAB].handler = &WorldSession::handleGuildBankUpdateTab;
    WorldPacketHandlers[CMSG_GUILD_BANK_SWAP_ITEMS].handler = &WorldSession::handleGuildBankSwapItems;
    WorldPacketHandlers[CMSG_GUILD_BANK_WITHDRAW_MONEY].handler = &WorldSession::handleGuildBankWithdrawMoney;
    WorldPacketHandlers[CMSG_GUILD_BANK_DEPOSIT_MONEY].handler = &WorldSession::handleGuildBankDepositMoney;
    WorldPacketHandlers[CMSG_GUILD_BANK_QUERY_TAB].handler = &WorldSession::handleGuildBankQueryTab;
    WorldPacketHandlers[MSG_GUILD_BANK_LOG_QUERY].handler = &WorldSession::handleGuildBankLogQuery;
    WorldPacketHandlers[MSG_GUILD_PERMISSIONS].handler = &WorldSession::handleGuildPermissions;

    // Tutorials
    WorldPacketHandlers[CMSG_TUTORIAL_FLAG].handler = &WorldSession::handleTutorialFlag;
    WorldPacketHandlers[CMSG_TUTORIAL_CLEAR].handler = &WorldSession::handleTutorialClear;
    WorldPacketHandlers[CMSG_TUTORIAL_RESET].handler = &WorldSession::handleTutorialReset;

    // Pets
    WorldPacketHandlers[MSG_LIST_STABLED_PETS].handler = &WorldSession::handleStabledPetList;

    WorldPacketHandlers[CMSG_PET_ACTION].handler = &WorldSession::handlePetAction;
    WorldPacketHandlers[CMSG_PET_NAME_QUERY].handler = &WorldSession::handlePetNameQuery;
    WorldPacketHandlers[CMSG_BUY_STABLE_SLOT].handler = &WorldSession::handleBuyStableSlot;
    WorldPacketHandlers[CMSG_STABLE_PET].handler = &WorldSession::handleStablePet;
    WorldPacketHandlers[CMSG_UNSTABLE_PET].handler = &WorldSession::handleUnstablePet;
    WorldPacketHandlers[CMSG_STABLE_SWAP_PET].handler = &WorldSession::handleStableSwapPet;
    WorldPacketHandlers[CMSG_PET_SET_ACTION].handler = &WorldSession::handlePetSetActionOpcode;
    WorldPacketHandlers[CMSG_PET_RENAME].handler = &WorldSession::handlePetRename;
    WorldPacketHandlers[CMSG_PET_ABANDON].handler = &WorldSession::handlePetAbandon;
    WorldPacketHandlers[CMSG_PET_UNLEARN].handler = &WorldSession::handlePetUnlearn;
    WorldPacketHandlers[CMSG_PET_SPELL_AUTOCAST].handler = &WorldSession::handlePetSpellAutocast;
    WorldPacketHandlers[CMSG_PET_CANCEL_AURA].handler = &WorldSession::handlePetCancelAura;
    WorldPacketHandlers[CMSG_PET_LEARN_TALENT].handler = &WorldSession::handlePetLearnTalent;
    WorldPacketHandlers[CMSG_DISMISS_CRITTER].handler = &WorldSession::handleDismissCritter;

    // Battlegrounds
    WorldPacketHandlers[CMSG_BATTLEFIELD_PORT].handler = &WorldSession::handleBattlefieldPortOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_STATUS].handler = &WorldSession::handleBattlefieldStatusOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_LIST].handler = &WorldSession::handleBattlefieldListOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_HELLO].handler = &WorldSession::handleBattleMasterHelloOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN_ARENA].handler = &WorldSession::handleArenaJoinOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN].handler = &WorldSession::handleBattleMasterJoinOpcode;
    WorldPacketHandlers[CMSG_LEAVE_BATTLEFIELD].handler = &WorldSession::handleLeaveBattlefieldOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUERY].handler = &WorldSession::handleAreaSpiritHealerQueryOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUEUE].handler = &WorldSession::handleAreaSpiritHealerQueueOpcode;
    WorldPacketHandlers[MSG_BATTLEGROUND_PLAYER_POSITIONS].handler = &WorldSession::handleBattlegroundPlayerPositionsOpcode;
    WorldPacketHandlers[MSG_PVP_LOG_DATA].handler = &WorldSession::handlePVPLogDataOpcode;
    WorldPacketHandlers[MSG_INSPECT_HONOR_STATS].handler = &WorldSession::handleInspectHonorStatsOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIONBAR_TOGGLES].handler = &WorldSession::handleSetActionBarTogglesOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE].handler = &WorldSession::HandleBgInviteResponse;

    // GM Ticket System
    WorldPacketHandlers[CMSG_GMTICKET_CREATE].handler = &WorldSession::handleGMTicketCreateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_UPDATETEXT].handler = &WorldSession::handleGMTicketUpdateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_DELETETICKET].handler = &WorldSession::handleGMTicketDeleteOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_GETTICKET].handler = &WorldSession::handleGMTicketGetTicketOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_SYSTEMSTATUS].handler = &WorldSession::handleGMTicketSystemStatusOpcode;
    WorldPacketHandlers[CMSG_GMTICKETSYSTEM_TOGGLE].handler = &WorldSession::handleGMTicketToggleSystemStatusOpcode;

    // Lag report
    WorldPacketHandlers[CMSG_GM_REPORT_LAG].handler = &WorldSession::handleReportLag;
    WorldPacketHandlers[CMSG_GM_REPORT_LAG].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].handler = &WorldSession::handleGMSurveySubmitOpcode;
    WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].status = STATUS_LOGGEDIN;

    // Meeting Stone / Instances
    WorldPacketHandlers[CMSG_SUMMON_RESPONSE].handler = &WorldSession::handleSummonResponseOpcode;
    WorldPacketHandlers[CMSG_RESET_INSTANCES].handler = &WorldSession::handleResetInstanceOpcode;
    WorldPacketHandlers[CMSG_SELF_RES].handler = &WorldSession::handleSelfResurrect;
    WorldPacketHandlers[MSG_RANDOM_ROLL].handler = &WorldSession::handleRandomRollOpcode;
    WorldPacketHandlers[MSG_SET_DUNGEON_DIFFICULTY].handler = &WorldSession::handleDungeonDifficultyOpcode;
    WorldPacketHandlers[MSG_SET_RAID_DIFFICULTY].handler = &WorldSession::handleRaidDifficultyOpcode;
    
    // Misc
    WorldPacketHandlers[CMSG_OPEN_ITEM].handler = &WorldSession::handleOpenItemOpcode;
    WorldPacketHandlers[CMSG_COMPLETE_CINEMATIC].handler = &WorldSession::handleCompleteCinematic;
    WorldPacketHandlers[CMSG_NEXT_CINEMATIC_CAMERA].handler = &WorldSession::handleNextCinematic;
    WorldPacketHandlers[CMSG_MOUNTSPECIAL_ANIM].handler = &WorldSession::handleMountSpecialAnimOpcode;
    WorldPacketHandlers[CMSG_TOGGLE_CLOAK].handler = &WorldSession::handleToggleCloakOpcode;
    WorldPacketHandlers[CMSG_TOGGLE_HELM].handler = &WorldSession::handleToggleHelmOpcode;
    WorldPacketHandlers[CMSG_SET_TITLE].handler = &WorldSession::handleSetTitle;
    WorldPacketHandlers[CMSG_COMPLAIN].handler = &WorldSession::handleReportSpamOpcode;
    WorldPacketHandlers[CMSG_GAMEOBJ_REPORT_USE].handler = &WorldSession::handleGameobjReportUseOpCode;
    WorldPacketHandlers[CMSG_PET_CAST_SPELL].handler = &WorldSession::handlePetCastSpell;
    WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler = &WorldSession::handleWorldStateUITimerUpdate;
    WorldPacketHandlers[CMSG_SET_TAXI_BENCHMARK_MODE].handler = &WorldSession::handleSetTaxiBenchmarkOpcode;
    WorldPacketHandlers[CMSG_UNLEARN_SKILL].handler = &WorldSession::handleUnlearnSkillOpcode;

    // Chat
    WorldPacketHandlers[CMSG_CHAT_IGNORED].handler = &WorldSession::handleChatIgnoredOpcode;
    WorldPacketHandlers[CMSG_SET_CHANNEL_WATCH].handler = &WorldSession::handleChatChannelWatchOpcode;

    // Arenas
    WorldPacketHandlers[CMSG_ARENA_TEAM_QUERY].handler = &WorldSession::handleArenaTeamQueryOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_ROSTER].handler = &WorldSession::handleArenaTeamRosterOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_INVITE].handler = &WorldSession::handleArenaTeamAddMemberOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_ACCEPT].handler = &WorldSession::handleArenaTeamInviteAcceptOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_DECLINE].handler = &WorldSession::handleArenaTeamInviteDenyOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_LEAVE].handler = &WorldSession::handleArenaTeamLeaveOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_REMOVE].handler = &WorldSession::handleArenaTeamRemoveMemberOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_DISBAND].handler = &WorldSession::handleArenaTeamDisbandOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_LEADER].handler = &WorldSession::handleArenaTeamPromoteOpcode;
    WorldPacketHandlers[MSG_INSPECT_ARENA_TEAMS].handler = &WorldSession::handleInspectArenaStatsOpcode;

    // cheat/gm commands?
    WorldPacketHandlers[CMSG_WORLD_TELEPORT].handler = &WorldSession::handleWorldTeleportOpcode;

    // Vehicle
    WorldPacketHandlers[CMSG_DISMISS_CONTROLLED_VEHICLE].handler = &WorldSession::handleDismissVehicle;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_EXIT].handler = &WorldSession::handleLeaveVehicle;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_PREV_SEAT].handler = &WorldSession::handleRequestVehiclePreviousSeat;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_NEXT_SEAT].handler = &WorldSession::handleRequestVehicleNextSeat;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_SWITCH_SEAT].handler = &WorldSession::handleRequestVehicleSwitchSeat;
    WorldPacketHandlers[CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE].handler = &WorldSession::handleChangeSeatsOnControlledVehicle;
    WorldPacketHandlers[CMSG_PLAYER_VEHICLE_ENTER].handler = &WorldSession::handleEnterVehicle;
    WorldPacketHandlers[CMSG_EJECT_PASSENGER].handler = &WorldSession::handleRemoveVehiclePassenger;

    // Unsorted
    //WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler = &WorldSession::HandleTimeSyncResp;
    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::handleReadyForAccountDataTimes;
    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler = &WorldSession::handleSetAutoLootPassOpcode;
    WorldPacketHandlers[CMSG_REMOVE_GLYPH].handler = &WorldSession::handleRemoveGlyph;
    WorldPacketHandlers[CMSG_ALTER_APPEARANCE].handler = &WorldSession::handleBarberShopResult;
    WorldPacketHandlers[CMSG_GET_MIRRORIMAGE_DATA].handler = &WorldSession::HandleMirrorImageOpcode;

    // Calendar - Unhandled
    WorldPacketHandlers[CMSG_CALENDAR_GET_CALENDAR].handler = &WorldSession::handleCalendarGetCalendar;
    WorldPacketHandlers[CMSG_CALENDAR_COMPLAIN].handler = &WorldSession::handleCalendarComplain;
    WorldPacketHandlers[CMSG_CALENDAR_GET_NUM_PENDING].handler = &WorldSession::handleCalendarGetNumPending;
    WorldPacketHandlers[CMSG_CALENDAR_ADD_EVENT].handler = &WorldSession::handleCalendarAddEvent;

    WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].handler = &WorldSession::handleCalendarGetEvent;
    WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].handler = &WorldSession::handleCalendarGuildFilter;
    WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].handler = &WorldSession::handleCalendarArenaTeam;
    WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].handler = &WorldSession::handleCalendarUpdateEvent;
    WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].handler = &WorldSession::handleCalendarRemoveEvent;
    WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].handler = &WorldSession::handleCalendarCopyEvent;
    WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].handler = &WorldSession::handleCalendarEventInvite;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].handler = &WorldSession::handleCalendarEventRsvp;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].handler = &WorldSession::handleCalendarEventRemoveInvite;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].handler = &WorldSession::handleCalendarEventStatus;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].handler = &WorldSession::handleCalendarEventModeratorStatus;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].status = STATUS_LOGGEDIN;

    //Misc - Unhandled
    WorldPacketHandlers[CMSG_FAR_SIGHT].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_LFG_GET_STATUS].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].handler = &WorldSession::Unhandled;
}
